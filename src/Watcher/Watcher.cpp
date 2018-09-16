/**
 * @file Watcher.cpp
 * @brief Watcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "Watcher.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace PUMP {

////////////////////////////////////////////////
//                   Watcher
////////////////////////////////////////////////


////////////////////////////////////////////////
//              CentralizedWatcher
////////////////////////////////////////////////

bool CentralizedWatcher::insert(ev_prior_t prior, PtrCbFn pfn) {
  return m_pMbMgr->insert(prior, pfn);
}

}