#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <algorithm>
#include <functional>
#include <iomanip>

#include "../include/spinlocklock.h"

enum class PhilosopherStatus{
  THINKING,
  HUNGRY,
  EATING
};

struct PhilosopherState{
  int id;
  PhilosopherStatus status = PhilosopherStatus::THINKING;
  bool holding_left_fork = false;
  bool holding_right_fork = false;
  std::chrono::steady_clock::time_point became_hungry_at;
  PhilosopherState() : id(-1) {};
  PhilosopherState(int p_id) : id(p_id) {};
};

bool CheckIfArgumentIsNumber(const std::string& str){
  for(char c : str){
    if(!std::isdigit(c)){
      return false;
    }
  }
  return !str.empty();
}

std::string StatusToString(PhilosopherStatus status){
  switch(status){
    case PhilosopherStatus::THINKING: return "Thinking";
    case PhilosopherStatus::HUNGRY:   return "Hungry  ";
    case PhilosopherStatus::EATING:   return "Eating  ";
    default:                          return "Unknown ";
  }
}

void PhilosophersLife(int id, int num_philosophers, std::vector<SpinlockLock>& forks,
                      std::atomic<bool>& running_flag, std::vector<PhilosopherState>& states,
                      SpinlockLock& state_lock){
  int left_fork_id = id;
  int right_fork_id = (id + 1) % num_philosophers;
  int first_fork_id = std::min(left_fork_id, right_fork_id);
  int second_fork_id = std::max(left_fork_id, right_fork_id);

  std::random_device rd;
  std::mt19937 gen(rd() + id); 
  std::uniform_int_distribution<int> dist(100, 800);

  while(running_flag.load(std::memory_order_relaxed)) {
    // THINKING
    {
      state_lock.Lock();
      states[id].status = PhilosopherStatus::THINKING;
      state_lock.Unlock();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

    // HUNGRY
    {
      state_lock.Lock();
      states[id].status = PhilosopherStatus::HUNGRY;
      states[id].became_hungry_at = std::chrono::steady_clock::now();
      state_lock.Unlock();
    }

    // Grab first fork
    forks[first_fork_id].Lock();
    {
      state_lock.Lock();
      if (first_fork_id == left_fork_id){ states[id].holding_left_fork = true; }
      else{ states[id].holding_right_fork = true; }
      state_lock.Unlock();
    }

    // Grab second fork -> EATING
    forks[second_fork_id].Lock();
    {
      state_lock.Lock();
      if (second_fork_id == left_fork_id){ states[id].holding_left_fork = true; }
      else { states[id].holding_right_fork = true; }
      states[id].status = PhilosopherStatus::EATING;
      state_lock.Unlock();
    }

    // EATING
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));

    forks[second_fork_id].Unlock();
    forks[first_fork_id].Unlock();
    {
      state_lock.Lock();
      states[id].holding_left_fork = false;
      states[id].holding_right_fork = false;
      state_lock.Unlock();
    }
  }
}

int main(int argc, char **argv){
  if (argc < 2 || argc > 3) {
    std::cerr << "Error: Program requires one or two arguments\n"
      << "Correct usage: " << argv[0] << " <num_philosophers> [simulation_seconds]\n"
      << "  num_philosophers: (Required) Number of philosophers (>= 2)\n"
      << "  simulation_seconds: (Optional) Duration of the simulation in seconds (default: 30)\n";
    return 1;
  }

  if (!CheckIfArgumentIsNumber(argv[1])) {
    std::cerr << "Error: First argument (num_philosophers) is not a valid number.\n";
    return 1;
  }
  int num_philosophers = 0;
  try {
    num_philosophers = std::stoi(argv[1]);
  } catch (const std::exception& e) {
    std::cerr << "Error parsing num_philosophers: " << e.what() << std::endl;
    return 1;
  }
  if (num_philosophers < 2) {
    std::cerr << "Error: Number of philosophers must be 2 or more.\n";
    return 1;
  }
  int simulation_duration_seconds = 30;
  if (argc == 3) {
    if (!CheckIfArgumentIsNumber(argv[2])) {
      std::cerr << "Error: Second argument (simulation_seconds) is not a valid number.\n";
      return 1;
    }
    try {
      simulation_duration_seconds = std::stoi(argv[2]);
    } catch (const std::exception& e) {
      std::cerr << "Error parsing simulation_seconds: " << e.what() << std::endl;
      return 1;
    }
    if (simulation_duration_seconds <= 0) {
      std::cerr << "Error: Simulation duration must be a positive number of seconds.\n";
      return 1;
    }
  }
  auto simulation_duration = std::chrono::seconds(simulation_duration_seconds);

  std::cout << "Initializing " << num_philosophers << " philosophers...\n";
  std::cout << "Simulation duration: " << simulation_duration_seconds << " seconds.\n"; 

  std::vector<SpinlockLock> forks(num_philosophers);
  std::vector<std::thread> philosophers;
  philosophers.reserve(num_philosophers);

  std::vector<PhilosopherState> philosopher_states(num_philosophers);
  for(int i = 0; i < num_philosophers; i++){
    philosopher_states[i].id = i;
  }

  std::atomic<bool> running_flag = true;
  SpinlockLock state_lock;

  std::cout << "Launching threads...\n";
  for (int i = 0; i < num_philosophers; ++i) {
    philosophers.emplace_back(
      PhilosophersLife,
      i,
      num_philosophers,
      std::ref(forks),
      std::ref(running_flag),
      std::ref(philosopher_states),
      std::ref(state_lock)
    );
  }

  std::cout << "Starting UI...\n";
  auto simulation_start_time = std::chrono::steady_clock::now();

  while (std::chrono::steady_clock::now() - simulation_start_time < simulation_duration) { 
    std::cout << "\033[2J\033[H";

    std::cout << "--- Dining Philosophers Status ("
      << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - simulation_start_time).count()
      << "s / "
      << simulation_duration.count() 
      << "s) ---\n";
    std::cout << std::setw(4) << "ID" << " | "
      << std::setw(10) << "Status" << " | "
      << std::setw(7) << "Forks" << " | "
      << "Waiting (ms)\n";
    std::cout << "--------------------------------------------------\n";

    state_lock.Lock();
    auto now = std::chrono::steady_clock::now();
    for (const auto& state : philosopher_states) {
      std::string forks_held = "";
      forks_held += (state.holding_left_fork ? std::to_string(state.id) : "-");
      forks_held += " ";
      forks_held += (state.holding_right_fork ? std::to_string((state.id + 1) % num_philosophers) : "-");

      long long waiting_ms = 0;
      if (state.status == PhilosopherStatus::HUNGRY) {
        waiting_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.became_hungry_at).count();
      }

      std::cout << std::setw(4) << state.id << " | "; 
      std::cout << std::setw(10) << StatusToString(state.status) << " | "; 
      std::cout << std::setw(7) << forks_held << " | ";

      if (state.status == PhilosopherStatus::HUNGRY) {
        std::cout << waiting_ms;
      } else {
        std::cout << "-";
      }
      std::cout << "\n"; 
    }
    state_lock.Unlock();


    std::this_thread::sleep_for(std::chrono::milliseconds(150)); 
  } 

  std::cout << "\n--- Simulation time ended ---\n";
  std::cout << "Stopping philosopher threads...\n";
  running_flag.store(false); 

  std::cout << "Waiting for threads to join...\n";
  for(std::thread &p_thread : philosophers) { 
    if (p_thread.joinable()) {
      p_thread.join();
    }
  }

  std::cout << "All philosophers have finished. Exiting.\n";
  return 0; 
}
