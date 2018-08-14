/**
 * @file Watcher.h
 * @brief Watcher 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#ifndef LIBPUMP_WATHER_H
#define LIBPUMP_WATHER_H

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "pumpdef.h"
#include "CbMailbox.h"
#include "MultiplexBackend.h"

namespace nsp_boost = ::boost;

namespace PUMP {

/**
 * @class EventContainer
 * @brief Event 事件容器
 *
 * Watcher 对象中存放注册 Event 事件的容器
 */
PUMP_ABSTRACT
class EventContainer
  : public nsp_boost::noncopyable {
public:
  EventContainer() {}
  virtual ~EventContainer() {}
  virtual size_t runAll() = 0;
};

typedef nsp_boost::shared_ptr<EventContainer> PtrEvContainer;

/**
 * @class PreEvContainer
 * @brief 存放 Pre-Event 的容器抽象类
 */
PUMP_ABSTRACT
class PrePostEvContainer {
public:
  PrePostEvContainer() {}
  virtual ~PrePostEvContainer() {}
  virtual size_t runAll() = 0;
private:
  /**
   * @var PtrEvContainer m_level0
   * @brief 框架级, 周期性Pre-Event
   */
  PtrEvContainer m_level_0;
  
  /**
   * @var PtrCbContainer m_level1
   * @brief 用户级, 一次性Pre-Event, 直接插入 CbFn 对象
   */
  PtrCbContainer m_level_1;
};

typedef nsp_boost::shared_ptr<PrePostEvContainer> PtrPreEvContainer;
typedef nsp_boost::shared_ptr<PrePostEvContainer> PtrPostEvContainer;

/**
 * @class Wather []
 * @brief 事件观察者对象
 *
 * 注册, 监听, 激活, 处理(投递)事件
 */
PUMP_ABSTRACT
class Wather {
public:
  Wather() {}
  
  // FIXME 目前没有解决pMbMgr参数由谁传入的问题
  Wather(PtrCbMailboxMgr pMbMgr);
  
  ~Wather() {}

public:
  virtual void doWatching(/* FIXME 参数是否需要? */) = 0;
  
  void setArgIn(PtrArg _IN);
  
  PtrArg getArgOut();

private:
  virtual void preProcess() = 0;
  
  virtual int dispatch() = 0;
  
  virtual void postProcess() = 0;

private:
  //! < Watcher 对象名
  nsp_std::string m_strName;
  /**
   * @var PtrCbMailboxMgr m_pMbMgr
   * @brief CbMailbox 管理对象, 可以向邮箱增删回调对象
   */
  PtrCbMailboxMgr m_pMbMgr;
  /**
   * @var PtrEvContainer m_pEvContainer
   * @brief Event 容器, 可向其中注册 Event
   */
  PtrEvContainer m_pEvContainer;
  /**
   * @var PtrArg m_argIn
   * @brief Wather 对象的输入参数
   *
   * 指针对象, 因此可以是任何类型的数据, 由 Wather 对象的实现派生解释
   */
  PtrArg m_argIn;
  /**
   * @var PtrArg m_argOut
   * @brief Wather 对象的输出
   *
   * 指针对象, 因此可以是任何类型的数据, 由 Wather 对象的实现派生解释
   */
  PtrArg m_argOut;
};

/**
 * @struct IoFd []
 * @brief IO 文件描述符对象
 */
struct IoFd {
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
   * @var PtrEvContainer m_pEvents
   * @brief io文件描述符的注册事件容器
   */
  PtrEvContainer m_pEvents;
  /**
 * @var PtrEvContainer m_pEvents
 * @brief io文件描述符的注册事件容器
 */
  FdState m_state;
};

typedef nsp_boost::shared_ptr<IoFd> PtrFD;

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
  PtrFD m_arrFds[1024];
};

/**
 * @class IoWather []
 * @brief Wather对象的一个实现,主要用于单元测试
 */
PUMP_IMPLEMENT
class IoWather
  : public Wather {
public:
  IoWather();
  
  IoWather(PtrCbMailboxMgr pMbMgr);
  
  virtual ~IoWather();

public:
  virtual void doWatching();
  void PostAccept(/* FIXME 需要决定参数 */);
  void PostConnect(/* FIXME 需要决定参数 */);
  void PostSend(/* FIXME 需要决定参数 */);
  void PostShutdown(/* FIXME 需要决定参数 */);
  void PostClose(/* FIXME 需要决定参数 */);

private:
  virtual void preProcess();
  
  virtual int dispatch();
  
  virtual void postProcess();

private:
  PtrPreEvContainer m_pPreEvents;
  PtrPostEvContainer m_pPostEvents;
};

}

#endif //LIBPUMP_WATHER_H
