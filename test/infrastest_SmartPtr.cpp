//
// Created by yz on 18-9-17.
//

#include "MemMgr/SmartPtr.h"
#include "MemMgr/VoidSPtr.hpp"
#include "MemMgr/VoidWPtr.hpp"
#include "MemMgr/SharedPtr.hpp"
#include "MemMgr/WeakPtr.hpp"
#include "Logger.h"

using namespace Pump::MemMgr;

class test_A {
public:
  test_A(int a){
    LOG(INFO) << "test_A()";
    pInt = new int(3);
  }
  ~test_A(){
    LOG(INFO) << "~test_A()";
  }
  bool varify(int a) {
    LOG(INFO) << "varify() ";
    return (a==*pInt);
  }
  void set(int a) {
    *pInt = a;
  }
private:
  int * pInt;
};

class test_B {
public:
  test_B() : pInt(nullptr){
    LOG(INFO) << "test_B()";
  }
  test_B(int b) : pInt(nullptr){
    LOG(INFO) << "test_B()";
    pInt.construct(b);
  }
  ~test_B(){
    LOG(INFO) << "~test_B()";
  }
  bool varify(int a) {
    LOG(INFO) << "varify() ";
    return (*pInt.get<int>()==a);
  }
public:
  SharedPtr<int> pInt;
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
  nsp_boost::shared_ptr<test_A> pA1;
  pA1 = pA;
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

class myDel : public _Del {
public:
  virtual void operator()(void *p, size_t iSize, std::allocator<char> &a) const {
    LOG(INFO) << "myDel()";
    _Del::operator()((typename std::allocator<char>::pointer) p, iSize, a);
  }
};

void test_VoidSPtr_1() {
  std::allocator<char> loc_alloc;
  // test 1
  LOG(INFO)<<">>>>>>>test 1 [4]<<<<<<<<";
  VoidSPtr p_0;
  LOG_IF(INFO,(p_0 == nullptr))<<"[OK] test_SmartPrt_1 test 1-1";
  VoidSPtr p_1(loc_alloc, sizeof(int));
  p_0 = p_1;
  LOG_IF(INFO,(p_0.get<int>()==p_1.get<int>()))<<"[OK] test_SmartPrt_1 test 1-2";
  VoidSPtr p_1_2 = p_1;
  LOG_IF(INFO,(p_1_2.use_count()==p_0.use_count()))<<"[OK] test_SmartPrt_1 test 1-3 ";
  
  // test 2
  int * pInt = new int(1024);
  VoidSPtr p_2(loc_alloc, (void*)pInt, sizeof(int));
  p_1_2 = p_2;
  LOG_IF(INFO,(p_0.use_count()==2))<<"[OK] test_SmartPrt_1 test 1-4 ";
  p_2.reset();
  LOG(INFO)<<">>>>>>>test 2 [3]<<<<<<<<";
  LOG_IF(INFO,(p_1_2.use_count()==1))<<"[OK] test_SmartPrt_1 test 2-1 ";
  p_1_2.reset();
  LOG_IF(INFO,(p_2 == nullptr)&&(p_2.use_count()==0))<<"[OK] test_SmartPrt_1 test 2-2";
  LOG_IF(INFO,(nullptr == p_2)&&(p_2.use_count()==0))<<"[OK] test_SmartPrt_1 test 2-3";
  
  LOG(INFO)<<">>>>>>>test 3 [3]<<<<<<<<";
  VoidSPtr p_3(loc_alloc, 25);
  VoidSPtr p_4(45);
  VoidSPtr p_3_1(p_3);
  LOG_IF(INFO,(p_3_1.capacity() == 25)&&(p_4.capacity()==45))<<"[OK] test_SmartPrt_1 test 3-1";
  VoidSPtr p_5(loc_alloc, 18, _Del());
  p_5.reset();
  LOG_IF(INFO,(p_5 == nullptr))<<"[OK] test_SmartPrt_1 test 3-2";
  p_5.reset(p_4);
  LOG_IF(INFO,(p_5.use_count()==1)&&(p_4.use_count()==0))<<"[OK] test_SmartPrt_1 test 3-3";
  
  // test 4 注意, 一下只是测试, 实际会报错, 因为内存被重复 free()
//  LOG(INFO)<<">>>>>>>test 4 [3]<<<<<<<<";
//  int * rpInt0 = new int(1024);
//  char * rpCh = (char*)rpInt0 + 1;
//  LOG(INFO)<< *rpInt0 << ", " << rpCh;
//  VoidSPtr pInt0((void*)rpInt0, sizeof(int));
//  VoidSPtr pCh((void*)rpCh, sizeof(char));
//  LOG(INFO)<< "pInt0 "<< pInt0;
//  LOG(INFO)<< "pCh "<< pCh;
//  LOG_IF(INFO,(pInt0.relative(pCh) == RELATIVE_INCLUDE))<<"[OK] test_SmartPrt_1 test 4-1";
}

void test_VoidSPtr_2() {
  LOG(INFO) << "*****test 1*****";
  VoidSPtr p1(sizeof(int), 0);
  VoidSPtr p1_1(sizeof(int), &p1);
  VoidSPtr p1_2(sizeof(int), &p1);
  VoidSPtr p2(sizeof(int), 0);
  VoidSPtr p2_1(sizeof(int), &p2);
  VoidSPtr p3(sizeof(int), 0);
  VoidSPtr p3_1(sizeof(int), &p3);
  // 指向自身
  p1_2 = p1;
  // 循环指向
  p1_1 = p2;
  p2_1 = p3;
  p3_1 = p1;
  
}

void test_SharedPtr_1() {
  std::allocator<char> alloc;
  
  // test 1
  LOG(INFO) << "**********test 1 <4>**********";
  SharedPtr<int> spInt;
  LOG_IF(INFO, (spInt== nullptr)&&(spInt.state()==SP_STATE_NULL))<<"[OK] test 1-1";
  SharedPtr<test_A> spA_1(XXX);
  LOG_IF(INFO, (spA_1!= nullptr)&&(spA_1.state()==SP_STATE_NEW))<<"[OK] test 1-2";
  spA_1.construct(32);
  LOG_IF(INFO, (spA_1!= nullptr)&&(spA_1.state()==SP_STATE_INIT))<<"[OK] test 1-3";
  test_A const & rA_1 = *spA_1/*.get<test_A>()*/;
  spA_1->set(33);
  LOG_IF(INFO, (spA_1->varify(33)))<<"[OK] test 1-4";
  
  // test 2
  LOG(INFO) << "**********test 2 <3>**********";
  SharedPtr<test_B> spB_1(alloc,XXX);
  spB_1.construct(245);
  LOG_IF(INFO, (spB_1 != nullptr)&&(spB_1->varify(245)))<<"[OK] test 2-1";
  SharedPtr<test_B> spB_1_1(spB_1);
  SharedPtr<test_B> spB_1_2 = spB_1;
  LOG_IF(INFO, (spB_1 == spB_1_1)&&(spB_1 == spB_1_2))<<"[OK] test 2-2";
  SharedPtr<Void> spVoid;
  spVoid = spB_1_2;
  VoidSPtr pVoid = &spB_1_2;
  LOG_IF(INFO, (pVoid == spB_1_2) && (pVoid.use_count()==5))<<"[OK] test 2-3";
  
  // test 3
  LOG(INFO) << "**********test 3 <2>**********";
  SharedPtr<test_B> spB_2(alloc, myDel());
  New(int, int_0, 10)
  int_0.destroy();
}

void test_template() {
  template_A<int> a;
  template_A<float> b;
  template_A<> c;
  template_A<std::allocator<char> > d;
  LOG_IF(INFO, (c.strType==d.strType)) << "[OK] test_template";
  template_A<std::allocator<int> > e;
}

void test_VoidWPtr() {
  std::allocator<char> alloc;
  // test 1
  VoidSPtr spB_1(alloc, 10);
  VoidWPtr wpB_1(spB_1);
  LOG_IF(INFO, (spB_1.use_count() == wpB_1.use_count())) << "[OK] test 1-1";
  wpB_1.reset();
  LOG_IF(INFO, (spB_1 != nullptr && wpB_1.expired())) << "[OK] test 1-2";
  spB_1.reset();
  spB_1 = wpB_1.lock_raw();
  LOG_IF(INFO, (spB_1 == nullptr && wpB_1.expired())) << "[OK] test 1-3";
}

void test_WeakPtr() {
  std::allocator<char> alloc;
  // test 1
  SharedPtr<int> spInt_0(alloc, XXX);
  spInt_0.construct(1);
  WeakPtr<int> wpInt(spInt_0);
  SharedPtr<int> spInt_1 = wpInt.lock();
  LOG_IF(INFO, (spInt_1 != nullptr
                && spInt_0.use_count() == spInt_1.use_count()
                && *spInt_1.get<int>() == 1)) << "[OK] test 1-1";
  spInt_0.reset();
  spInt_0 = wpInt.lock();
  LOG_IF(INFO, (spInt_0 != nullptr
                && spInt_0.use_count() == spInt_1.use_count()
                && *spInt_0.get<int>() == 1)) << "[OK] test 1-2";
  spInt_0.reset();
  spInt_1.reset();
  spInt_0 = wpInt.lock();
  LOG_IF(INFO, (spInt_0 == nullptr && spInt_0.state() == SP_STATE_NULL)) << "[OK] test 1-3";
  
  // test 2
  SharedPtr<int> spInt_2;
  WeakPtr<int> wpInt_2(spInt_2);
  WeakPtr<int> wpInt_3;
  LOG_IF(INFO, (wpInt_2 == nullptr)) << "[OK] test 2-1";
  LOG_IF(INFO, (wpInt_2 == wpInt_3)) << "[OK] test 2-2";
}

int main(){
//  test_SmartPrt_0();
//  test_template();
  test_VoidSPtr_1();
  test_SharedPtr_1();
  test_VoidSPtr_2();
  test_VoidWPtr();
  test_WeakPtr();
  
//  boost::shared_ptr<int> spint = boost::make_shared<int>(1);
//  boost::weak_ptr<int> wpint(spint);
//  spint.reset();
//  spint = wpint.lock();
  return 0;
}

