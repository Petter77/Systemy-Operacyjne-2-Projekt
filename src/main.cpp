#include <iostream>
#include <string>
#include <cctype>
#include "../include/SpinlockLock.h"
#include <vector>
#include <thread>
#include <chrono>

bool CheckIfArgumentIsNumber(const std::string& str){
  for(char c : str){
    if(!std::isdigit(c)){
      return false;
    }
  }
  return true;
}

void PhilofopherLife(int id, int num_philosphers, std::vector<SpinlockLock>& chopsticks, std::atomic<bool>& running_flat){
  while(running_flat.load()){
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1000ms);
  }  
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

  const int num_philosphers = std::stoi(argv[1]);

  if(num_philosphers < 2){
    std::cerr <<  "Error: The required number must be 2 or more\n";
    return 1;
  }
  
  std::cout << "Starting simulation with " << num_philosphers << " philosophers\n";
  std::cout << "Creating " << num_philosphers << " forks\n"; 
  std::vector<SpinlockLock> forks(num_philosphers);

  std::vector<std::thread> philosopers(num_philosphers);
  return 0;
}
