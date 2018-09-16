//
// Created by yz on 18-9-15.
//

#ifndef LIBPUMP_THREAD_H
#define LIBPUMP_THREAD_H

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

#include "pumpdef.h"
#include "CbMailbox.h"

namespace nsp_boost = ::boost;

namespace PUMP {

class PThread
  : public nsp_boost::noncopyable {
public:
  PThread() {}
  
  virtual ~PThread() {}

public:
  PtrCbMailbox m_pMailbox;
  nsp_boost::shared_ptr<nsp_boost::thread> m_pRealThread;
};

typedef nsp_boost::shared_ptr<PThread> PtrThread;

}



#endif //LIBPUMP_THREAD_H
