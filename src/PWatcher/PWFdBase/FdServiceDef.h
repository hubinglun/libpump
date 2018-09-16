/**
 * @file FdServiceDef.h
 * @brief FdService 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.09
 */

#ifndef LIBPUMP_FDSERVICEDEF_H
#define LIBPUMP_FDSERVICEDEF_H

#include <boost/shared_ptr.hpp>

#include "FdDef.h"
#include "../Service.h"

namespace nsp_boost = ::boost;

namespace PUMP {

class PWIo;
class FdBase;
typedef nsp_boost::shared_ptr<FdBase> PtrFD;

/**
 * @class NetService
 * @brief 与 IoFd 绑定的服务对象
 */
PUMP_INTERFACE
class FdService
  : public Service {
public:
  typedef int(*pFdCb)(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData);

public:
  FdService()
    : m_pCbIN(NULL), m_pCbOUT(NULL)
#ifdef _GNU_SOURCE
    , m_pCbRDHUP(NULL)
#endif // _GNU_SOURCE
    , m_pCbERR(NULL), m_pCbHUP(NULL), m_pCbNVAL(NULL)
#ifdef _XOPEN_SOURCE
    , m_pCbRDBAND(NULL), m_pCbWRBAND(NULL)
#endif //_XOPEN_SOURCE
  {}
  
  virtual ~FdService() {}
  
  pFdCb m_pCbIN;
  pFdCb m_pCbOUT;
#ifdef _GNU_SOURCE
  pFdCb m_pCbRDHUP;
#endif // _GNU_SOURCE
  pFdCb m_pCbERR;
  pFdCb m_pCbHUP;
  pFdCb m_pCbNVAL;
#ifdef _XOPEN_SOURCE
  pFdCb m_pCbRDBAND;
  pFdCb m_pCbWRBAND;
#endif //_XOPEN_SOURCE
};

typedef nsp_boost::shared_ptr<FdService> PtrFdService;

/**
 * @class NetService
 * @brief 与 Socket 绑定的Tcp层服务对象, 用户派生此对象实现接口已完成规定任务
 */
PUMP_INTERFACE
class NetService
  : public Service {
public:
  typedef int(*pFdCb)(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData);

public:
  NetService() {}
  
  virtual ~NetService() {}
  
  virtual int acceptCb(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid()) = 0;
  
  virtual int sendCb(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid()) = 0;
  
  virtual int recvCb(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid()) = 0;
  
  virtual int closeCb(PWIo &rIoWatcher, PtrFD pFd, PtrVoid pData = PtrVoid()) = 0;
  
  /*FIXME 需要修改, 取消纯虚函数改用成员函数指针*/
  pFdCb m_pCbAccept;
  pFdCb m_pCbSend;
  pFdCb m_pCbRecv;
  pFdCb m_pCbClose;
};

typedef nsp_boost::shared_ptr<NetService> PtrTcpService;

}

#endif //LIBPUMP_FDSERVICEDEF_H
