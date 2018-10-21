//
// Created by yz on 18-10-21.
//

#ifndef LIBPUMP_POLICY_HPP
#define LIBPUMP_POLICY_HPP

#include <utility>

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/detail/sp_counted_base.hpp>
#include <boost/detail/sp_typeinfo.hpp>
#include <boost/function.hpp>

#include "block.hpp"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;
namespace nsp_b_detail = ::boost::detail;

namespace Pump {
namespace SmartMem {

/**
 * @class Policy
 * @brief 托管内存的管理(释放)策略对象基类
 *  \a注：本对象及其派生对象必为堆对象。每一个实例进程内唯一，且必定绑定一个Block对象， \
 *       Policy对Block有销毁权限
 */
class Policy
  : public nsp_boost::noncopyable {
public:
  /**
   * @var m_px
   * @brief 内存描述块的地址 \
   *    \a本对象及其派生只允许赋值和释放
   */
  Block *m_px;
  
  explicit Policy(Block *px) : m_px(px) {}
  
  virtual ~Policy() {}
  
  /**
   * @fn destroy
   * @brief 销毁自身
   */
  virtual void destroy() = 0; // nothrow
  /**
   * @fn release
   * @brief 释放Block*的方法
   */
  virtual void release() = 0; // nothrow

private:
  Policy();
};

template<typename _Ax = nsp_std::allocator<char> >
class SHeapPolicy
  : public Policy {
private:
  
  SHeapPolicy(SHeapPolicy const &);
  
  SHeapPolicy &operator=(SHeapPolicy const &);
  
  SHeapPolicy(Block *px);
  
  int use_count_;        // #shared
  int weak_count_;       // #weak + (#shared != 0)

public:
  typedef typename _Ax::template rebind<char>::other _Alty;
  typedef Heap <_Ax> type_block;
  typedef nsp_boost::function<void(void *)> func_t;
  
  /**
   * @var fn_destruct
   * @brief 用户申请内存后，可能会基于该地址内存中调用构造函数
   */
  func_t fn_destruct;
  
  explicit SHeapPolicy(type_block *px)
    : Policy(static_cast<Block *>(px)),
      use_count_(1),
      weak_count_(1),
      fn_destruct() {
  }
  
  virtual ~SHeapPolicy() // nothrow
  {
  }
  
  // dispose() is called when use_count_ drops to zero, to release
  // the resources managed by *this.
  
  virtual void dispose() = 0; // nothrow
  
  // destroy() is called when weak_count_ drops to zero.
  
  virtual void destroy() // nothrow
  {
    delete this;
  }
  
  virtual void *get_deleter(nsp_b_detail::sp_typeinfo const &ti) = 0;
  
  virtual void *get_untyped_deleter() = 0;
  
  void add_ref_copy() {
    nsp_b_detail::atomic_increment(&use_count_);
  }
  
  bool add_ref_lock() // true on success
  {
    return nsp_b_detail::atomic_conditional_increment(&use_count_) != 0;
  }
  
  virtual void release() // nothrow
  {
    if (nsp_b_detail::atomic_exchange_and_add(&use_count_, -1) == 1) {
      dispose();
      weak_release();
    }
  }
  
  void weak_add_ref() // nothrow
  {
    nsp_b_detail::atomic_increment(&weak_count_);
  }
  
  void weak_release() // nothrow
  {
    if (nsp_b_detail::atomic_exchange_and_add(&weak_count_, -1) == 1) {
      destroy();
    }
  }
  
  long use_count() const // nothrow
  {
    return static_cast<int const volatile &>( use_count_ );
  }
  
};

template<class _Ax, class _D>
class SHeapPolicyImpl_ad
  : public SHeapPolicy<_Ax> {
private:
  _Ax a_; // copy constructor must not throw
  _D d_; // copy constructor must not throw
  
  SHeapPolicyImpl_ad(SHeapPolicyImpl_ad const &);
  
  SHeapPolicyImpl_ad &operator=(SHeapPolicyImpl_ad const &);
  
  typedef SHeapPolicyImpl_ad<_Ax, _D> this_type;
  typedef typename SHeapPolicy<_Ax>::type_block type_block;

public:
  
  // pre: d( p ) must not throw
  
  SHeapPolicyImpl_ad(type_block *p, _Ax &a, _D d)
    : SHeapPolicy<_Ax>(p), a_(a), d_(d) {
  }
  
  SHeapPolicyImpl_ad(type_block *p, size_t iSize, _D d)
    : SHeapPolicy<_Ax>(p), a_(), d_(d) {
  }
  
  // 销毁托管内存对象
  virtual void dispose() // nothrow
  {
    // 1)调用析构函数
    if (!this->fn_destruct.empty())
      this->fn_destruct(this->m_px);
    // 2)调用删除器归还托管数据
    d_(this->m_px);
    // 3)回收内存描述块
    typedef typename std::allocator_traits<_Ax>::template rebind_alloc<type_block> A2;
    A2 a2(a_);
    a2.deallocate((typename A2::pointer) this->m_px, 1);
    this->m_px = 0;
  }
  
  // 销毁本对象
  virtual void destroy() // nothrow
  {
    /** [ASK] 下面代码有问题, 并且暂时不清楚本函数的作用, 已知在sp_counted_base::weak_release()被调用 \
     * 这个函数应该是用于销毁 SHeapPolicyImpl_ad 对象自身, 但是为什么使用std::allocator销毁, 这点暂时不清楚, 可能是因为带 \
     * 有删除器, 就默认为 SHeapPolicyImpl_ad 对象是由分配器构造的, 所以销毁时自动归还分配器
     *
     * [ANSWER]  allocator_traits::rebind_alloc重新为allocator设定分配单位
     */
    typedef typename nsp_std::allocator_traits<_Ax>::template rebind_alloc<this_type> A2;
//
    A2 a2(a_);
    std::allocator_traits<A2>::destroy(a2, this); // 调用本对象析构函数
    a2.deallocate((typename A2::pointer) this, 1); // 归还内存给分配器
  }
  
  virtual void *get_deleter(nsp_boost::detail::sp_typeinfo const &ti) {
//    return ti == BOOST_SP_TYPEID( _D )? &reinterpret_cast<char&>( d_ ): 0;
    return 0;
  }
  
  virtual void *get_untyped_deleter() {
//    return &reinterpret_cast<char&>( d_ );
    return 0;
  }
};

template<class _Ax>
class SHeapPolicyImpl_a
  : public SHeapPolicy<_Ax> {
private:
  _Ax a_; // copy constructor must not throw
  
  SHeapPolicyImpl_a(SHeapPolicyImpl_a const &);
  
  SHeapPolicyImpl_a &operator=(SHeapPolicyImpl_a const &);
  
  typedef SHeapPolicyImpl_a<_Ax> this_type;
  typedef typename SHeapPolicy<_Ax>::type_block type_block;

public:
  
  // pre: d(p) must not throw
  
  SHeapPolicyImpl_a(type_block *p, _Ax &a)
    : SHeapPolicy<_Ax>(p), a_(a) {
  }
  
  SHeapPolicyImpl_a(type_block *p)
    : SHeapPolicy<_Ax>(p), a_() {
  }
  
  virtual void dispose() // nothrow
  {
    // 1)调用析构函数
    if (!this->fn_destruct.empty())
      this->fn_destruct(this->m_px);
    // 2)回收托管内存
    this->m_px->deallocate();
    // 3)回收内存描述块
    typedef typename std::allocator_traits<_Ax>::template rebind_alloc<type_block> A2;
    A2 a2(a_);
    a2.deallocate((typename A2::pointer) this->m_px, 1);
    this->m_px = 0;
  }
  
  virtual void destroy() // nothrow
  {
    /** FIXME [urgency] 下面代码有问题, 并且暂时不清楚本函数的作用, 已知在sp_counted_base::weak_release()被调用 \
     * 这个函数应该是用于销毁 SHeapPolicyImpl_ad 对象自身, 但是为什么使用std::allocator销毁, 这点暂时不清楚, 可能是因为带 \
     * 有删除器, 就默认为 SHeapPolicyImpl_ad 对象是由分配器构造的, 所以销毁时自动归还分配器
     */
    typedef typename std::allocator_traits<_Ax>::template rebind_alloc<this_type> A2;
    
    A2 a2(a_);
    std::allocator_traits<A2>::destroy(a2, this);
    a2.deallocate((typename A2::pointer) this, 1);
  }
  
  virtual void *get_deleter(nsp_boost::detail::sp_typeinfo const &ti) {
    return 0;
  }
  
  virtual void *get_untyped_deleter() {
    return 0;
  }
};

/**
 * @class PolicyGuider
 * @brief Policy管理对象，功能与BlockGuider类似。同样不包含申请Policy内存的操作
 */
class PolicyGuider {
protected:
  Policy *m_pp;
public:
  PolicyGuider() : m_pp(0) {}
  
  PolicyGuider(const PolicyGuider &r) : m_pp(r.m_pp) {}
  
  virtual ~PolicyGuider() {}
  
  bool empty() const {
    return (m_pp == 0);
  }
  
  PolicyGuider &operator=(const PolicyGuider &r) {
    m_pp = (r.m_pp);
    return *this;
  }
  
  void swap(PolicyGuider &r) {
    std::swap(m_pp, r.m_pp);
  }
  
};

class WHeapPolicyGuider;

template<typename _Ax>
class SHeapPolicyGuider
  : public PolicyGuider {
private:
  typedef SHeapPolicy<_Ax> type_policy_base;
  typedef typename type_policy_base::type_block type_block;
  typedef typename _Ax::template rebind<char>::other _Alty;
#define p_policy(var) static_cast<type_policy_base*>(var)
  
  friend class WHeapPolicyGuider;

protected:
  _Alty a_;
public:
  SHeapPolicyGuider() : PolicyGuider() {};
  
  explicit SHeapPolicyGuider(type_block *p, _Ax &a)
    : PolicyGuider()/*,
      pAdj_(0)*/,
      a_(a) {
    try {
      m_pp = (SHeapPolicyImpl_a<_Ax> *) a_.allocate(sizeof(SHeapPolicyImpl_a<_Ax>));
      a_.construct(m_pp, p, a);
//      ::new(m_pp)SHeapPolicyImpl_a<_Ax>(p, iSize, alloc);
      // FIXME [general] 应该改为使用分配器构造,但是SpCountedImpl_a基类sp_counted_base不允许, 需要改造
//      m_pp = new SHeapPolicyImpl_a<_Ax>(p, iSize, alloc);
    }
    catch (...) {
      // 引用计数器构造失败, 则需要删除对象
      a_.deallocate((typename _Alty::pointer) p, sizeof(SHeapPolicyImpl_a<_Ax>));
      throw;
    }
//    try {
//      pAdj_ = (AdjacencyNode *) alloc.allocate(sizeof(AdjacencyNode_a<_Ax>));
//      ::new(pAdj_)AdjacencyNode_a<_Ax>(alloc);
//    }
//    catch (...) {
//      // 引用计数器构造失败, 则需要删除对象
//      alloc.deallocate((typename _Ax::pointer) m_pp, sizeof(SHeapPolicyImpl_a<_Ax>));
//      alloc.deallocate((typename _Ax::pointer) p, iSize);
//      throw;
//    }
  }
  
  explicit SHeapPolicyGuider(HeapGuider <_Ax> &p)
    : PolicyGuider()/*,
      pAdj_(0)*/,
      a_(p.get_alloc()) {
    try {
      m_pp = (SHeapPolicyImpl_a<_Ax> *) a_.allocate(sizeof(SHeapPolicyImpl_a<_Ax>));
//      a_.construct(m_pp, p.get_block_priv());
      ::new(m_pp)SHeapPolicyImpl_a<_Ax>(p.get_block_priv());
    }
    catch (...) {
      // 引用计数器构造失败, 则需要删除对象
      a_.deallocate((typename _Alty::pointer) p.get_block_priv(), sizeof(SHeapPolicyImpl_a<_Ax>));
      throw;
    }
//    try {
//      pAdj_ = (AdjacencyNode *) alloc.allocate(sizeof(AdjacencyNode_a<_Ax>));
//      ::new(pAdj_)AdjacencyNode_a<_Ax>(alloc);
//    }
//    catch (...) {
//      // 引用计数器构造失败, 则需要删除对象
//      alloc.deallocate((typename _Ax::pointer) m_pp, sizeof(SHeapPolicyImpl_a<_Ax>));
//      alloc.deallocate((typename _Ax::pointer) p, iSize);
//      throw;
//    }
  }
  
  template<class D>
  SHeapPolicyGuider(type_block *p, _Ax &a, D d)
    : PolicyGuider()/*,
      pAdj_(0)*/,
      a_(a) {
    try {
      m_pp = (SHeapPolicyImpl_ad<_Ax, D> *) a_.allocate(sizeof(SHeapPolicyImpl_ad<_Ax, D>));
//      a_.construct(m_pp, p, a, d);
      ::new(m_pp)SHeapPolicyImpl_ad<_Ax, D>(p.get_block_priv(), a, d);
      // FIXME [general] 应该改为使用分配器构造,但是SpCountedImpl_ad基类sp_counted_base不允许, 需要改造
//      m_pp = new SHeapPolicyImpl_ad<_Ax, D>(p,iSize, alloc, d);
    }
    catch (...) {
      d(p, a); // delete p
      throw;
    }
//    try {
//      pAdj_ = (AdjacencyNode *) alloc.allocate(sizeof(AdjacencyNode_a<_Ax>));
//      ::new(pAdj_)AdjacencyNode_a<_Ax>(alloc);
//    }
//    catch (...) {
//      // 引用计数器构造失败, 则需要删除对象
//      alloc.deallocate((typename _Ax::pointer) m_pp, sizeof(SHeapPolicyImpl_ad<_Ax, D>));
//      alloc.deallocate((typename _Ax::pointer) p, iSize);
//      throw;
//    }
  }
  
  template<class D>
  SHeapPolicyGuider(HeapGuider <_Ax> &p, D d)
    : PolicyGuider()/*,
      pAdj_(0)*/,
      a_(p.get_alloc()) {
    try {
      m_pp = (SHeapPolicyImpl_ad<_Ax, D> *) a_.allocate(sizeof(SHeapPolicyImpl_ad<_Ax, D>));
//      a_.construct(m_pp, p.get_block_priv(), a, d);
      ::new(m_pp)SHeapPolicyImpl_ad<_Ax, D>(p.get_block_priv(), p.get_alloc(), d);
    }
    catch (...) {
      d(p.get_block_priv()); // delete p
      throw;
    }
//    try {
//      pAdj_ = (AdjacencyNode *) alloc.allocate(sizeof(AdjacencyNode_a<_Ax>));
//      ::new(pAdj_)AdjacencyNode_a<_Ax>(alloc);
//    }
//    catch (...) {
//      // 引用计数器构造失败, 则需要删除对象
//      alloc.deallocate((typename _Ax::pointer) m_pp, sizeof(SHeapPolicyImpl_ad<_Ax, D>));
//      alloc.deallocate((typename _Ax::pointer) p, iSize);
//      throw;
//    }
  }
  
  virtual ~SHeapPolicyGuider() // nothrow
  {
    if (m_pp != 0) p_policy(m_pp)->release();
    // FIXME [urgent] 目前邻接点存在bug，会导致崩溃，暂时不知道原因。先屏蔽下代码
//    if (pAdj_ != 0 && m_pp != 0 && m_pp->use_count() == 0) pAdj_->release();
  }
  
  SHeapPolicyGuider(SHeapPolicyGuider const &r) : PolicyGuider(r)/*, pAdj_(r.pAdj_)*/ // nothrow
  {
    if (m_pp != 0) p_policy(m_pp)->add_ref_copy();
  }
  
  SHeapPolicyGuider(SHeapPolicyGuider &&r) : PolicyGuider(r)/*, pAdj_(r.pAdj_)*/ // nothrow
  {
    r.m_pp = 0;
  }

//  explicit SHeapPolicyGuider(WHeapPolicyGuider const &r); // throws bad_weak_ptr when r.use_count() == 0
//  SHeapPolicyGuider(WHeapPolicyGuider const &r,
//              nsp_boost::detail::sp_nothrow_tag); // constructs an empty *this when r.use_count() == 0
//
  SHeapPolicyGuider &operator=(SHeapPolicyGuider const &r) // nothrow
  {
    SHeapPolicyGuider *tmp = r.m_pp;
    bool bDel = false;
    if (tmp != m_pp) {
      if (tmp != 0) tmp->add_ref_copy();
      if (m_pp != 0) {
        if (p_policy(m_pp)->use_count() == 1) {
          bDel = true;
        }
        p_policy(m_pp)->release();
      }
      m_pp = tmp;
    }

//    AdjacencyNode *tmp2 = r.pAdj_;
//
//    if (tmp2 != pAdj_) {
//      if (pAdj_ != 0 && bDel) pAdj_->release();
//      pAdj_ = tmp2;
//    }
    
    return *this;
  }

//  void addIndegree(const void *in) {
//    pAdj_->addIndegree(in);
//  }
//
//  void addOutdegree(const void *out) {
//    pAdj_->addOutdegree(out);
//  }
  
  void swap(SHeapPolicyGuider &r) // nothrow
  {
    PolicyGuider::swap(r);
    nsp_std::swap(a_, r.a_);

//    AdjacencyNode *tmp2 = r.pAdj_;
//    r.pAdj_ = pAdj_;
//    pAdj_ = tmp2;
  }
  
  long use_count() const // nothrow
  {
    return (m_pp != 0 ? p_policy(m_pp)->use_count() : 0);
  }
  
  bool unique() const // nothrow
  {
    return (use_count() == 1);
  }
  
  friend inline bool operator==(SHeapPolicyGuider const &a, SHeapPolicyGuider const &b) {
    return (a.m_pp == b.m_pp);
  }
  
  friend inline bool operator<(SHeapPolicyGuider const &a, SHeapPolicyGuider const &b) {
    return std::less<SHeapPolicyGuider *>()(a.m_pp, b.m_pp);
  }

//  void * get_deleter( sp_typeinfo const & ti ) const
//  {
//    return m_pp? m_pp->get_deleter( ti ): 0;
//  }
  
  void *get_untyped_deleter() const {
    return (m_pp ? p_policy(m_pp)->get_untyped_deleter() : 0);
  }

public:
  typename type_policy_base::func_t &fn_destruct() { return p_policy(m_pp)->fn_destruct; }

#undef p_policy
};

}
}

#endif //LIBPUMP_POLICY_HPP
