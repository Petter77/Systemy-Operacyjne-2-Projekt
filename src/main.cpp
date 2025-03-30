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

#include "../include/spinlocklock.h"

bool CheckIfArgumentIsNumber(const std::string& str){
  for(char c : str){
    if(!std::isdigit(c)){
      return false;
    }
  }
  return !str.empty();
}

void PhilosophersLife(int id, int num_philosophers, std::vector<SpinlockLock>& forks, std::atomic<bool>& running_flag /* Poprawiona nazwa */){
  int left_fork_id = id;
  int right_fork_id = (id + 1) % num_philosophers;

  int first_fork_id = std::min(left_fork_id, right_fork_id);
  int second_fork_id = std::max(left_fork_id, right_fork_id);

  std::random_device rd;

  std::mt19937 gen(rd() + id);
  std::uniform_int_distribution<int> dist(100, 800);

  std::cout << "Philosopher " << id << " started. Needs forks: " << left_fork_id << " & " << right_fork_id << ". Order: " << first_fork_id << " -> " << second_fork_id << std::endl;

  while(running_flag.load(std::memory_order_relaxed)){


    int think_time = dist(gen);
    std::cout << "Philosopher " << id << " thinking for " << think_time << "ms..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(think_time));

    forks[first_fork_id].Lock();
    std::cout << "Philosopher " << id << " got fork " << first_fork_id << ", trying fork " << second_fork_id << std::endl;
    forks[second_fork_id].Lock();
    std::cout << "Philosopher " << id << " got both forks." << std::endl;

    int eat_time = dist(gen);
    std::cout << "Philosopher " << id << " EATING for " << eat_time << "ms." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(eat_time));

    std::cout << "Philosopher " << id << " finished eating, releasing forks." << std::endl;
    forks[second_fork_id].Unlock();
    forks[first_fork_id].Unlock();
  }
  std::cout << "Philosopher " << id << " stopping." << std::endl;
}

int main(int argc, char **argv){
  if(argc != 2){
    std::cerr <<  "Error: Program requires one argument\n"
                  "Correct usage: ./MyProgram <number of philosophers>\n";
    return 1;
  }

  if(!CheckIfArgumentIsNumber(argv[1])){
    std::cerr <<  "Error: Argument is not a valid number\n"
                  "Integer type number required\n";
    return 1;
  }

  const int num_philosophers = std::stoi(argv[1]);

  if(num_philosophers < 2){
    std::cerr <<  "Error: The required number must be 2 or more\n";
    return 1;
  }

  std::cout << "Starting simulation with " << num_philosophers << " philosophers\n";
  std::cout << "Creating " << num_philosophers << " forks\n";
  std::vector<SpinlockLock> forks(num_philosophers);
  std::vector<std::thread> philosophers;
  philosophers.reserve(num_philosophers);


  std::atomic<bool> running_flag = true;

  std::cout << "Launching philosopher threads...\n";
  for (int i = 0; i < num_philosophers; ++i) {
    philosophers.emplace_back(
      PhilosophersLife,
      i,
      num_philosophers,
      std::ref(forks),
      std::ref(running_flag)
    );
  }

  std::cout << "All threads launched. Simulation will run for approx. 15 seconds...\n";


  std::this_thread::sleep_for(std::chrono::seconds(15));


  std::cout << "Stopping simulation...\n";
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
