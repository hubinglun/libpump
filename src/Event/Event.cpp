/**
 * @file Event.cpp
 * @brief Event 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.29
 */
#include "Event.h"

namespace Pump {

////////////////////////////////////////////////
//                   Event
////////////////////////////////////////////////

Event::Event()
  : emEvPriority_(EVPRIOR_DEFAULT),
    emEvState_(EVSTATE_INIT),
    emEvType_(EVTYPE_NORM) {}

Event::Event(enum EventType emEvType)
  : emEvPriority_(EVPRIOR_DEFAULT),
    emEvState_(EVSTATE_INIT),
    emEvType_(emEvType) {}

////////////////////////////////////////////////
//                   PreEvent
////////////////////////////////////////////////

PreEvent::PreEvent()
  : Event(EVTYPE_PRE) {}

PreEvent::~PreEvent() {}

////////////////////////////////////////////////
//                   PostEvent
////////////////////////////////////////////////

PostEvent::PostEvent()
  : Event(EVTYPE_POST) {}

PostEvent::~PostEvent() {}

////////////////////////////////////////////////
//                   NormEvent
////////////////////////////////////////////////

NormEvent::NormEvent()
  : Event(EVTYPE_NORM) {}

NormEvent::~NormEvent() {}

////////////////////////////////////////////////
//                   FdEvent
////////////////////////////////////////////////

FdEvent::FdEvent() {}

FdEvent::~FdEvent() {}

////////////////////////////////////////////////
//                   IoEvent
////////////////////////////////////////////////

void IoEvent::init() {
}

////////////////////////////////////////////////
//                   TimerEvent
////////////////////////////////////////////////

void TimerEvent::init() {

}

////////////////////////////////////////////////
//                   SignalEvent
////////////////////////////////////////////////

void SignalEvent::init() {

}

}