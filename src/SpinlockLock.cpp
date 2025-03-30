#include "../include/SpinlockLock.h"
#include <thread>

void SpinlockLock::Lock(){
  while(flag_.test_and_set(std::memory_order_acquire)){
    std::this_thread::yield();
  }
}

void SpinlockLock::Unlock(){
  flag_.clear(std::memory_order_release);
}
