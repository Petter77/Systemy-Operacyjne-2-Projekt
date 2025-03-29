#include <iostream>

int main(int argc, char **argv){
    if(argc != 2){
        std::cerr <<    "Error: Program requires one argument\n" 
                        "Correct usage: ./MyProgram <number of philosophers>\n"; 
        return 1;
    }
    return 0;
}
