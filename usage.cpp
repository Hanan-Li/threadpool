#include <iostream>
#include "threadpool.h"

void sample(string input){
    std::cout << "Hello Concurrent " << input << endl;
}

int main(){
    threadpool threadpool_(10);
    threadpool_.enqueue({sample("one");});
    threadpool_.enqueue({sample("two");});
}