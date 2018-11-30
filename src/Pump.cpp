/**
 * @file Pump.cpp
 * @brief Pump 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Pump.h"

namespace Pump {

////////////////////////////////////////////////
//                   Pump
////////////////////////////////////////////////

Pump::Pump()
  : m_emState(PUMPSTATE_NEW), m_emType(PUMPTYPE_SYNC) {}

#ifdef _TEST_LEVEL_DEBUG

Pump::Pump(PtrArg pIn, PtrArg pOut, PumpType emType)
  : m_emState(PUMPSTATE_NEW),
    m_emType(emType),
//    m_pThread((emType == PUMPTYPE_ASYNC)
//              ? nsp_boost::make_shared<PThread>()
//              : PtrThread()),
    m_pArgIn(pIn),
    m_pArgOut(pOut) {
  
}

#endif //_TEST_LEVEL_DEBUG

void Pump::init() {
  m_emState = PUMPSTATE_INIT;
}

int Pump::start() {
  m_emState = PUMPSTATE_START;
  return 0;
}

int Pump::pause() {
  m_emState = PUMPSTATE_PAUSE;
  return 0;
}

int Pump::stop() {
  m_emState = PUMPSTATE_STOP;
  return 0;
}

void Pump::setArgIn(PtrArg pArgIn) {
  m_pArgIn = pArgIn;
}

PtrArg Pump::getArgOut() {
  return m_pArgOut;
}

////////////////////////////////////////////////
//                   AsyncPump
////////////////////////////////////////////////

AsyncPump::AsyncPump() { }

#ifdef _TEST_LEVEL_DEBUG
AsyncPump::AsyncPump(PtrArg pIn, PtrArg pOut)
  : Pump(pIn, pOut, PUMPTYPE_ASYNC){
  
}
#endif //_TEST_LEVEL_DEBUG

AsyncPump::~AsyncPump(){

}

////////////////////////////////////////////////
//                   PWitness
////////////////////////////////////////////////

PWitness::PWitness() {}

#ifdef _TEST_LEVEL_DEBUG

PWitness::PWitness(PtrArg pIn, PtrArg pOut)
  : AsyncPump(pIn, pOut) {
  m_pThread = nsp_boost::make_shared<PWitnessThread>();
}

#endif //_TEST_LEVEL_DEBUG

PWitness::~PWitness() {}

int PWitness::preWatch() {
  // 暂时不知道干啥, 但是肯定是与Watcher对象相关的预处理
  return 0;
}

void PWitness::routine() {
  while (1) {
    routine_core();
    sleep(2);
  }
}

int PWitness::routine_core() {
  LOG(INFO) << "routine_core()";
  preWatch();
  process();
  postWatch();
  return 0;
}

int PWitness::process() {

//  if (m_pWatchers->empty()) {
//    // FIXME 应该设置错误码 error !!!
//    return 0;
//  }
//  for (PtrWatcher t_wpW = m_pWatchers->begin();
//       t_wpW != NULL && t_wpW != m_pWatchers->end();
//       t_wpW = m_pWatchers->next()) {
//    // 执行 PWatcher 对象的"观察"操作
//    t_wpW->doWatching();
//  }
  return 0;
}

int PWitness::postWatch() {
//  // 目前 postWatch 仅安排执行回调
//  PtrICbMailboxEvoker t_pMailEvoker = nsp_boost::dynamic_pointer_cast<ICbMailboxEvoker>(m_pMbEvoker);
//  if (t_pMailEvoker == NULL) {
//    // FIXME 应该设置错误码 error !!!
//    return 0;
//  }
//  size_t n = t_pMailEvoker->runAll();
  return 0;
}

void PWitness::init() {
  Pump::init();
  m_pThread->m_pMailbox = nsp_boost::make_shared<CbQueueMailbox>();
}

int PWitness::start() {
  if (m_emState < PUMPSTATE_INIT) {
    LOG(WARNING) << "Pump 对象执行前必须初始化";
    return -1;
  }
  Pump::start();
  if (m_pThread == NULL) {
    LOG(ERROR) << "用户线程对象必须非空";
    return -1;
  }
  nsp_boost::function0<void> cb = nsp_boost::bind(&PWitness::routine, this);
  m_pThread->m_pRealThread = nsp_boost::shared_ptr<nsp_boost::thread>(new nsp_boost::thread(cb));
  return 0;
}

int PWitness::pause() {
  Pump::pause();
  return 0;
}

int PWitness::stop() {
  Pump::stop();
  return 0;
}

void PWitness::join() {
  if (m_emState >= PUMPSTATE_START)
    m_pThread->m_pRealThread->join();
}

}
