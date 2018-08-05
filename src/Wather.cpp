/**
 * @file Watcher.cpp
 * @brief Watcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Wather.h"

namespace PUMP {

////////////////////////////////////////////////
//                   Wather
////////////////////////////////////////////////

Wather::Wather(PtrCbMailboxMgr pMbMgr)
  : m_pMbMgr(pMbMgr) {
  
}

void Wather::setArgIn(PtrArg _IN) {
  m_argIn = _IN;
}

PtrArg Wather::getArgOut() {
  return m_argOut;
}

////////////////////////////////////////////////
//                   Wather
////////////////////////////////////////////////  

IWather::IWather() {}

IWather::IWather(PtrCbMailboxMgr pMbMgr)
  : Wather(pMbMgr) {}

IWather::~IWather() {}

void IWather::doWatching() {
  preProcess();
  dispatch();
  postProcess();
}

void IWather::preProcess() {

}

int IWather::dispatch() {

}

void IWather::postProcess() {

}
  
}