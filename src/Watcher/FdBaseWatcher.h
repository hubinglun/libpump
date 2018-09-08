/**
 * @file FdWatcher.h
 * @brief FdWatcher 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.08
 */

#ifndef LIBPUMP_FDBASEWATCHER_H
#define LIBPUMP_FDBASEWATCHER_H

#include <vector>
#include <map>

#ifdef linux
#include <unistd.h>
#include <sys/socket.h>
#endif // linux

#include "Watcher.h"
#include "Event/Event.h"
#include "Event/EventContainer.h"
#include "MultiplexBackend.h"
#include "Buffer/Buffer.h"
#include "Logger.h"

namespace PUMP {

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
   * @var FdState m_state
   * @brief io文件描述符的生命周期状态
   */
  FdState m_state;
  
  FdBase()
    : fd_(-1),
      fd_ev_(IO_EV_NONE),
      re_fd_ev_(IO_EV_NONE),
      m_state(FD_STATE_INIT) {}
  
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
 * @class FdBaseWatcher []
 * @brief 用于监听文件描述符的事件检测器
 */
PUMP_ABSTRACT
class FdBaseWatcher
  : public CentralizedWatcher {
public:
  FdBaseWatcher() {}
  
  explicit FdBaseWatcher(PtrCbMailboxMgr pMbMgr)
    : CentralizedWatcher(pMbMgr) {}
  
  virtual ~FdBaseWatcher() {}

protected:
  PtrFdContainer m_pFds;
  PtrMultiBackend m_pBackend;
};

}

#endif //LIBPUMP_FDBASEWATCHER_H
