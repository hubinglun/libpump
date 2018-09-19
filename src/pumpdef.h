/**
 * @file pumpdef.h
 * @brief 定义 libpump 库的全局类型, 将来可能拆分成多个文件
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.29
 */

#ifndef LIBPUMP_PUMPDEF_H
#define LIBPUMP_PUMPDEF_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#include <boost/shared_ptr.hpp>

namespace nsp_boost = ::boost;

namespace Pump {

#define PUMP_ABSTRACT  /** 修饰抽象类 */
#define PUMP_INTERFACE  /** 修饰接口类 */
#define PUMP_IMPLEMENT  /** 修饰实现类 */

/** 事件优先级的数值类型, 与 enum EventPriority 对应 */
typedef unsigned short ev_prior_t;

#ifdef _WIN32
typedef HANDLE pump_fd_t;
#else
#ifdef __linux
typedef int pump_fd_t;
#endif // __linux
#endif // _WIN32

/**
 * @enum EventPriority
 * @brief Event 优先级
 *
 * 定义了事件对象 Event 优先级, 从上往下优先级由高到低*/
enum EventPriority {
  EVPRIOR_LEVEL0 = 0,  //! 最高优先级
  EVPRIOR_LEVEL1,      //! 次高优先级
  EVPRIOR_LEVEL2,      //! 最低优先级
  EVPRIOR_DEFAULT,    //! 默认优先级, 即最低
};

/**
 * @enum EventType
 * @brief Event 生命周期状态
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
enum EventState {
  EVSTATE_INIT,           //! 初始化态
  EVSTATE_REGISTERED,     //! 注册态
  EVSTATE_ACTIVE,         //! 激活态
  EVSTATE_SOLVED,         //! 已处理态
};

/**
 * @enum EventType
 * @brief Event 基本类型
 *
 * 定义了事件对象 Event 基本类型
 * > EVTYPE_PRE 事前事件, 加入到 PWatcher 的 PreEventsList 中在每一轮 preWatch() 中被执行
 * > EVTYPE_NORM 普通事件, 加入到 PWatcher 的 NormEventsList 中在每一轮 watch() 中检测并激活
 * > EVTYPE_POST 事后事件, 加入到 PWatcher 的 PostEventsList 中在每一轮 postWatch() 中被执行
 */
enum EventType {
  EVTYPE_PRE,   //! 前置事件
  EVTYPE_NORM,  //! 普通事件
  EVTYPE_POST,  //! 后置事件
};

/**
 * @enum enum PumpState
 * @brief Pump 生命周期状态
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
 */
enum PumpState {
  PUMPSTATE_NEW,    //! 新建态
  PUMPSTATE_INIT,   //! 初始化态
  PUMPSTATE_START,  //! 运行态
  PUMPSTATE_PAUSE,  //! 暂停态
  PUMPSTATE_WAITE,  //! 挂起态
  PUMPSTATE_STOP,   //! 终止态
};

enum PumpType {
  PUMPTYPE_SYNC,  //! 同步执行Pump
  PUMPTYPE_ASYNC, //! 异步执行Pump
};

/**
 * @enum TimerType
 * @brief 定时器类型
 *
 * - TMTYPE_ONETIME 一次性定时器
 * - TMTYPE_PERIODIC 周期性定时器
 */
enum TimerType {
  TMTYPE_DEFAULT,      //! 默认类型
  TMTYPE_ONETIME,   //! 一次性定时器
  TMTYPE_PERIODIC   //! 周期性定时器
};

const unsigned short IO_EV_NONE = 0x000;    //! io fd 无监听事件
const unsigned short IO_EV_IN = 0x001;      //! io fd 发生可读事件
const unsigned short IO_EV_OUT = 0x002;     //! io fd 发生可写事件
const unsigned short IO_EV_ERR = 0x004;     //! io fd 发生异常事件

/**
 * @enum FdCtlTyge
 * @brief 描述对 IoFd 对象修改类型
 */
enum FdCtlTyge {
  FD_CTL_DEFAULT, //! 默认类型
  FD_CTL_ADD,  //! 增加监听事件类型
  FD_CTL_DEL,  //! 删除监听事件类型
  FD_CTL_MOD   //! 修改监听事件类型
};

/**
 * @enum FdState
 * @brief 文件描述符的生命周期状态
 *
 * - FD_STATE_INIT        fd 初始态
 * - FD_STATE_LISTENED    fd 监听态
 * - FD_STATE_CONNECTED   fd 连接态
 * - FD_STATE_SHUTDOWNED  fd 半闭态
 * - FD_STATE_CLOSED      fd 关闭态
 */
enum FdState {
  FD_STATE_INIT,        //! fd 初始态
  FD_STATE_LISTENED,    //! fd 监听态
  FD_STATE_CONNECTED,   //! fd 连接态
  FD_STATE_SHUTDOWNED,  //! fd 半闭态
  FD_STATE_CLOSED       //! fd 关闭态
};

typedef nsp_boost::shared_ptr<void> PtrVoid;

}

#endif //LIBPUMP_PUMPDEF_H
