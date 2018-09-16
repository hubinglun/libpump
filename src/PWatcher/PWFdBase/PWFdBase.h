/**
 * @file FdBaseWatcher.h
 * @brief FdBaseWatcher 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.09
 */

#ifndef LIBPUMP_FDBASEWATCHER_H
#define LIBPUMP_FDBASEWATCHER_H

#include <vector>
#include <map>

#ifdef linux
#include <unistd.h>
#include <sys/socket.h>
#endif // linux

#include "PWatcher/PWatcher.h"
#include "FdDef.h"
#include "Event/Event.h"
#include "Event/EventContainer.h"
#include "MultiplexBackend.h"
#include "Buffer/Buffer.h"
#include "Logger.h"

namespace PUMP {

/**
 * @class FdBaseWatcher []
 * @brief 用于监听文件描述符的事件检测器
 */
PUMP_ABSTRACT
class PWFdBase
  : public PWCentralized {
public:
  PWFdBase() {}

#ifdef _TEST_LEVEL_INFO
  
  PWFdBase(PtrArg pIn, PtrArg pOut, PtrCbMailboxMgr pMbMgr);

#endif //_TEST_LEVEL_INFO
  
  virtual ~PWFdBase() {}
  
  // FIXME 此处TcpSockService设置为友元主要为解决m_pFds访问权限问题, 感觉不太安全
  friend class TcpSockService;

protected:
  PtrFdContainer m_pFds;
  PtrMultiBackend m_pBackend;
};

}

#endif //LIBPUMP_FDBASEWATCHER_H
