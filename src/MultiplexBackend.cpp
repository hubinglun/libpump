//
// Created by yz on 18-8-14.
//

#include "MultiplexBackend.h"

namespace PUMP {

Select::Select() {
  init();
}

Select::~Select() {

}

int Select::init() {
  FD_ZERO(&m_setWork.fdRead);
  FD_ZERO(&m_setWork.fdWrite);
  FD_ZERO(&m_setWork.fdError);
  FD_ZERO(&m_setBackup.fdRead);
  FD_ZERO(&m_setBackup.fdWrite);
  FD_ZERO(&m_setBackup.fdError);
}

int Select::update(const IoFdCtl &change) {
  switch (change.type_) {
    case FD_CTL_ADD: {
      if (m_fds.find(change.fd_) != m_fds.end()) {
        return -1;
      }
      if ((change.fd_ev_ & IO_EV_IN) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdRead);
      }
      if ((change.fd_ev_ & IO_EV_OUT) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdWrite);
      }
      if ((change.fd_ev_ & IO_EV_ERR) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdError);
      }
      // 添加到监听fd集合
      m_fds.insert(change.fd_);
      break;
    }
    case FD_CTL_DEL: {
      if (m_fds.find(change.fd_) == m_fds.end()) {
        return -1;
      }
      FD_CLR(change.fd_, &m_setBackup.fdRead);
      FD_CLR(change.fd_, &m_setBackup.fdWrite);
      FD_CLR(change.fd_, &m_setBackup.fdError);
      // 从监听fd集合删除
      m_fds.erase(change.fd_);
      break;
    }
    case FD_CTL_MOD: {
      if (change.fd_ev_ == 0
          || (m_fds.find(change.fd_) == m_fds.end())) {
        return -1;
      }
      
      // 先清除 change.fd_
      FD_CLR(change.fd_, &m_setBackup.fdRead);
      FD_CLR(change.fd_, &m_setBackup.fdWrite);
      FD_CLR(change.fd_, &m_setBackup.fdError);
      // 重新按照参数设置
      if ((change.fd_ev_ & IO_EV_IN) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdRead);
      }
      if ((change.fd_ev_ & IO_EV_OUT) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdWrite);
      }
      if ((change.fd_ev_ & IO_EV_ERR) != 0) {
        FD_SET(change.fd_, &m_setBackup.fdError);
      }
      break;
    }
    default: {
      break;
    }
  }
  return 0;
}

pump_fd_t Select::max() {
  return *(nsp_std::max_element(m_fds.begin(), m_fds.end()));
}

int Select::wait(IoFdRetList &fdRetList, timeval &tmv) {
  // 监听fd集合为空, 直接返回
  if (m_fds.empty()) {
    return 0;
  }
  
  // 使用备份fd_set恢复工作fd_set
  recover();
  // 调用 ::select()
  int ret = ::select(max() + 1,
                     &m_setWork.fdRead,
                     &m_setWork.fdWrite,
                     &m_setWork.fdError,
                     &tmv);
  if (ret == 0) { // select() 超时返回
    // TODO 超时处理
    return 0;
  } else if (ret == -1) { // 函数调用出错
    // FIXME 应该使用日志库
#ifdef _TEST_LEVEL_INFO
    nsp_std::cout << "[Error]: " << strerror(errno) << nsp_std::endl;
#endif // _TEST_LEVEL_INFO
    // TODO 错误处理
    return -1;
  } else {
    for (nsp_std::set<pump_fd_t>::iterator it = m_fds.begin();
         it != m_fds.end(); ++it) { // 遍历所有监听套接字
      unsigned short re_ev = 0;
      if (FD_ISSET(*it, &m_setWork.fdRead)) { // fd 有读事件
        re_ev |= IO_EV_IN;
      }
      if (FD_ISSET(*it, &m_setWork.fdWrite)) { // fd 有写事件
        re_ev |= IO_EV_OUT;
      }
      if (FD_ISSET(*it, &m_setWork.fdError)) { // fd 异常事件
        re_ev |= IO_EV_ERR;
      }
      
      if (re_ev == 0) { // 当前fd上未产生任何io事件
        continue;
      } else { // fd 有事件发生
        IoFdRet ioFdRet;
        ioFdRet.fd_ = *it;
        ioFdRet.re_fd_ev_ = re_ev;
        // 放入 fd 返回集合
        fdRetList.push_back(ioFdRet);
      }
    }
  }
  return static_cast<int>(m_fds.size());
}

void Select::recover() {
  FD_ZERO(&m_setWork.fdRead);
  FD_ZERO(&m_setWork.fdWrite);
  FD_ZERO(&m_setWork.fdError);
  ::memcpy(&m_setWork, &m_setBackup, sizeof(m_setBackup));
}

}