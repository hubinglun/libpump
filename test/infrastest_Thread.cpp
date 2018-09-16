//
// Created by yz on 18-9-15.
//
#include <iostream>
#include "Thread.h"
#include "Logger.h"

using namespace std;
using namespace PUMP;
using namespace boost;

class TestThread
  : public PThread {
public:
  virtual void threadCb(PtrArg pIn, PtrArg pOut) {
    while (1) {
      LOG(INFO)<<1;
    }
  }
};

int main(){
  TestThread athread;
  athread.start(PtrArg(), PtrArg());
  
  return 0;
}

