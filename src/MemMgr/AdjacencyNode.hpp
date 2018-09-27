//
// Created by yz on 18-9-27.
//

#ifndef LIBPUMP_ADJACENNODE_H
#define LIBPUMP_ADJACENNODE_H

#include <list>

#include "SmartPtr.h"

namespace nsp_std = std;

namespace Pump {
namespace MemMgr {

class AdjacencyNode {
public:
  virtual void addIndegree(const void *in) = 0;
  virtual void addOutdegree(const void *out) = 0;
  virtual void release() = 0;
};

template<class A = nsp_std::allocator<char> >
class AdjacencyNode_a : public AdjacencyNode {
protected:
  typedef AdjacencyNode_a<A> this_type;
  A a_; // copy constructor must not throw
  nsp_std::list<const void *> m_lIndegree;
  nsp_std::list<const void *> m_lOutdegree;
public:
  AdjacencyNode_a()
    : m_lIndegree(a_),
      m_lOutdegree(a_) {}
  
  AdjacencyNode_a(A &a)
    : a_(a),
      m_lIndegree(a),
      m_lOutdegree(a) {
    
  }
  
  virtual void addIndegree(const void *in) {
    m_lIndegree.push_back(in);
  }
  
  virtual void addOutdegree(const void *out) {
    m_lOutdegree.push_back(out);
  }
  
  virtual void release() // nothrow
  {
    typedef typename std::allocator_traits<A>::template rebind_alloc< this_type > A2;
  
    A2 a2( a_ );
    std::allocator_traits<A2>::destroy( a2, this );
//    a2.deallocate( this, 1 );
    a2.deallocate((typename A2::pointer)this, sizeof(SpCountedImpl_a<A>));
  }
};

}
}

#endif //LIBPUMP_ADJACENNODE_H
