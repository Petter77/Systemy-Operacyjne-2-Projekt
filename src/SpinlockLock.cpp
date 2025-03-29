#include "../include/SpinlockLock.hpp"

void SpinlockLock::lock(){
  while(flag_.test_and_set(std::memory_order_acquire)){
    std::this_thread::yield();
  }
}

void SpinlockLock::unlock(){
  flag_.clear(std::memory_order_release);
}
