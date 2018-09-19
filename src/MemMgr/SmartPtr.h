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
