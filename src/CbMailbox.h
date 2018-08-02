/**
 * @file CbMailbox.h
 * @brief 定义回调函数邮箱相关对象
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.07.27
 */
#pragma once

#ifndef LIBPUMP_CBMAILBOX_H
#define LIBPUMP_CBMAILBOX_H

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

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace PUMP {

/**
 * @class CbFn [CbMailbox.h]
 *
 * @brief 托管回调函数仿函数对象基类,也是函数列表对象的元素;
 * 任何回调函数需要继承此对象,并实现对象的operator()成员,该成员供CbFnList对象逐个执行回调时调用;
 * 本对象不允许拷贝;
 *
 * - 派生规则:
 * 1. 派生子回调对象时,用户根据回调函数原型,定义 boost::function<...> func_t;
 * 2. 用户为派生对象必须定义参数值成员对象和返回值成员对象,使整个回调对象形成闭包,用户可以通过定义 getReturn() 成员函数获取返回值结果
 *    (建议回调参数为指针时使用 boost::shared_ptr 封装, 避免多线程下的竞态条件发生。另外,由于回调函数的产生和执行是异步的,所以应该
 *    避免栈上对象的指针作为参数传入);
 * 3. 函数的是否具备返回值,在定义类时确定。参数值则尽量在构造时与回调函数对象绑定,一般情况下应该禁止修改函数对象的参数值,
 *    以免发生用户意料之外的执行结果;
 * 4. 派生类实现纯虚函数 operator().
 */
PUMP_ABSTRACT
class CbFn
  : public nsp_boost::noncopyable {
public:
  CbFn(bool bReturn = false)
    : m_bReturn(bReturn) {}
  
  virtual ~CbFn() {}
  
  /**
   * @fn bool isReturnAValue() const
   *
   * @brief 判断回调对象是否具有返回值
   * @return true 表示有返回值
   * */
  inline bool isReturnAValue() const { return m_bReturn; }
  
  /**
   * @fn void operator()()
   *
   * @brief 纯虚函数, 重载"()"运算符
   *
   * 每一个派生于 CbFn 对象的回调都必须实现, 是CbFn作为
   * 函数对象的证明, 也是调用接口
   * */
  virtual void operator()() = 0;

private:
  /**
   * @var bool m_bReturn
   *
   * 标记回调对象是否有返回值, 定义对象时确定
   * */
  const bool m_bReturn;
};

/**
 * @class CbFnWithoutReturn [CbMailbox.h]
 *
 * @brief 派生于 CbFn, 所有无返值回调函数基类
 */
PUMP_ABSTRACT
class CbFnWithoutReturn
  : public CbFn {
public:
  CbFnWithoutReturn()
    : CbFn(false) {}
  
  ~CbFnWithoutReturn() {}
};

/** @class CbFnWithReturn [CbMailbox.h]
 *
 * @brief 派生于 CbFn, 所有有返值回调函数基类
 */
PUMP_ABSTRACT
class CbFnWithReturn
  : public CbFn {
public:
  CbFnWithReturn()
    : CbFn(true) {}
  
  ~CbFnWithReturn() {}
};

/** 指向 CbFn 对象的指针
 *
 * 定义本指针目的在于加入 list<PrtCbFn> 实现回调函数的托管
 * */
typedef nsp_boost::shared_ptr<CbFn> PrtCbFn;

/**　class CbFnContainerManager [CbMailbox.h]
 *
 * 回调函数容器的管理接口, 增删改
 * */
PUMP_INTERFACE
class CbFnContainerManager
  : virtual public nsp_boost::noncopyable {
public:
  CbFnContainerManager() {}
  
  virtual ~CbFnContainerManager() {}
  
  /** 向 m_pRevLFns 尾插入一个回调对象
   *
   * @param[in] pfn 要插入链表的回调对象(注：由于回调对象会被修改,禁止将参数声明为const)
   * @return 插入成功返回true
   * */
  virtual bool insert(PrtCbFn pfn) = 0;
};

/**　class CbFnContainerCaller [CbMailbox.h]
 *
 * 回调函数容器的执行接口
 **/
PUMP_INTERFACE
class CbFnContainerCaller
  : virtual public nsp_boost::noncopyable {
public:
  CbFnContainerCaller() {}
  
  virtual ~CbFnContainerCaller() {}
  
  /** 遍历链表 m_pRunLFns 执行其中回调对象
   *
   * @return 返回执行的回调对象数量
   **/
  virtual size_t runAll() = 0;
};

/** class CbFnContainer [CbMailbox.h]
 *
 * 存放回调函数容器接口类
 * 必须实现接口,并且配上真正的容器存储对象,才是完整的回调函数容器
 * */
PUMP_INTERFACE
class CbFnContainer
  : public CbFnContainerCaller,
    public CbFnContainerManager {
public:
  CbFnContainer() {}
  
  virtual ~CbFnContainer() {}
};

/** class CbFnList [CbMailbox.h]
 *
 * 回调函数链表,回调对象的托管对象.在优先级队列中存放某一个优先级下的所有函数对象
 * 接收任意参数类型, 个数及返回值类型的回调函数对象.
 **/
PUMP_IMPLEMENT
class CbFnList
  : public CbFnContainer {
public:
  CbFnList();
  
  ~CbFnList();
  
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
  std::list<PrtCbFn> *m_pRevLFns;
  std::list<PrtCbFn> *m_pRunLFns;
};

/**　class CbFnContainerManager [CbMailbox.h]
 *
 * 回调函数容器的管理接口, 增删改
 * */
PUMP_INTERFACE
class CbMailboxManager
  : virtual public nsp_boost::noncopyable {
public:
  CbMailboxManager() {}
  
  virtual ~CbMailboxManager() {}
  
  /** 向 m_pRevLFns 尾插入一个回调对象
   *
   * @param[in] prior 回调函数的优先级, 决定放入的容器
   * @param[in] pfn 要放入邮箱的回调对象(注：由于回调对象会被修改,禁止将参数声明为const)
   * @return 插入成功返回true
   * */
  virtual bool insert(ev_prior_t prior, PrtCbFn pfn) = 0;
};

/**　class CbFnContainerCaller [CbMailbox.h]
 *
 * 回调函数容器的执行接口
 **/
PUMP_INTERFACE
class CbMailboxCaller
  : virtual public nsp_boost::noncopyable {
public:
  CbMailboxCaller() {}
  
  virtual ~CbMailboxCaller() {}
  
  /** 遍历链表 m_pRunLFns 执行其中回调对象
   *
   * @return 返回执行的回调对象数量
   **/
  virtual size_t runAll() = 0;
};

typedef nsp_boost::weak_ptr<CbMailboxCaller> WPtrCbMailboxCaller;
typedef nsp_boost::shared_ptr<CbMailboxCaller> PtrCbMailboxCaller;
typedef nsp_boost::weak_ptr<CbMailboxManager> WPtrCbMailboxManager;
typedef nsp_boost::shared_ptr<CbMailboxManager> PtrCbMailboxManager;

/** class CbFnContainer [CbMailbox.h]
 *
 * 存放回调函数容器接口类
 * 必须实现接口,并且配上真正的容器存储对象,才是完整的回调函数容器
 * */
PUMP_INTERFACE
class CbMailbox
  : public CbMailboxCaller,
    public CbMailboxManager {
public:
  
  /** 回调优先级队列元素类型
   *
   * 优先级队列中存放智能指针的好处是内存托管, 另外也因为 CbFnList 是不允许拷贝的,
   * 不符合STL元素对象标准
   * */
  typedef nsp_boost::shared_ptr<CbFnContainer> PtrCbFnContainer;
  
  CbMailbox() {}
  
  virtual ~CbMailbox() {}
};

typedef nsp_boost::weak_ptr<CbMailbox> WPtrCbMailbox;
typedef nsp_boost::shared_ptr<CbMailbox> PtrCbMailbox;

/** class CbMailbox [CbMailbox.h]
 *
 * 回调函数邮箱类, 负责调度回调函数
 * */
PUMP_ABSTRACT
class CbQueueMailbox
  : public CbMailbox {
public:
  
  /** 回调优先级队列对象
   *
   * 由 map 容器实现
   *  map key 值为枚举类型 EventPriority
   *  value 值为回调链表的智能指针
   * */
  typedef nsp_std::map<ev_prior_t, CbMailbox::PtrCbFnContainer> CbPriorQueue;
  
  CbQueueMailbox();
  
  ~CbQueueMailbox();
  
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

/**　class CbFnContainerManager [CbMailbox.h]
 *
 * 回调函数容器的管理接口, 增删改
 * */
PUMP_IMPLEMENT
class ICbMailboxManager
  : virtual public CbMailboxManager {
public:
  explicit ICbMailboxManager(nsp_boost::weak_ptr<CbMailbox> pMailbox);
  
  virtual ~ICbMailboxManager();
  
  /** 向 m_pRevLFns 尾插入一个回调对象
   *
   * @param[in] prior 回调函数的优先级, 决定放入的容器
   * @param[in] pfn 要放入邮箱的回调对象(注：由于回调对象会被修改,禁止将参数声明为const)
   * @return 插入成功返回true
   * */
  bool insert(ev_prior_t prior, PrtCbFn pfn);

private:
  nsp_boost::weak_ptr<CbMailbox> m_pMailbox;
};

/**　class CbFnContainerCaller [CbMailbox.h]
 *
 * 回调函数容器的执行接口
 * */
PUMP_IMPLEMENT
class ICbMailboxCaller
  : public CbMailboxCaller {
public:
  explicit ICbMailboxCaller(nsp_boost::weak_ptr<CbMailbox> pMailbox);
  
  virtual ~ICbMailboxCaller();
  
  /** 遍历链表 m_pRunLFns 执行其中回调对象
   *
    * @return 返回执行的回调对象数量
    * */
  size_t runAll();

private:
  nsp_boost::weak_ptr<CbMailbox> m_pMailbox;
};

}

#endif //LIBPUMP_CBMAILBOX_H
