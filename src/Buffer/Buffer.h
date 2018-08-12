/**
 * @file Buffer.h
 * @brief 定义 libpump 库的 IO 缓冲区模块
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.08.12
 */
#pragma once

#ifndef LIBPUMP_BUFFER_H
#define LIBPUMP_BUFFER_H

#include <cstring>
#include <deque>
#include <ctime>
#include <iostream>

#include "KMP.h"

namespace nsp_std = std;

namespace PUMP {

/**
 * @class Buffer
 * @brief 缓冲区对象
 * @tparam _Elem 元素类型
 * @tparam _Alloc 内存分配器, 默认为 std::allocator<_Elem>
 *
 * 用于缓存IO数据, 使用多个非连续等长内存块存储数据, 并使用内存池申请
 * 和释放内存, 获得了较std::string更高的IO性能
 */
template<class _Elem,
  class _Alloc = nsp_std::allocator<_Elem> >
class Buffer {
public:
  Buffer(const size_t iBufLen, _Alloc *alloc = NULL);
  
  ~Buffer();
  
  /**
   * @fn long append(const char * szSrc, const size_t iLen);
   * @brief 向缓冲区末尾写一段数据
   * @param szSrc
   * @param iLen
   * @return
   */
  long append(const char *szSrc, const size_t iSLen);
  
  long erase(const size_t iLen);
  
  long find(const char *szSrc, const size_t iSLen);

private:
  _Alloc *m_alloc;
  nsp_std::deque<_Elem *> m_vBufs;
  const size_t m_iBufLen;
  long m_iIndxDel;
  KMP m_kmp;
};

template<class _Elem,
  class _Alloc>
Buffer<_Elem, _Alloc>::Buffer(const size_t iBufLen, _Alloc *alloc)
  :m_iBufLen(iBufLen),
   m_iIndxDel(-1),
   m_alloc(alloc) {
  if (m_alloc == NULL) {
    m_alloc = new nsp_std::allocator<char>();
  }
}

template<class _Elem,
  class _Alloc>
Buffer<_Elem, _Alloc>::~Buffer() {
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::append(const char *szSrc, const size_t iSLen) {
  const char *szWork = szSrc;
  long iRestLen = static_cast<long>(iSLen);
  double tDiff1 = 0;
  
  if (!m_vBufs.empty()) {
    char *szBuf = m_vBufs.back();
    size_t iSize = strlen(szBuf);
    if (iSize < m_iBufLen) {
      if (iRestLen <= m_iBufLen - iSize) {
        strncat(szBuf + iSize, szWork, static_cast<size_t >(iRestLen));
        iRestLen -= iRestLen;
      } else {
        strncat(szBuf + iSize, szWork, m_iBufLen - iSize);
        szWork += m_iBufLen - iSize;
        iRestLen -= (m_iBufLen - iSize);
      }
    }
  }
  
  while (iRestLen > 0) {
#ifdef _TEST_CODE
    clock_t start, end;
    start = clock();
#endif // _TEST_CODE
    //char * szNewBuf = new char[m_iBufLen + 1];
    char *szNewBuf = m_alloc->allocate(m_iBufLen + 1);
#ifdef _TEST_CODE
    end = clock();
    tDiff1 += (double)(end - start) / CLOCKS_PER_SEC;
#endif // _TEST_CODE
    memset(szNewBuf, 0, m_iBufLen + 1);
    strncat(szNewBuf, szWork, iRestLen - m_iBufLen >= 0 ? m_iBufLen : iRestLen);
    szWork += (iRestLen - m_iBufLen >= 0 ? m_iBufLen : iRestLen);
    iRestLen -= (iRestLen - m_iBufLen >= 0 ? m_iBufLen : iRestLen);
    m_vBufs.push_back(szNewBuf);
  }
#ifdef _TEST_CODE
  nsp_std::cout << "new op cost: " << tDiff1 << nsp_std::endl;
#endif // _TEST_CODE
  return 1;
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::find(const char *szSrc, const size_t iSLen) {
  return m_kmp.apply(m_vBufs, m_iBufLen, szSrc, iSLen);
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::erase(const size_t iLen) {
  if (m_vBufs.empty())
    return 0;
  char *pWork = m_vBufs.front();
  m_vBufs.pop_front();
  m_alloc->deallocate(pWork, m_iBufLen + 1);
  ++m_iIndxDel;
  return 1;
}

/**
 * @brief Io 缓冲区对象
 */
typedef Buffer<char> IoBuffer;

}

#endif //LIBPUMP_BUFFER_H
