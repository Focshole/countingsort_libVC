#include "kernel.hpp"
#include "TimeMonitor.hpp"
#include "utils.hpp"
#include "Workload.hpp"

#include "versioningCompiler/Utils.hpp"
#include "versioningCompilerProd/networking.hpp"
#include "versioningCompilerProd/producer-utils.hpp"
#include "versioningCompilerCons/networking.hpp"
#include "versioningCompilerCons/consumer-utils.hpp"
#include <sched.h>
#include <map>
#include <thread>
#include <string>
#include <mutex>

#ifndef _PATH_TO_KERNEL
#define PATH_TO_KERNEL "../"
#else
#define PATH_TO_KERNEL _PATH_TO_KERNEL
#endif

typedef void (*kernel_t)(std::vector<int32_t> &array);
std::map<std::pair<int32_t, int32_t>, vc::version_ptr_t> memo;
std::map<std::pair<int32_t, int32_t>, vc::version_ptr_t> memoDHT;

uint32_t seed = 666;
const size_t MAX_ITERATIONS = 100;
const float similarity_ratio_recompilation_threshold = 2.0f;

void run_test(size_t data_size, size_t iterations, std::pair<int, int> range);

int main(int argc, char const *argv[])
{

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);

  const std::vector<std::pair<int, int>> data_range = {
      std::make_pair<int, int>(0, 256),
      std::make_pair<int, int>(0, 512),
      std::make_pair<int, int>(0, 1024),
      std::make_pair<int, int>(0, 2048),
      std::make_pair<int, int>(0, 4096),
      std::make_pair<int, int>(0, 8192),
      std::make_pair<int, int>(0, 16384),
  };

  const std::vector<size_t> data_size = {
      10 * 1000 * 1000,
      100 * 1000 * 1000,
      // 1000*1000*1000, // I don't have a hpc to test it
  };
  // initialize common libvc utilities
  vc::vc_utils_init();
  for (const size_t s : data_size)
  {
    for (const auto r : data_range)
    {
      run_test(s, MAX_ITERATIONS, r);
    }
  }

  return 0;
}

// dynamically create a new version
vc::version_ptr_t dynamicCompile(int32_t min, int32_t max)
{
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "kernel.cpp";
  const std::string functionName = "vc_sort";
  const vc::opt_list_t opt_list = {
      vc::make_option("-O3"),
      vc::make_option("-std=c++17"),
      vc::make_option("-I" + kernel_dir),
      vc::make_option("-D_MIN_VALUE_RANGE=" + std::to_string(min)),
      vc::make_option("-D_MAX_VALUE_RANGE=" + std::to_string(max)),
      vc::make_option("-fPIC")};
  vc::version_ptr_t version = vc::createVersion(kernel_file, functionName, opt_list);
  // compiling...
  kernel_t f = (kernel_t)vc::compileAndGetSymbol(version);
  if (f)
  {
    return version;
  }
  return nullptr;
}

// lookup in previously compiled versions
vc::version_ptr_t getDynamicVersion(const int32_t min, const int32_t max)
{
  std::pair<int32_t, int32_t> key = std::make_pair(min, max);
  auto elem = memo.find(key);
  if (elem != memo.end())
  {
    elem->second->compile();
    return elem->second;
  }
  auto v = dynamicCompile(min, max);
  if (v != nullptr)
  {
    memo[key] = v;
  }
  return v;
}

// this is the producer code that will generate the version and serve it to the consumer
std::mutex producer_ready_mutex;
void producerThread(const int32_t min, const int32_t max, std::filesystem::path &generatedBinPath, std::shared_ptr<dht::DhtRunner> &prodNode)
{
  producer_ready_mutex.unlock();
  std::cout << "TODO Bootstrapped producer" << std::endl;
  const std::string kernel_dir = PATH_TO_KERNEL;
  const std::string kernel_file = kernel_dir + "kernel.cpp";
  const std::string functionName = "vc_sort";
  std::string dhtKey = functionName + "_" + std::to_string(min) + "_" + std::to_string(max);
  vc::opt_list_t opt_list = {
      vc::make_option("-O3"),
      vc::make_option("-std=c++17"),
      vc::make_option("-I" + kernel_dir),
      vc::make_option("-D_MIN_VALUE_RANGE=" + std::to_string(min)),
      vc::make_option("-D_MAX_VALUE_RANGE=" + std::to_string(max)),
      vc::make_option("-fPIC")};
  generatedBinPath = dht_prod::generateVersion(kernel_file, functionName, opt_list);
  prodNode->put(dhtKey, "tcp://127.0.0.1:4554");
  std::cout << "TODO Producer placed put on network" << std::endl;
  if (dht_prod::serveVersion(kernel_file, "tcp://*:4554") != 0)
    generatedBinPath = "";

  prodNode->join();
}

// Those are required to synchronize the consumer callback with the consumer thread
std::mutex callback_mutex;
std::mutex fetchedVersion_mutex;
vc::version_ptr_t fetchedVersion = nullptr;

// this is the code that be called once the dht entry had been generated
bool dhtCallbackConsumer(const std::vector<std::shared_ptr<dht::Value>> &values)
{
  std::cout << "TODO DHT callback called" << std::endl;
  if (values.empty() || values.size() > 1)
  {
    std::cerr << "Bad value placed on DHT, exiting"
              << std::endl;
    return false;
  }
  auto socket = std::string(values[0]->data.begin() + 1, values[0]->data.end());
  fetchedVersion_mutex.lock();
  fetchedVersion = dht_cons::downloadVersion(socket, {"vc_sort"}, "dht_tmp.so");
  fetchedVersion_mutex.unlock();
  callback_mutex.unlock();
  return true;
}
// this is the consumer code that will fetch the version from the producer
void consumerThread(const int32_t min, const int32_t max, std::shared_ptr<dht::DhtRunner> &consNode)
{
  std::cout << "TODO Bootstrapped consumer" << std::endl;
  // generate the version
  const std::string functionName = "vc_sort";
  std::string dhtKey = functionName + "_" + std::to_string(min) + "_" + std::to_string(max);
  callback_mutex.lock();
  consNode->get(dhtKey, dhtCallbackConsumer);
  std::cout << "TODO placed get" << std::endl;
  while (!callback_mutex.try_lock())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  callback_mutex.lock();
  callback_mutex.unlock();

  consNode->join();
}
vc::version_ptr_t dynamicDHTCompile(int32_t min, int32_t max, std::shared_ptr<dht::DhtRunner> &prodNode, std::shared_ptr<dht::DhtRunner> &consNode)
{
  std::filesystem::path generatedBinPath;
  producer_ready_mutex.lock();
  std::thread t1(producerThread, min, max, std::ref(generatedBinPath), std::ref(prodNode));
  std::thread t2(consumerThread, min, max, std::ref(consNode));
  t1.join();
  t2.join();
  fetchedVersion_mutex.lock();
  auto tmp_version = fetchedVersion;
  fetchedVersion_mutex.unlock();
  if (tmp_version == nullptr)
  {
    return nullptr;
  }
  kernel_t f = (kernel_t)vc::compileAndGetSymbol(tmp_version);
  if (f)
  {
    return tmp_version;
  }
  return nullptr;
}
// lookup in previously compiled versions
vc::version_ptr_t getDHTVersion(const int32_t min, const int32_t max, std::shared_ptr<dht::DhtRunner> &prodNode, std::shared_ptr<dht::DhtRunner> &consNode)
{
  std::pair<int32_t, int32_t> key = std::make_pair(min, max);
  auto elem = memoDHT.find(key);
  if (elem != memoDHT.end())
  {
    elem->second->compile();
    return elem->second;
  }
  std::cout << "dynamicDHTCompile begin" << std::endl;
  auto v = dynamicDHTCompile(min, max, prodNode, consNode);
  std::cout << "dynamicDHTCompile end" << std::endl;
  if (v != nullptr)
  {
    memoDHT[key] = v;
  }
  return v;
}

void run_test(size_t data_size, size_t iterations, std::pair<int, int> range)
{

  TimeMonitor time_monitor_ref;
  TimeMonitor time_monitor_dyn;
  TimeMonitor time_monitor_dyn_ovh;
  TimeMonitor time_monitor_dht;
  TimeMonitor time_monitor_dht_ovh;

  // initialize libvc DHT clients
  auto consNode = dht_cons::bootstrapDHTNode({dht::crypto::generateIdentity("consNode"), std::string(""), 4224});
  sleep(3);
  auto prodNode = dht_prod::bootstrapDHTNode({dht::crypto::generateIdentity("prodNode"), std::string("tcp://127.0.0.1:4224"), 4242});
  // produce DHT version
  time_monitor_dht_ovh.start();
  auto v_dht = getDHTVersion(range.first, range.second, prodNode, consNode);
  kernel_t dynamic_dht_sort = (kernel_t)v_dht->getSymbol(0);
  time_monitor_dht_ovh.stop();

  // running dynamic version - dynamically compiled
  for (size_t i = 0; i < iterations; i++)
  {
    auto wl = WorkloadProducer<int32_t>::get_WL_with_bounds_size(range.first,
                                                                 range.second,
                                                                 data_size,
                                                                 seed + i);
    time_monitor_dht.start();
    dynamic_dht_sort(wl.data);
    time_monitor_dht.stop();
  }

  // running reference version - statically linked to main program
  for (size_t i = 0; i < iterations; i++)
  {
    auto wl = WorkloadProducer<int32_t>::get_WL_with_bounds_size(range.first,
                                                                 range.second,
                                                                 data_size,
                                                                 seed + i);
    const auto meta = wl.getMetadata();
    time_monitor_ref.start();
    sort(wl.data, meta.minVal, meta.maxVal);
    time_monitor_ref.stop();
  }
  // Produce dynamic version
  time_monitor_dyn_ovh.start();
  auto v = getDynamicVersion(range.first, range.second);
  kernel_t dynamic_sort = (kernel_t)v->getSymbol(0);
  time_monitor_dyn_ovh.stop();

  if (!dynamic_sort)
  {
    std::cerr << "Error while processing item "
              << range.first << " - " << range.second << std::endl;
    return;
  }

  // running dynamic version - dynamically compiled
  for (size_t i = 0; i < iterations; i++)
  {
    auto wl = WorkloadProducer<int32_t>::get_WL_with_bounds_size(range.first,
                                                                 range.second,
                                                                 data_size,
                                                                 seed + i);
    time_monitor_dyn.start();
    dynamic_sort(wl.data);
    time_monitor_dyn.stop();
  }

  v->fold();

  std::cout << "range width"
            << "\t"
            << "workload size"
            << "\t"
            << "timing" << std::endl;
  std::cout << range.second << "\t" << data_size << "\t Avg ref     " << time_monitor_ref.getAvg() << " ms" << std::endl;
  std::cout << range.second << "\t" << data_size << "\t Avg dyn     " << time_monitor_dyn.getAvg() << " ms" << std::endl;
  std::cout << range.second << "\t" << data_size << "\t Avg ovh     " << time_monitor_dyn_ovh.getAvg() << " ms" << std::endl;
  std::cout << range.second << "\t" << data_size << "\t Avg dht-ovh " << time_monitor_dht_ovh.getAvg() << " ms" << std::endl;
  std::cout << range.second << "\t" << data_size << "\t Avg dht     " << time_monitor_dht.getAvg() << " ms" << std::endl
            << std::endl
            << std::endl;
  return;
}
