#ifndef LIBPUMP_VOIDPTR_H
#define LIBPUMP_VOIDPTR_H

#include <cstring>
#include <cassert>
#include <iostream>

//#include <boost/smart_ptr/detail/sp_counted_base.hpp>
//#include <boost/smart_ptr/detail/sp_counted_impl.hpp>

#include "SmartPtr.h"
#include "SharedCount.hpp"

namespace nsp_boost = boost;

namespace Pump {
namespace MemMgr {

const unsigned short MEM_PACK_LEN = 8;

//#pragma pack(push)
//#pragma pack(MEM_PACK_LEN)

class _Del {
public:
  virtual void operator()(void *p, size_t iSize, std::allocator<char> &a) const {
    a.deallocate((typename std::allocator<char>::pointer) p, iSize);
  }
};

enum RelativeType {
  RELATIVE_INTERSECT,
  RELATIVE_INCLUDE,
  RELATIVE_NONE,
  RELATIVE_EQUAL,
};

class VoidPtr;

bool operator==(VoidPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator==(boost::detail::sp_nullptr_t, VoidPtr const &p) BOOST_NOEXCEPT;

bool operator!=(VoidPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT;

bool operator!=(boost::detail::sp_nullptr_t, VoidPtr const &p) BOOST_NOEXCEPT;

bool operator==(VoidPtr const &a, VoidPtr const &b) BOOST_NOEXCEPT;

bool operator!=(VoidPtr const &a, VoidPtr const &b) BOOST_NOEXCEPT;

std::ostream &operator<<(std::ostream &os, VoidPtr const &p);

class VoidPtr {
private:
  typedef VoidPtr this_type;
public:
  VoidPtr() BOOST_NOEXCEPT
    : m_pParent(0),
    m_iCapacity(0),
    m_iSize(0),
    m_px(0),
    m_pn() {
    
  }

#ifndef BOOST_NO_CXX11_NULLPTR
  
  VoidPtr(boost::detail::sp_nullptr_t, VoidPtr * const parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(0),
    m_iSize(0),
    m_px(0),
    m_pn() {
    
  }

#endif //BOOST_NO_CXX11_NULLPTR
  
  template<class A = std::allocator<char> >
  explicit VoidPtr(A &a, const size_t size, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(0),
    m_pn() {
    // FIXME [general] std::allocator<T> 分配实际空间为(sizeof(T) * size)
    m_px = (void *) a.allocate(size);
    ::memset(m_px, 0, size);
    /** FIXME [urgency] [fixed] shared_count不支持仅输入构造指针和分配器的构造函数
     * 重写了 SharedCount 对象
     */
    m_pn = SharedCount(m_px, m_iCapacity, a);
  }
  
  explicit VoidPtr(const size_t size, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(0),
    m_pn() {
    // FIXME [general] std::allocator<T> 分配实际空间为(sizeof(T) * size)
    m_px = (void *) getAlloc().allocate(size);
    ::memset(m_px, 0, size);
    /** FIXME [urgency] [fixed] shared_count不支持仅输入构造指针和分配器的构造函数
     * 重写了 SharedCount 对象
     */
    m_pn = SharedCount(m_px, m_iCapacity, getAlloc());
  }
  
  template<class A = std::allocator<char>, class D>
  explicit VoidPtr(A &a, const size_t size, D del, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(0),
    m_pn() {
    // FIXME [general] std::allocator<T> 分配实际空间为(sizeof(T) * size)
    m_px = (void *) a.allocate(size);
    ::memset(m_px, 0, size);
    /** FIXME [urgency] [fixed] shared_count不支持仅输入构造指针和分配器的构造函数
     * 重写了 SharedCount 对象
     */
    m_pn = SharedCount(m_px, m_iCapacity, a, del);
  }
  
  template<class D>
  explicit VoidPtr(const size_t size, D del, nullptr_t , VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(0),
    m_pn() {
    // FIXME [general] std::allocator<T> 分配实际空间为(sizeof(T) * size)
    m_px = (void *) getAlloc().allocate(size);
    ::memset(m_px, 0, size);
    /** FIXME [urgency] [fixed] shared_count不支持仅输入构造指针和分配器的构造函数
     * 重写了 SharedCount 对象
     */
    m_pn = SharedCount(m_px, m_iCapacity, getAlloc(), del);
  }
  
  template<class A = std::allocator<char> >
  VoidPtr(A &a, void *px, const size_t size, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(px),
    m_pn() {
    m_pn = SharedCount(m_px, m_iCapacity, a);
  }
  
  VoidPtr(void *px, const size_t size, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(px),
    m_pn() {
    m_pn = SharedCount(m_px, m_iCapacity, getAlloc());
  }
  
  template<class A = std::allocator<char>, class D>
  VoidPtr(A &a, void *px, const size_t size, D del, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(px),
    m_pn() {
    m_pn = SharedCount(m_px, m_iCapacity, a, del);
  }
  
  template<class D>
  VoidPtr(void *px, const size_t size, D del, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(size),
    m_iSize(0),
    m_px(px),
    m_pn() {
    m_pn = SharedCount(m_px, m_iCapacity, getAlloc(), del);
  }
  
  VoidPtr(VoidPtr const &r, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(r.m_iCapacity),
    m_iSize(r.m_iSize),
    m_px(r.m_px),
    m_pn(r.m_pn) {
  }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
  
  VoidPtr(VoidPtr &&r, VoidPtr * parent = 0) BOOST_NOEXCEPT
    : m_pParent(parent),
    m_iCapacity(r.m_iCapacity),
    m_iSize(r.m_iSize),
    m_px(r.m_px),
    m_pn() {
    m_pn.swap(r.m_pn);
    r.m_px = nullptr;
  }

#endif //BOOST_NO_CXX11_RVALUE_REFERENCES
  
  virtual ~VoidPtr() {
#ifdef _TEST_LEVEL_INFO
    LOG(INFO) << "~VoidPtr()";
#endif //_TEST_LEVEL_INFO
  }

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES
  
  VoidPtr &operator=(VoidPtr &&r) BOOST_NOEXCEPT {
#ifdef _TEST_LEVEL_DEBUG
    assert(m_iCapacity >= m_iSize);
#endif //_TEST_LEVEL_DEBUG
  
    // 跟新指针的入度&出度
    this->m_pParent->m_pn.addOutdegree(r.get());
    r.m_pn.addIndegree(this->m_pParent->get());
    
    this_type(static_cast< VoidPtr && >(r)).swap(*this);
    
    return *this;
  }

#endif //BOOST_NO_CXX11_RVALUE_REFERENCES
  
  VoidPtr &operator=(VoidPtr const &r) BOOST_NOEXCEPT {
#ifdef _TEST_LEVEL_DEBUG
    assert(m_iCapacity >= m_iSize);
#endif //_TEST_LEVEL_DEBUG
  
    // 跟新指针的入度&出度
    m_pParent->m_pn.addOutdegree(r.get());
    const_cast<VoidPtr&>(r).m_pn.addIndegree(m_pParent->get());
    
    this_type(r).swap(*this);
    
    return *this;
  }

//  // [deprecate] 直接内存拷贝不符合智能指针标准, 因为复制内容可能包含智能指针, 直接拷贝不会增加引用计数
//  template<class T>
//  VoidPtr &operator=(const T &right) {
//    size_t iSize = sizeof(T);
//    if (iSize > this->m_iCapacity) {
//      // FIXME 抛出异常, 或者其他识别错误的方案
//      throw 1;
//    }
//    ::memset(m_px, 0, m_iCapacity);
//    ::memcpy(m_px, &right, iSize);
//    return *this;
//  }
  
  void reset() BOOST_NOEXCEPT {
    this_type().swap(*this);
  }
  
  template<class A>
  void reset(A &a, void *p, const size_t iSize) // Y must be complete
  {
#ifdef _TEST_LEVEL_DEBUG
    BOOST_ASSERT(p == 0 || p != m_px); // catch self-reset errors
#endif //_TEST_LEVEL_DEBUG
    this_type(a, p, iSize).swap(*this);
  }
  
  template<class A, class D>
  void reset(A &a, void *p, const size_t iSize, D d) {
    this_type(a, p, iSize, d).swap(*this);
  }
  
  void reset(VoidPtr &other) {
    other.swap(*this);
  }
  
  size_t capacity() const {
    return m_iCapacity;
  }
  
  size_t size() const {
    return m_iSize;
  }
  
  void swap(VoidPtr &r) BOOST_NOEXCEPT {
    std::swap(m_iCapacity, r.m_iCapacity);
    std::swap(m_iSize, r.m_iSize);
    std::swap(m_px, r.m_px);
    m_pn.swap(r.m_pn);
  }
  
  long use_count() const // nothrow
  {
    return m_pn.use_count();
  }
  
  bool unique() const BOOST_NOEXCEPT {
    return m_pn.unique();
  }
  
  enum RelativeType relative(VoidPtr const &r) const {
    if (*this == nullptr || r == nullptr){
      return RELATIVE_NONE;
    }
    if(m_px==r.m_px && (char*)m_px+m_iCapacity == (char*)r.m_px + r.m_iCapacity) {
      return RELATIVE_EQUAL;
    }
    else if((m_px <= r.m_px && (char*)m_px+m_iCapacity >= (char*)r.m_px + r.m_iCapacity)
            ||(m_px >= r.m_px && (char*)m_px+m_iCapacity <= (char*)r.m_px + r.m_iCapacity)) {
      return RELATIVE_INCLUDE;
    }
    else{
      return RELATIVE_INTERSECT;
    }
  }
  
  template<class T>
  T *get() const {
    if (sizeof(T) > this->m_iCapacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<T *>(m_px));
  }
  
  void *get() const {
    return (m_px);
  }
  
  VoidPtr * const parent() {
    return m_pParent;
  }

protected:
  template<class T>
  T *rget() {
    if (sizeof(T) > this->m_iCapacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<T *>(m_px));
  }

public:
  // FIXME 需要一个功能更加强大的元素访问接口,应该能支持多维数组
//  template<class T>
//  T &operator[](size_t indx) {
//
//  }

protected:
  /**
   * @var m_pParent
   * @brief 父指针对象, 父指针托管的内存成员中包含本指针托管的内存
   */
  VoidPtr * const m_pParent;
  /**
   * @var m_iCapacity
   * @brief 托管内存大小
   */
  size_t m_iCapacity;
  size_t m_iSize;
  /**
   * @var m_px
   * @brief 托管内存起始地址
   */
  void * m_px;
  /**
   * @var m_pn
   * @brief 应用计数对象
   */
  SharedCount m_pn;
};

//template<class E, class T, class Y>
//std::basic_ostream<E, T> & operator<< (std::basic_ostream<E, T> & os, VoidPtr const & p) {
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

inline bool operator==(VoidPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return (p.get() == 0);
}

inline bool operator==(boost::detail::sp_nullptr_t, VoidPtr const &p) BOOST_NOEXCEPT {
  return (p.get() == 0);
}

inline bool operator!=(VoidPtr const &p, boost::detail::sp_nullptr_t) BOOST_NOEXCEPT {
  return (p.get() != 0);
}

inline bool operator!=(boost::detail::sp_nullptr_t, VoidPtr const &p) BOOST_NOEXCEPT {
  return (p.get() != 0);
}

inline bool operator==(VoidPtr const &a, VoidPtr const &b) BOOST_NOEXCEPT {
  return (a.relative(b) == RELATIVE_EQUAL);
}

inline bool operator!=(VoidPtr const &a, VoidPtr const &b) BOOST_NOEXCEPT {
  return (a.relative(b) != RELATIVE_EQUAL);
}

std::ostream &operator<<(std::ostream &os, VoidPtr const &p) {
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
      ss << " 0x" << std::hex << std::setw(4) << std::setfill('0') << (unsigned long) (szBegin) << " ";
    }
    ss << std::hex << std::setw(4) << std::setfill('0') << static_cast<short>(ch)
       << (j == MEM_PACK_LEN - 1 ? "\n" : " ");
    j = (j == MEM_PACK_LEN - 1 ? 0 : j + 1);
    szBegin++;
  }
  ss << "\n]" << std::endl;
  os << ss.str();
  return os;
}

//#pragma pack (pop)

}
}

#endif //LIBPUMP_VOIDPTR_H