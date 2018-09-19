/**
 * @file MultiplexBackend.h
 * @brief MultiplexBackend 相关对象的定义
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.09.02
 */

#ifndef LIBPUMP_MULTIPLEXBACKEND_H
#define LIBPUMP_MULTIPLEXBACKEND_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifdef linux
#include <sys/select.h>
#include <sys/time.h>
#endif // linux

#include <cstring>
#include <cerrno>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>
#include <stack>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include "pumpdef.h"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace Pump {

/**
 * @struct IoFdCtl []
 * @brief 描述 IoFd 变化的对象
 */
struct IoFdCtl {
  FdCtlTyge type_;
  pump_fd_t fd_;
  unsigned short fd_ev_;
  IoFdCtl()
    : type_(FD_CTL_DEFAULT)
    ,fd_(-1)
    ,fd_ev_(IO_EV_NONE){}
};

/**
 * @struct IoFdRet []
 * @brief IO 多路复用返回fd检测结果
 */
struct IoFdRet {
  /**
   * @var pump_fd_t fd_
   * @brief 涉及fd
   */
  pump_fd_t fd_;
  
  /**
   * @var unsigned short fd_ev_
   * @brief 发生事件, 必须是 IO_EV_XXX
   */
  unsigned short re_fd_ev_;
  IoFdRet()
    : fd_(-1)
    , re_fd_ev_(IO_EV_NONE){}
};

typedef nsp_std::vector<IoFdRet> IoFdRetList;

/**
 * @class MultiplexBackend
 * @brief IO 多路复用后端对象抽象类
 */
PUMP_ABSTRACT
class MultiplexBackend
  : public nsp_boost::noncopyable {
public:
  MultiplexBackend() {}
  
  virtual ~MultiplexBackend() {}
  
  /**
   * @fn virtual int init() = 0
   * @brief 初始化IO多路复用
   * @return 成功返回非负值
   */
  virtual int init() = 0;
  
  /**
   * @fn int update(const IoFdCtl & change) = 0
   * @brief 控制对文件描述符的监听行为
   * @param change [in] 需要更新的 fd 及事件
   * @return 成功返回非负值
   */
  virtual int update(const IoFdCtl &change) = 0;
  
  /**
   * @fn int wait(IoFdRetList &fdRetList, timeval & tmv) = 0
   * @brief 等待事件的产生, 内部调用 select() 等系统调用
   * @param fdRetList [out] 发生事件的 fd 及事件集合
   * @param tmv [in] io多路复用超时值
   * @return 成功返回参与io多路复用的fd数量, 失败返回-1
   */
  virtual int wait(IoFdRetList &fdRetList, timeval &tmv) = 0;
};

typedef nsp_boost::shared_ptr<MultiplexBackend> PtrMultiBackend;

/**
 * @class Select []
 * @brief select 模型封装
 */
PUMP_IMPLEMENT
class Select
  : public MultiplexBackend {
public:
  typedef struct {
    fd_set fdRead;
    fd_set fdWrite;
    fd_set fdError;
  } fd_sets;
public:
  Select();
  
  ~Select();
  
  virtual int init();
  
  virtual int update(const IoFdCtl &change);
  
  virtual int wait(IoFdRetList &fdRetList, timeval &tmv);

private:
  /**
   * @fn void recover()
   * @brief 从 m_setBackup 恢复 m_setWork
   */
  void recover();
  
  /**
   * @fn pump_fd_t max()
   * @brief 获取 m_fds 中最大的 fd
   * @return maximum fd
   */
  pump_fd_t max();

private:
  /**
   * @var fd_sets m_setWork
   * @brief 工作文件描述符集合组
   */
  fd_sets m_setWork;
  /**
   * @var fd_sets m_setBackup
   * @brief 备份文件描述符集合组
   */
  fd_sets m_setBackup;
  /**
   * @var nsp_std::set<pump_fd_t> m_fds
   * @brief 存放所有监听fd
   */
  nsp_std::set<pump_fd_t> m_fds;
};

PUMP_IMPLEMENT
class Poll
  : public MultiplexBackend {
  
};

PUMP_IMPLEMENT
class Epoll
  : public MultiplexBackend {
  
};

}

#endif //LIBPUMP_MULTIPLEXBACKEND_H
