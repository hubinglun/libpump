#ifndef LIBPUMP_SHAREDPTR_H
#define LIBPUMP_SHAREDPTR_H

#include <boost/shared_ptr.hpp>

#include "SmartPtr.h"
#include "VoidPtr.hpp"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

enum SpState {
  SP_STATE_NULL,   //! 空指针
  SP_STATE_NEW,    //! 分配空间
  SP_STATE_INIT,    //! 已构造
};

#define XXX nullptr

template<class T>
class SharedPtr
  : public VoidPtr {
protected:
  typedef SharedPtr<T> this_type;
public:
  
  typedef typename nsp_boost::detail::sp_element<T>::type element_type;
  
  SharedPtr() BOOST_NOEXCEPT
    : VoidPtr(),
      m_state(SP_STATE_NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }


#ifndef BOOST_NO_CXX11_NULLPTR
  
  SharedPtr(boost::detail::sp_nullptr_t/*占位参数*/) BOOST_NOEXCEPT
    : VoidPtr(getAlloc(), sizeof(element_type)),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }

#endif //BOOST_NO_CXX11_NULLPTR
  
  template<class A>
  explicit SharedPtr(A &a, boost::detail::sp_nullptr_t/*占位参数*/) BOOST_NOEXCEPT
    : VoidPtr(a, sizeof(element_type)),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template<class A, class D>
  SharedPtr(A &a, D d) BOOST_NOEXCEPT
    : VoidPtr(a, sizeof(element_type), d),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  SharedPtr(SharedPtr const &r) BOOST_NOEXCEPT
    : VoidPtr(static_cast<VoidPtr const &>(r)),
      m_state(r.m_state) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  virtual ~SharedPtr() {
    if (*this != nullptr
      && m_state==SP_STATE_INIT) {
      destroy();
    }
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "~SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template <class Y>
  SharedPtr &operator=(SharedPtr<Y> const &r) {
    VoidPtr::operator=(static_cast<VoidPtr const &>(r));
    m_state = r.state();
    return *this;
  }
  
  template<class... Args>
  SharedPtr &construct(Args &&...args) {
    if (*this != nullptr
        && m_state == SP_STATE_NEW) {
      ::new(this->rget<element_type>())element_type(nsp_std::forward<Args>(args)...);
      m_state = SP_STATE_INIT;
    }
  }
  
  void destroy() {
    if(typeid(element_type) != typeid(void)) {
      this->rget<element_type>()->~element_type();
      m_state = SP_STATE_NEW;
    }
  }
  
  const element_type &operator*() const {
    return *(VoidPtr::get<element_type>());
  }

  VoidPtr operator&() const {
    return static_cast<VoidPtr>(*this);
  }
  
  element_type *operator->() const {
    return (VoidPtr::get<element_type>());
  }
  
  enum SpState state() const {
    return m_state;
  }

protected:
  enum SpState m_state; //! 指针生命周期
};

}
}

#endif //LIBPUMP_SHAREDPTR_H