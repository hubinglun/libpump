#ifndef LIBPUMP_VOIDPTR_H
#define LIBPUMP_VOIDPTR_H

#include <cstring>
#include <cassert>
#include <iostream>

#include "SmartPtr.h"

namespace Pump {
namespace MemMgr {

// FIXME 这个对象非常关键, 需要解决多线程同步问题
class _shared_count {
public:
  _shared_count() {
    use_count_ = 1;
  }
  
  void add_ref_copy() {
    // FIXME 加锁
    ++use_count_;
  }
  
  void release() {
    // FIXME 加锁
    --use_count_;
  }
  
  long use_count() const {
    return static_cast<long const volatile &>(use_count_);
  }
  
  template<class _A>
  friend bool swap(_shared_count **left, _shared_count **right, _A &alloc);

private:
  volatile long use_count_; // #shared
};

template<class _A>
bool swap(_shared_count **left, _shared_count **right, _A &alloc) {
  // FIXME 加锁
  bool ret = false;
  if ((*right) == nullptr) {
    return ret;
  }
  (*right)->add_ref_copy();
  if ((*left) == nullptr) {
    (*left) = (*right);
    return ret;
  }
  (*right)->release();
  if ((*left)->use_count() == 0) {
    alloc.deallocate((char *) (*left), sizeof(_shared_count));
    ret = true;
  }
  (*left) = (*right);
  return ret;
}

template<class _A> class VoidPtr;

template<class _A>
bool operator==(const VoidPtr<_A> &, std::nullptr_t);

template<class _A>
bool operator==(std::nullptr_t, const VoidPtr<_A> &);

template<class _A = std::allocator<char> >
class VoidPtr {
public:
  typedef _A allocator_t;
public:
  VoidPtr()
    : m_alloc(getAlloc()),
      m_iCopacity(0),
      m_iSize(0),
      m_pn(nullptr),
      m_px(nullptr) {
    
  }
  
  explicit VoidPtr(allocator_t &alloc)
    : m_alloc(alloc),
      m_iCopacity(0),
      m_iSize(0),
      m_pn(nullptr),
      m_px(nullptr) {
    
  }
  
  explicit VoidPtr(allocator_t &alloc, const size_t size)
    : m_alloc(alloc),
      m_iCopacity(size),
      m_iSize(0),
      m_px(nullptr) {
    m_pn = (_shared_count *) alloc.allocate(sizeof(_shared_count));
    ::new(m_pn)_shared_count();
    m_px = (void *) alloc.allocate(size);
    ::memset(m_px, 0, size);
  }
  
  VoidPtr(allocator_t &alloc, void *px, const size_t size)
    : m_alloc(alloc),
      m_iCopacity(size),
      m_iSize(0),
      m_pn(nullptr),
      m_px(px) {
    m_pn = (_shared_count *) alloc.allocate(sizeof(_shared_count));
    ::new(m_pn)_shared_count();
  }
  
  VoidPtr(VoidPtr &other)
    : m_alloc(other.m_alloc),
      m_iCopacity(other.m_iCopacity),
      m_iSize(other.m_iSize),
      m_px(nullptr),
      m_pn(nullptr) {
#ifdef _TEST_LEVEL_INFO
    assert(m_iCopacity >= m_iSize);
#endif //_TEST_LEVEL_INFO
    if (swap(&this->m_pn, &other.m_pn, m_alloc)/* == true*/) {
      m_alloc.deallocate((char *) m_px, m_iCopacity);
    }
    m_alloc.deallocate((char *) m_pn, sizeof(_shared_count));
  }
  
  virtual void release() {
    if (m_pn == nullptr) {
      return;
    }
    m_pn->release();
    if (m_pn->use_count() == 0) {
      if (m_iCopacity > 0) {
        m_alloc.deallocate((char *) m_px, m_iCopacity);
        m_px = nullptr;
        m_iCopacity = 0;
      }
      m_alloc.deallocate((char *) m_pn, sizeof(_shared_count));
      m_pn = nullptr;
    }
  }
  
  VoidPtr &operator=(VoidPtr &other) {
    m_iCopacity = other.m_iCopacity;
    m_iSize = other.m_iSize;
#ifdef _TEST_LEVEL_INFO
    assert(m_iCopacity >= m_iSize);
#endif //_TEST_LEVEL_INFO
    if (swap(&this->m_pn, &other.m_pn, m_alloc)/* == true*/) {
      m_alloc.deallocate((char *) m_px, m_iCopacity);
    }
    m_px = other.m_px;
    return *this;
  }
  
  template<class _Y>
  VoidPtr &operator=(const _Y &right) {
    size_t iSize = sizeof(_Y);
    if (iSize > this->m_iCopacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    ::memset(m_px, 0, m_iCopacity);
    ::memcpy(m_px, &right, iSize);
    return *this;
  }
  
  template<class _Y>
  _Y *get() {
    if (sizeof(_Y) > this->m_iCopacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<_Y *>(m_px));
  }
  
  template<class _Y>
  const _Y *cget() const {
    if (sizeof(_Y) > this->m_iCopacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<const _Y *>(m_px));
  }
  
  // FIXME 需要一个功能更加强大的元素访问接口,应该能支持多维数组
//  template<class _Y>
//  _Y &operator[](size_t indx) {
//
//  }
  
//  template<class _A_0>
  friend bool operator==<_A>(const VoidPtr<_A> &p, std::nullptr_t);
  
//  template<class _A_0>
  friend bool operator==<_A>(std::nullptr_t, const VoidPtr<_A> &p);

protected:
  allocator_t m_alloc;
  size_t m_iCopacity;
  size_t m_iSize;
  _shared_count *m_pn;
  void *m_px;
};

template<typename _A>
bool operator==(const VoidPtr<_A> &p, std::nullptr_t) {
#ifdef _TEST_LEVEL_INFO
  LOG(INFO) << typeid(_A).name();
#endif //_TEST_LEVEL_INFO
  return (p.m_px == nullptr);
}

template<typename _A>
bool operator==(std::nullptr_t, const VoidPtr<_A> &p) {
  return (p.m_px == nullptr);
}

}
}

#endif //LIBPUMP_VOIDPTR_H