//
// Created by yz on 18-10-23.
//

#ifndef LIBPUMP_VOIDWPTR_HPP
#define LIBPUMP_VOIDWPTR_HPP

#include <utility>
#include <cstring>
#include <cassert>
#include <iostream>
#include <iomanip>

#include <boost/function.hpp>

#include "block.hpp"
#include "policy.hpp"
#include "voidsptr.hpp"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace Pump {
namespace SmartMem {

bool operator==(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator==(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT;

bool operator!=(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator!=(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT;

bool operator==(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

bool operator!=(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

std::ostream &operator<<(std::ostream &os, VoidSPtr const &p);

class VoidWPtr {
protected:
  
  // Borland 5.5.1 specific workarounds
  typedef VoidWPtr this_type;

public:
  
  VoidWPtr() BOOST_NOEXCEPT // never throws in 1.30+
    : m_block(),
      m_policy() {
  }

//  generated copy constructor, assignment, destructor are fine...

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// ... except in C++0x, move disables the implicit copy
  
  VoidWPtr(VoidWPtr const &r) BOOST_NOEXCEPT
    : m_block(r.m_block),
      m_policy(r.m_policy) {
  }
  
  VoidWPtr &operator=(VoidWPtr &r) BOOST_NOEXCEPT {
    m_block = r.m_block;
    m_policy = r.m_policy;
    return *this;
  }

#endif

//
//  The "obvious" converting constructor implementation:
//
//  template<class Y>
//  VoidWPtr(VoidWPtr<Y> const & r): m_px(r.m_px), m_policy(r.m_policy) // never throws
//  {
//  }
//
//  has a serious problem.
//
//  r.m_px may already have been invalidated. The m_px(r.m_px)
//  conversion may require access to *r.m_px (virtual inheritance).
//
//  It is not possible to avoid spurious access violations since
//  in multithreaded programs r.m_px may be invalidated at any point.
//

//  template<class Y>
//#if !defined( BOOST_SP_NO_SP_CONVERTIBLE )
//
//  VoidWPtr( VoidWPtr<Y> const & r, typename boost::detail::sp_enable_if_convertible<Y,T>::type = boost::detail::sp_empty() )
//
//#else
//
//  VoidWPtr( VoidWPtr<Y> const & r )
//
//#endif
//  BOOST_NOEXCEPT : m_px(r.lock().get()), m_policy(r.m_policy)
//  {
//    boost::detail::sp_assert_convertible< Y, T >();
//  }

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

//  template<class Y>
//#if !defined( BOOST_SP_NO_SP_CONVERTIBLE )
//
//  VoidWPtr( VoidWPtr<Y> && r, typename boost::detail::sp_enable_if_convertible<Y,T>::type = boost::detail::sp_empty() )
//
//#else
//
//  VoidWPtr( VoidWPtr<Y> && r )
//
//#endif
//  BOOST_NOEXCEPT : m_px( r.lock().get() ), m_policy( static_cast< WeakCount && >( r.m_policy ) )
//  {
//    boost::detail::sp_assert_convertible< Y, T >();
//    r.m_px = 0;
//  }
  
  // for better efficiency in the T == Y case
  VoidWPtr(VoidWPtr &&r)
  BOOST_NOEXCEPT : m_block(r.m_block), m_policy(static_cast< WHeapPolicyGuider<> && >( r.m_policy )) {
    r.m_block = HeapGuider<>();
  }
  
  // for better efficiency in the T == Y case
  VoidWPtr &operator=(VoidWPtr &&r) BOOST_NOEXCEPT {
    this_type(static_cast< VoidWPtr && >( r )).swap(*this);
    return *this;
  }


#endif
  VoidWPtr(VoidSPtr const &r) BOOST_NOEXCEPT
    : m_block(r.m_block),
      m_policy(r.m_policy) {
  }
  
  VoidWPtr & operator=(VoidSPtr const &r) BOOST_NOEXCEPT {
    m_block = r.m_block;
    m_policy = r.m_policy;
    return *this;
  }

//  template<class Y>
//#if !defined( BOOST_SP_NO_SP_CONVERTIBLE )
//
//  VoidWPtr( SharedPtr<Y> const & r, typename boost::detail::sp_enable_if_convertible<Y,T>::type = boost::detail::sp_empty() )
//
//#else
//
//  VoidWPtr( SharedPtr<Y> const & r )
//
//#endif
//  BOOST_NOEXCEPT : m_px( r.m_px ), m_policy( r.m_policy )
//  {
//    boost::detail::sp_assert_convertible< Y, T >();
//  }
  
  VoidSPtr lock_raw() const BOOST_NOEXCEPT {
    return VoidSPtr(*this, boost::detail::sp_nothrow_tag());
  }
  
  long use_count() const BOOST_NOEXCEPT {
    return m_policy.use_count();
  }
  
  bool expired() const BOOST_NOEXCEPT {
    return m_policy.use_count() == 0;
  }
  
  bool _empty() const // extension, not in std::VoidWPtr
  {
    return m_policy.empty();
  }
  
  void reset() BOOST_NOEXCEPT // never throws in 1.30+
  {
    this_type().swap(*this);
  }
  
  void swap(this_type &other) BOOST_NOEXCEPT {
    m_block.swap(other.m_block);
    m_policy.swap(other.m_policy);
  }

//  template<class Y> bool owner_before( VoidWPtr<Y> const & rhs ) const BOOST_NOEXCEPT
//  {
//    return m_policy < rhs.m_policy;
//  }
//
//  template<class Y> bool owner_before( SharedPtr<Y> const & rhs ) const BOOST_NOEXCEPT
//  {
//    return m_policy < rhs.m_policy;
//  }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS

protected:
  
  friend class VoidSPtr;

#endif
  /**
   * @var m_block
   * @brief 托管内存对象
   */
  HeapGuider<> m_block;
  /**
 * @var m_policy
 * @brief 引用计数对象
 */
  WHeapPolicyGuider<> m_policy; // reference counter
};

bool operator==(VoidWPtr const &a, VoidWPtr const &b) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  VoidSPtr sp_b = b.lock_raw();
  return (sp_a == sp_b);
}

bool operator!=(VoidWPtr const &a, VoidWPtr const &b) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  VoidSPtr sp_b = b.lock_raw();
  return (sp_a != sp_b);
}

bool operator==(VoidWPtr const &a, nullptr_t) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  return (sp_a == nullptr);
}

bool operator!=(VoidWPtr const &a, nullptr_t) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  return (sp_a != nullptr);
}

bool operator==(nullptr_t, VoidWPtr const &a) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  return (sp_a == nullptr);
}

bool operator!=(nullptr_t, VoidWPtr const &a) BOOST_NOEXCEPT {
  VoidSPtr sp_a = a.lock_raw();
  return (sp_a != nullptr);
}

////////////////////////////////////////////////
//                  VoidSPtr
////////////////////////////////////////////////

VoidSPtr::VoidSPtr(const VoidWPtr &r, boost::detail::sp_nothrow_tag)
  : /*m_pParent(0),*/
  m_block(),
  m_policy(r.m_policy, boost::detail::sp_nothrow_tag())
{
  if(!m_policy.empty()){
    m_block = r.m_block;
  }
}

}
}

#endif //LIBPUMP_VOIDWPTR_HPP
