/******************************************************************************
*   libpump Basic tool library                                                *
*   Copyright (C) 2018 YangZheng  263693992@qq.com.                           *
*                                                                             *
*   This file is part of libpump.                                             *
*                                                                             *
*   This program is free software; you can redistribute it and/or modify      *
*   it under the terms of the GNU General Public License version 3 as         *
*   published by the Free Software Foundation.                                *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with OST. If not, see <http://www.gnu.org/licenses/>.               *
*                                                                             *
*   Unless required by applicable law or agreed to in writing, software       *
*   distributed under the License is distributed on an "AS IS" BASIS,         *
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*   See the License for the specific language governing permissions and       *
*   limitations under the License.                                            *
*                                                                             *
* @file     Pump.h                                                            *
* @brief                                                                      *
*   Details.                                                                  *
*                                                                             *
* @author   YangZheng                                                         *
* @email    263693992@qq.com                                                  *
* @version  1.0.0.0                                                           *
* @date     2018-07-27                                                        *
* @license  GNU General Public License (GPL)                                  *
*                                                                             *
*-----------------------------------------------------------------------------*
* Remark: Description                                                         *
*-----------------------------------------------------------------------------*
* Change History:                                                             *
* <Date>      | <Version> |   <Author>   |  <Description>                     *
*-----------------------------------------------------------------------------*
* 2018/07/27  | 1.0.0.0   |   YangZheng  |  Create file                       *
*-----------------------------------------------------------------------------*
*                                                                             *
******************************************************************************/
#pragma once

#ifndef LIBPUMP_PUMP_H
#define LIBPUMP_PUMP_H

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "pumpdef.h"
#include "Wather.h"
#include "CbMailbox.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

/**
 * @brief PUMP
 * 命名空间的详细概述
 */
namespace PUMP{

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
	explicit Pump(WPtrCbMailboxCaller pMbCaller);
	~Pump();
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

public:
	/**
	 * @brief watching()前置处理
	 * */
	int postWatching();
	/** @} */
private:
	//! 存放 Wather 对象指针的数组对象
	WatcherList m_watchers;
	//! 回调函数邮箱
	WPtrCbMailboxCaller m_wpMbCaller;
	//! 标志 Pump 当前的运行状态
	enum PumpState m_state;
};

}

#endif //LIBPUMP_PUMP_H
