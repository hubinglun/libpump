//
// Created by yz on 18-9-2.
//

#include "CbMailbox.h"
#include "Watcher.h"
#include "Logger.h"

using namespace PUMP;

void test_IoWatcher(){
  LOG(INFO) << "----------test_IoWatcher-------------";
  
  PtrCbMailbox pMailbox = nsp_boost::make_shared<CbQueueMailbox>();
  PtrCbMailboxMgr pCbMBMgr = nsp_boost::make_shared<ICbMailboxMgr>(pMailbox);
  
  PfnOnAccept onAccept = nsp_boost::make_shared<OnAccept>();
//  onAccept->m_fn = nsp_boost::bind()
  PfnOnRecv onRecv = nsp_boost::make_shared<OnRecv>();
  PfnOnSend onSend = nsp_boost::make_shared<OnSend>();
  
  IoWatcher watcher(pCbMBMgr);
  watcher.init();
  watcher.newAccept("127.0.0.1", 8001, onAccept, onRecv, onSend);
  while (1) {
    watcher.doWatching();
  }
}

int main() {
  test_IoWatcher();
  
  return 0;
}