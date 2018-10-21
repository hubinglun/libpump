//
// Created by yz on 18-10-21.
//

#ifndef LIBPUMP_VOIDSPTR2_HPP
#define LIBPUMP_VOIDSPTR2_HPP

#include <utility>
#include <cstring>
#include <cassert>
#include <iostream>
#include <iomanip>

#include <boost/function.hpp>

#include "block.hpp"
#include "policy.hpp"
#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace Pump {
namespace SmartMem {

#define XXX nullptr

const unsigned short MEM_PACK_LEN = 8;

class _Del {
public:
  void operator()(Block *p) {
    if (p != 0) {
//      pre_del(p);
      p->deallocate();
      post_del(data_);
    }
  }

public:
  virtual void pre_del(const Block *p) {
  
  }
  
  virtual void post_del(size_t data) {
  
  }
  
  size_t data_;
};

class VoidSPtr;

class VoidWPtr;

bool operator==(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator==(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT;

bool operator!=(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator!=(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT;

bool operator==(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

bool operator!=(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT;

std::ostream &operator<<(std::ostream &os, VoidSPtr const &p);

class VoidSPtr {
private:
  typedef VoidSPtr this_type;
public:
  VoidSPtr() BOOST_NOEXCEPT
    : /*m_pParent(0),*/
    m_block(),
    m_policy() {
    
  }

#ifndef BOOST_NO_CXX11_NULLPTR
  
  VoidSPtr(boost::detail::sp_nullptr_t/*, VoidSPtr *const parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
//    m_iCapacity(0),
//    m_iSize(0),
//    m_px(0),
    m_block(),
    m_policy() {
    
  }

#endif //BOOST_NO_CXX11_NULLPTR
  
  template<class A = std::allocator<char> >
  explicit VoidSPtr(size_t n, A &a/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
    
    m_block(n, a),
    m_policy(m_block) {
  }

//  VoidSPtr(const VoidWPtr &r, boost::detail::sp_nothrow_tag)
//    : /*m_pParent(0),*/
//      m_block(),
//      m_policy(r.m_policy, boost::detail::sp_nothrow_tag()) {
//    if(!m_policy.empty()){
//      m_block = r.m_block;
//    }
//  }
  
  explicit VoidSPtr(const size_t n/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
    m_block(n),
    m_policy(m_block) {
  }
  
  template<class A = std::allocator<char>, class D>
  explicit VoidSPtr(size_t n, A &a, D d/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
    m_block(n, a),
    m_policy(m_block, d) {
  }
  
  template<class D>
  explicit VoidSPtr(size_t n, nsp_std::nullptr_t, D d/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
    m_block(n),
    m_policy(m_block, d) {
  }
  
  VoidSPtr(VoidSPtr const &r/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
    : /*m_pParent(parent),*/
    m_block(r.m_block),
    m_policy(r.m_policy) {
  }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES

//  VoidSPtr(VoidSPtr &&r/*, VoidSPtr *parent = 0*/) BOOST_NOEXCEPT
//    : /*m_pParent(parent),*/
//      m_block(r.m_block),
//      m_policy() {
//    m_policy.swap(r.m_policy);
//    r.m_block.m_px = nullptr;
//  }

#endif //BOOST_NO_CXX11_RVALUE_REFERENCES
  
  virtual ~VoidSPtr() {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "~VoidSPtr()";
#endif //_TEST_LEVEL_INFO
  }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES

//  VoidSPtr &operator=(VoidSPtr &&r) BOOST_NOEXCEPT {
//#ifdef _TEST_LEVEL_DEBUG
//    assert(m_block.m_iCapacity >= m_block.m_iSize);
//#endif //_TEST_LEVEL_DEBUG
//
//    if(m_pParent!= nullptr) {
//      // 跟新指针的入度&出度
//      this->m_pParent->m_policy.addOutdegree(r.get());
//      r.m_policy.addIndegree(this->m_pParent->get());
//    }
//
//    this_type(static_cast< VoidSPtr && >(r)).swap(*this);
//
//    return *this;
//  }

#endif //BOOST_NO_CXX11_RVALUE_REFERENCES
  
  VoidSPtr &operator=(VoidSPtr const &r) BOOST_NOEXCEPT {
#ifdef _TEST_LEVEL_DEBUG
    assert(m_block.capacity() >= m_block.capacity());
#endif //_TEST_LEVEL_DEBUG

//    if(m_pParent!= nullptr) {
//      // 跟新指针的入度&出度
//      m_pParent->m_policy.addOutdegree(r.get());
//      const_cast<VoidSPtr &>(r).m_policy.addIndegree(m_pParent->get());
//    }
    
    this_type(r).swap(*this);
    
    return *this;
  }

//  // [deprecate] 直接内存拷贝不符合智能指针标准, 因为复制内容可能包含智能指针, 直接拷贝不会增加引用计数
//  template<class T>
//  VoidSPtr &operator=(const T &right) {
//    size_t iSize = sizeof(T);
//    if (iSize > this->m_iCapacity) {
//      // FIXME 抛出异常, 或者其他识别错误的方案
//      throw 1;
//    }
//    ::memset(m_px, 0, m_iCapacity);
//    ::memcpy(m_px, &right, iSize);
//    return *this;
//  }
  
  virtual void reset() BOOST_NOEXCEPT {
    this_type().swap(*this);
  }
  
  void reset(VoidSPtr &other) {
    other.swap(*this);
  }
  
  size_t capacity() const {
    return m_block.capacity();
  }
  
  size_t size() const {
    return m_block.size();
  }
  
  void swap(VoidSPtr &r) BOOST_NOEXCEPT {
    m_block.swap(r.m_block);
    m_policy.swap(r.m_policy);
  }
  
  long use_count() const // nothrow
  {
    return m_policy.use_count();
  }
  
  bool unique() const BOOST_NOEXCEPT {
    return m_policy.unique();
  }
  
  enum RelativeType relative(VoidSPtr const &r) const {
    return m_block.relative(r.m_block);
  }

//  VoidSPtr *const parent() {
//    return m_pParent;
//  }
  
  template<class T>
  T *get() const {
    m_block.get<T>();
  }
  
  void *get() const {
    return (m_block.get<void *>());
  }

protected:
  friend class VoidWPtr;
  
  template<class Y> friend
  class WeakPtr;

protected:
//public:
  template<class T>
  T *rget() {
    return m_block.rget<T>();
  }

public:
  // FIXME 需要一个功能更加强大的元素访问接口,应该能支持多维数组
//  template<class T>
//  T &operator[](size_t indx) {
//
//  }

protected:
  /**
   * FIXME 研究使用VoidWPtr替换 ???
   * @var m_pParent [试用阶段]
   * @brief 父指针对象, 父指针托管的内存成员中包含本指针托管的内存
   */
  
  /**
   * @var m_block
   * @brief 托管内存对象
   */
  HeapGuider<> m_block;
  /**
   * @var m_policy
   * @brief 托管策略对象
   */
  SHeapPolicyGuider<> m_policy;
};

//template<class E, class T, class Y>
//std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, VoidSPtr const & p) {
//  os << "[" << std::endl;
//  if (p == nullptr) {
//    os << "]" << std::endl;
//    return os;
//  }
//  unsigned char *szBegin = p.get<unsigned char>();
//  for (size_t i = 0, j = 0; i < p.capacity(); ++i) {
//    unsigned char ch = *szBegin;
//    if (j == 0) {
//      os << "0x" << std::hex << std::setw(8) << std::setfill('0') << static_cast<unsigned long>(szBegin);
//    }
//    os << std::hex << std::setw(8) << std::setfill('0') << static_cast<short>(ch)
//       << (j == MEM_PACK_LEN - 1 ? "\n" : "");
//    j = (j == MEM_PACK_LEN - 1 ? 0 : j + 1);
//    szBegin++;
//  }
//  return os;
//}

inline bool operator==(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return (p.get() == 0);
}

inline bool operator==(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT {
  return (p.get() == 0);
}

inline bool operator!=(VoidSPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return (p.get() != 0);
}

inline bool operator!=(boost::detail::sp_nullptr_t, VoidSPtr const &p) BOOST_NOEXCEPT {
  return (p.get() != 0);
}

inline bool operator==(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT {
  return (a.relative(b) == RELATIVE_EQUAL);
}

inline bool operator!=(VoidSPtr const &a, VoidSPtr const &b) BOOST_NOEXCEPT {
  return (a.relative(b) != RELATIVE_EQUAL);
}

std::ostream &operator<<(std::ostream &os, VoidSPtr const &p) {
  os << "[" << std::endl;
  if (p == nullptr) {
    os << "]" << std::endl;
    return os;
  }
  nsp_std::stringstream ss;
  unsigned char *szBegin = p.get<unsigned char>();
  for (size_t i = 0, j = 0; i < p.capacity(); ++i) {
    unsigned char ch = *szBegin;
    if (j == 0) {
      ss << " 0x" << nsp_std::hex << std::setw(4) << nsp_std::setfill('0') << (unsigned long) (szBegin) << " ";
    }
    ss << nsp_std::hex << nsp_std::setw(4) << nsp_std::setfill('0') << static_cast<short>(ch)
       << (j == MEM_PACK_LEN - 1 ? "\n" : " ");
    j = (j == MEM_PACK_LEN - 1 ? 0 : j + 1);
    szBegin++;
  }
  ss << "\n]" << nsp_std::endl;
  os << ss.str();
  return os;
}

////////////////////////////////////////////////
//                  VoidWPtr
////////////////////////////////////////////////

//VoidSPtr VoidWPtr::lock_raw() const BOOST_NOEXCEPT {
//  return VoidSPtr(*this, boost::detail::sp_nothrow_tag());
//}

//VoidWPtr::VoidWPtr(VoidSPtr const &r) BOOST_NOEXCEPT
//  : m_block(r.m_block),
//    m_policy(r.m_policy) {
//}
//
//VoidWPtr & VoidWPtr::operator=(VoidSPtr const &r) BOOST_NOEXCEPT {
//  m_block = r.m_block;
//  m_policy = r.m_policy;
//  return *this;
//}
//
//bool operator==(VoidWPtr const &a, VoidWPtr const &b) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  VoidSPtr sp_b = b.lock_raw();
//  return (sp_a == sp_b);
//}
//
//bool operator!=(VoidWPtr const &a, VoidWPtr const &b) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  VoidSPtr sp_b = b.lock_raw();
//  return (sp_a != sp_b);
//}
//
//bool operator==(VoidWPtr const &a, nullptr_t) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  return (sp_a == nullptr);
//}
//
//bool operator!=(VoidWPtr const &a, nullptr_t) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  return (sp_a != nullptr);
//}
//
//bool operator==(nullptr_t, VoidWPtr const &a) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  return (sp_a == nullptr);
//}
//
//bool operator!=(nullptr_t, VoidWPtr const &a) BOOST_NOEXCEPT {
//  VoidSPtr sp_a = a.lock_raw();
//  return (sp_a != nullptr);
//}

}
}
#endif //LIBPUMP_VOIDSPTR2_HPP
