//
// Created by yz on 18-9-8.
//

#ifndef LIBPUMP_IOWATCHER_H
#define LIBPUMP_IOWATCHER_H

#ifdef linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // linux

#include "FdBaseWatcher.h"

namespace PUMP {

class IoWatcher;
typedef nsp_boost::shared_ptr<IoWatcher> PtrIoWatcher;

class TcpService;
typedef nsp_boost::shared_ptr<TcpService> PtrTcpService;

/**
 * @struct IoFd []
 * @brief IO 文件描述符对象
 */
class IoFd
  : public FdBase {
public:
  /**
   * @var PtrEvContainer m_pEvents
   * @brief io文件描述符的注册事件容器, 不允许拷贝和移动
   */
  PtrEvContainer m_pEvents;
  /**
   * @var PtrTcpService m_pTcpService
   * @brief Tcp 层的服务对象, 主要是传递用户的回调函数
   */
  PtrTcpService m_pTcpService;
  /**
   * @var PtrIoBuffer m_pIobufRecv
   * @brief 输入缓冲区
   * 注: 仅io套接字有效
   */
  PtrIoBuffer m_pIobufRecv;
  /**
   * @var PtrIoBuffer m_pIobufSend
   * @brief 输出缓冲区
   * 注: 仅io套接字有效
   */
  PtrIoBuffer m_pIobufSend;
  /**
   * @var PtrVoid m_pData
   * @brief 用户数据, 一般为结构体
   */
  PtrVoid m_pData;
  
  /**
   * @fn 构造函数
   * @brief 主要是初始化 m_pEvents 对象
   */
  IoFd()
    : FdBase(),
      m_pEvents(nsp_boost::make_shared<EvList>()) {}
  
  virtual ~IoFd() {
    this->close();
  }
  
  int close() {
    do {
      int ret = ::close(fd_);
      if (ret == -1) {
        switch (errno) {
          case EINTR: {
            LOG(INFO) << "[warning] ::close() is interupted!";
            continue;
          }
            break;
          case EBADF:
          case EIO:
          default: {
            // FIXME 设置程序错误码
            LOG(INFO) << "[error(" << errno << ")] ::close()";
            return -1;
          }
        }
      }
      break;
    } while (1);
  
    return 0;
  }
  
  int shutdown(int how) {
    int ret = ::shutdown(fd_, how);
    if (ret == -1) {
      switch (errno) {
        case EBADF:
        case EINVAL:
        case ENOTCONN:
        case ENOTSOCK:
        default: {
          // FIXME 设置程序错误码
          LOG(INFO) << "[error(" << errno << ")] ::shutdown()";
          return -1;
        }
      }
    }
    return 0;
  }
};

typedef nsp_boost::shared_ptr<IoFd> PtrIoFd;

/**
 * @class NetService
 * @brief 与 IoFd 绑定的服务对象
 */
PUMP_ABSTRACT
class TcpService
  : public Service {
public:
  TcpService() {}
  
  virtual ~TcpService() {}

  virtual int acceptCb(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData=PtrVoid()) = 0;
  
  virtual int sendCb(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData=PtrVoid()) = 0;
  
  virtual int recvCb(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData=PtrVoid()) = 0;
  
  virtual int closeCb(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData=PtrVoid()) = 0;
};

/**
 * @class IoWatcher []
 * @brief Watcher对象的一个实现,主要用于单元测试
 * FIXME 仅用于测试,之后应该改为派生 FdBaseWatcher
 */
PUMP_IMPLEMENT
class IoWatcher
  : public FdBaseWatcher {
public:
#define IOBUF_LEN 128
  
  IoWatcher();
  
  explicit IoWatcher(PtrCbMailboxMgr pMbMgr);
  
  virtual ~IoWatcher();

public:
  virtual void doWatching();
  
  void init();
  
  int newAccept(const char *szIp, int iPort,
                PtrTcpService pTcpService);
  
  int enableAccept(pump_fd_t fd);
  
  int disableAccept(pump_fd_t fd);
  
  /*void newConnection(const char* szIp,int iPort,
                     PtrTcpService pTcpService);*/
  
  int enableRecv(pump_fd_t fd);
  
  int disableRecv(pump_fd_t fd);
  
  int enableSend(pump_fd_t fd);
  
  int disableSend(pump_fd_t fd);
  
  int PostSend(pump_fd_t fd, const nsp_std::string &strMsg);
  
  int PostShutdown(pump_fd_t fd);
  
  int PostClose(pump_fd_t fd);

protected:
  virtual int preProcess();
  
  virtual int dispatch();
  
  virtual int postProcess();
  
  /* FIXME 以下几个函数为IO事件的一级服务句柄. 用于测试临时放在此处, 后期考虑 \
   * 写到单独的网络层服务对象中*/
  int acceptHandle(PtrIoFd pFdAccept);
  
  int recvHandle(PtrIoFd pFdRecv);
  
  int sendHandle(PtrIoFd pFdSend);

//protected:
//  PtrFdContainer m_pFds;
//  PtrMultiBackend m_pBackend;
};

}

#endif //LIBPUMP_IOWATCHER_H
