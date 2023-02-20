#ifndef WORKLOAD_IMPLEMENTATION_TEMPLATE_HEADER
#define WORKLOAD_IMPLEMENTATION_TEMPLATE_HEADER

#include <cstdlib>

const float float_range_hard_bounds_up = 256.0f;
const float float_range_hard_bounds_down = -255.0f;
const int32_t int_range_hard_bounds_up = 4096;
const int32_t int_range_hard_bounds_down = -2048;
const size_t max_size = 10000;

// ---------------------------------- float ----------------------------------

template <>
Workload<float> WorkloadProducer<float>::get_WL_with_bounds_size(float min, float max, size_t size, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  Workload<float> wl;
  wl.meta.minVal = min;
  wl.meta.maxVal = max;
  if (wl.meta.minVal > wl.meta.maxVal)
  {
    float tmp = wl.meta.minVal;
    wl.meta.minVal = wl.meta.maxVal;
    wl.meta.maxVal = wl.meta.minVal;
    if (wl.meta.maxVal - wl.meta.minVal < 0.001f)
    {
      wl.meta.maxVal += 0.001f;
    }
  }
  wl.meta.size = size;
  wl.data.reserve(size);
  const float data_range = wl.meta.maxVal - wl.meta.minVal;
  for (size_t i = 0; i < size; i++)
  {
    wl.data.push_back(static_cast<float>(rand()) / static_cast<float>((((float)RAND_MAX) - 1) / data_range) + wl.meta.minVal);
  }
  srand(time(NULL));
  return wl;
}

template <>
Workload<float> WorkloadProducer<float>::get_WL_with_bounds(float min, float max, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  const size_t size = rand() / ((((float)RAND_MAX) - 1) / max_size) + 1;
  return WorkloadProducer<float>::get_WL_with_bounds_size(min, max, size, seed);
}

template <>
Workload<float> WorkloadProducer<float>::get_WL_with_size(size_t size, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  const float bound_range = float_range_hard_bounds_up - float_range_hard_bounds_down;
  float min = static_cast<float>(rand()) / static_cast<float>((((float)RAND_MAX) - 1) / bound_range) + float_range_hard_bounds_down;
  float max = static_cast<float>(rand()) / static_cast<float>((((float)RAND_MAX) - 1) / bound_range) + float_range_hard_bounds_down;
  return WorkloadProducer<float>::get_WL_with_bounds_size(min, max, size, seed);
}

// --------------------------------- int32_t ----------------------------------

template <>
Workload<int32_t> WorkloadProducer<int32_t>::get_WL_with_bounds_size(int32_t min, int32_t max, size_t size, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  Workload<int32_t> wl;
  wl.meta.minVal = min;
  wl.meta.maxVal = max;
  if (wl.meta.minVal > wl.meta.maxVal)
  {
    int32_t tmp = wl.meta.minVal;
    wl.meta.minVal = wl.meta.maxVal;
    wl.meta.maxVal = wl.meta.minVal;
    if (wl.meta.maxVal - wl.meta.minVal < 1)
    {
      wl.meta.maxVal += 1;
    }
  }
  wl.meta.size = size;
  wl.data.reserve(size);
  const int32_t data_range = wl.meta.maxVal - wl.meta.minVal;
  for (size_t i = 0; i < size; i++)
  {
    wl.data.push_back((rand() % data_range) + wl.meta.minVal);
  }
  srand(time(NULL));
  return wl;
}

template <>
Workload<int32_t> WorkloadProducer<int32_t>::get_WL_with_bounds(int32_t min, int32_t max, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  const size_t size = (rand() % max_size) + 1;
  return WorkloadProducer<int32_t>::get_WL_with_bounds_size(min, max, size, seed);
}

template <>
Workload<int32_t> WorkloadProducer<int32_t>::get_WL_with_size(size_t size, size_t seed)
{
  if (seed)
  {
    srand(seed);
  }
  const int32_t bound_range = int_range_hard_bounds_up - int_range_hard_bounds_down;
  int32_t min = rand() % bound_range + int_range_hard_bounds_down;
  int32_t max = rand() % bound_range + int_range_hard_bounds_down;
  return WorkloadProducer<int32_t>::get_WL_with_bounds_size(min, max, size, seed);
}

#endif /* end of include guard: WORKLOAD_IMPLEMENTATION_TEMPLATE_HEADER */
