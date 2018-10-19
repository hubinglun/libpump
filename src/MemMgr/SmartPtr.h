//
// Created by yz on 18-9-17.
//

#ifndef LIBPUMP_SMART_PTR_H
#define LIBPUMP_SMART_PTR_H

#include <list>
#include <utility>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "Logger.h"

namespace nsp_std = ::std;
namespace nsp_boost = ::boost;

namespace Pump {
namespace MemMgr {

nsp_std::allocator<char> &getAlloc();

nsp_std::list<nsp_boost::shared_ptr<void> > &getHeapList();

class Void {
};

/**
 * @enum RelativeType
 * @brief 描述内存区域间关系
 */
enum RelativeType {
  RELATIVE_INTERSECT, //! 相交，不包含
  RELATIVE_INCLUDE,   //! 真包含，非重叠
  RELATIVE_EXCLUDE,      //! 互斥
  RELATIVE_EQUAL,     //! 重叠
};

typedef struct tagBlock {
  /**
   * @var m_iCapacity
   * @brief 托管内存大小
   */
  size_t m_iCapacity;
  /**
   * @var m_iSize
   */
  size_t m_iSize;
  /**
   * @var m_px
   * @brief 托管内存起始地址
   */
  void *m_px;
  
  tagBlock()
    : m_iCapacity(0),
      m_iSize(0),
      m_px(0) {
  }
  
  tagBlock(const size_t iCapacity, const size_t iSize, void *px)
    : m_iCapacity(iCapacity),
      m_iSize(iSize),
      m_px(px) {
  }
  
  tagBlock(const tagBlock &r)
    : m_iCapacity(r.m_iCapacity),
      m_iSize(r.m_iSize),
      m_px(r.m_px) {
  }
  
  tagBlock &operator==(const tagBlock &r) {
    m_iCapacity = (r.m_iCapacity);
    m_iSize = (r.m_iSize);
    m_px = (r.m_px);
    return *this;
  }
  
  void swap(tagBlock &r) {
    std::swap(m_iCapacity, r.m_iCapacity);
    std::swap(m_iSize, r.m_iSize);
    std::swap(m_px, r.m_px);
  }
  
  template<class T>
  T *rget() {
    if (sizeof(T) > this->m_iCapacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<T *>(this->m_px));
  }
  
  template<class T>
  T *get() const {
    if (sizeof(T) > this->m_iCapacity) {
      // FIXME 抛出异常, 或者其他识别错误的方案
      throw 1;
    }
    return (static_cast<T *>(m_px));
  }
  
  enum RelativeType relative(tagBlock const &r) const {
    if (m_px == r.m_px
        && (char *) m_px + m_iCapacity ==
           (char *) r.m_px + r.m_iCapacity) {
      return RELATIVE_EQUAL;
    } else if ((m_px <= r.m_px
                && (char *) m_px + m_iCapacity >=
                   (char *) r.m_px + r.m_iCapacity)
               || (m_px >= r.m_px
                   && (char *) m_px + m_iCapacity <=
                      (char *) r.m_px + r.m_iCapacity)) {
      return RELATIVE_INCLUDE;
    } else if (((char *) m_px + m_iCapacity < r.m_px)
               || ((char *) r.m_px + r.m_iCapacity > m_px)) {
      return RELATIVE_EXCLUDE;
    } else {
      return RELATIVE_INTERSECT;
    }
  }
} Block;

template<class _T>
void DelMem(_T *p) {
  p->~_T();
  getAlloc().deallocate((char *) p, sizeof(_T));
  LOG(INFO) << "DelMem()";
}

template<class _T, class... _Args>
nsp_boost::shared_ptr<_T> NewMem(_Args &&... args) {
  _T *praw = (_T *) getAlloc().allocate(sizeof(_T));
  ::new(praw)_T(nsp_std::forward<_Args>(args)...);
  if (praw == nullptr) {
    return nsp_boost::shared_ptr<_T>();
  }
  nsp_boost::shared_ptr<_T> ptr = nsp_boost::shared_ptr<_T>(praw, DelMem<_T>);
  getHeapList().push_back(ptr);
  return ptr;
};

class SimpleGC {
public:
  SimpleGC(unsigned int tv) : m_tv(tv) {}
  
  int start() {
    nsp_boost::function0<void> fn = nsp_boost::bind<void>(&SimpleGC::cbGc, this);
    m_pReadThread = nsp_boost::shared_ptr<nsp_boost::thread>(
      new nsp_boost::thread(fn));
  }

private:
  void cbGc() {
    while (1) {
      for (nsp_std::list<nsp_boost::shared_ptr<void> >::iterator it = getHeapList().begin();
           it != getHeapList().end();) {
        if (it->use_count() == 1) {
          nsp_boost::shared_ptr<void> item = *it;
          it = getHeapList().erase(it);
          LOG(INFO) << "[warning] 删除一个";
        } else {
          ++it;
        }
      }
      LOG(INFO) << "[warning] 完成轮寻";
      sleep(m_tv);
    }
  }

private:
  nsp_boost::shared_ptr<nsp_boost::thread> m_pReadThread;
  int m_tv;
};

}
}

#endif //LIBPUMP_SMART_PTR_H
