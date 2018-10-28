//
// Created by yz on 18-10-28.
//

#ifndef LIBPUMP_WEAKPTR_HPP
#define LIBPUMP_WEAKPTR_HPP

#include "voidsptr.hpp"
#include "voidwptr.hpp"
#include "sharedptr.hpp"

namespace nsp_boost = boost;
namespace nsp_std = std;

namespace Pump {
namespace SmartMem {

template <class T>
class WeakPtr
  : public VoidWPtr {
protected:
  typedef WeakPtr<T> this_type;
public:
  
  typedef typename nsp_boost::detail::sp_element<T>::type element_type;
  
  WeakPtr() BOOST_NOEXCEPT
    : VoidWPtr(),
      m_state(SP_STATE_NULL){
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "WeakPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  explicit WeakPtr(WeakPtr const &r) BOOST_NOEXCEPT
    : VoidWPtr(static_cast<VoidWPtr const &>(r)),
      m_state(r.m_state){
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "WeakPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template <class Y>
  explicit WeakPtr(SharedPtr<Y> const &r) BOOST_NOEXCEPT
    : VoidWPtr(static_cast<VoidSPtr const &>(r)),
      m_state(r.m_state) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "WeakPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  virtual ~WeakPtr() {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "~WeakPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template<class Y>
  WeakPtr<T> &operator=(WeakPtr<Y> const &r) {
    VoidWPtr::operator=(static_cast<VoidWPtr const &>(r));
    m_state = (r.m_state);
    return *this;
  }
  
  template<class Y>
  WeakPtr<T> & operator=(SharedPtr<Y> const & r) BOOST_NOEXCEPT
  {
    VoidWPtr::operator=(static_cast<VoidSPtr const &>(r));
    m_state = (r.m_state);
    return *this;
  }
  
  SharedPtr<T> lock() const BOOST_NOEXCEPT
  {
    SharedPtr<T> sp = SharedPtr<T>(lock_raw());
    if(sp != nullptr)
    {
      sp.m_state = m_state;
    }
    else {
      sp.m_state = SP_STATE_NULL;
    }
    return sp;
  }
protected:
  enum SpState m_state; //! 指针生命周期
};

}
}

#endif //LIBPUMP_WEAKPTR_HPP
