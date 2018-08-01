//
// Created by yz on 18-7-23.
//

#include "Pump.h"

namespace PUMP{

////////////////////////////////////////////////
//                   Pump
////////////////////////////////////////////////

Pump::Pump(WPtrCbMailboxCaller pMbCaller)
	:m_wpMbCaller(pMbCaller){
}

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
	PtrCbMailboxCaller t_pMailCaller = m_wpMbCaller.lock();
	if(t_pMailCaller==NULL){
		return 0;
	}
	t_pMailCaller->runAll();
	return 0;
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

}
