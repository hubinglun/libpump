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
#include "PumpContainer.h"
#include "Thread.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @namespace namespace Pump
 * @brief libpump 的顶级命名空间
 */
namespace Pump {

class Pump;

typedef nsp_boost::shared_ptr<Pump> PtrPump;

/**
 * @class Pump [Pump.cpp]
 * @brief Pump 对象名字源于强健有力的“泵”, 正如“泵”的将机械能或其他外部能量传送给液体那样,
 * Pump 对象是整个事件循环框架的动力源泉, 它负责驱动 PWatcher 对象监听、激活、处理
 * 事件对象
 */
PUMP_ABSTRACT
class Pump
  : virtual public nsp_boost::noncopyable {
  // FIXME 应该增加参数配置功能
public:
  Pump();

#ifdef _TEST_LEVEL_DEBUG
  
  Pump(PtrArg pIn, PtrArg pOut, PumpType emType);

#endif //_TEST_LEVEL_DEBUG
  
  virtual ~Pump() {}
  
  /**
   * @fn \a virtual void init() = 0
   * @brief 初始化Pump对象，实际上按照配置初始化WatcherList
   */
  virtual void init();
  
  /**
   * @fn \a virtual void start() = 0
   * @brief 启动 Pump 服务
   */
  virtual int start();
  
  /**
   * @fn \a virtual void pause() = 0
   * @brief 暂停Pump循环，挂起
   */
  virtual int pause();
  
  /**
   * @fn \a virtual void stop() = 0
   * @brief 终结Pump循环，需要清理资源，释放fd等
   */
  virtual int stop();
  
  void setArgIn(PtrArg pArgIn);
  
  PtrArg getArgOut();

protected:
  /**
   * @fn virtual void routine() = 0
   * @brief 当Pump对象需要作为一个独立线程的服务时, 这个函数作为线程的回调函数
   *
   */
  virtual void routine() = 0;
  
  virtual int routine_core() = 0;

protected:
  //! < PWatcher 对象名
  nsp_std::string m_strName;
  //! 标志 Pump 当前的运行状态
  enum PumpState m_emState;
  const enum PumpType m_emType;
  PtrThread m_pThread;
  /**
   * @var PtrArg m_argIn
   * @brief PWatcher 对象的输入参数
   *
   * 指针对象, 因此可以是任何类型的数据, 由 PWatcher 对象的实现派生解释
   */
  PtrArg m_pArgIn;
  /**
   * @var PtrArg m_argOut
   * @brief PWatcher 对象的输出
   *
   * 指针对象, 因此可以是任何类型的数据, 由 PWatcher 对象的实现派生解释
   */
  PtrArg m_pArgOut;
};

class AsyncPump
  : public Pump {

public:
  AsyncPump();
  
#ifdef _TEST_LEVEL_DEBUG
  
  AsyncPump(PtrArg pIn, PtrArg pOut);

#endif //_TEST_LEVEL_DEBUG
  
  virtual ~AsyncPump();

protected:
  virtual void routine() = 0;

protected:
  PtrThread m_pThread;
};

/**
 * @class PWitness []
 * @brief Pump 对象的实现类, 主要用于测试 Pump 对象的功能
 */
PUMP_IMPLEMENT
class PWitness
  : public AsyncPump {
public:
  PUMP_IMPLEMENT
  class PWitnessThread
    : public PThread {
  public:
    PWitnessThread() {}
    ~PWitnessThread() {}
  };
public:
  
  PWitness();

#ifdef _TEST_LEVEL_DEBUG
  
  PWitness(PtrArg pIn, PtrArg pOut);

#endif //_TEST_LEVEL_DEBUG
  
  ~PWitness();
  
  /** 初始化Pump对象，实际上按照配置初始化WatcherList */
  virtual void init();
  
  /** 启动 Pump 服务 */
  virtual int start();
  
  /** 暂停Pump循环，挂起 */
  virtual int pause();
  
  /** 终结Pump循环，需要清理资源，释放fd等 */
  virtual int stop();

public:
  
  virtual void routine();
  
  virtual int routine_core();
  
  void join();

private:
  /**
   * @brief process() 前置处理
   */
  virtual int preWatch();
  
  /**
   * @brief process()
   *
   * 遍历 m_watchers 数组, 逐个调用 PWatcher 对象的三个阶段函数
   */
  virtual int process();
  
  /**
   * @brief process()前置处理
   */
  virtual int postWatch();
};

}

#endif //LIBPUMP_PUMP_H
