//
// Created by yz on 18-7-29.
//
#pragma once

#ifndef LIBPUMP_PUMPDEF_H
#define LIBPUMP_PUMPDEF_H

namespace PUMP{

/** 事件优先级的数值类型, 与 enum EventPriority 对应 */
typedef unsigned short ev_prior_t;

/** 枚举常量
 *
 * 定义了事件对象 Event 优先级, 从上往下优先级由高到低*/
enum EventPriority{
	EVPRIOR_LEVEL0 = 0,	//! 最高优先级
	EVPRIOR_LEVEL1,			//! 次高优先级
	EVPRIOR_LEVEL2,			//! 最低优先级
	EVPRIOR_DEFAULT,		//! 默认优先级, 即最低
};

/** 枚举常量
 *
 * 定义了事件对象 Event 生命周期状态
 * > EVSTATE_INIT 初始化状态, 表示事件刚刚被构造
 * > EVSTATE_REGISTERED 事件已注册, 即加入到事件循环中开始监听
 * > EVSTATE_ACTIVE 事件被激活, 说明事件发生
 * > EVSTATE_SOLVED 激活事件已被处理, 或投递给邮箱或实时调用
 *
 * 状态转换规则
 * EVSTATE_INIT　<-> EVSTATE_REGISTERED
 * EVSTATE_REGISTERED <-> EVSTATE_ACTIVE
 * EVSTATE_ACTIVE -> EVSTATE_SOLVED -> EVSTATE_REGISTERED
 * */
enum EventState{
	EVSTATE_INIT,				//! 初始化态
	EVSTATE_REGISTERED,	//! 注册态
	EVSTATE_ACTIVE, 		//! 激活态
	EVSTATE_SOLVED, 		//! 已处理态
};

/** 枚举常量
 *
 * 定义了事件对象 Event 基本类型
 * > EVTYPE_PRE 事前事件, 加入到 Watcher 的 PreEventsList 中在每一轮 preProcess() 中被执行
 * > EVTYPE_NORM 普通事件, 加入到 Watcher 的 NormEventsList 中在每一轮 dispatch() 中检测并激活
 * > EVTYPE_POST 事后事件, 加入到 Watcher 的 PostEventsList 中在每一轮 postProcess() 中被执行
 * */
enum EventType{
	EVTYPE_PRE,   //! 事前事件
	EVTYPE_NORM,  //! 普通事件
	EVTYPE_POST,  //! 事后事件
};

/** 枚举常量
 *
 * 定义了事件循环对象 Pump 生命周期状态
 * > PUMPSTATE_NEW 表示 Pump 对象刚刚构造, 需要初始化 WatcherList 后才能运行
 * > PUMPSTATE_INIT 表示 Pump 对象完成初始化
 * > PUMPSTATE_STOP 表示 Pump 对象未启动运行
 * > PUMPSTATE_START 表示 Pump 对象正在运行
 * > PUMPSTATE_PAUSE　表示 Pump 对象处于挂起(暂停)状态
 *
 * 状态转换规则
 * PUMPSTATE_NEW　-> PUMPSTATE_INIT
 * PUMPSTATE_INIT -> PUMPSTATE_STOP
 * PUMPSTATE_INIT -> PUMPSTATE_START
 * PUMPSTATE_INIT -> PUMPSTATE_PAUSE
 * PUMPSTATE_STOP <-> PUMPSTATE_START <-> PUMPSTATE_PAUSE
 * */
enum PumpState{
	PUMPSTATE_NEW,		//! 新建态
	PUMPSTATE_INIT, 	//! 初始化态
	PUMPSTATE_STOP,		//! 终止态
	PUMPSTATE_START,	//! 运行态
	PUMPSTATE_PAUSE,	//! 挂起态
};

}

#endif //LIBPUMP_PUMPDEF_H
