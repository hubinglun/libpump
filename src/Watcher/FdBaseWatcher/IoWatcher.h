/**
 * @file IoWatcher.h
 * @brief IoWatcher 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.08
 */

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
#include "FdDef.h"

namespace PUMP {

class IoWatcher;

class Socket;

typedef nsp_boost::shared_ptr<Socket> PtrSock;

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
  
  /**
   * @fn nt newAccept(const char *szIp, int iPort,
                PtrTcpService pTcpService);
   * @brief 表示新建一个 Tcp 接收套接字
   * @param szIp ip地址
   * @param iPort 监听端口
   * @param pTcpService 注册Tcp服务
   * @return
   */
  int newAccept(const char *szIp, int iPort,
                PtrTcpService pTcpService);
  
  int enableAccept(pump_fd_t fd);
  
  int disableAccept(pump_fd_t fd);

//  int newConnection(const char* szIp, PtrTcpService pTcpService);
  
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
  
  /* FIXME [FIXED] 以下几个函数为IO事件的一级服务句柄. 用于测试临时放在此处, 后期考虑 \
   * 写到单独的网络层服务对象中 */
//  int acceptHandle(PtrIoFd pFdAccept);
//
//  int recvHandle(PtrIoFd pFdRecv);
//
//  int sendHandle(PtrIoFd pFdSend);

//protected:
//  PtrFdContainer m_pFds;
//  PtrMultiBackend m_pBackend;
};

PUMP_IMPLEMENT
class TcpSockService
  : public FdService {
public:
  TcpSockService();
  
  ~TcpSockService();
  
  static int INHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());
  
  static int OUTHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());

#ifdef _GNU_SOURCE
  
  static int RDHUPHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());

#endif // _GNU_SOURCE
  
  static int ERRHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());
  
  static int HUPHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());
  
  static int NVALHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());

#ifdef _XOPEN_SOURCE
  
  static int RDBANDHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());
  
  static int WRBANDHandle(IoWatcher &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid());

#endif //_XOPEN_SOURCE
private:
  static int acceptHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData = PtrVoid());
  
  static int readHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData = PtrVoid());
  
  static int sendHandle(IoWatcher &rIoWatcher, PtrSock pFd, PtrVoid pData = PtrVoid());
};

class Socket
  : public IoFd {
public:
  /**
   * @var PtrTcpService m_pTcpService
   * @brief Tcp 层的服务对象, 主要是传递用户的回调函数
   */
  PtrTcpService m_pTcpService;
  
  Socket() {
    m_pFdService = nsp_boost::make_shared<TcpSockService>();
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

}

#endif //LIBPUMP_IOWATCHER_H
