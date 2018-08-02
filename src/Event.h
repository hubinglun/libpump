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

#include "pumpdef.h"

namespace PUMP {

class Event {
private:
  //! 描述事件的优先级
  enum EventPriority m_evPriority;
  //! 事件的状态
  enum EventState m_evState;
  //! 事件的类型，事前、普通、事后
  enum EventType m_evType;
};

}

#endif //LIBPUMP_EVENT_H
