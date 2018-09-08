/**
 * @file Pump.cpp
 * @brief Pump 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Pump.h"

namespace PUMP {

////////////////////////////////////////////////
//                   Pump
////////////////////////////////////////////////

Pump::Pump(PtrCbMailboxEvoker pMbEvoker)
  : m_state(PUMPSTATE_NEW),
    m_pMbEvoker(pMbEvoker),
    m_pWatchers(nsp_boost::make_shared<WatcherList>()) {
  
}

////////////////////////////////////////////////
//                   IPump
////////////////////////////////////////////////

IPump::IPump(PtrCbMailboxEvoker pMbEvoker)
  : Pump(pMbEvoker) {}

IPump::~IPump() {}

size_t IPump::preWatching() {
  // 暂时不知道干啥, 但是肯定是与Watcher对象相关的预处理
  return 0;
}

size_t IPump::watching() {
  
  if (m_pWatchers->empty()) {
    // FIXME 应该设置错误码 error !!!
    return 0;
  }
  for (PtrWatcher t_wpW = m_pWatchers->begin();
       t_wpW != NULL && t_wpW != m_pWatchers->end();
       t_wpW = m_pWatchers->next()) {
    // 执行 Watcher 对象的"观察"操作
    t_wpW->doWatching();
  }
  return 0;
}

size_t IPump::postWatching() {
  // 目前 postWatching 仅安排执行回调
  PtrICbMailboxEvoker t_pMailEvoker = nsp_boost::dynamic_pointer_cast<ICbMailboxEvoker>(m_pMbEvoker);
  if (t_pMailEvoker == NULL) {
    // FIXME 应该设置错误码 error !!!
    return 0;
  }
  size_t n = t_pMailEvoker->runAll();
  return n;
}

void IPump::init() {

}

void IPump::start() {
  // 暂时就这么简陋
  while (m_state == PUMPSTATE_START) {
    preWatching();
    watching();
    postWatching();
  }
}

void IPump::pause() {
  m_state = PUMPSTATE_PAUSE;
}

void IPump::stop() {
  m_state = PUMPSTATE_STOP;
}

#ifdef _TEST_LEVEL_INFO

void IPump::test_initWatcherList() {
  PtrRealWContainer t_pWl = nsp_boost::dynamic_pointer_cast<WatcherList>(m_pWatchers);
  // FIXME 需要先实现 Watcher
//  t_pWl->test_createWatcher<>();
}

#endif // _TEST_LEVEL_INFO

}
