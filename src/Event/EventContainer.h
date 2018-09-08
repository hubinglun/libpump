//
// Created by yz on 18-9-8.
//

#ifndef LIBPUMP_EVENTCONTAINER_H
#define LIBPUMP_EVENTCONTAINER_H

#include "Event.h"

#include <boost/scoped_ptr.hpp>

namespace nsp_boost = ::boost;

namespace PUMP {

/**
 * @class EventContainer
 * @brief Event 事件容器
 *
 * Watcher 对象中存放注册 Event 事件的容器
 *
 * FIXME 事件容器设计的不太好, 比如增删改接口太过简单
 */
PUMP_ABSTRACT
class EventContainer
  : public nsp_boost::noncopyable {
public:
  EventContainer() {}
  
  virtual ~EventContainer() {}
  
  virtual size_t runAll() = 0;
  
  virtual void insert(nsp_std::string &strName, PtrEvent pEv) = 0;
  
  virtual void erase(nsp_std::string &strName) = 0;
  
  virtual PtrEvent at(nsp_std::string &strName) = 0;
};

typedef nsp_boost::shared_ptr<EventContainer> PtrEvContainer;
typedef nsp_boost::scoped_ptr<EventContainer> SPtrEvContainer;

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

protected:
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
 * @class EvList []
 * @brief EventContainer 实现类, 存放Event对象的map
 */
PUMP_IMPLEMENT
class EvList
  : public EventContainer {
public:
  EvList() {}
  
  ~EvList() {}
  
  virtual void insert(nsp_std::string &strName, PtrEvent pEv);
  
  virtual void erase(nsp_std::string &strName);
  
  virtual PtrEvent at(nsp_std::string &strName);

private:
  virtual size_t runAll() { return 0; }

private:
  nsp_std::map<nsp_std::string, PtrEvent> m_lEvs;
};

typedef nsp_boost::shared_ptr<EvList> PtrEvList;

PUMP_IMPLEMENT
class PrePostList
  : public PrePostEvContainer {
public:
  PrePostList();
  
  virtual ~PrePostList();
  
  virtual size_t runAll();
};

}

#endif //LIBPUMP_EVENTCONTAINER_H
