//
// Created by yz on 18-9-2.
//

#include "infratest_Watcher.h"

int main() {
  LOG(INFO) << "----------test-------------";
  
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
  
  return 0;
}