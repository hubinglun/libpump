/**
 * @file Watcher.cpp
 * @brief Watcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Wather.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

void IoWather::init(){
  m_pFds = nsp_boost::make_shared<FdHashTable>();
  m_pBackend = nsp_boost::make_shared<Select>();
  m_pBackend->init();
}

void IoWather::doWatching() {
  preProcess();
  dispatch();
  postProcess();
}

int IoWather::newAccept(const char* szIp,int iPort,
                        PfnOnAccept onAccept,
                        PfnOnRecv onRecv,
                        PfnOnSend onSend) {
  int fd;
  sockaddr_in servaddr;
  
  fd = ::socket(AF_INET, SOCK_STREAM, 0);
  
  if (fd == -1) {
    LOG(INFO) << stderr << "create socket fail, erron: %d ,reason: %s\n" <<
              errno << strerror(errno);
    return -1;
  }
  
  /*一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用*/
  int reuse = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
    return -1;
  }
  
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  ::inet_pton(AF_INET, szIp, &servaddr.sin_addr);
  servaddr.sin_port = ::htons(iPort);
  
  if (::bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
    LOG(INFO) << "bind() error: " << errno;
    return -1;
  }
  
  if (::listen(fd, 5/* FIXME 应该由参数决定 */) == -1) {
    LOG(INFO) << "listen() error: " << errno;
    return -1;
  }
  
  PtrFD pIoFd = nsp_boost::make_shared<IoFd>();
  pIoFd->fd_ = fd;
  
  PtrEvList pEvList = nsp_boost::make_shared<EvList>();
  /* 构造OnAccept回调对象 */
  if(!onAccept->m_fn.empty()) {
    PtrIoEvent pEvAccept = nsp_boost::make_shared<IoEvent>();
    pEvAccept->strName_ = "OnAccept";
    pEvAccept->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvAccept->emEvState_ = EVSTATE_INIT;
    pEvAccept->pEvCallback_ = onAccept;
    pEvAccept->fd_ = fd;
    pEvList->insert(pEvAccept->strName_, pEvAccept);
  }
  
  /* 构造OnRecv回调对象 */
  if(!onRecv->m_fn.empty()) {
    PtrIoEvent pEvRecv = nsp_boost::make_shared<IoEvent>();
    pEvRecv->strName_ = "OnRecv";
    pEvRecv->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvRecv->emEvState_ = EVSTATE_INIT;
    pEvRecv->pEvCallback_ = onRecv;
    pEvRecv->fd_ = fd;
    pEvList->insert(pEvRecv->strName_, pEvRecv);
  }
  
  /* 构造OnSend回调对象 */
  if(!onSend->m_fn.empty()) {
    PtrIoEvent pEvSend = nsp_boost::make_shared<IoEvent>();
    pEvSend->strName_ = "OnSend";
    pEvSend->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvSend->emEvState_ = EVSTATE_INIT;
    pEvSend->pEvCallback_ = onRecv;
    pEvSend->fd_ = fd;
    pEvList->insert(pEvSend->strName_, pEvSend);
  }
  pIoFd->m_pEvents = pEvList;
  
  IoFdCtl change;
  change.fd_ = fd;
  change.type_ = FD_CTL_ADD;
  change.fd_ev_ = IO_EV_IN;
  
  if(m_pBackend->update(change)==-1){
    return -1;
  }
  
  pIoFd->m_state = FD_STATE_LISTENED;
  pIoFd->fd_ev_ = IO_EV_IN;
  m_pFds->insert(pIoFd);
  
  return 0;
}

int IoWather::enableAccept(pump_fd_t fd) {
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pIoFd->m_state) {
    case FD_STATE_LISTENED: {
      /*已经在监听状态*/
      return 0;
    }
    case FD_STATE_INIT: {
      IoFdCtl change;
      change.fd_ = fd;
      change.type_ = FD_CTL_ADD;
      change.fd_ev_ = IO_EV_IN;
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->m_state = FD_STATE_LISTENED;
      pIoFd->fd_ev_ = IO_EV_IN;
      return 0;
    }
    case FD_STATE_CLOSED:
    case FD_STATE_CONNECTED:
    case FD_STATE_SHUTDOWNED:
    default: {
      /* FIXME 设置错误码 */
      LOG(INFO) << "fd 所在生命周期状态不允许 enableAccept() 操作";
      return -1;
    }
  }
}

int IoWather::disableAccept(pump_fd_t fd){
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pIoFd->m_state) {
    case FD_STATE_LISTENED: {
      /*从多路复用对象删除fd*/
      IoFdCtl change;
      change.fd_ = fd;
      change.type_ = FD_CTL_DEL;
  
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->m_state = FD_STATE_INIT;
      pIoFd->fd_ev_ = 0;
      return 0;
    }
    case FD_STATE_INIT: {
      return 0;
    }
    case FD_STATE_CLOSED:
    case FD_STATE_CONNECTED:
    case FD_STATE_SHUTDOWNED:
    default: {
      /* FIXME 设置错误码 */
      LOG(INFO) << "fd 所在生命周期状态不允许 disableAccept() 操作";
      return -1;
    }
  }
}

int IoWather::enableRecv(pump_fd_t fd){
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pIoFd->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if (pIoFd->fd_ev_ == 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_ADD;
        change.fd_ev_ = IO_EV_IN;
      } else if ((pIoFd->fd_ev_ & IO_EV_IN) != 0) {
        break;
      } else {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pIoFd->fd_ev_ | IO_EV_IN;
      }
  
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->fd_ev_ |= IO_EV_IN;
      return 0;
    }
    case FD_STATE_INIT:
    case FD_STATE_LISTENED:
    case FD_STATE_CLOSED:
    case FD_STATE_SHUTDOWNED:
    default: {
      /* FIXME 设置错误码 */
      LOG(INFO) << "fd 所在生命周期状态不允许 disableAccept() 操作";
      return -1;
    }
  }
}
int IoWather::disableRecv(pump_fd_t fd){
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pIoFd->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if ((pIoFd->fd_ev_ & IO_EV_IN) != 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pIoFd->fd_ev_ & (~IO_EV_IN);
      } else {
        break;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->fd_ev_ = pIoFd->fd_ev_ & (~IO_EV_IN);
      return 0;
    }
    case FD_STATE_INIT:
    case FD_STATE_LISTENED:
    case FD_STATE_CLOSED:
    case FD_STATE_SHUTDOWNED:
    default: {
      /* FIXME 设置错误码 */
      LOG(INFO) << "fd 所在生命周期状态不允许 disableRecv() 操作";
      return -1;
    }
  }
}
int IoWather::enableSend(pump_fd_t fd){

}
int IoWather::disableSend(pump_fd_t fd){

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

////////////////////////////////////////////////
//                    EvList
////////////////////////////////////////////////

void EvList::insert(nsp_std::string & strName, PtrEvent pEv) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it != m_lEvs.end()) {
    /* 已存在对应关键字元素, 重置元素 */
    it->second.reset();
    it->second = pEv;
    return;
  }
  m_lEvs.insert(nsp_std::make_pair(strName, pEv));
}

void EvList::erase(nsp_std::string & strName){
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return;
  }
  it->second.reset();
  m_lEvs.erase(it);
}

PtrEvent EvList::at(nsp_std::string & strName){
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return nsp_boost::make_shared<Event>();
  }
  return it->second;
}

}