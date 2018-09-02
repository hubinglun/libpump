/**
 * @file Pump.h
 * @brief Pump 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#pragma once

#ifndef LIBPUMP_PUMP_H
#define LIBPUMP_PUMP_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "pumpdef.h"
#include "Logger.h"
#include "Watcher.h"
#include "CbMailbox.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @namespace namespace PUMP
 * @brief libpump 的顶级命名空间
 */
namespace PUMP {

/** Watcher 内存托管 */
typedef nsp_boost::shared_ptr<Watcher> PtrWatcher;
/** Watcher 内存托管 */
typedef nsp_boost::weak_ptr<Watcher> WPtrWatcher;

/**
 * @class WatcherContainer []
 * @brief Watcher 对象的容器抽象对象
 *
 * 用户必须派生 WatcherContainer 对象实现接口, 并实现存储 Watcher 的对象.
 * Watcher 存储逻辑结构可以使任意的
 *
 * > WARNING 现阶段不支持Watcher动态创建
 */
PUMP_ABSTRACT
class WatcherContainer
  : virtual public nsp_boost::noncopyable {
public:
  WatcherContainer() {}
  
  virtual ~WatcherContainer() {}
  
  /**
   * @fn virtual size_t init() = 0
   * @brief 初始化容器对象
   * @return 成功构造的容器个数
   */
  virtual size_t init() = 0;
  
  /**
   * @fn virtual PtrWatcher begin() = 0
   * @brief 获取容器中第一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher begin() = 0;
  
  /**
   * @fn virtual PtrWatcher end() = 0
   * @brief 获取容器中最后一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher end() = 0;
  
  /**
   * @fn virtual PtrWatcher next() = 0
   * @brief 获取容器中当前Watcher指针的下一个
   * @return　Watcher指针
   */
  virtual PtrWatcher next() = 0;
  
  /**
   * @fn virtual bool empty() = 0
   * @brief 判断容器是否为空
   * @return　数量
   */
  virtual bool empty() = 0;
  
  /**
   * @fn virtual size_t count() = 0
   * @brief Watcher 对象的数量
   * @return　数量
   */
  virtual size_t count() = 0;
};

/**
 * @typedef typedef nsp_boost::shared_ptr<WatcherContainer> PtrWatcherContainer
 * @brief WatcherContainer 内存托管对象
 */
typedef nsp_boost::shared_ptr<WatcherContainer> PtrWatcherContainer;

/**
 * @class WatcherList []
 * @brief 以 std::vector<PtrWatcher> 存储Watcher的容器对象
 *
 */
PUMP_IMPLEMENT
class WatcherList
  : public WatcherContainer {
  /* FIXME 缺少初始化 Watcher 对象的方法*/
public:
  WatcherList();
  
  virtual ~WatcherList();
  
  /**
   * @fn virtual size_t init()
   * @brief 初始化容器对象
   * @return 成功构造的容器个数
   */
  virtual size_t init();

#ifdef _TEST_LEVEL_INFO
  /**
   * @brief
   * @tparam T
   * @return
   */
  template <class T>
  void test_createWatcher();
#endif // _TEST_LEVEL_INFO
  
  /**
   * @fn virtual PtrWatcher begin()
   * @brief 获取容器中第一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher begin();
  
  /**
   * @fn virtual PtrWatcher end()
   * @brief 获取容器中最后一个Watcher指针
   * @return　Watcher指针
   */
  virtual PtrWatcher end();
  
  /**
   * @fn virtual PtrWatcher next()
   * @brief 获取容器中当前Watcher指针的下一个
   * @return　Watcher指针
   */
  virtual PtrWatcher next();
  
  /**
   * @fn virtual bool empty()
   * @brief 判断容器是否为空
   * @return　数量
   */
  virtual bool empty();
  
  /**
   * @fn virtual size_t count()
   * @brief Watcher 对象的数量
   * @return　数量
   */
  virtual size_t count();

private:
  nsp_std::vector<PtrWatcher> m_vecWatcher;
};

/**
 * @class Pump [Pump.cpp]
 * @brief Pump 对象名字源于强健有力的“泵”, 正如“泵”的将机械能或其他外部能量传送给液体那样,
 * Pump 对象是整个事件循环框架的动力源泉, 它负责驱动 Watcher 对象监听、激活、处理
 * 事件对象
 */
PUMP_ABSTRACT
class Pump
  : virtual public nsp_boost::noncopyable {
  // FIXME 应该增加参数配置功能
public:
  Pump() {}
  
  explicit Pump(PtrCbMailboxEvoker pMbEvoker);
  
  virtual ~Pump() {}
  
  /**
   * @fn \a virtual void init() = 0
   * @brief 初始化Pump对象，实际上按照配置初始化WatcherList
   */
  virtual void init() = 0;
  
  /**
   * @fn \a virtual void start() = 0
   * @brief 启动 Pump 服务
   */
  virtual void start() = 0;
  
  /**
   * @fn \a virtual void pause() = 0
   * @brief 暂停Pump循环，挂起
   */
  virtual void pause() = 0;
  
  /**
   * @fn \a virtual void stop() = 0
   * @brief 终结Pump循环，需要清理资源，释放fd等
   */
  virtual void stop() = 0;

protected:
  //! 标志 Pump 当前的运行状态
  enum PumpState m_state;
  /**
   * @var WPtrCbMailboxEvoker m_wpMbEvoker
   * @brief 回调函数邮箱函数执行执行接口对象
   *
   * * CbMailboxEvoker 是接口类, 其指针对象必须由 Pump 类的派生类 \
   * 指向一个 CbMailboxEvoker 的实现类
   */
  PtrCbMailboxEvoker m_pMbEvoker;
  /**
   * @var PtrWatcherContainer m_pWatchers
   * @brief 存放 Watcher 对象指针的数组对象
   *
   * * WatcherContainer 是存放 Watcher 对象的容器, 其指针对象必须由 Pump 类的派生类 \
   * 指向一个 WatcherContainer 的实现类
   */
  PtrWatcherContainer m_pWatchers;
};

/**
 * @class IPump []
 * @brief Pump 对象的实现类, 主要用于测试 Pump 对象的功能
 */
PUMP_IMPLEMENT
class IPump
  : public Pump {
public:
  typedef nsp_boost::shared_ptr<WatcherList> PtrRealWContainer;
public:
  IPump();
  
  explicit IPump(PtrCbMailboxEvoker pMbEvoker);
  
  ~IPump();
  
  /** 初始化Pump对象，实际上按照配置初始化WatcherList */
  virtual void init();
  
  /** 启动 Pump 服务 */
  virtual void start();
  
  /** 暂停Pump循环，挂起 */
  virtual void pause();
  
  /** 终结Pump循环，需要清理资源，释放fd等 */
  virtual void stop();

#ifdef _TEST_LEVEL_INFO
  /**
   * @fn void test_initWatcherList()
   * @brief 手动初始化 WatcherList
   */
  void test_initWatcherList();
#endif // _TEST_LEVEL_INFO

private:
  /**
   * @brief watching() 前置处理
   */
  virtual size_t preWatching();
  
  /**
   * @brief watching()
   *
   * 遍历 m_watchers 数组, 逐个调用 Watcher 对象的三个阶段函数
   */
  virtual size_t watching();
  
  /**
   * @brief watching()前置处理
   */
  virtual size_t postWatching();

#ifdef _TEST_LEVEL_INFO
public:
  size_t test_MbEvoker_runAll() {
    PtrICbMailboxEvoker t_pMailEvoker = nsp_boost::dynamic_pointer_cast<ICbMailboxEvoker>(m_pMbEvoker);
    if (t_pMailEvoker == NULL) {
      return 0;
    }
    return t_pMailEvoker->runAll();
  }

#endif // _TEST_LEVEL_INFO

};

}

#endif //LIBPUMP_PUMP_H
