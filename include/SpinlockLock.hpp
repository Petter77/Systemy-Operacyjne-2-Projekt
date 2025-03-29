#include <atomic>
#include <thread>

class SpinlockLock{
private:
  std::atomic_flag flag_ = ATOMIC_FLAG_INIT; 

public:
  SpinlockLock() = default;
  ~SpinlockLock() = default;
  void lock();
  void unlock();
};
