#ifndef LIBPUMP_SAHREDCOUNT_HPP
#define LIBPUMP_SAHREDCOUNT_HPP

#include <boost/smart_ptr/detail/sp_counted_base.hpp>
#include <boost/smart_ptr/detail/sp_counted_impl.hpp>

#include "SmartPtr.h"
#include "SpCountedImpl.hpp"
#include "AdjacencyNode.hpp"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

//template <class A=std::allocator<char> >

class SharedCount {
private:
  
  nsp_boost::detail::sp_counted_base *pi_;
  
  AdjacencyNode * pAdj_;

//  friend class weak_count;

public:
  
  SharedCount() : pi_(0), pAdj_(0) // nothrow
  {
  }
  
  template<class A>
  explicit SharedCount(void *p, size_t iSize, A &alloc)
    : pi_(0),
      pAdj_(0){
    try {
      pi_ = (SpCountedImpl_a<A>*)alloc.allocate(sizeof(SpCountedImpl_a<A>));
      ::new(pi_)SpCountedImpl_a<A>(p, iSize, alloc);
      // FIXME [general] 应该改为使用分配器构造,但是SpCountedImpl_a基类sp_counted_base不允许, 需要改造
//      pi_ = new SpCountedImpl_a<A>(p, iSize, alloc);
    }
    catch (...) {
      // 引用计数器构造失败, 则需要删除对象
      alloc.deallocate((typename A::pointer)p, iSize);
      throw;
    }
    try {
      pAdj_ = (AdjacencyNode*)alloc.allocate(sizeof(AdjacencyNode_a<A>));
      ::new(pAdj_)AdjacencyNode_a<A>(alloc);
    }
    catch (...) {
      // 引用计数器构造失败, 则需要删除对象
      alloc.deallocate((typename A::pointer)pi_, sizeof(SpCountedImpl_a<A>));
      alloc.deallocate((typename A::pointer)p, iSize);
      throw;
    }
  }
  
  template<class A, class D>
  SharedCount(void *p, size_t iSize, A &alloc, D d)
    : pi_(0), pAdj_(0) {
    try {
      pi_ = (SpCountedImpl_ad<A, D>*)alloc.allocate(sizeof(SpCountedImpl_ad<A, D>));
      ::new(pi_)SpCountedImpl_ad<A, D>(p,iSize, alloc, d);
      // FIXME [general] 应该改为使用分配器构造,但是SpCountedImpl_ad基类sp_counted_base不允许, 需要改造
//      pi_ = new SpCountedImpl_ad<A, D>(p,iSize, alloc, d);
    }
    catch (...) {
      d(p, iSize, alloc); // delete p
      throw;
    }
    try {
      pAdj_ = (AdjacencyNode*)alloc.allocate(sizeof(AdjacencyNode_a<A>));
      ::new(pAdj_)AdjacencyNode_a<A>(alloc);
    }
    catch (...) {
      // 引用计数器构造失败, 则需要删除对象
      alloc.deallocate((typename A::pointer)pi_, sizeof(SpCountedImpl_ad<A, D>));
      alloc.deallocate((typename A::pointer)p, iSize);
      throw;
    }
  }
  
  ~SharedCount() // nothrow
  {
    if (pi_ != 0) pi_->release();
    if (pAdj_ != 0 && pi_->use_count()==0) pAdj_->release();
  }
  
  SharedCount(SharedCount const &r) : pi_(r.pi_), pAdj_(r.pAdj_) // nothrow
  {
    if (pi_ != 0) pi_->add_ref_copy();
  }
  
  SharedCount(SharedCount &&r) : pi_(r.pi_), pAdj_(r.pAdj_) // nothrow
  {
    r.pi_ = 0;
  }

//  explicit SharedCount(weak_count const & r); // throws bad_weak_ptr when r.use_count() == 0
//  SharedCount( weak_count const & r, nsp_boost::detail::sp_nothrow_tag ); // constructs an empty *this when r.use_count() == 0
  
  SharedCount &operator=(SharedCount const &r) // nothrow
  {
    nsp_boost::detail::sp_counted_base *tmp = r.pi_;
    bool bDel = false;
    if (tmp != pi_) {
      if (tmp != 0) tmp->add_ref_copy();
      if (pi_ != 0) {
        if(pi_->use_count()==1) {
          bDel = true;
        }
        pi_->release();
      }
      pi_ = tmp;
    }
  
    AdjacencyNode *tmp2 = r.pAdj_;
  
    if (tmp2 != pAdj_) {
      if (pAdj_ != 0 && bDel) pAdj_->release();
      pAdj_ = tmp2;
    }
    
    return *this;
  }
  
  void addIndegree(const void *in) {
    pAdj_->addIndegree(in);
  }
  
  void addOutdegree(const void *out) {
    pAdj_->addOutdegree(out);
  }
  
  void swap(SharedCount &r) // nothrow
  {
    nsp_boost::detail::sp_counted_base *tmp = r.pi_;
    r.pi_ = pi_;
    pi_ = tmp;
  
    AdjacencyNode * tmp2 = r.pAdj_;
    r.pAdj_ = pAdj_;
    pAdj_ = tmp2;
  }
  
  long use_count() const // nothrow
  {
    return pi_ != 0 ? pi_->use_count() : 0;
  }
  
  bool unique() const // nothrow
  {
    return use_count() == 1;
  }
  
  bool empty() const // nothrow
  {
    return pi_ == 0;
  }
  
  friend inline bool operator==(SharedCount const &a, SharedCount const &b) {
    return a.pi_ == b.pi_;
  }
  
  friend inline bool operator<(SharedCount const &a, SharedCount const &b) {
    return std::less<nsp_boost::detail::sp_counted_base *>()(a.pi_, b.pi_);
  }

//  void * get_deleter( sp_typeinfo const & ti ) const
//  {
//    return pi_? pi_->get_deleter( ti ): 0;
//  }
  
  void *get_untyped_deleter() const {
    return pi_ ? pi_->get_untyped_deleter() : 0;
  }
};

}
}

#endif //LIBPUMP_SAHREDCOUNT_HPP