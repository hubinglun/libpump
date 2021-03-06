//
// Created by yz on 18-8-1.
//

#include "CbMailbox.h"

namespace Pump {

////////////////////////////////////////////////
//                 CbList
////////////////////////////////////////////////

CbList::CbList()
  : m_pRevLFns(&m_lFns_0),
    m_pRunLFns(&m_lFns_1) {
}

CbList::~CbList() {

}

bool CbList::insert(PtrCbFn pfn) {
  m_pRevLFns->push_back(pfn);
}

size_t CbList::runAll() {
  if (m_pRunLFns->empty()) {
    // (双缓冲) 交换 m_pRevLFns 与 m_pRunLFns
    swapLRef();
  }
  size_t t_iN = m_pRunLFns->size();
  for (nsp_std::list<PtrCbFn>::iterator it = m_pRunLFns->begin();
       it != m_pRunLFns->end();
       it++) {
    (*(*it))();
  }
  // 回调处理完毕，需要清理链表
  m_pRunLFns->clear();
  
  return t_iN;
}

void CbList::swapLRef() {
  std::list<PtrCbFn> *t_pRun = m_pRunLFns;
  
  // 加锁, 不允许其他线程再向“前” m_pRevLFns 插入回调
  lockRevLFns();
  m_pRunLFns = m_pRevLFns;
  m_pRevLFns = t_pRun;
  unlockRevLFns();
}

bool CbList::lockRevLFns() {
  m_mtxRevLFns.lock();
  return true;
}

bool CbList::unlockRevLFns() {
  m_mtxRevLFns.unlock();
  return true;
}

////////////////////////////////////////////////
//                 CbQueueMailbox
////////////////////////////////////////////////

CbQueueMailbox::CbQueueMailbox() {
  initCbPriorQueue();
}

CbQueueMailbox::~CbQueueMailbox() {
  disposeCbPriorQueue();
}

bool CbQueueMailbox::insert(ev_prior_t prior, PtrCbFn pfn) {
  CbPriorQueue::iterator it;
  
  if (prior == EVPRIOR_DEFAULT) {
    // 默认优先级则加入最低优先级队列
    it = m_queCb.find(EVPRIOR_DEFAULT - 1);
  } else {
    // 其他优先级则加入对应队列
    it = m_queCb.find(prior);
  }
  
  if (it == m_queCb.end())
    return false;
  // 加入回调对象
  ((*it).second)->insert(pfn);
  return true;
}

size_t CbQueueMailbox::runAll() {
  // 记录执行回调数量
  size_t t_iN = 0;
  
  for (CbPriorQueue::iterator it = m_queCb.begin();
       it != m_queCb.end();
       ++it) {
    // 按优先级执行各回调链表的回调
    t_iN += (*((*it).second)).runAll();
  }
  return t_iN;
}

void CbQueueMailbox::initCbPriorQueue() {
  for (ev_prior_t i = 0;
       i < EVPRIOR_DEFAULT;
       ++i) {
    // 按照优先级标准, 构造优先级队列
//		m_queCb.insert(nsp_std::pair<ev_prior_t, PtrCbContainer>(i,nsp_boost::make_shared<CbList>()));
    // FIXME 此处CbFn容器CbList现在是写死的, 之后应该以修改为使用配置文件的配合工厂对象的形式
    m_queCb.insert(nsp_std::make_pair(i, nsp_boost::make_shared<CbList/*FIXME*/>()));
  }
}

void CbQueueMailbox::disposeCbPriorQueue() {
  // Nothing To Do!!!
}

////////////////////////////////////////////////
//            ICbMailboxMgr
////////////////////////////////////////////////

ICbMailboxMgr::
ICbMailboxMgr(nsp_boost::weak_ptr<CbMailbox> pMailbox)
  : m_pMailbox(pMailbox) {}

ICbMailboxMgr::
~ICbMailboxMgr() {}

bool
ICbMailboxMgr::
insert(ev_prior_t prior, PtrCbFn pfn) {
  nsp_boost::shared_ptr<CbMailbox> p_tMailbox = m_pMailbox.lock();
  if (p_tMailbox == NULL) {
    return false;
  }
  p_tMailbox->insert(prior, pfn);
}

////////////////////////////////////////////////
//            ICbMailboxEvoker
////////////////////////////////////////////////

ICbMailboxEvoker::
ICbMailboxEvoker(WPtrCbMailbox pMailbox)
  : m_pMailbox(pMailbox) {}

ICbMailboxEvoker::
~ICbMailboxEvoker() {}

size_t
ICbMailboxEvoker::
runAll() {
  nsp_boost::shared_ptr<CbMailbox> p_tMailbox = m_pMailbox.lock();
  if (p_tMailbox == NULL) {
    return 0;
  }
  return p_tMailbox->runAll();
}

}