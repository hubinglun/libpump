//
// Created by yz on 18-7-23.
//

#include "Pump.h"

namespace PUMP{

////////////////////////////////////////////////
//                   Pump
////////////////////////////////////////////////

Pump::Pump(){}

Pump::~Pump(){}

int Pump::preWatching() {
	// 暂时不知道干啥, 但是肯定是与Watcher对象相关的预处理
	return 0;
}

int Pump::watching(){
	for(WatcherList::iterator it = m_watchers.begin();
		it!=m_watchers.end();
		++it){
		(*it)->doWatching();
	}
	return 0;
}

int Pump::postWatching(){
	// 目前 postWatching 仅安排执行回调
	m_cbMailbox.runAll();
	return 0;
}

void Pump::deliver(ev_prior_t prior, PrtCbFn pfn) {
	m_cbMailbox.insert(prior, pfn);
}

void Pump::init(){

}

void Pump::start() {
	// 暂时就这么简陋
	while (m_state == PUMPSTATE_START) {
		preWatching();
		watching();
		postWatching();
	}
}


void Pump::pause(){
	m_state = PUMPSTATE_PAUSE;
}
void Pump::stop(){
	m_state = PUMPSTATE_STOP;
}

////////////////////////////////////////////////
//                 CbFuncList
////////////////////////////////////////////////

CbFuncList::CbFuncList()
	: m_pRevLFns(&m_lFns_0),
		m_pRunLFns(&m_lFns_1){
}

CbFuncList::~CbFuncList(){

}

bool CbFuncList::insert(PrtCbFn pfn){
	m_pRevLFns->push_back(pfn);
}

size_t CbFuncList::runAll(){
	if(m_pRunLFns->empty()){
		// (双缓冲) 交换 m_pRevLFns 与 m_pRunLFns
		swapLRef();
	}
	size_t t_iN = m_pRunLFns->size();
	for(nsp_std::list<PrtCbFn>::iterator it = m_pRunLFns->begin();
			it!=m_pRunLFns->end();
			it++){
		(*(*it))();
	}
	// 回调处理完毕，需要清理链表
	m_pRunLFns->clear();

	return t_iN;
}

void CbFuncList::swapLRef(){
	std::list<PrtCbFn> * t_pRun = m_pRunLFns;

	// 加锁, 不允许其他线程再向“前” m_pRevLFns 插入回调
	lockRevLFns();
	m_pRunLFns = m_pRevLFns;
	m_pRevLFns = t_pRun;
	unlockRevLFns();
}

bool CbFuncList::lockRevLFns(){
	m_mtxRevLFns.lock();
	return true;
}

bool CbFuncList::unlockRevLFns(){
	m_mtxRevLFns.unlock();
	return true;
}

////////////////////////////////////////////////
//                 CbMailbox
////////////////////////////////////////////////

CbMailbox::CbMailbox(){
	initCbPriorQueue();
}

CbMailbox::~CbMailbox(){
	disposeCbPriorQueue();
}

bool CbMailbox::insert(ev_prior_t prior, PrtCbFn pfn){
	CbPriorQueue::iterator it;

	if(prior == EVPRIOR_DEFAULT) {
		// 默认优先级则加入最低优先级队列
		it = m_queCb.find(EVPRIOR_DEFAULT - 1);
	}
	else{
		// 其他优先级则加入对应队列
		it = m_queCb.find(prior);
	}

	if(it == m_queCb.end())
		return false;
	// 加入回调对象
	((*it).second)->insert(pfn);
	return true;
}

size_t CbMailbox::runAll() {
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

void CbMailbox::initCbPriorQueue() {
	for (ev_prior_t i = 0;
			 i < EVPRIOR_DEFAULT;
			 ++i) {
		// 按照优先级标准, 构造优先级队列
//		m_queCb.insert(nsp_std::pair<ev_prior_t, PtrCbFuncList>(i,nsp_boost::make_shared<CbFuncList>()));
		m_queCb.insert(nsp_std::make_pair(i, nsp_boost::make_shared<CbFuncList>()));
	}
}

void CbMailbox::disposeCbPriorQueue(){
	// Nothing To Do!!!
}

}
