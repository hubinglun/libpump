//
// Created by yz on 18-9-2.
//

#include <boost/shared_ptr.hpp>
#include "CbMailbox.h"
#include "Watcher/IoWatcher.h"
#include "Logger.h"

using namespace PUMP;

/**
 * @class OnAccept
 * @brief 接收到新连接请求时回调对象
 */
class OnAccept
  : public CbFnWithoutReturn {
  typedef nsp_boost::function<void(pump_fd_t fd)/*FIXME 确定参数列表*/> func_t;
public:
  OnAccept() {}
  
  void operator()() {
    m_fn(m_arg1);
  }
  
  func_t m_fn;
  pump_fd_t m_arg1;
};

typedef nsp_boost::shared_ptr<OnAccept> PfnOnAccept;

/**
 * @class OnRecv
 * @brief 接收到新数据时回调对象
 */
class OnRecv
  : public CbFnWithoutReturn {
  typedef nsp_boost::function<void(void)/*FIXME 确定参数列表*/> func_t;
public:
  OnRecv() {}
  
  void operator()() {
    m_fn();
  }
  
  func_t m_fn;
};

typedef nsp_boost::shared_ptr<OnRecv> PfnOnRecv;

/**
 * @class OnSend
 * @brief 成功发送数据后回调对象
 */
class OnSend
  : public CbFnWithoutReturn {
  typedef nsp_boost::function<void(void)/*FIXME 确定参数列表*/> func_t;
public:
  OnSend() {}
  
  void operator()() {
    m_fn();
  }
  
  func_t m_fn;
};

typedef nsp_boost::shared_ptr<OnSend> PfnOnSend;

/**
 * @class OnSend
 * @brief 成功发送数据后回调对象
 */
class OnClose
  : public CbFnWithoutReturn {
  typedef nsp_boost::function<void(void)/*FIXME 确定参数列表*/> func_t;
public:
  OnClose() {}
  
  void operator()() {
    m_fn();
  }
  
  func_t m_fn;
};

typedef nsp_boost::shared_ptr<OnClose> PfnOnClose;

class TcpTest
  : public TcpService {
public:
  explicit TcpTest(PtrCbMailboxMgr pMbMgr) : m_pMbMgr(pMbMgr) {
    m_pfnRecv = nsp_boost::make_shared<OnRecv>();
    m_pfnSend = nsp_boost::make_shared<OnSend>();
    m_pfnClose = nsp_boost::make_shared<OnClose>();
    m_pfnAccept = nsp_boost::make_shared<OnAccept>();
    m_pfnSend->m_fn = nsp_boost::bind(&TcpTest::test_Send, this);
    m_pfnRecv->m_fn = nsp_boost::bind(&TcpTest::test_Send, this);
    m_pfnAccept->m_fn = nsp_boost::bind(&TcpTest::test_Accept, this, _1);
    m_pfnClose->m_fn = nsp_boost::bind(&TcpTest::test_Close, this);
  }
  
  virtual ~TcpTest() {}

//  virtual int acceptCb(PtrFD pFdAccept) = 0;
  
  virtual int recvCb(IoWatcher & rIoWatcher, PtrFD pFd, PtrVoid pData) {
    PfnOnRecv pRecv = nsp_boost::dynamic_pointer_cast<OnRecv>(m_pfnRecv);
    m_pMbMgr->insert(EVPRIOR_DEFAULT, pRecv);
    nsp_std::string strMsg("Hello World!!!\n");
    rIoWatcher.PostSend(pFd->fd_, strMsg);
    return 0;
  }
  
  virtual int sendCb(IoWatcher & rIoWatcher, PtrFD pFd, PtrVoid pData) {
    PfnOnSend pSend = nsp_boost::dynamic_pointer_cast<OnSend>(m_pfnSend);
    m_pMbMgr->insert(EVPRIOR_DEFAULT, pSend);
    return 0;
  }
  
  virtual int acceptCb(IoWatcher & rIoWatcher, PtrFD pFd, PtrVoid pData) {
    PfnOnAccept pFn = nsp_boost::dynamic_pointer_cast<OnAccept>(m_pfnAccept);
    pFn->m_arg1 = pFd->fd_;
    m_pMbMgr->insert(EVPRIOR_DEFAULT, pFn);
    return 0;
  }
  
  virtual int closeCb(IoWatcher & rIoWatcher, PtrFD pFd, PtrVoid pData) {
    PfnOnClose pFn = nsp_boost::dynamic_pointer_cast<OnClose>(m_pfnClose);
    m_pMbMgr->insert(EVPRIOR_DEFAULT, pFn);
    return 0;
  }
  
  void test_Send() {
    LOG(INFO) << "----------Send-------------";
  }
  
  void test_Recv() {
    LOG(INFO) << "----------Recv-------------";
  }
  
  void test_Accept(pump_fd_t fd) {
    LOG(INFO) << "----------Accept(" << fd << ")-------------";
  }
  
  void test_Close() {
    LOG(INFO) << "----------Close-------------";
  }
  
  nsp_boost::shared_ptr<OnRecv> m_pfnRecv;
  nsp_boost::shared_ptr<OnSend> m_pfnSend;
  nsp_boost::shared_ptr<OnAccept> m_pfnAccept;
  nsp_boost::shared_ptr<OnClose> m_pfnClose;
private:
  PtrCbMailboxMgr m_pMbMgr;
};

void test_IoWatcher(){
  LOG(INFO) << "----------test_IoWatcher-------------";
  PtrCbMailbox pMailbox = nsp_boost::make_shared<CbQueueMailbox>();
  PtrCbMailboxMgr pCbMBMgr = nsp_boost::make_shared<ICbMailboxMgr>(pMailbox);
  
//  PfnOnAccept onAccept = nsp_boost::make_shared<OnAccept>();
//  onAccept->m_fn = nsp_boost::bind()
//  PfnOnRecv onRecv = nsp_boost::make_shared<OnRecv>();
//  PfnOnSend onSend = nsp_boost::make_shared<OnSend>();
  
  PtrTcpService tcpTest = nsp_boost::make_shared<TcpTest>(pCbMBMgr);
  
  IoWatcher watcher(pCbMBMgr);
  watcher.init();
  watcher.newAccept("127.0.0.1", 8001, tcpTest);
  while (1) {
    watcher.doWatching();
    pMailbox->runAll();
  }
}

int main() {
  test_IoWatcher();
  
  return 0;
}