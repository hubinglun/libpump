//
// Created by yz on 18-10-20.
//

#include "MemMgr/test_mem/block.hpp"
#include "Logger.h"

using namespace Pump;
using namespace Pump::SmartMem;

bool test_Block() {
  LOG(INFO) << ">>>>>>>> test_Block <<<<<<<<<";
  Heap<> heap0;
  // test 1
  LOG_ASSERT(heap0.empty()) << "[ failed ] 1-1";
  heap0.allocate(0);
  LOG_ASSERT((heap0.block_segsz_==0 && heap0.empty())) << "[ failed ] 1-2";
  heap0.allocate(10);
  LOG_ASSERT((heap0.block_segsz_==10 && !heap0.empty())) << "[ failed ] 1-3";
  heap0.deallocate();
  LOG_ASSERT((heap0.block_segsz_==0 && heap0.empty())) << "[ failed ] 1-4";
  
  // test 2
  std::allocator<char> a;
  HeapMgr<> bmgr0;
  HeapMgr<> bmgr1(sizeof(int),a);
  HeapMgr<> bmgr2(sizeof(int));
  LOG_ASSERT((bmgr0.empty())) << "[ failed ] 2-1";
  bmgr1.ref<int>() = 10;
  bmgr2.ref<int>() = 100;
  LOG_ASSERT((!bmgr1.empty()&&bmgr1.ref<int>() == 10)) << "[ failed ] 2-2";
  bmgr0 = bmgr1;
  LOG_ASSERT((bmgr0.relative(bmgr1)==RELATIVE_EQUAL)) << "[ failed ] 2-3";
  HeapMgr<> bmgr1_2(bmgr1);
  LOG_ASSERT((bmgr1_2.relative(bmgr1)==RELATIVE_EQUAL)) << "[ failed ] 2-4";
  bmgr1.swap(bmgr2);
  LOG_ASSERT((bmgr1.ref<int>() == 100&&bmgr2.ref<int>() == 10)) << "[ failed ] 2-5";
  
  return true;
}

int main() {
  LOG_IF(INFO, test_Block()) << "[ OK ] test_Block()";
  
  return 0;
}

