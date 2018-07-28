/******************************************************************************
*  libpump Basic tool library                                                 *
*  Copyright (C) 2018 YangZheng  263693992@qq.com.                         		*
*                                                                            	*
*  This file is part of libpump.                                                 *
*                                                                            	*
*  This program is free software; you can redistribute it and/or modify      	*
*  it under the terms of the GNU General Public License version 3 as         	*
*  published by the Free Software Foundation.                                	*
*                                                                            	*
*  You should have received a copy of the GNU General Public License         	*
*  along with OST. If not, see <http://www.gnu.org/licenses/>.               	*
*                                                                            	*
*  Unless required by applicable law or agreed to in writing, software       	*
*  distributed under the License is distributed on an "AS IS" BASIS,         	*
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  	*
*  See the License for the specific language governing permissions and       	*
*  limitations under the License.                                            	*
*                                                                            	*
*  @file     Pump.h                                                       		*
*  @brief    					                                                      	*
*  Details.                                                                  	*
*                                                                            	*
*  @author   YangZheng                                                       	*
*  @email    263693992@qq.com																									*
*  @version  1.0.0.0			                                                  	*
*  @date     2018-07-27	                                              				*
*  @license  GNU General Public License (GPL)                                	*
*                                                                            	*
*-----------------------------------------------------------------------------*
*  Remark         : Description                                              	*
*-----------------------------------------------------------------------------*
*  Change History :                                                          	*
*    <Date>   | <Version> |    <Author>    | <Description>                   	*
*-----------------------------------------------------------------------------*
*  2018/07/27 | 1.0.0.0   |    YangZheng   | Create file                     	*
*-----------------------------------------------------------------------------*
*                                                                            	*
******************************************************************************/

#ifndef LIBPUMP_PUMP_H
#define LIBPUMP_PUMP_H

#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include "Wather.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @brief PUMP
 * 命名空间的详细概述
 */
namespace PUMP{

class CbFunc;
class CbMailbox;


/**Wather对象数组*/
typedef nsp_std::vector<nsp_boost::weak_ptr<Wather> > WatcherList;

/** class CbMailbox [Pump.h]
 *
 * 回调函数邮箱类, 负责调度回调函数
 * */
class CbMailbox{
};

/** class CbFunc [Pump.h]
 *
 * 托管回调函数仿函数对象基类，也是函数列表对象的元素;
 * 任何回调函数需要继承此对象，并实现对象的operator()成员，该成员供CbFuncList对象逐个执行回调时调用;
 * 本对象不允许拷贝;
 *
 * 派生规则:
 * 	1. 	派生子回调对象时，用户根据回调函数原型，定义 boost::function<...> func_t;
 * 	2. 	用户为派生对象定义参数值对象成员和返回值对象成员（建议回调参数为指针时使用 boost::shared_ptr 封装,
 * 			避免多线程下的竞态条件发生。另外，由于回调函数的执行是异步的，所以避免栈上对象的指针作为参数传入）;
 * 	3.	函数的是否具备返回值，在定义类时确定。参数值则尽量在构造时与函数对象绑定，一般情况下不允许修改函数对象的参数值,
 * 			以免发生用户意料之外的执行结果;
 * 	4. 	派生类实现纯虚函数 operator().
 */
class CbFunc
	:public nsp_boost::noncopyable{
public:
	CbFunc(bool bReturn=false)
		: m_bReturn(bReturn) {}
	virtual ~CbFunc(){}
	inline bool returnAValue() const { return m_bReturn; }
	virtual void operator()() = 0;
private:
	const bool m_bReturn;
};

/** class CbFuncWithoutReturn [Pump.h]
 *
 * 派生于 CbFunc, 所有无返值回调函数基类
 */
class CbFuncWithoutReturn
	:public CbFunc{
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
	:public CbFunc{
public:
	CbFuncWithReturn()
		: CbFunc(true) {}
	~CbFuncWithReturn(){}
};

/** 指向 CbFunc 对象的指针
 *
 * 定义本指针目的在于加入 vector<PrtCbFn> 实现回调函数的托管
 * */
typedef nsp_boost::shared_ptr<CbFunc> PrtCbFn;

/** class CbFuncList []
 *
 * 回调函数链表，在优先级队列中存放某一个优先级下的所有函数对象
 * 接收任意参数类型, 个数及返回值类型的回调函数对象.
 * */
class CbFuncList{
public:
	CbFuncList();
	~CbFuncList();
	bool insert(PrtCbFn pfn);
	size_t runAll();
private:
	void swap(std::list<PrtCbFn>* &prRun,std::list<PrtCbFn>* &prRev);
	void swapLRef();
	bool lockRevLFns();
	bool unlockRevLFns();
private:
	/**< 访问函数列表的互斥量, 读不加锁 */
	nsp_boost::mutex m_mtxRevLFns;
	/**< 存放回调函数的队列, 由双向链表实现, 函数先进先出 */
	std::list<PrtCbFn> m_lFns_0;
	std::list<PrtCbFn> m_lFns_1;
	std::list<PrtCbFn> * m_pRevLFns;
	std::list<PrtCbFn> * m_pRunLFns;
};

/** class Pump [Pump.cpp]
 *
 * ! 	Pump 对象名字源于强健有力的“泵”, 正如“泵”的将机械能或其他外部能量传送给液体那样,
 * 		Pump 对象是整个事件循环框架的动力源泉, 它负责驱动 Watcher 对象监听、激活、处理
 * 		事件对象
 * */
class Pump {
public:
	Pump();
	~Pump();

public:
//		void deliver(IN prior, IN function);

private:
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
private:
	//! 存放 Wather 对象指针的数组对象
	WatcherList m_watchers;
	//! 回调函数邮箱
	CbMailbox m_cbMailbox;
};

}



#endif //LIBPUMP_PUMP_H
