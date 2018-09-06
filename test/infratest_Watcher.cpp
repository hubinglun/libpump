//
// Created by yz on 18-9-2.
//

#include <boost/shared_ptr.hpp>
#include "CbMailbox.h"
#include "Watcher.h"
#include "Logger.h"

using namespace PUMP;

class TcpTest
  : public TcpService {
public:
  explicit TcpTest(PtrCbMailboxMgr pMbMgr) : m_pMbMgr(pMbMgr) {
    m_pfnRecv = nsp_boost::make_shared<OnRecv>();
    m_pfnSend = nsp_boost::make_shared<OnSend>();
    m_pfnSend->m_fn = nsp_boost::bind(&TcpTest::test_Send, this);
    m_pfnRecv->m_fn = nsp_boost::bind(&TcpTest::test_Send, this);
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
  
  void test_Send() {
    LOG(INFO) << "----------Send-------------";
  }
  
  void test_Recv() {
    LOG(INFO) << "----------Recv-------------";
  }
  
  nsp_boost::shared_ptr<OnRecv> m_pfnRecv;
  nsp_boost::shared_ptr<OnSend> m_pfnSend;
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