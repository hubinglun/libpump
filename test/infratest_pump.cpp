/**
 * @file infratest_pump.cpp
 * @brief Pump 对象的测试代码
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Pump.h"

#include <glog/logging.h>

#include "infratest_CbMailbox.h"

using namespace google;
using namespace PUMP;

void initMailbox(PtrCbMailbox pMailbox) {
//	CbQueueMailbox cbMB;
  google::InitGoogleLogging("test");
  google::SetStderrLogging(google::GLOG_INFO);
  
  LOG(INFO)<<"test info";
  
  PtrCbFn afn(new cb_func1(3));
  boost::shared_ptr<cb_func1> ptr1 = dynamic_pointer_cast<cb_func1>(afn);
  ptr1->m_fn = bind(func1, _1);
  
  /**< 测试用例<2> (通过)*/
  PtrCbFn afn2(new cb_func2(1, 65));
  boost::shared_ptr<cb_func2> ptr2 = dynamic_pointer_cast<cb_func2>(afn2);
  ptr2->m_fn = bind(func2, _1, _2);
  
  /**< 测试用例<3> (通过)*/
  PtrCbFn afn3(new cb_makeA(3));
  boost::shared_ptr<cb_makeA> ptr3 = dynamic_pointer_cast<cb_makeA>(afn3);
  ptr3->m_fn = bind(makeA, _1);
  
  /**< 测试用例<4> (未通过)
   *
   * @bug 栈上对象跳出作用于就析构, 到执行回调对象时, argA 已被析构掉了*
   */
//	A argA(1);
//	PtrCbFn afn4(new cb_modifyA(argA));
//	boost::shared_ptr<cb_modifyA> ptr4 = dynamic_pointer_cast<cb_modifyA>(afn4);
//	ptr4->m_fn = bind(modifyA,_1);
  
  pMailbox->insert(EVPRIOR_LEVEL0, afn);
  pMailbox->insert(EVPRIOR_LEVEL1, afn2);
  pMailbox->insert(EVPRIOR_LEVEL2, afn3);
//	pMailbox->insert(EVPRIOR_LEVEL0,afn4);
  
  google::ShutdownGoogleLogging();
}

int main() {
  // 构造一个 CbMailbox 的托管对象
  PtrCbMailbox pMailbox = nsp_boost::make_shared<CbQueueMailbox>();
  initMailbox(pMailbox);
  // 构造一个 ICbMailboxEvoker 的托管对象
  PtrICbMailboxEvoker pEvoker = nsp_boost::make_shared<ICbMailboxEvoker>(pMailbox);
  IPump t_pump(pEvoker);
  nsp_std::cout << t_pump.test_MbEvoker_runAll() << nsp_std::endl;
  return 0;
}
