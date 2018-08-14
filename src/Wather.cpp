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
//                   IoWather
////////////////////////////////////////////////  

IoWather::IoWather() {}

IoWather::IoWather(PtrCbMailboxMgr pMbMgr)
  : Wather(pMbMgr) {}

IoWather::~IoWather() {}

void IoWather::doWatching() {
  preProcess();
  dispatch();
  postProcess();
}

void IoWather::PostAccept(/* FIXME 需要决定参数 */) {

}

void IoWather::PostConnect(/* FIXME 需要决定参数 */) {

}

void IoWather::PostSend(/* FIXME 需要决定参数 */) {

}

void IoWather::PostShutdown(/* FIXME 需要决定参数 */) {

}

void IoWather::PostClose(/* FIXME 需要决定参数 */) {

}

void IoWather::preProcess() {
  m_pPreEvents->runAll();
}

int IoWather::dispatch() {

}

void IoWather::postProcess() {
  m_pPostEvents->runAll();
}

////////////////////////////////////////////////
//                   FdHashTable
////////////////////////////////////////////////

PtrFD FdHashTable::get(pump_fd_t fd) {
  if (fd >= 1024 || fd < 0) {
    return PtrFD();
  }
  return m_arrFds[fd];
}

void FdHashTable::insert(PtrFD pFd) {
  if (pFd == NULL || m_arrFds[pFd->fd_] != NULL) {
    // FIXME 因设置错误码
    return;
  }
  m_arrFds[pFd->fd_] = pFd;
}

void FdHashTable::remove(pump_fd_t fd) {
  if (fd < 0 || fd >= 1024 || m_arrFds[fd] == NULL) {
    // FIXME 因设置错误码
    return;
  }
  m_arrFds[fd].reset();
}
  
}