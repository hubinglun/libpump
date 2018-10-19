#ifndef LIBPUMP_SPCOUNTEDIMPL_HPP
#define LIBPUMP_SPCOUNTEDIMPL_HPP

#include <boost/smart_ptr/detail/sp_counted_base.hpp>
#include <functional>

#include "SmartPtr.h"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

class SPCountBase
  : public nsp_boost::detail::sp_counted_base {
public:
  typedef nsp_boost::function<void(void*)> func_t;
  func_t fn_destroy;
};

template<class A, class D>
class SpCountedImpl_ad
  : public SPCountBase
{
private:
  
  void * p_; // copy constructor must not throw
  size_t iSize_; // p_ 指针指向内存大小
  D d_; // copy constructor must not throw
  A a_; // copy constructor must not throw
  
  SpCountedImpl_ad( SpCountedImpl_ad const & );
  SpCountedImpl_ad & operator= ( SpCountedImpl_ad const & );
  
  typedef SpCountedImpl_ad<D, A> this_type;

public:
  
  // pre: d( p ) must not throw
  
  SpCountedImpl_ad( void * p, size_t iSize, A & a, D d )
    : p_( p ), iSize_(iSize), d_( d ), a_( a )
  {
  }
  
  SpCountedImpl_ad( void * p, size_t iSize, D d )
    : p_( p ), iSize_(iSize), d_( d ), a_( getAlloc() )
  {
  }
  
  virtual void dispose() // nothrow
  {
    // 调用析构函数
    if(!fn_destroy.empty())
      fn_destroy(p_);
    // 调用删除器归还托管数据
    d_( p_ , iSize_, a_ );
  }
  
  virtual void destroy() // nothrow
  {
    /** FIXME [urgency] 下面代码有问题, 并且暂时不清楚本函数的作用, 已知在sp_counted_base::weak_release()被调用 \
     * 这个函数应该是用于销毁 SpCountedImpl_ad 对象自身, 但是为什么使用std::allocator销毁, 这点暂时不清楚, 可能是因为带 \
     * 有删除器, 就默认为 SpCountedImpl_ad 对象是由分配器构造的, 所以销毁时自动归还分配器
     */
    typedef typename std::allocator_traits<A>::template rebind_alloc< this_type > A2;
//
    A2 a2( a_ );
    std::allocator_traits<A2>::destroy( a2, this ); // 调用本对象析构函数
//    a2.deallocate( this, 1 );
    a2.deallocate((typename A2::pointer)this, sizeof(SpCountedImpl_ad<A,D>)); // 归还内存给分配器
  }
  
  virtual void * get_deleter( nsp_boost::detail::sp_typeinfo const & ti )
  {
//    return ti == BOOST_SP_TYPEID( D )? &reinterpret_cast<char&>( d_ ): 0;
    return 0;
  }
  
  virtual void * get_untyped_deleter()
  {
//    return &reinterpret_cast<char&>( d_ );
    return 0;
  }
};

template<class A> 
class SpCountedImpl_a
  : public SPCountBase
{
private:
  
  void * p_; // copy constructor must not throw
  size_t iSize_; // p_ 指针指向内存大小
  A a_; // copy constructor must not throw
  
  SpCountedImpl_a( SpCountedImpl_a const & );
  SpCountedImpl_a & operator= ( SpCountedImpl_a const & );
  
  typedef SpCountedImpl_a<A> this_type;

public:
  
  // pre: d(p) must not throw
  
  SpCountedImpl_a( void * p, size_t iSize, A & a )
    : p_( p ), iSize_(iSize), a_( a )
  {
  }
  
  SpCountedImpl_a( void * p, size_t iSize )
    : p_( p ), iSize_(iSize),  a_(getAlloc())
  {
  }
  
  virtual void dispose() // nothrow
  {
    // 调用析构函数
    if(!fn_destroy.empty())
      fn_destroy(p_);
    // 回收托管内存
    typedef typename std::allocator_traits<A>::template rebind_alloc< this_type > A2;
    A2 a2(a_);
    a2.deallocate((typename A2::pointer)p_, iSize_);
  }
  
  virtual void destroy() // nothrow
  {
    /** FIXME [urgency] 下面代码有问题, 并且暂时不清楚本函数的作用, 已知在sp_counted_base::weak_release()被调用 \
     * 这个函数应该是用于销毁 SpCountedImpl_ad 对象自身, 但是为什么使用std::allocator销毁, 这点暂时不清楚, 可能是因为带 \
     * 有删除器, 就默认为 SpCountedImpl_ad 对象是由分配器构造的, 所以销毁时自动归还分配器
     */
    typedef typename std::allocator_traits<A>::template rebind_alloc< this_type > A2;

    A2 a2( a_ );
    std::allocator_traits<A2>::destroy( a2, this );
//    a2.deallocate( this, 1 );
    a2.deallocate((typename A2::pointer)this, sizeof(SpCountedImpl_a<A>));
  }
  
  virtual void * get_deleter( nsp_boost::detail::sp_typeinfo const & ti )
  {
    return 0;
  }
  
  virtual void * get_untyped_deleter()
  {
    return 0;
  }
};

}
}

#endif //LIBPUMP_SPCOUNTEDIMPL_HPP
