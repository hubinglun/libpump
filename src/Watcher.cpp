/**
 * @file Watcher.cpp
 * @brief Watcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Watcher.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace PUMP {

////////////////////////////////////////////////
//                   Watcher
////////////////////////////////////////////////

Watcher::Watcher(PtrCbMailboxMgr pMbMgr)
  : m_pMbMgr(pMbMgr) {
  
}

void Watcher::setArgIn(PtrArg _IN) {
  m_argIn = _IN;
}

PtrArg Watcher::getArgOut() {
  return m_argOut;
}

////////////////////////////////////////////////
//                   IoWatcher
////////////////////////////////////////////////

IoWatcher::IoWatcher() {}

IoWatcher::IoWatcher(PtrCbMailboxMgr pMbMgr)
  : Watcher(pMbMgr) {}

IoWatcher::~IoWatcher() {}

void IoWatcher::init() {
  // FIXME 前置及后置事件容器未初始化
  m_pFds = nsp_boost::make_shared<FdHashTable>();
  m_pBackend = nsp_boost::make_shared<Select>();
  m_pBackend->init();
}

void IoWatcher::doWatching() {
  preProcess();
  dispatch();
  postProcess();
}

int IoWatcher::newAccept(const char *szIp, int iPort,
                         PfnOnAccept onAccept,
                         PfnOnRecv onRecv,
                         PfnOnSend onSend) {
  pump_fd_t fd;
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
  
  /* 构造OnAccept回调对象 */
  if (!onAccept->m_fn.empty()) {
    PtrIoEvent pEvAccept = nsp_boost::make_shared<IoEvent>();
    // FIXME 应该使用构造函数构造, 事件名称应该与事件类型绑定
    pEvAccept->strName_ = "OnAccept";
    pEvAccept->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvAccept->emEvState_ = EVSTATE_INIT;
    pEvAccept->pEvCallback_ = onAccept;
    pEvAccept->fd_ = fd;
    pIoFd->m_pEvents->insert(pEvAccept->strName_, pEvAccept);
  }
  
  /* 构造OnRecv回调对象 */
  if (!onRecv->m_fn.empty()) {
    PtrIoEvent pEvRecv = nsp_boost::make_shared<IoEvent>();
    // FIXME 应该使用构造函数构造, 事件名称应该与事件类型绑定
    pEvRecv->strName_ = "OnRecv";
    pEvRecv->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvRecv->emEvState_ = EVSTATE_INIT;
    pEvRecv->pEvCallback_ = onRecv;
    pEvRecv->fd_ = fd;
    pIoFd->m_pEvents->insert(pEvRecv->strName_, pEvRecv);
  }
  
  /* 构造OnSend回调对象 */
  if (!onSend->m_fn.empty()) {
    PtrIoEvent pEvSend = nsp_boost::make_shared<IoEvent>();
    // FIXME 应该使用构造函数构造, 事件名称应该与事件类型绑定
    pEvSend->strName_ = "OnSend";
    pEvSend->emEvPriority_ = EVPRIOR_DEFAULT;
    pEvSend->emEvState_ = EVSTATE_INIT;
    pEvSend->pEvCallback_ = onRecv;
    pEvSend->fd_ = fd;
    pIoFd->m_pEvents->insert(pEvSend->strName_, pEvSend);
  }
  
  IoFdCtl change;
  change.fd_ = fd;
  change.type_ = FD_CTL_ADD;
  change.fd_ev_ = IO_EV_IN;
  
  if (m_pBackend->update(change) == -1) {
    return -1;
  }
  
  pIoFd->m_state = FD_STATE_LISTENED;
  pIoFd->fd_ev_ = IO_EV_IN;
  m_pFds->insert(pIoFd);
  
  return 0;
}

int IoWatcher::enableAccept(pump_fd_t fd) {
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

int IoWatcher::disableAccept(pump_fd_t fd) {
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

int IoWatcher::enableRecv(pump_fd_t fd) {
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
    }
      break;
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
  
  // 构造接受缓冲区
  pIoFd->m_spIobufRecv.reset(new IoBuffer(IOBUF_LEN));
  return 0;
}

int IoWatcher::disableRecv(pump_fd_t fd) {
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
  return 0;
}

int IoWatcher::enableSend(pump_fd_t fd) {
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
        change.fd_ev_ = IO_EV_OUT;
      } else if ((pIoFd->fd_ev_ & IO_EV_OUT) != 0) {
        break;
      } else {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pIoFd->fd_ev_ | IO_EV_OUT;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->fd_ev_ |= IO_EV_OUT;
    }
      break;
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
  
  // 构造接受缓冲区
  pIoFd->m_spIobufSend.reset(new IoBuffer(IOBUF_LEN));
  return 0;
}

int IoWatcher::disableSend(pump_fd_t fd) {
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pIoFd->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if ((pIoFd->fd_ev_ & IO_EV_OUT) != 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pIoFd->fd_ev_ & (~IO_EV_OUT);
      } else {
        break;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pIoFd->fd_ev_ = pIoFd->fd_ev_ & (~IO_EV_OUT);
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
  return 0;
}

int IoWatcher::PostSend(pump_fd_t fd, const nsp_std::string &strMsg) {
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  if ((pIoFd->m_state != FD_STATE_CONNECTED)
      || (pIoFd->fd_ev_ & IO_EV_OUT == 0)
      || (!pIoFd->m_spIobufSend)) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "不允许使用发送缓冲区";
    return -1;
  }
  pIoFd->m_spIobufSend->append(strMsg.c_str(), strMsg.size());
  return 0;
}

int IoWatcher::PostShutdown(pump_fd_t fd) {
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  if (pIoFd->m_state == FD_STATE_CONNECTED) {
    pIoFd->shutdown(SHUT_RD);
  }
  return 0;
}

int IoWatcher::PostClose(pump_fd_t fd) {
  PtrFD pIoFd = m_pFds->get(fd);
  if (pIoFd == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  if (pIoFd->m_state == FD_STATE_CONNECTED) {
    pIoFd->close();
  }
  return 0;
}

int IoWatcher::preProcess() {
  // FIXME 运行前置事件列表
//  m_pPreEvents->runAll();
  return 0;
}

int IoWatcher::dispatch() {
  IoFdRetList fdRetList;
#ifdef _TEST_LEVEL_INFO
  // FIXME 超时时间目前给了一个固定的时间
  timeval tmOut;
  tmOut.tv_sec = 3;
  tmOut.tv_usec = 0;
#endif //_TEST_LEVEL_INFO
  int ret = m_pBackend->wait(fdRetList, tmOut);
  if (ret == -1) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "MultiplexBackend::wait() fail";
#endif // _TEST_LEVEL_INFO
  } else {
    // TODO 添加返回处理
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "MultiplexBackend::wait() succ";
#endif // _TEST_LEVEL_INFO
    
    // 遍历发生事件的文件描述符, 并按照事件类型处理
    for (IoFdRetList::iterator it = fdRetList.begin();
         it != fdRetList.end(); ++it) {
      PtrFD pIoFd = m_pFds->get((*it).fd_);
      if (pIoFd == NULL) {
        /* FIXME 设置错误码 */
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "error: 未找到fd";
#endif // _TEST_LEVEL_INFO
        continue;
      }
      
      if (((pIoFd->fd_ev_ & IO_EV_IN) != 0)
          && (((*it).re_fd_ev_ & IO_EV_IN) != 0)) {
        switch (pIoFd->m_state) {
          case FD_STATE_LISTENED: {
            acceptHandle(pIoFd);
          }
            break;
          case FD_STATE_CONNECTED: {
            recvHandle(pIoFd);
          }
            break;
          default:
            break;
        }
      }
      if (((pIoFd->fd_ev_ & IO_EV_OUT) != 0)
          && (((*it).re_fd_ev_ & IO_EV_OUT) != 0)) {
        switch (pIoFd->m_state) {
          case FD_STATE_CONNECTED: {
            sendHandle(pIoFd);
          }
            break;
          default:
            break;
        }
      }
      if (((pIoFd->fd_ev_ & IO_EV_ERR) != 0)
          && (((*it).re_fd_ev_ & IO_EV_ERR) != 0)) {
        
      }
    }
  }
  return ret;
}

int IoWatcher::postProcess() {
  // FIXME 运行后置事件列表
//  m_pPostEvents->runAll();
  return 0;
}

int IoWatcher::acceptHandle(PtrFD pFdAccept) {
  if (pFdAccept == NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "error: pFdAccept == NULL";
#endif // _TEST_LEVEL_INFO
    // FIXME 设置错误码
    return -1;
  }
  
  pump_fd_t fdConnector;
  sockaddr_in addrConnector;
  socklen_t addrLen = sizeof(addrConnector);

ACCEPT:
  fdConnector = ::accept(pFdAccept->fd_, (struct sockaddr *) &addrConnector, &addrLen);
  if (fdConnector == -1) {
    if (errno == EINTR) { // 系统调用被中断，需要重新进行
      goto ACCEPT;
    } else {
#ifdef _TEST_LEVEL_INFO
      LOG(INFO) << "accept fail, error(" << errno << "): " << strerror(errno);
#endif // _TEST_LEVEL_INFO
      return -1;
    }
  }

#ifdef _TEST_LEVEL_INFO
  LOG(INFO) << "accept a new client: " << inet_ntoa(addrConnector.sin_addr) << ":" << addrConnector.sin_port;
#endif // _TEST_LEVEL_INFO
  
  // 构造新的IoFd, 将新的连接描述符添加到数组中
  PtrFD pFdConnector = nsp_boost::make_shared<IoFd>();
  pFdConnector->fd_ = fdConnector;
  nsp_std::string strOnRecv("OnRecv");
  nsp_std::string strOnSend("OnSend");
  pFdConnector->m_pEvents->insert(strOnRecv, pFdAccept->m_pEvents->at(strOnRecv));
  pFdConnector->m_pEvents->insert(strOnSend, pFdAccept->m_pEvents->at(strOnSend));
  pFdConnector->m_state = FD_STATE_CONNECTED;
  pFdConnector->fd_ev_ = IO_EV_NONE;
  m_pFds->insert(pFdConnector);
  
  // 允许读事件
  this->enableRecv(fdConnector);
#ifdef _TEST_LEVEL_INFO
  // 测试时, 才启用发送
  this->enableSend(fdConnector);
#endif // _TEST_LEVEL_INFO
  
  return 0;
}

int IoWatcher::recvHandle(PtrFD pFdRecv) {
  if (pFdRecv == NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "error: pFdRecv == NULL";
#endif // _TEST_LEVEL_INFO
    // FIXME 设置错误码
    return -1;
  }
  
  ssize_t ret = 0;
  char buf[1024] = {0};
  
  ret = ::read(pFdRecv->fd_, buf, 1024);
  if (ret == 0) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "warrning: 链接断开";
#endif // _TEST_LEVEL_INFO
    // TODO 链接断开处理
  } else {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "recv: " << buf;
#endif // _TEST_LEVEL_INFO
    pFdRecv->m_spIobufRecv->append(buf, ret);
#ifdef _TEST_LEVEL_INFO
    // test code: 接收一条数据发送一条
    nsp_std::string strMsg("Hello World!!!");
    this->PostSend(pFdRecv->fd_, strMsg);
#endif // _TEST_LEVEL_INFO
  }
  
  return 0;
}

int IoWatcher::sendHandle(PtrFD pFdSend) {
  if (pFdSend == NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "error: pFdSend == NULL";
#endif // _TEST_LEVEL_INFO
    // FIXME 设置错误码
    return -1;
  }
  
  // 从发送缓冲区拷贝要发送数据
  // FIXME 拷贝长度用宏替换
  nsp_std::string strSendBuf;
  long iWillSend = pFdSend->m_spIobufSend->get(strSendBuf, 1024);
  switch (iWillSend) {
    case -1: {
#ifdef _TEST_LEVEL_INFO
      LOG(INFO) << "error: 发送缓冲区取数据失败";
#endif // _TEST_LEVEL_INFO
      // FIXME 设置错误码
      return -1;
    }
      break;
    case 0: {
#ifdef _TEST_LEVEL_INFO
      LOG(INFO) << "warning: 发送缓冲区无数据";
#endif // _TEST_LEVEL_INFO
      return 0;
    }
      break;
    default: {
      ssize_t iRealSend = ::write(pFdSend->fd_, strSendBuf.c_str(), iWillSend);
      if (iRealSend == -1) { // 发送失败
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "::write() error(" << errno << "): " << strerror(errno);
#endif // _TEST_LEVEL_INFO
      } else { // 发送成功
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "send: " << strSendBuf;
#endif // _TEST_LEVEL_INFO
        pFdSend->m_spIobufSend->erase(iRealSend);
      }
    }
  }
  
  return 0;
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

void EvList::insert(nsp_std::string &strName, PtrEvent pEv) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it != m_lEvs.end()) {
    /* 已存在对应关键字元素, 重置元素 */
    it->second.reset();
    it->second = pEv;
    return;
  }
  m_lEvs.insert(nsp_std::make_pair(strName, pEv));
}

void EvList::erase(nsp_std::string &strName) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return;
  }
  it->second.reset();
  m_lEvs.erase(it);
}

PtrEvent EvList::at(nsp_std::string &strName) {
  nsp_std::map<nsp_std::string, PtrEvent>::iterator it = m_lEvs.find(strName);
  if (it == m_lEvs.end()) {
    /* 无查找元素 */
    return PtrEvent();
  }
  return it->second;
}

}