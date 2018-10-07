/**
 * @file FdDef.h
 * @brief FdDef 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.09
 */

#ifndef LIBPUMP_FDDEF_H
#define LIBPUMP_FDDEF_H

#ifdef linux
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // linux

#include <boost/noncopyable.hpp>

#include "pumpdef.h"
#include "ptl/Buffer/Buffer.h"
#include "FdServiceDef.h"

namespace nsp_boost = ::boost;

namespace Pump {

class FdService;
typedef nsp_boost::shared_ptr<FdService> PtrFdService;
class NetService;
typedef nsp_boost::shared_ptr<NetService> PtrTcpService;

/**
 * @struct FdBase
 * @brief 文件描述符(句柄)基类对象
 */
PUMP_ABSTRACT
class FdBase {
public:
  /**
   * @var pump_fd_t fd_
   * @brief io文件描述符
   */
  pump_fd_t fd_;
  /**
   * @var unsigned short fd_ev_
   * @brief io文件描述符的监听事件, 可取(非互斥):
   * - IO_EV_IN
   * - IO_EV_OUT
   * - IO_EV_ERR
   */
  unsigned short fd_ev_;
  /**
   * @var unsigned short re_fd_ev_
   * @brief io文件描述符的发生事件, 可取(非互斥):
   * - IO_EV_IN
   * - IO_EV_OUT
   * - IO_EV_ERR
   */
  unsigned short re_fd_ev_;
  /**
   * @var FdState m_emState
   * @brief io文件描述符的生命周期状态
   */
  FdState m_emState;
  /**
   * @var PtrFdService m_pFdService
   * @brief Tcp 层的服务对象, 主要用于处理操作系统级的fd事件
   */
  PtrFdService m_pFdService;
  
  FdBase()
    : fd_(-1),
      fd_ev_(IO_EV_NONE),
      re_fd_ev_(IO_EV_NONE),
      m_emState(FD_STATE_INIT) {}
  
  virtual ~FdBase() {}
};

typedef nsp_boost::shared_ptr<FdBase> PtrFD;

/**
 * @class FdContainer []
 * @brief Fd 对象的容器
 */
PUMP_ABSTRACT
class FdContainer
  : public nsp_boost::noncopyable {
public:
  FdContainer() {}
  
  virtual ~FdContainer() {}
  
  virtual PtrFD get(pump_fd_t fd) = 0;
  
  virtual void insert(PtrFD pFd) = 0;
  
  virtual void remove(pump_fd_t fd) = 0;
};

typedef nsp_boost::shared_ptr<FdContainer> PtrFdContainer;

/**
 * @class FdHashTable []
 * @brief 存放 IoFd 的哈希表, 以 fd 值作为索引值
 */
PUMP_IMPLEMENT
class FdHashTable
  : public FdContainer {
public:
  FdHashTable() {}
  
  ~FdHashTable() {}
  
  virtual PtrFD get(pump_fd_t fd);
  
  virtual void insert(PtrFD pFd);
  
  virtual void remove(pump_fd_t fd);

private:
  /** FIXME 1024 改用宏 */
  PtrFD m_arrFds[1024];
};

/**
 * @struct IoFd []
 * @brief IO 文件描述符对象
 */
class IoFd
  : public FdBase {
public:
//  /**
//   * @var PtrEvContainer m_pEvents
//   * @brief io文件描述符的注册事件容器, 不允许拷贝和移动
//   * @deprecated [20180909]
//   */
//  PtrEvContainer m_pEvents;
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
    : FdBase()
//    , m_pEvents(nsp_boost::make_shared<EvList>())
  {}
  
  virtual ~IoFd() {
    if(m_emState==FD_STATE_CONNECTED){
      this->close();
    }
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
    
    m_emState = FD_STATE_CLOSED;
    return 0;
  }

//  int shutdown(int how) {
//    int ret = ::shutdown(fd_, how);
//    if (ret == -1) {
//      switch (errno) {
//        case EBADF:
//        case EINVAL:
//        case ENOTCONN:
//        case ENOTSOCK:
//        default: {
//          // FIXME 设置程序错误码
//          LOG(INFO) << "[error(" << errno << ")] ::shutdown()";
//          return -1;
//        }
//      }
//    }
//    return 0;
//  }
};

typedef nsp_boost::shared_ptr<IoFd> PtrIoFd;

}

#endif //LIBPUMP_FDDEF_H
