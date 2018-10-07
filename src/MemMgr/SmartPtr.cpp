//
// Created by yz on 18-9-17.
//

#include "SmartPtr.h"

namespace Pump {
namespace MemMgr {

nsp_std::allocator<char> g_alloc;

nsp_std::list<nsp_boost::shared_ptr<void> > g_lPtr;

nsp_std::allocator<char> &getAlloc() {
  return g_alloc;
}

nsp_std::list<nsp_boost::shared_ptr<void> > &getHeapList() {
  return g_lPtr;
}

}
}