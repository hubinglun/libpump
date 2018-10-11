//
// Created by yz on 18-10-6.
//

#ifndef LIBPUMP_VOIDWPTR_HPP
#define LIBPUMP_VOIDWPTR_HPP

#include <boost/shared_ptr.hpp>

#include "SmartPtr.h"
#include "SharedCount.hpp"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

class VoidSPtr;

bool operator==(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

bool operator!=(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

class VoidWPtr {
protected:
  
  // Borland 5.5.1 specific workarounds
  typedef VoidWPtr this_type;

public:
  
  VoidWPtr() BOOST_NOEXCEPT // never throws in 1.30+
    : m_struBlock(),
      m_pn() {
  }

//  generated copy constructor, assignment, destructor are fine...

#if !defined( BOOST_NO_CXX11_RVALUE_REFERENCES )

// ... except in C++0x, move disables the implicit copy
  
  VoidWPtr(VoidWPtr const &r) BOOST_NOEXCEPT
    : m_struBlock(r.m_struBlock),
      m_pn(r.m_pn) {
  }
  
  VoidWPtr &operator=(VoidWPtr const &r) BOOST_NOEXCEPT {
    m_struBlock = r.m_struBlock;
    m_pn = r.m_pn;
    return *this;
  }
  
  VoidWPtr &operator=(VoidSPtr const &r) BOOST_NOEXCEPT;

#endif

  VoidWPtr(VoidSPtr const &r) BOOST_NOEXCEPT;

//
//  The "obvious" converting constructor implementation:
//
//  template<class Y>
//  VoidWPtr(VoidWPtr<Y> const & r): m_px(r.m_px), m_pn(r.m_pn) // never throws
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
//  BOOST_NOEXCEPT : m_px(r.lock().get()), m_pn(r.m_pn)
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
//  BOOST_NOEXCEPT : m_px( r.lock().get() ), m_pn( static_cast< WeakCount && >( r.m_pn ) )
//  {
//    boost::detail::sp_assert_convertible< Y, T >();
//    r.m_px = 0;
//  }
  
  // for better efficiency in the T == Y case
  VoidWPtr(VoidWPtr &&r)
  BOOST_NOEXCEPT : m_struBlock(r.m_struBlock), m_pn(static_cast< WeakCount && >( r.m_pn )) {
    r.m_struBlock = Block();
  }
  
  // for better efficiency in the T == Y case
  VoidWPtr &operator=(VoidWPtr &&r) BOOST_NOEXCEPT {
    this_type(static_cast< VoidWPtr && >( r )).swap(*this);
    return *this;
  }


#endif

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
//  BOOST_NOEXCEPT : m_px( r.m_px ), m_pn( r.m_pn )
//  {
//    boost::detail::sp_assert_convertible< Y, T >();
//  }
  
  VoidSPtr lock_raw() const BOOST_NOEXCEPT;
//  {
//    return VoidSPtr(*this,boost::detail::sp_nothrow_tag());
//  }
  
  long use_count() const BOOST_NOEXCEPT {
    return m_pn.use_count();
  }
  
  bool expired() const BOOST_NOEXCEPT {
    return m_pn.use_count() == 0;
  }
  
  bool _empty() const // extension, not in std::VoidWPtr
  {
    return m_pn.empty();
  }
  
  void reset() BOOST_NOEXCEPT // never throws in 1.30+
  {
    this_type().swap(*this);
  }
  
  void swap(this_type &other) BOOST_NOEXCEPT {
    m_struBlock.swap(other.m_struBlock);
    m_pn.swap(other.m_pn);
  }

//  template<class Y> bool owner_before( VoidWPtr<Y> const & rhs ) const BOOST_NOEXCEPT
//  {
//    return m_pn < rhs.m_pn;
//  }
//
//  template<class Y> bool owner_before( SharedPtr<Y> const & rhs ) const BOOST_NOEXCEPT
//  {
//    return m_pn < rhs.m_pn;
//  }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS

protected:
  
  friend class VoidSPtr;

#endif
  /**
   * @var m_struBlock
   * @brief 托管内存对象
   */
  Block m_struBlock;
  /**
 * @var m_pn
 * @brief 引用计数对象
 */
  WeakCount m_pn; // reference counter
};

}
}

#endif //LIBPUMP_VOIDWPTR_HPP
