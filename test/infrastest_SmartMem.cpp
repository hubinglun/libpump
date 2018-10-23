//
// Created by yz on 18-10-20.
//

#include <vector>
#include <list>
#include <deque>
#include<stdio.h>
#include<stdlib.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "MemMgr/test_mem/block.hpp"
#include "MemMgr/test_mem/policy.hpp"
#include "MemMgr/test_mem/voidsptr.hpp"
#include "MemMgr/test_mem/voidwptr.hpp"

#include "Logger.h"

using namespace std;
using namespace Pump;
using namespace Pump::SmartMem;

bool test_Block() {
  LOG(INFO) << ">>>>>>>> test_Block <<<<<<<<<";
  Heap<> heap0;
  // test 1
  LOG_ASSERT(heap0.empty()) << "[ failed ] 1-1";
  heap0.allocate(0);
  LOG_ASSERT((heap0.block_segsz_==0 && heap0.empty())) << "[ failed ] 1-2";
  heap0.allocate(10);
  LOG_ASSERT((heap0.block_segsz_==10 && !heap0.empty())) << "[ failed ] 1-3";
  heap0.deallocate();
  LOG_ASSERT((heap0.block_segsz_==0 && heap0.empty())) << "[ failed ] 1-4";
  
  // test 2
  std::allocator<char> a;
  HeapGuider<> bmgr0;
  HeapGuider<> bmgr1(sizeof(int),a);
  HeapGuider<> bmgr2(sizeof(int));
  LOG_ASSERT((bmgr0.empty())) << "[ failed ] 2-1";
  bmgr1.ref<int>() = 10;
  bmgr2.ref<int>() = 100;
  LOG_ASSERT((!bmgr1.empty()&&bmgr1.ref<int>() == 10)) << "[ failed ] 2-2";
  bmgr0 = bmgr1;
  LOG_ASSERT((bmgr0.relative(bmgr1)==RELATIVE_EQUAL)) << "[ failed ] 2-3";
  HeapGuider<> bmgr1_2(bmgr1);
  LOG_ASSERT((bmgr1_2.relative(bmgr1)==RELATIVE_EQUAL)) << "[ failed ] 2-4";
  bmgr1.swap(bmgr2);
  LOG_ASSERT((bmgr1.ref<int>() == 100&&bmgr2.ref<int>() == 10)) << "[ failed ] 2-5";
  
  return true;
}

class test_del{
public:
  void del() {
    delete(this);
  }
  ~test_del() {
    printf("sss\n");
  }
};

class TDel : public _Del {
public:
  virtual void pre_del(const Block *p) {
    LOG(INFO) << "call [yangzheng] pre_del";
  }
  void post_del(void * data) {
    LOG(INFO) << "call [yangzheng] post_del";
  }
};

bool test_VoidSPtr_1() {
  VoidSPtr sp1_1;
  VoidSPtr sp1_2;
  // test 1
  LOG_ASSERT((sp1_1==nullptr)
             && (sp1_1.capacity()==0)
             && (sp1_2==nullptr)
             && (sp1_2!=sp1_1)) << "[ failed ] 1-1";
  VoidSPtr sp2(100000);
  LOG_ASSERT((sp2.capacity()==100000)) << "[ failed ] 1-2";
  
  // test 2
  TDel d0;
  VoidSPtr sp3(100000,XXX, d0);
  VoidSPtr sp4(100000,XXX, d0);
  VoidSPtr sp5(100000,XXX, d0);
  VoidSPtr sp6(100000,XXX, d0);
  LOG_ASSERT((sp3.capacity()==100000)) << "[ failed ] 1-2";
  sp3.reset();
  return true;
}

bool test_voidwptr(){
  VoidSPtr sp_0(10);
  VoidSPtr sp_1;
  VoidWPtr wp_0;
  
  LOG_ASSERT((wp_0.expired())) << "[ failed ] 1-1";
  wp_0 = sp_0;
  LOG_ASSERT(!(wp_0.expired())) << "[ failed ] 1-2";
  sp_1 = wp_0.lock_raw();
  LOG_ASSERT(sp_1==sp_0) << "[ failed ] 1-3";
  VoidWPtr wp_1(sp_1);
  LOG_ASSERT(wp_1==wp_0) << "[ failed ] 1-4";
  return true;
}

/////////////////////////////////////
//    <>VoidSPtr 线程安全冒烟测试<>
/////////////////////////////////////
namespace nm_test_VoidSPtr {

static list<VoidSPtr> g_container[5];
static boost::mutex g_mtx[5];

const int thread_num = 5;

class TDel : public _Del {
public:
  virtual void pre_del(const Block *p) {
    LOG(INFO) << "call [yangzheng] pre_del";
  }
  virtual void post_del(size_t data) {
    LOG(INFO) << "[Del] " << data;
  }
  TDel(size_t a) {
    data_ = a;
  }
};

void create(size_t i){
  size_t indx = 0;
  g_mtx[i].lock();
    if((indx = g_container[i].size()) > 100) {
      g_mtx[i].unlock();
      sleep(2);
      return;
    }
    g_container[i].push_back(VoidSPtr((rand()%500),XXX, TDel(indx)));
    g_mtx[i].unlock();
}

void productor() {
  while(1) {
    create(0);
    create(1);
    create(2);
    create(3);
    create(4);
  }
}

void destroy(size_t i){
    g_mtx[i].lock();
    if(g_container[i].size()<thread_num) {
      g_mtx[i].unlock();
      sleep(2);
      return;
    }
    g_container[i].pop_front();
    g_mtx[i].unlock();
}

void comsumer() {
  while(1) {
    destroy(0);
    destroy(1);
    destroy(2);
    destroy(3);
    destroy(4);
  }
}

void smoke_test1() {
  list<VoidSPtr> container;
  while(1) {
    for(int i = 0;i<thread_num;i++) {
      container.push_back(VoidSPtr((rand()%1000),XXX, TDel(container.size())));
    }
    for(int i = 0;i<thread_num;i++) {
      container.pop_front();
    }
  }
  getchar();
}

void smoke_test2() {
  boost::thread tclose;
  for(int i = 0;i<thread_num;i++) {
     tclose = boost::thread(productor);
  }
  for(int i = 0;i<thread_num*2;i++) {
    boost::thread thrd2(comsumer);
  }
  tclose.join();
}

}
/////////////////////////////////////
//    </>VoidSPtr 线程安全冒烟测试</>
/////////////////////////////////////

int main() {
  LOG_IF(INFO, test_Block()) << "[ OK ] test_Block()";
  LOG_IF(INFO, test_voidwptr()) << "[ OK ] test_voidwptr()";
//  while(1) {
//    LOG_IF(INFO, test_VoidSPtr_1()) << "[ OK ] test_VoidSPtr()";
//  }
//  nm_test_VoidSPtr::smoke_test2();
//  test_del * a = new test_del;
  return 0;
}

