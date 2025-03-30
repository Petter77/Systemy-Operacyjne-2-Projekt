#include <atomic>

class SpinlockLock{
private:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT; 

public:
  SpinlockLock() = default;
  ~SpinlockLock() = default;
  SpinlockLock(const SpinlockLock&) = delete;
  SpinlockLock& operator=(const SpinlockLock&) = delete;

  void Lock();
  void Unlock();
};
