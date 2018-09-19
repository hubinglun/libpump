//
// Created by yz on 18-9-17.
//

#include "MemMgr/SmartPtr.h"
#include "MemMgr/VoidPtr.hpp"
#include "Logger.h"

using namespace Pump::MemMgr;

class test_A {
public:
  test_A(int a){
    LOG(INFO) << "test_A()";
    pInt = NewMem<int>(a);
  }
  ~test_A(){
    LOG(INFO) << "~test_A()";
  }
private:
  nsp_boost::shared_ptr<int> pInt;
};

class test_B {
public:
  test_B(){
    LOG(INFO) << "test_B()";
    pA = NewMem<test_A>(1024);
  }
  ~test_B(){
    LOG(INFO) << "~test_B()";
  }
private:
  nsp_boost::shared_ptr<test_A> pA;
};

class test_AA : public test_A {
public:
  test_AA(int a, float b) : test_A(a){
    LOG(INFO) << "test_AA()";
    pFloat = NewMem<float>(b);
  }
  ~test_AA(){
    LOG(INFO) << "~test_AA()";
  }
private:
  nsp_boost::shared_ptr<float> pFloat;
};

template <class _T = std::allocator<char> >
class template_A {
public:
  template_A(){
    strType = typeid(_T).name();
    LOG(INFO) << strType;
  }
  std::string strType;
};

void test_SmartPrt_0() {
  SimpleGC gc(1);
  gc.start();
  
  nsp_boost::shared_ptr<test_A> pA = NewMem<test_A>(255);
  nsp_boost::shared_ptr<test_A> pA1 = pA;
  nsp_boost::shared_ptr<test_A> pA2 = pA;
  nsp_boost::shared_ptr<test_B> pB = NewMem<test_B>();
  nsp_boost::shared_ptr<test_B> pB1 = pB;
  nsp_boost::shared_ptr<test_B> pB2 = pB;
  nsp_boost::shared_ptr<test_AA> pAA = NewMem<test_AA>(512, 0.125);
  nsp_boost::shared_ptr<test_AA> pAA1 = pAA;
  nsp_boost::shared_ptr<test_AA> pAA2 = pAA;
  
  sleep(4);
  
  pA.reset();
  pA1.reset();
//  pA2.reset();
  
  sleep(4);
  
  pB.reset();
  pB1.reset();
  pB2.reset();
  
  sleep(4);
  
  pAA.reset();
  pAA1.reset();
  pAA2.reset();
  
  getchar();
}

void test_SmartPrt_1() {
  std::allocator<char> loc_alloc;
  // test 0
  VoidPtr<> p_0;
  VoidPtr<> p_1(loc_alloc, sizeof(int));
  p_0 = p_1;
  LOG_IF(INFO,(p_0.get<int>()==p_1.get<int>()))<<"[OK] test_SmartPrt_1 test 0";
  
  // test 1
  int * pInt = new int(1024);
  VoidPtr<> p_2(loc_alloc, (void*)pInt, sizeof(int));
  p_2.release();
  LOG_IF(INFO,(p_2 == nullptr))<<"[OK] test_SmartPrt_1 test 1.1";
  LOG_IF(INFO,(nullptr == p_2))<<"[OK] test_SmartPrt_1 test 1.2";
}

void test_template() {
  template_A<int> a;
  template_A<float> b;
  template_A<> c;
  template_A<std::allocator<char> > d;
  LOG_IF(INFO, (c.strType==d.strType)) << "[OK] test_template";
  template_A<std::allocator<int> > e;
}

int main(){
//  test_SmartPrt_0();
  test_SmartPrt_1();
  test_template();
  
  return 0;
}

