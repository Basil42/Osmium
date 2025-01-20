//
// Created by nicolas.gerard on 2025-01-20.
//

#ifndef RESOURCEARRAY_H
#define RESOURCEARRAY_H
#include <array>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <vector>

template<typename T, size_t MAX_Capacity>
class  ResourceArray {
  static_assert(MAX_Capacity > 0, "MAX_Capacity must be greater than 0");
  static_assert(MAX_Capacity < 0xffffffffffffffff);
  std::array<unsigned int,MAX_Capacity> backingArray ;//int value indicate index in the vector,
  std::vector<T> resourceVector;
  unsigned int nextHandle = 0;//handle to be used for the next assignement, loops around to look for unused handles when reaching the maximum handle
  public:
  ResourceArray();
  void Reserve(size_t newCapacity);
  unsigned int Add(T resource);
  bool Remove(unsigned int handle);
  T get(unsigned int handle);
};

template<typename T, size_t MAX_Capacity>
ResourceArray<T, MAX_Capacity>::ResourceArray() {
  backingArray.fill(MAX_Capacity +1);
  //I could reserve the vector to max capacity here
}

template<typename T, size_t MAX_Capacity>
void ResourceArray<T, MAX_Capacity>::Reserve(size_t newCapacity) {
  if (newCapacity > MAX_Capacity) {
    throw std::out_of_range("Resource array resource capacity exceeded");
  }
  backingArray.reserve(newCapacity);
}

template<typename T, size_t MAX_Capacity>
unsigned int ResourceArray<T, MAX_Capacity>::Add(T resource) {
  if (resourceVector.size() == MAX_Capacity) [[unlikely]] {
    throw std::out_of_range("Resource array resource capacity exceeded");
  }
  resourceVector.push_back(resource);
  unsigned int checks = 0;//sanity check
  while (backingArray[nextHandle % MAX_Capacity] > MAX_Capacity && ++checks < MAX_Capacity) {
    nextHandle++;
  }
  if (checks >= MAX_Capacity) [[unlikely]] {
    throw std::runtime_error("No available handle.");//This is an implementation error, as it should not happen
  }
  unsigned int newHandle = nextHandle;
  nextHandle = (nextHandle +1) % MAX_Capacity;
  backingArray[newHandle] = resourceVector.size() -1;
  return newHandle;
}

template<typename T, size_t MAX_Capacity>
bool ResourceArray<T, MAX_Capacity>::Remove(unsigned int handle) {
  unsigned int vectorIndex = backingArray[handle];
  if (vectorIndex >= MAX_Capacity +1) [[unlikely]]{
    std::cout << "Attempted to remove a non-valid resource handle" << std::endl;
    return false;
  }
  resourceVector.erase(resourceVector.begin() + vectorIndex);
  backingArray[handle] = MAX_Capacity + 1;
  //realigning backing array
  for (unsigned int &index : backingArray) {
    if (index > vectorIndex && index < MAX_Capacity +1) --index;
  }
  return true;
}

template<typename T, size_t MAX_Capacity>
T ResourceArray<T, MAX_Capacity>::get(unsigned int handle) {
  if (backingArray[handle] >= MAX_Capacity) [[unlikely]] {
    std::cout << "Attempted to get a non valid resource,it might have been unloaded or the handle might be invalid" << std::endl;
    return T();
  }
  return resourceVector[backingArray[handle]];
}
#endif //RESOURCEARRAY_H
