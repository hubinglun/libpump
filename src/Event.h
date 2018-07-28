//
// Created by yz on 18-7-27.
//

#ifndef LIBPUMP_EVENT_H
#define LIBPUMP_EVENT_H

namespace PUMP{

class Event {
public:
	enum EventPriority{
		EVPRIOR_LEVEL0,
		EVPRIOR_LEVEL1,
		EVPRIOR_LEVEL2,
		EVPRIOR_DEFAULT,
	};
	enum EventState{
		EVSTATE_INIT,				//! 新建
		EVSTATE_REGISTERED,	//! 已注册
		EVSTATE_ACTIVE, 		//! 激活
		EVSTATE_SOLVED, 		//! 已处理， 或投递给邮箱或实时调用
	};
	enum EventType{
		EVTYPE_PRE,   //! 事前事件
		EVTYPE_NORM,  //! 普通事件
		EVTYPE_POST,  //! 事后事件
	};
private:
	//! 描述事件的优先级
	enum EventPriority m_evPriority;
	//! 事件的状态
	enum EventState m_evState;
	//! 事件的类型，事前、普通、事后
	enum EventType m_evType;
};

}

#endif //LIBPUMP_EVENT_H
