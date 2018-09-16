/**
 * @file PWatcher.cpp
 * @brief PWatcher 相关对象的实现
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#include "PWatcher.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace PUMP {

////////////////////////////////////////////////
//                   PWatcher
////////////////////////////////////////////////

#ifdef _TEST_LEVEL_INFO

PWatcher::PWatcher(PtrArg pIn, PtrArg pOut)
  : AsyncPump(pIn, pOut) {}

#endif // _TEST_LEVEL_INFO

////////////////////////////////////////////////
//              PWCentralized
////////////////////////////////////////////////

PWCentralized::PWCentralized(PtrArg pIn,
                             PtrArg pOut,
                             PtrCbMailboxMgr pMbMgr)
  : PWatcher(pIn, pOut),
    m_pMbMgr(pMbMgr) {}

bool PWCentralized::insert(ev_prior_t prior, PtrCbFn pfn) {
  return m_pMbMgr->insert(prior, pfn);
}

}