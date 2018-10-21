/**
 * @file PWatcher.h
 * @brief PWatcher 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */

#ifndef LIBPUMP_WATHER_H
#define LIBPUMP_WATHER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cassert>

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include "pumpdef.h"
#include "Logger.h"
#include "CbMailbox.h"
#include "../Pump.h"
#include "Event/Event.h"
#include "Event/EventContainer.h"

namespace nsp_boost = ::boost;

namespace Pump {

/**
 * @class PWatcher []
 * @brief 事件观察者对象
 *
 * 注册, 监听, 激活, 处理(投递)事件
 */
PUMP_ABSTRACT
class PWatcher
  : public AsyncPump {
public:
  PWatcher() {}

#ifdef _TEST_LEVEL_DEBUG
  
  PWatcher(PtrArg pIn, PtrArg pOut);

#endif // _TEST_LEVEL_DEBUG
  
  virtual ~PWatcher() {}

protected:
  virtual int preWatch() = 0;
  
  virtual int watch() = 0;
  
  virtual int postWatch() = 0;

protected:
  /**
   * @var PtrEvContainer m_pEvents
   * @brief Event 容器, 可向其中注册 Event
   */
  PtrEvContainer m_pEvents;
  /**
   * @var PtrPreEvContainer m_pPreEvents
   * @brief PreEvent 容器, 可向其中注册 PreEvent
   */
  PtrPreEvContainer m_pPreEvents;
  /**
   * @var PtrPostEvContainer m_pPostEvents
   * @brief PostEvent 容器, 可向其中注册 PostEvent
   */
  PtrPostEvContainer m_pPostEvents;
};

/** PWatcher 内存托管 */
typedef nsp_boost::shared_ptr<PWatcher> PtrWatcher;

/** PWatcher 内存托管 */
typedef nsp_boost::weak_ptr<PWatcher> WPtrWatcher;

/**
 * @class PWCentralized []
 * @brief 集中式事件检测器, 向 "回调函数邮箱" 投递事件回调, 集中处理
 */
PUMP_ABSTRACT
class PWCentralized
  : public PWatcher {
public:
  PWCentralized() {}

#ifdef _TEST_LEVEL_DEBUG
  
  PWCentralized(PtrArg pIn, PtrArg pOut, PtrCbMailboxMgr pMbMgr);

#endif // _TEST_LEVEL_DEBUG
  
  virtual ~PWCentralized() {}
  
  bool insert(ev_prior_t prior, PtrCbFn pfn);

protected:
  /**
   * @var PtrCbMailboxMgr m_pMbMgr
   * @brief CbMailbox 管理对象, 可以向邮箱增删回调对象, 非所有者
   */
  PtrCbMailboxMgr m_pMbMgr;
//  PostEvent m_PstEvPostCb;
};

}

#endif //LIBPUMP_WATHER_H
