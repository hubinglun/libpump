#include "Buffer.h"

#include <memory>

nsp_std::allocator<char> s_allocIobuf;

namespace Pump {

////////////////////////////////////////////////
//                   IoBuffer
////////////////////////////////////////////////

IoBuffer::IoBuffer(size_t iBufLen)
  : Buffer<char>(iBufLen, &s_allocIobuf) {
  
}

IoBuffer::~IoBuffer() {

}

long IoBuffer::get(nsp_std::string &strOBuf, size_t iLen) {
  if (iLen <= 0) {
    return -1;
  }
  if (m_aiBufSize == 0) {
    return 0;
  }
  
  long iRestLen, iGetSize;
  iRestLen = iGetSize = static_cast<long>(iLen);
  if (iRestLen > m_aiBufSize) {
    iRestLen = iGetSize = m_aiBufSize;
  }
  nsp_std::deque<char *>::iterator it = m_vBufs.begin();
  char *pszBegin = (*it) + m_iBegin;
  if (iRestLen < (m_iChunkSize - m_iBegin)) {
    strOBuf.append(pszBegin, static_cast<size_t >(iRestLen));
    goto RET_GOOD;
  }
  strOBuf.append(pszBegin, (m_iChunkSize - m_iBegin));
  iRestLen -= (m_iChunkSize - m_iBegin);
  ++it;
  while (iRestLen > 0) {
    pszBegin = (*it);
    if (iRestLen < (m_iChunkSize)) {
      strOBuf.append(pszBegin, static_cast<size_t >(iRestLen));
      goto RET_GOOD;
    }
    strOBuf.append(pszBegin, m_iChunkSize);
    iRestLen -= (m_iChunkSize);
    ++it;
  }

RET_GOOD:
  return iGetSize;
}

}

