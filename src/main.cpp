#include <iostream>
#include <string>
#include <cctype>
#include "../include/SpinlockLock.hpp"

bool checkIfArgumentIsNumber(const std::string& str){
  for(char c : str){
    if(!std::isdigit(c)){
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv){

  if(argc != 2){
    std::cerr <<    "Error: Program requires one argument\n" 
      "Correct usage: ./MyProgram <number of philosophers>\n"; 
    return 1;
  }

  if(!checkIfArgumentIsNumber(argv[1])){
    std::cerr <<    "Error: Argument is not a valid number\n"
      "Integer type number required\n";
    return 1;
  }

  int countOfPhilosophers = std::stoi(argv[1]);

  if(countOfPhilosophers < 2){
    std::cerr <<    "Error: The required number must be 2 or more\n";
    return 1;
  }

  return 0;
}
