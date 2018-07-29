/******************************************************************************
*		libpump Basic tool library																								*
*		Copyright (C) 2018 YangZheng  263693992@qq.com.														*
*																																							*
*		This file is part of libpump.																							*
*																																							*
*		This program is free software; you can redistribute it and/or modify			*
*		it under the terms of the GNU General Public License version 3 as					*
*		published by the Free Software Foundation.																*
*																																							*
*		You should have received a copy of the GNU General Public License					*
*		along with OST. If not, see <http://www.gnu.org/licenses/>.								*
*																																							*
*		Unless required by applicable law or agreed to in writing, software				*
*		distributed under the License is distributed on an "AS IS" BASIS,					*
*		WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.	*
*		See the License for the specific language governing permissions and				*
*		limitations under the License.																						*
*																																							*
*	@file			Pump.h																														*
*	@brief																																			*
*		Details.																																	*
*																																							*
*	@author		YangZheng																													*
*	@email		263693992@qq.com																									*
*	@version	1.0.0.0																														*
*	@date			2018-07-27																												*
*	@license	GNU General Public License (GPL)																	*
*																																							*
*-----------------------------------------------------------------------------*
*	Remark:	Description																													*
*-----------------------------------------------------------------------------*
*	Change History:																															*
*	<Date>			|	<Version>	|		<Author>		|	<Description>											*
*-----------------------------------------------------------------------------*
*	2018/07/27	|	1.0.0.0		|		YangZheng		|	Create file												*
*-----------------------------------------------------------------------------*
*																																							*
******************************************************************************/
#pragma once

#ifndef LIBPUMP_PUMP_H
#define LIBPUMP_PUMP_H

#include <list>
#include <vector>
#include <map>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include "pumpdef.h"
#include "Wather.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @brief PUMP
 * 命名空间的详细概述
 */
namespace PUMP{

/** class CbFunc [Pump.h]
 *
 * 托管回调函数仿函数对象基类,也是函数列表对象的元素;
 * 任何回调函数需要继承此对象,并实现对象的operator()成员,该成员供CbFuncList对象逐个执行回调时调用;
 * 本对象不允许拷贝;
 *
 * 派生规则:
 * 1. 派生子回调对象时,用户根据回调函数原型,定义 boost::function<...> func_t;
 * 2. 用户为派生对象必须定义参数值成员对象和返回值成员对象,使整个回调对象形成闭包,用户可以通过定义 getReturn() 成员函数获取返回值结果
 *    (建议回调参数为指针时使用 boost::shared_ptr 封装, 避免多线程下的竞态条件发生。另外,由于回调函数的产生和执行是异步的,所以应该
 *    避免栈上对象的指针作为参数传入);
 * 3. 函数的是否具备返回值,在定义类时确定。参数值则尽量在构造时与回调函数对象绑定,一般情况下应该禁止修改函数对象的参数值,
 *    以免发生用户意料之外的执行结果;
 * 4. 派生类实现纯虚函数 operator().
 */
class CbFunc
	:public nsp_boost::noncopyable{
public:
	CbFunc(bool bReturn=false)
		: m_bReturn(bReturn) {}
	virtual ~CbFunc(){}

	/** 判断回调对象是否具有返回值 */
	inline bool isReturnAValue() const { return m_bReturn; }

	/** 纯虚函数, 重载()运算符
	 *
	 * 每一个派生于 CbFunc 对象的回调都必须实现,
	 * */
	virtual void operator()() = 0;

private:
	//!< 标记回调对象是否有返回值, 定义对象时确定
	const bool m_bReturn;
};

/** class CbFuncWithoutReturn [Pump.h]
 *
 * 派生于 CbFunc, 所有无返值回调函数基类
 */
class CbFuncWithoutReturn
	: public CbFunc{
public:
	CbFuncWithoutReturn()
		: CbFunc(false) {}
	~CbFuncWithoutReturn(){}
};

/** class CbFuncWithReturn [Pump.h]
 *
 * 派生于 CbFunc, 所有有返值回调函数基类
 */
class CbFuncWithReturn
	: public CbFunc{
public:
	CbFuncWithReturn()
		: CbFunc(true) {}
	~CbFuncWithReturn(){}
};

/** 指向 CbFunc 对象的指针
 *
 * 定义本指针目的在于加入 list<PrtCbFn> 实现回调函数的托管
 * */
typedef nsp_boost::shared_ptr<CbFunc> PrtCbFn;

/** class CbFuncList []
 *
 * 回调函数链表,回调对象的托管对象.在优先级队列中存放某一个优先级下的所有函数对象
 * 接收任意参数类型, 个数及返回值类型的回调函数对象.
 * */
class CbFuncList
	: public nsp_boost::noncopyable {
public:
	CbFuncList();
	~CbFuncList();

	/** 向 m_pRevLFns 尾插入一个回调对象
	 *
	 * @param[in] pfn 要插入链表的回调对象(注：由于回调对象会被修改,禁止将参数声明为const)
	 * @return 插入成功返回true
	 * */
	bool insert(PrtCbFn pfn);

	/** 遍历链表 m_pRunLFns 执行其中回调对象
	 *
	 * @return 返回执行的回调对象数量
	 * */
	size_t runAll();
private:
	/** 交换 m_rRevLFns 与 m_rRunLFns
	 *
	 * 意味着 m_rRunLFns 内的回调函数执行完成.
	 * 本函数仅限于再 runAll() 中调用.
	 * */
	void swapLRef();

	/** 请求对 m_mtxRevLFns 加锁 */
	bool lockRevLFns();

	/** 请求对 m_mtxRevLFns 释放锁 */
	bool unlockRevLFns();
private:
	/**< 访问 m_pRevLFns 函数链表的互斥量 */
	nsp_boost::mutex m_mtxRevLFns;

	/** 函数链表使用双缓冲机制,同一时间一个链表用于遍历执行,另一个用于接收新回调函数
	 *
	 * 这样设计的目的是, 若 Pump 对象和 Watcher 对象工作在不同线程, 即IO与CPU计算分离后,
	 * 双缓冲提高并发性和吞吐量, 且避免出现竞态条件 */
	std::list<PrtCbFn> m_lFns_0;
	std::list<PrtCbFn> m_lFns_1;
	std::list<PrtCbFn> * m_pRevLFns;
	std::list<PrtCbFn> * m_pRunLFns;
};

/** class CbMailbox [Pump.h]
 *
 * 回调函数邮箱类, 负责调度回调函数
 * */
class CbMailbox
	: public nsp_boost::noncopyable {
public:

	/** 回调优先级队列元素类型
	 *
	 * 优先级队列中存放智能指针的好处是内存托管, 另外也因为 CbFuncList 是不允许拷贝的,
	 * 不符合STL元素对象标准
	 * */
	typedef nsp_boost::shared_ptr<CbFuncList> PtrCbFuncList;

	/** 回调优先级队列对象
	 *
	 * 由 map 容器实现
	 *  map key 值为枚举类型 EventPriority
	 *  value 值为回调链表的智能指针
	 * */
	typedef nsp_std::map<ev_prior_t, PtrCbFuncList> CbPriorQueue;

	CbMailbox();
	~CbMailbox();

	/** 向优先级队列插入一个回调对象
	 *
	 * @param prior 回调对象的优先级
	 * @param pFn 回调对象
	 *
	 * */
	bool insert(ev_prior_t prior, PrtCbFn pFn);

	/**  按优先级遍历各级函数链表, 执行回调
	 * */
	size_t runAll();
private:
	void initCbPriorQueue();
	void disposeCbPriorQueue();
private:
	//! 回调优先级队列
	CbPriorQueue m_queCb;
};

/** class Pump [Pump.cpp]
 *
 * Pump 对象名字源于强健有力的“泵”, 正如“泵”的将机械能或其他外部能量传送给液体那样,
 * Pump 对象是整个事件循环框架的动力源泉, 它负责驱动 Watcher 对象监听、激活、处理
 * 事件对象
 * */
class Pump
	: public nsp_boost::noncopyable {
public:
	/** Watcher 内存托管 */
	typedef nsp_boost::shared_ptr<Wather> PtrWatcher;
	/** Watcher 对象数组 */
	typedef nsp_std::vector<PtrWatcher> WatcherList;

public:
	Pump();
	~Pump();
	/** 向回调邮箱 m_cbMailbox 投递一个回调 */
	void deliver(ev_prior_t prior, PrtCbFn pfn);

	/**
   * @name Pump 生命周期控制
   * @{
   */

	/** 初始化Pump对象，实际上按照配置初始化WatcherList */
	void init();
	/** 启动 Pump 服务 */
	void start();
	/** 暂停Pump循环，挂起 */
	void pause();
	/** 终结Pump循环，需要清理资源，释放fd等 */
	void stop();

	/** @} */

private:
	/**
   * @name WatcherList 生命周期控制
   * @{
   */

	/**
	 * @brief watching() 前置处理
	 * */
	int preWatching();

	/**
	 * @brief watching()
	 *
	 * 遍历 m_watchers 数组, 逐个调用 Wather 对象的三个阶段函数
	 * */
	int watching();

	/**
	 * @brief watching()前置处理
	 * */
	int postWatching();

	/** @} */
private:
	//! 存放 Wather 对象指针的数组对象
	WatcherList m_watchers;
	//! 回调函数邮箱
	CbMailbox m_cbMailbox;
	//! 标志 Pump 当前的运行状态
	enum PumpState m_state;
};

}

#endif //LIBPUMP_PUMP_H
