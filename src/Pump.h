/**
 * @file Pump.h
 * @brief Pump 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */

#ifndef LIBPUMP_PUMP_H
#define LIBPUMP_PUMP_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>

#include "pumpdef.h"
#include "Logger.h"
#include "CbMailbox.h"
#include "Watcher/Watcher.h"
#include "Watcher/WatcherContainer.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @namespace namespace PUMP
 * @brief libpump 的顶级命名空间
 */
namespace PUMP {

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
