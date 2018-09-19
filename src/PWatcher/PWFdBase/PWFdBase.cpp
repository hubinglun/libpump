//
// Created by yz on 18-9-8.
//

#include "PWFdBase.h"

namespace Pump {

////////////////////////////////////////////////
//                   PWFdBase
////////////////////////////////////////////////
#ifdef _TEST_LEVEL_INFO
PWFdBase::PWFdBase(PtrArg pIn, PtrArg pOut, PtrCbMailboxMgr pMbMgr)
: PWCentralized(pIn,pOut,pMbMgr) {}
#endif //_TEST_LEVEL_INFO

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
