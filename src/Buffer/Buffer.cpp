#include "Buffer.h"

#include <memory>

nsp_std::allocator<char> s_allocIobuf;

namespace PUMP {

IoBuffer::IoBuffer(size_t iBufLen)
  : Buffer<char>(iBufLen, &s_allocIobuf) {
  
}

IoBuffer::~IoBuffer() {

}

}

