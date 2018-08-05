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
  
};

typedef nsp_boost::shared_ptr<EventContainer> PtrEvContainer;

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
  virtual void doWatching() = 0;
  
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
 * @class IWather []
 * @brief Wather对象的一个实现,主要用于单元测试
 */
PUMP_IMPLEMENT
class IWather
  : public Wather {
public:
  IWather();
  
  IWather(PtrCbMailboxMgr pMbMgr);
  
  virtual ~IWather();
  
public:
  virtual void doWatching();

private:
  virtual void preProcess();
  
  virtual int dispatch();
  
  virtual void postProcess();
};

}

#endif //LIBPUMP_WATHER_H
