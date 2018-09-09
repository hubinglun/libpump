//
// Created by yz on 18-9-9.
//

#include "boost_interprocess_test.h"

#include <cassert>

#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/smart_ptr/shared_ptr.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/smart_ptr/deleter.hpp>

using namespace boost::interprocess;

//This is type of the object we want to share
class MyType
{
public:
  int a;
  char b = 'a';
  char c[5];
  
  MyType() {
    std::cout << "MyType()\n";
    strcpy(c, "1234\0");
  }
  ~MyType() {std::cout<<"~MyType\n";}
  void print() {std::cout<<c<<std::endl;}
};

// 共享内存的管理对象
typedef managed_shared_memory::segment_manager segment_manager_type;

// 内存分配器, 此分配器从共享内存分配空间, 而非直接从内存分配, 应该也不会在空间不足时额外malloc
typedef allocator<void, segment_manager_type>  void_allocator_type;

// 内存删除器
typedef deleter<MyType, segment_manager_type>  deleter_type;

//智能指针
typedef shared_ptr<MyType, void_allocator_type, deleter_type> my_shared_ptr;

int main ()
{
  //Remove shared memory on construction and destruction
  struct shm_remove
  {
    shm_remove() { shared_memory_object::remove("MySharedMemory"); }
    ~shm_remove(){ shared_memory_object::remove("MySharedMemory"); }
  } remover;
  
  // 1.定义一个共享内存对象, 大小为4096
  managed_shared_memory segment(create_only, "MySharedMemory", 4096);
  
  //Create a shared pointer in shared memory
  //pointing to a newly created object in the segment
  my_shared_ptr shared_ptr_instance = my_shared_ptr(
    segment.construct<MyType>("object to share")()      //object to own
    , void_allocator_type(segment.get_segment_manager()) //allocator
    , deleter_type(segment.get_segment_manager())         //deleter
  );
  assert(shared_ptr_instance.use_count() == 1);
  
  shared_ptr_instance->print();
  
  //Destroy "shared ptr". "object to share" will be automatically destroyed
//  segment.destroy_ptr(&shared_ptr_instance);
  
  return 0;
}

