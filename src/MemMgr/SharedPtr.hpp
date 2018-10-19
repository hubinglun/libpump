#ifndef LIBPUMP_SHAREDPTR_H
#define LIBPUMP_SHAREDPTR_H

#include <boost/shared_ptr.hpp>

#include "SmartPtr.h"
#include "VoidSPtr.hpp"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

enum SpState {
  SP_STATE_NULL,   //! 空指针
  SP_STATE_NEW,    //! 分配空间
  SP_STATE_INIT,   //! 已构造
};

/**
 * @def XXX占位符
 */
#define XXX nullptr

/**
 * FIXME 迫切需要工厂函数
 * @def New(type,name,...)
 * @brief 为对象申请空间, 并构造
 */
#define New(type,name,...) \
SharedPtr< type > name(XXX); \
name.construct(__VA_ARGS__);

template<class T>
class SharedPtr
  : public VoidSPtr {
protected:
  typedef SharedPtr<T> this_type;
  
  template<class Y> friend class WeakPtr;
public:
  
  typedef typename nsp_boost::detail::sp_element<T>::type element_type;
  
  SharedPtr() BOOST_NOEXCEPT
    : VoidSPtr(),
      m_state(SP_STATE_NULL) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }

#ifndef BOOST_NO_CXX11_NULLPTR
  
  SharedPtr(boost::detail::sp_nullptr_t/*占位参数*/) BOOST_NOEXCEPT
    : VoidSPtr(getAlloc(), sizeof(element_type)),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }

#endif //BOOST_NO_CXX11_NULLPTR
  
  explicit SharedPtr(const VoidSPtr &r)
    : VoidSPtr(r),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template<class A>
  explicit SharedPtr(A &a, boost::detail::sp_nullptr_t/*占位参数*/) BOOST_NOEXCEPT
    : VoidSPtr(a, sizeof(element_type)),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template<class A, class D>
  SharedPtr(A &a, D d) BOOST_NOEXCEPT
    : VoidSPtr(a, sizeof(element_type), d),
      m_state(SP_STATE_NEW) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  SharedPtr(SharedPtr const &r) BOOST_NOEXCEPT
    : VoidSPtr(static_cast<VoidSPtr const &>(r)),
      m_state(r.m_state) {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  virtual ~SharedPtr() {
    // FIXME [fixed] 不应该默认调用对象的析构函数，将析构函数放到SharedCount中调用，解决线程同步问题
    /**
     * @bug [fixed] 多线程下，如何保证 this->use_count() 到 this->destroy() 调用期间 \
     *      没有其他线程构造了一个SharedPtr，若是此期间其他线程构造了一个SharedPtr则导致不一致
     * @brief 对象的析构函数只有在成功调用构造函数以后才会在对象销毁前调用，因此为了满足线程安全必须将 \
     *        析构函数调用放在内存销毁前一刻，以避免对象销毁到一半，又被其他线程引用造成不一致的现象。 \
     *        因此在构造函数调用成功后注册私有析构函数，而在手动析构后注销私有析构，避免多次析构。
     */
    // 只有当需要释放资源时才调用析构函数
//    if (*this != nullptr
//        && m_state == SP_STATE_INIT) {
//      this->destroy();
//    }

#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "~SharedPtr()";
#endif //_TEST_LEVEL_INFO
  }
  
  template<class Y>
  SharedPtr &operator=(SharedPtr<Y> const &r) {
    VoidSPtr::operator=(static_cast<VoidSPtr const &>(r));
    m_state = r.state();
    return *this;
  }
  
  SharedPtr &operator=(SharedPtr const &r) {
    VoidSPtr::operator=(static_cast<VoidSPtr const &>(r));
    m_state = r.state();
    return *this;
  }
  
  template<class... Args>
  void construct(Args &&...args) {
    if (*this != nullptr
        && m_state == SP_STATE_NEW) {
      ::new(this->rget<element_type>())element_type(nsp_std::forward<Args>(args)...);
      m_state = SP_STATE_INIT;
      // 构造成功则注册私有析构函数
      m_pn.fn_destroy() = nsp_boost::bind(&SharedPtr::destroy_private, _1);
    }
  }
  
  void destroy() {
    if (typeid(element_type) != typeid(void)) {
      this->rget<element_type>()->~element_type();
      m_state = SP_STATE_NEW;
      // 手动析构后，注销私有析构函数，避免二次析构
      m_pn.fn_destroy().clear();
#ifdef _TEST_LEVEL_INFO
      LOG(INFO) << "destroy";
#endif //_TEST_LEVEL_INFO
    }
  }

protected:
  /**
   * @fn destroy_private
   * @param pself 托管内存指针
   * @return void.
   * @brief 为了实现释放托管内存
   */
  static void destroy_private(void * pself) {
    element_type * self = (element_type *)pself;
    if (typeid(element_type) != typeid(void)) {
      self->~element_type();
#ifdef _TEST_LEVEL_INFO
      LOG(INFO) << "destroy_private";
#endif //_TEST_LEVEL_INFO
    }
  }

public:
  const element_type &operator*() const {
    return *(VoidSPtr::get<element_type>());
  }
  
  VoidSPtr operator&() const {
    return static_cast<VoidSPtr>(*this);
  }
  
  element_type *operator->() const {
    if(m_state != SP_STATE_INIT) {
      // FIXME 设置错误码
      LOG(ERROR) << "Object not constructed.";
      throw 0;
    }
    return (VoidSPtr::get<element_type>());
  }
  
  virtual void reset() BOOST_NOEXCEPT {
    if (*this == nullptr) {
      return;
    }
//    this->destroy();
    VoidSPtr::reset();
    m_state = SP_STATE_NULL;
  }
  
  /**
   * @fn ref() [试用阶段]
   * @return element_type
   * @brief 返回
   */
  element_type & ref() {
    if(m_state != SP_STATE_INIT) {
      // FIXME 设置错误码
      LOG(ERROR) << "Object not constructed.";
      throw 0;
    }
    return *VoidSPtr::rget<element_type>();
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