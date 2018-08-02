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

Pump::Pump(WPtrICbMailboxEvoker pMbEvoker)
  : m_wpMbEvoker(pMbEvoker) {
}

Pump::~Pump() {}

int Pump::preWatching() {
  // 暂时不知道干啥, 但是肯定是与Watcher对象相关的预处理
  return 0;
}

int Pump::watching() {
  for (WatcherList::iterator it = m_watchers.begin();
       it != m_watchers.end();
       ++it) {
    (*it)->doWatching();
  }
  return 0;
}

int Pump::postWatching() {
  // 目前 postWatching 仅安排执行回调
  PtrICbMailboxEvoker t_pMailEvoker = m_wpMbEvoker.lock();
  if (t_pMailEvoker == NULL) {
    return 0;
  }
  size_t n = t_pMailEvoker->runAll();
  return n;
}

void Pump::init() {

}

void Pump::start() {
  // 暂时就这么简陋
  while (m_state == PUMPSTATE_START) {
    preWatching();
    watching();
    postWatching();
  }
}


void Pump::pause() {
  m_state = PUMPSTATE_PAUSE;
}

void Pump::stop() {
  m_state = PUMPSTATE_STOP;
}

}
