/**
 * @file IoWatcher.cpp
 * @brief IoWatcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.09
 */

#include "IoWatcher.h"

namespace PUMP {

////////////////////////////////////////////////
//                   IoWatcher
////////////////////////////////////////////////

IoWatcher::IoWatcher() {}

IoWatcher::IoWatcher(PtrCbMailboxMgr pMbMgr)
  : FdBaseWatcher(pMbMgr) {}

IoWatcher::~IoWatcher() {}

void IoWatcher::init() {
  m_pEvents = nsp_boost::make_shared<EvList>();
  m_pPostEvents = nsp_boost::make_shared<PrePostList>();
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

int IoWatcher::newAccept(const char *szIp, ushort iPort,
                         PtrTcpService pTcpService) {
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
  
  PtrSock pSock = nsp_boost::make_shared<Socket>();
  pSock->fd_ = fd;
  
  pSock->m_pTcpService = pTcpService;
  
  IoFdCtl change;
  change.fd_ = fd;
  change.type_ = FD_CTL_ADD;
  change.fd_ev_ = IO_EV_IN;
  if (m_pBackend->update(change) == -1) {
    return -1;
  }
  
  pSock->m_state = FD_STATE_LISTENED;
  pSock->fd_ev_ = IO_EV_IN;
  m_pFds->insert(pSock);
  
  return 0;
}

int IoWatcher::enableAccept(pump_fd_t fd) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
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
      pSock->m_state = FD_STATE_LISTENED;
      pSock->fd_ev_ = IO_EV_IN;
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
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
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
      pSock->m_state = FD_STATE_INIT;
      pSock->fd_ev_ = 0;
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
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if (pSock->fd_ev_ == 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_ADD;
        change.fd_ev_ = IO_EV_IN;
      } else if ((pSock->fd_ev_ & IO_EV_IN) != 0) {
        break;
      } else {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pSock->fd_ev_ | IO_EV_IN;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pSock->fd_ev_ |= IO_EV_IN;
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
  if (!pSock->m_pIobufRecv) {
    pSock->m_pIobufRecv.reset(new IoBuffer(IOBUF_LEN));
  }
  
  return 0;
}

int IoWatcher::disableRecv(pump_fd_t fd) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if ((pSock->fd_ev_ & IO_EV_IN) != 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pSock->fd_ev_ & (~IO_EV_IN);
      } else {
        break;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pSock->fd_ev_ = pSock->fd_ev_ & (~IO_EV_IN);
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
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if (pSock->fd_ev_ == 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_ADD;
        change.fd_ev_ = IO_EV_OUT;
      } else if ((pSock->fd_ev_ & IO_EV_OUT) != 0) {
        break;
      } else {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pSock->fd_ev_ | IO_EV_OUT;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pSock->fd_ev_ |= IO_EV_OUT;
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
  if (!pSock->m_pIobufSend) {
    pSock->m_pIobufSend.reset(new IoBuffer(IOBUF_LEN));
  }
  
  return 0;
}

int IoWatcher::disableSend(pump_fd_t fd) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  switch (pSock->m_state) {
    case FD_STATE_CONNECTED: {
      IoFdCtl change;
      if ((pSock->fd_ev_ & IO_EV_OUT) != 0) {
        change.fd_ = fd;
        change.type_ = FD_CTL_MOD;
        change.fd_ev_ = pSock->fd_ev_ & (~IO_EV_OUT);
      } else {
        break;
      }
      
      if (m_pBackend->update(change) == -1) {
        /* FIXME 设置错误码 */
        LOG(INFO) << "fd 跟新到 MutiplexBackend 失败";
        return -1;
      }
      /*跟新状态*/
      pSock->fd_ev_ = pSock->fd_ev_ & (~IO_EV_OUT);
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
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  // 启用发送事件
  this->enableSend(pSock->fd_);
  
  if ((pSock->m_state != FD_STATE_CONNECTED)
      || ((pSock->fd_ev_ & IO_EV_OUT) == 0)
      || (!pSock->m_pIobufSend)) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "不允许使用发送缓冲区";
    return -1;
  }
  // 向发送缓冲区投递数据
  pSock->m_pIobufSend->append(strMsg.c_str(), strMsg.size());
  return 0;
}

int IoWatcher::PostShutdown(pump_fd_t fd) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  if (pSock->m_state == FD_STATE_CONNECTED) {
    pSock->shutdown(SHUT_RD);
  }
  return 0;
}

int IoWatcher::PostClose(pump_fd_t fd) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(m_pFds->get(fd));
  if (pSock == NULL) {
    /* FIXME 设置错误码 */
    LOG(INFO) << "未找到fd";
    return -1;
  }
  
  if (pSock->m_state == FD_STATE_CONNECTED) {
    if (pSock->close() == -1) {
      // FIXME 处理错误
    }
    
    IoFdCtl change;
    change.fd_ = pSock->fd_;
    change.type_ = FD_CTL_DEL;
    m_pBackend->update(change);
    
    // 调用关闭回调
    pSock->m_pTcpService->closeCb(*this, pSock, PtrVoid()/*FIXME 决定参数*/);
    
    m_pFds->remove(pSock->fd_);
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
      PtrIoFd pIoFd = nsp_boost::dynamic_pointer_cast<IoFd>(m_pFds->get((*it).fd_));
      if (pIoFd == NULL) {
        /* FIXME 设置错误码 */
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "error: 未找到fd";
#endif // _TEST_LEVEL_INFO
        continue;
      }
      
      if (((pIoFd->fd_ev_ & IO_EV_IN) != 0)
          && (((*it).re_fd_ev_ & IO_EV_IN) != 0)) {
//        switch (pIoFd->m_state) {
//          case FD_STATE_LISTENED: {
//            acceptHandle(pIoFd);
//          }
//            break;
//          case FD_STATE_CONNECTED: {
//            recvHandle(pIoFd);
//          }
//            break;
//          default:
//            break;
//        }
        if (pIoFd->m_pFdService->m_pCbIN == NULL) {
#ifdef _TEST_LEVEL_INFO
          LOG(INFO) << "[warning] FdService::m_pCbIN == NULL";
#endif // _TEST_LEVEL_INFO
          break;
        }
        pIoFd->m_pFdService->m_pCbIN(*this, pIoFd, PtrVoid()/*FIXME 决定此参数*/);
      }
      if (((pIoFd->fd_ev_ & IO_EV_OUT) != 0)
          && (((*it).re_fd_ev_ & IO_EV_OUT) != 0)) {
//        switch (pIoFd->m_state) {
//          case FD_STATE_CONNECTED: {
//            // FIXME 现在
//            sendHandle(pIoFd);
//          }
//            break;
//          default:
//            break;
//        }
        if (pIoFd->m_pFdService->m_pCbOUT == NULL) {
#ifdef _TEST_LEVEL_INFO
          LOG(INFO) << "[warning] FdService::m_pCbOUT == NULL";
#endif // _TEST_LEVEL_INFO
          break;
        }
        pIoFd->m_pFdService->m_pCbOUT(*this, pIoFd, PtrVoid()/*FIXME 决定此参数*/);
      }
      if (((pIoFd->fd_ev_ & IO_EV_ERR) != 0)
          && (((*it).re_fd_ev_ & IO_EV_ERR) != 0)) {
        if (pIoFd->m_pFdService->m_pCbERR == NULL) {
#ifdef _TEST_LEVEL_INFO
          LOG(INFO) << "[warning] FdService::m_pCbERR == NULL";
#endif // _TEST_LEVEL_INFO
          break;
        }
        pIoFd->m_pFdService->m_pCbERR(*this, pIoFd, PtrVoid()/*FIXME 决定此参数*/);
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

//int IoWatcher::acceptHandle(PtrIoFd pFdAccept) {
//  if (pFdAccept == NULL) {
//#ifdef _TEST_LEVEL_INFO
//    LOG(INFO) << "error: pFdAccept == NULL";
//#endif // _TEST_LEVEL_INFO
//    // FIXME 设置错误码
//    return -1;
//  }
//
//  pump_fd_t fdConnector;
//  sockaddr_in addrConnector;
//  socklen_t addrLen = sizeof(addrConnector);
//
//ACCEPT:
//  fdConnector = ::accept(pFdAccept->fd_, (struct sockaddr *) &addrConnector, &addrLen);
//  if (fdConnector == -1) {
//    if (errno == EINTR) { // 系统调用被中断，需要重新进行
//      goto ACCEPT;
//    } else {
//#ifdef _TEST_LEVEL_INFO
//      LOG(INFO) << "accept fail, error(" << errno << "): " << strerror(errno);
//#endif // _TEST_LEVEL_INFO
//      return -1;
//    }
//  }
//
//#ifdef _TEST_LEVEL_INFO
//  LOG(INFO) << "accept a new client: " << inet_ntoa(addrConnector.sin_addr)
//            << ":" << ::ntohs(addrConnector.sin_port);
//#endif // _TEST_LEVEL_INFO
//
//  // 构造新的IoFd, 将新的连接描述符添加到数组中
//  PtrIoFd pFdConnector = nsp_boost::make_shared<IoFd>();
//  pFdConnector->fd_ = fdConnector;
//// [20180909] 删减
////  nsp_std::string strOnRecv("OnRecv");
////  nsp_std::string strOnSend("OnSend");
////  pFdConnector->m_pEvents->insert(strOnRecv, pFdAccept->m_pEvents->at(strOnRecv));
////  pFdConnector->m_pEvents->insert(strOnSend, pFdAccept->m_pEvents->at(strOnSend));
//  pFdConnector->m_state = FD_STATE_CONNECTED;
//  pFdConnector->fd_ev_ = IO_EV_NONE;
//  // [20180907] 新加Tcp服务
//  pFdConnector->m_pTcpService = pFdAccept->m_pTcpService;
//  m_pFds->insert(pFdConnector);
//
//  // 调用接受连接回调
//  pFdConnector->m_pTcpService->acceptCb(*this, pFdConnector);
//
//  // 允许读事件
//  this->enableRecv(fdConnector);
//
//  return 0;
//}
//
//int IoWatcher::recvHandle(PtrIoFd pFdRecv) {
//  if (pFdRecv == NULL) {
//#ifdef _TEST_LEVEL_INFO
//    LOG(INFO) << "error: pFdRecv == NULL";
//#endif // _TEST_LEVEL_INFO
//    // FIXME 设置错误码
//    return -1;
//  }
//
//  ssize_t ret = 0;
//  char buf[1024] = {0};
//
//  ret = ::read(pFdRecv->fd_, buf, 1024);
//  if (ret == 0) {
//#ifdef _TEST_LEVEL_INFO
//    LOG(INFO) << "warrning: 链接断开";
//#endif // _TEST_LEVEL_INFO
//    // FIXME [critical] 缺少链接断开处理
//    this->PostClose(pFdRecv->fd_);
//  } else {
//#ifdef _TEST_LEVEL_INFO
//    LOG(INFO) << "recv: " << buf;
//#endif // _TEST_LEVEL_INFO
//
//    // 压入接收缓冲区
//    pFdRecv->m_pIobufRecv->append(buf, ret);
//
//    if (pFdRecv->m_pTcpService == NULL) {
//      return -1;
//    }
//    pFdRecv->m_pTcpService->recvCb(*this, pFdRecv);
//
//#ifdef _TEST_LEVEL_INFO
//    // test code: 接收一条数据发送一条
////    nsp_std::string strMsg("Hello World!!!");
////    this->PostSend(pFdRecv->fd_, strMsg);
//#endif // _TEST_LEVEL_INFO
//  }
//
//  return 0;
//}
//
//int IoWatcher::sendHandle(PtrIoFd pFdSend) {
//  if (pFdSend == NULL) {
//#ifdef _TEST_LEVEL_INFO
//    LOG(INFO) << "error: pFdSend == NULL";
//#endif // _TEST_LEVEL_INFO
//    // FIXME 设置错误码
//    return -1;
//  }
//
//  // 从发送缓冲区拷贝要发送数据
//  // FIXME 拷贝长度用宏替换
//  nsp_std::string strSendBuf;
//  long iWillSend = pFdSend->m_pIobufSend->get(strSendBuf, 1024);
//  switch (iWillSend) {
//    case -1: {
//#ifdef _TEST_LEVEL_INFO
//      LOG(INFO) << "error: 发送缓冲区取数据失败";
//#endif // _TEST_LEVEL_INFO
//      // FIXME 设置错误码
//      return -1;
//    }
//      break;
//    case 0: {
//      // 发送缓冲区
//      // FIXME 发送缓冲区为0, 应该禁止再让IO多路复用监听写事件 [FIXED]
//      disableSend(pFdSend->fd_);
//      return 0;
//    }
//      break;
//    default: {
//      ssize_t iRealSend = ::write(pFdSend->fd_, strSendBuf.c_str(), iWillSend);
//      if (iRealSend == -1) { // 发送失败
//#ifdef _TEST_LEVEL_INFO
//        LOG(INFO) << "::write() error(" << errno << "): " << strerror(errno);
//#endif // _TEST_LEVEL_INFO
//      } else { // 发送成功
//#ifdef _TEST_LEVEL_INFO
//        LOG(INFO) << "send: " << strSendBuf;
//#endif // _TEST_LEVEL_INFO
//
//        // 发送成功就要从缓冲区删除
//        pFdSend->m_pIobufSend->erase(iRealSend);
//
//        // FIXME 用 NetService 对象来处理 [FIXED]
////        // 向函数邮箱投递一个
////        nsp_std::string strOnSend = "OnSend";
////        PtrEvent pEv = pFdSend->m_pEvents->at(strOnSend);
////        PfnOnSend pSend = nsp_boost::dynamic_pointer_cast<OnSend>(pEv->pEvCallback_);
////        m_pMbMgr->insert(EVPRIOR_DEFAULT, pSend);
//
//        if (pFdSend->m_pTcpService == NULL) {
//          return -1;
//        }
//        pFdSend->m_pTcpService->sendCb(*this, pFdSend);
//      }
//    }
//  }
//
//  return 0;
//}

////////////////////////////////////////////////
//                   SockService
////////////////////////////////////////////////

TcpSockService::TcpSockService() {
  m_pCbIN = INHandle;
  m_pCbOUT = OUTHandle;
#ifdef _GNU_SOURCE
  m_pCbRDHUP = RDHUPHandle;
#endif // _GNU_SOURCE
  m_pCbERR = ERRHandle;
  m_pCbHUP = HUPHandle;
  m_pCbNVAL = NVALHandle;
#ifdef _XOPEN_SOURCE
  m_pCbRDBAND = RDBANDHandle;
  m_pCbWRBAND = WRBANDHandle;
#endif // _XOPEN_SOURCE
}

TcpSockService::~TcpSockService() {

}

int TcpSockService::INHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(pFd);
  if (pSock == NULL) {
    // FIXME 设置错误码
    return -1;
  }
  switch (pFd->m_state) {
    case FD_STATE_LISTENED: {
      acceptHandle(rIoWatcher, pSock, pData);
    }
      break;
    case FD_STATE_CONNECTED: {
      readHandle(rIoWatcher, pSock, pData);
    }
      break;
    default:
      break;
  }
  return 0;
}

int TcpSockService::OUTHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  PtrSock pSock = nsp_boost::dynamic_pointer_cast<Socket>(pFd);
  if (pSock == NULL) {
    // FIXME 设置错误码
    return -1;
  }
  switch (pFd->m_state) {
    case FD_STATE_CONNECTED: {
      // FIXME 现在
      sendHandle(rIoWatcher, pSock, pData);
    }
      break;
    default:
      break;
  }
  return 0;
}

#ifdef _GNU_SOURCE

int TcpSockService::RDHUPHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

#endif // _GNU_SOURCE

int TcpSockService::ERRHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

int TcpSockService::HUPHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

int TcpSockService::NVALHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

#ifdef _XOPEN_SOURCE

int TcpSockService::RDBANDHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

int TcpSockService::WRBANDHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData) {
  return 0;
}

#endif //_XOPEN_SOURCE

int TcpSockService::acceptHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData) {
  if (pFd == NULL) {
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
  fdConnector = ::accept(pFd->fd_, (struct sockaddr *) &addrConnector, &addrLen);
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
  LOG(INFO) << "accept a new client: " << inet_ntoa(addrConnector.sin_addr)
            << ":" << ::ntohs(addrConnector.sin_port);
#endif // _TEST_LEVEL_INFO
  
  // 构造新的IoFd, 将新的连接描述符添加到数组中
  PtrSock pFdConnector = nsp_boost::make_shared<Socket>();
  pFdConnector->fd_ = fdConnector;
// [20180909] 删减
//  nsp_std::string strOnRecv("OnRecv");
//  nsp_std::string strOnSend("OnSend");
//  pFdConnector->m_pEvents->insert(strOnRecv, pFdAccept->m_pEvents->at(strOnRecv));
//  pFdConnector->m_pEvents->insert(strOnSend, pFdAccept->m_pEvents->at(strOnSend));
  pFdConnector->m_state = FD_STATE_CONNECTED;
  pFdConnector->fd_ev_ = IO_EV_NONE;
  // [20180907] 新加Tcp服务
  pFdConnector->m_pTcpService = pFd->m_pTcpService;
  rIoWatcher.m_pFds->insert(pFdConnector);
  
  // 调用接受连接回调
  pFdConnector->m_pTcpService->acceptCb(rIoWatcher, pFdConnector, pData);
  
  // 允许读事件
  rIoWatcher.enableRecv(fdConnector);
  
  return 0;
}

int TcpSockService::readHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData) {
  if (pFd == NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "error: pFdRecv == NULL";
#endif // _TEST_LEVEL_INFO
    // FIXME 设置错误码
    return -1;
  }
  
  // FIXME 缓冲区大小使用宏, 使可调节
  char buf[1024] = {0};
  ssize_t ret = ::read(pFd->fd_, buf, 1024);
  if (ret == 0) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "warrning: 链接断开";
#endif // _TEST_LEVEL_INFO
    // FIXME [critical] 缺少链接断开处理
    rIoWatcher.PostClose(pFd->fd_);
  } else {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "recv: " << buf;
#endif // _TEST_LEVEL_INFO
    
    // 压入接收缓冲区
    pFd->m_pIobufRecv->append(buf, static_cast<size_t >(ret));
    
    if (pFd->m_pTcpService == NULL) {
      return -1;
    }
    pFd->m_pTcpService->recvCb(rIoWatcher, pFd, pData);

#ifdef _TEST_LEVEL_INFO
    // test code: 接收一条数据发送一条
//    nsp_std::string strMsg("Hello World!!!");
//    this->PostSend(pFdRecv->fd_, strMsg);
#endif // _TEST_LEVEL_INFO
  }
  
  return 0;
}

int TcpSockService::sendHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData) {
  if (pFd == NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "error: pFdSend == NULL";
#endif // _TEST_LEVEL_INFO
    // FIXME 设置错误码
    return -1;
  }
  
  // 从发送缓冲区拷贝要发送数据
  // FIXME 拷贝长度用宏替换
  nsp_std::string strSendBuf;
  long iWillSend = pFd->m_pIobufSend->get(strSendBuf, 1024);
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
      // 发送缓冲区
      // FIXME 发送缓冲区为0, 应该禁止再让IO多路复用监听写事件 [FIXED]
      rIoWatcher.disableSend(pFd->fd_);
      return 0;
    }
      break;
    default: {
      ssize_t iRealSend = ::write(pFd->fd_, strSendBuf.c_str(), static_cast<size_t >(iWillSend));
      if (iRealSend == -1) { // 发送失败
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "::write() error(" << errno << "): " << strerror(errno);
#endif // _TEST_LEVEL_INFO
      } else { // 发送成功
#ifdef _TEST_LEVEL_INFO
        LOG(INFO) << "send: " << strSendBuf;
#endif // _TEST_LEVEL_INFO
        
        // 发送成功就要从缓冲区删除
        pFd->m_pIobufSend->erase(static_cast<size_t >(iRealSend));
        
        // FIXME 用 TcpService 对象来处理 [FIXED]
//        // 向函数邮箱投递一个
//        nsp_std::string strOnSend = "OnSend";
//        PtrEvent pEv = pFdSend->m_pEvents->at(strOnSend);
//        PfnOnSend pSend = nsp_boost::dynamic_pointer_cast<OnSend>(pEv->pEvCallback_);
//        m_pMbMgr->insert(EVPRIOR_DEFAULT, pSend);
        
        if (pFd->m_pTcpService == NULL) {
          return -1;
        }
        pFd->m_pTcpService->sendCb(rIoWatcher, pFd, pData);
      }
    }
  }
  
  return 0;
}

}