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
int Pump::preWatching(){
	return 0;
}
int Pump::watching(){
	return 0;
}
int Pump::postWatching(){
	return 0;
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
		//! (双缓冲) 交换 m_pRevLFns 与 m_pRunLFns
		swapLRef();
	}

	for(nsp_std::list<PrtCbFn>::iterator it = m_pRunLFns->begin();
		it!=m_pRunLFns->end();
		it++){
		(*(*it))();
	}
	m_pRunLFns->clear();

	return m_pRunLFns->size();
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

}
