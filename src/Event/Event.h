/**
 * @file Event.h
 * @brief Event 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.29
 */
#pragma once

#ifndef LIBPUMP_EVENT_H
#define LIBPUMP_EVENT_H

#include <string>
#include "CbMailbox.h"

#include <boost/noncopyable.hpp>

#include "pumpdef.h"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace PUMP {

/**
 * @struct Event []
 * @brief 事件对象的抽象类
 */
PUMP_ABSTRACT
struct Event
  : public nsp_boost::noncopyable {
  //! < 事件命
  nsp_std::string strName_;
  //! < 描述事件的优先级
  enum EventPriority emEvPriority_;
  //! < 事件的状态
  enum EventState emEvState_;
  /**
   * @var const enum EventType m_emEvType
   * @brief 事件的类型: 前置, 普通, 后置
   */
  const enum EventType emEvType_;
  /**
   * @var PtrCbFn m_pPreEvCallback
   * @brief 指向前置事件回调
   *
   * 由Event事件派生对象的构造者指定
   */
  PtrCbFn pPreEvCallback_;
  /**
   * @var PtrCbFn m_pEvCallback
   * @brief 指向事件回调
   *
   * 由Event事件派生对象的构造者指定
   */
  PtrCbFn pEvCallback_;
  /**
   * @var PtrCbFn m_pPostEvCallback
   * @brief 指向后置事件回调
   *
   * 由Event事件派生对象的构造者指定
   */
  PtrCbFn pPostEvCallback_;
  /**
   * @var PtrArg m_pData
   * @brief 指向用户数据
   *
   * 由Event事件派生对象的构造者指定
   */
  PtrArg pData_;
public:
  Event();
  
  Event(enum EventType emEvType);
  
  virtual ~Event() {}
};

/**
 * @typedef typedef nsp_boost::shared_ptr<Event> PtrEvent
 * @brief Event 的内存托管
 */
typedef nsp_boost::shared_ptr<Event> PtrEvent;

/**
* @struct PreEvent []
* @brief 前置事件
*/
PUMP_ABSTRACT
struct PreEvent
  : public Event {
  PreEvent();
  
  virtual ~PreEvent();
};

/**
 * @struct PostEvent []
 * @brief
 */
PUMP_ABSTRACT
struct PostEvent
  : public Event {
  PostEvent();
  
  virtual ~PostEvent();
};

/**
 * @struct NormEvent []
 * @brief
 */
PUMP_ABSTRACT
struct NormEvent
  : public Event {
  NormEvent();
  
  virtual ~NormEvent();
};

/**
 * @struct FdEvent []
 * @brief
 */
PUMP_ABSTRACT
struct FdEvent
  : public NormEvent {
  pump_fd_t fd_;
  
  FdEvent();
  
  virtual ~FdEvent();
  
  /**
   * @fn virtual void init() = 0
   * @brief 初始化 fd_ 成员
   */
  virtual void init() = 0;
};

PUMP_IMPLEMENT
struct IoEvent
  : public FdEvent {
  virtual void init();
};

typedef nsp_boost::shared_ptr<IoEvent> PtrIoEvent;

struct Timestamp {

};

struct TimerEvent
  : public FdEvent {
  Timestamp m_tsInit;
  Timestamp m_tsTimeout;
  TimerType m_enTpye;
  
  virtual void init();
};

struct SignalEvent
  : public FdEvent {
  virtual void init();
};

}

#endif //LIBPUMP_EVENT_H
