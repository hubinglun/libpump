//
// Created by yz on 18-9-15.
//
#include <iostream>
#include "Thread.h"
#include "Logger.h"

#include <boost/make_shared.hpp>

using namespace std;
using namespace Pump;
using namespace boost;

class TestThread
  : public PThread {
public:
  void threadCb(PtrArg pIn, PtrArg pOut) {
    while (1) {
      LOG(INFO)<<1;
      sleep(2);
    }
  }
};

int main(){
  TestThread athread;
  athread.m_pRealThread = boost::make_shared<boost::thread>(boost::bind(&TestThread::threadCb, &athread, PtrArg(), PtrArg()));
  athread.m_pRealThread->join();
  return 0;
}

