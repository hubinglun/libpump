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

#include <boost/thread/mutex.hpp>
#include <boost/atomic.hpp>

#include "KMP.h"

namespace nsp_std = std;
namespace nsp_boost = boost;

namespace PUMP {

/**
 * @class Buffer
 * @brief 缓冲区对象
 * @tparam _Elem 元素类型
 * @tparam _Alloc 内存分配器, 默认为 std::allocator<_Elem>
 *
 * - 用于缓存IO数据, 使用多个非连续等长内存块存储数据, 并使用内存池申请
 * 和释放内存, 获得了较std::string更高的IO性能.
 *
 * - IO缓冲区采用FIFO机制, 由于采用非连续内存块存储数据, 可以实现读/写的
 * 完全独立, 即读/写操作可并发, 因此读/写操作各占用一个互斥量. 并将缓冲区
 * 大小成员变量用原子数据类型实现, 保证正确性.
 */
template<class _Elem,
  class _Alloc = nsp_std::allocator<_Elem> >
class Buffer {
public:
  Buffer(size_t iBufLen, _Alloc *alloc = NULL);
  
  virtual ~Buffer();
  
  /**
   * @fn long append(const char * szSrc, const size_t iLen)
   * @brief 向缓冲区末尾写一段数据
   * @param szSrc [in] 数据内存地址
   * @param iSLen [in] 有效长度
   * @return ???
   */
  long append(const char *szSrc, size_t iSLen);
  
  /**
   * @fn long erase(size_t iLen)
   * @brief 从缓冲区头删除一段数据
   * @param iLen [in] 取数据长度
   * @return ???
   */
  long erase(size_t iLen);
  
  long find(const char *szSrc, size_t iSLen);
  
  size_t size() const;
  
  size_t getChunkSize() const { return m_iChunkSize; }
  
  bool lockAppend();
  
  bool unlockAppend();
  
  bool lockErase();
  
  bool unlockErase();

private:
  /**
   * @var _Alloc * const m_alloc
   * @brief 内存分配器
   * 只能使用(非所有者), 不允许释放
   */
  _Alloc *const m_alloc;
  
  /**
   * @defgroup Buffer::storage
   * @{
   */
  
  /**
   * @var nsp_std::deque<_Elem *> m_vBufs
   * @brief 缓冲区地址数组
   *
   * 不要求缓冲连续, 但要求每一块缓冲区大小相同
   */
  nsp_std::deque<_Elem *> m_vBufs;
  
  /**
   * @var const size_t m_iChunkSize
   * @brief 一块缓冲区的有效长度, 最后一个元素始终未0
   */
  const size_t m_iChunkSize;
  
  /**
   * @var size_t m_iBegin
   * @brief 缓冲组中的有效起始位置
   *
   * 缓冲的有效起始地址可能不是0, 0 <= m_iBegin <= m_iChunkSize
   */
  size_t m_iBegin;
  
  /**
   * @var boost::atomic_int64_t m_aiBufSize
   * @brief 缓冲内有效数据长度
   */
  nsp_boost::atomic_int64_t m_aiBufSize;
  
  /**
   * @var boost::mutex m_mtxAppend
   * @brief append 操作的锁
   */
  nsp_boost::mutex m_mtxAppend;
  
  /**
   * @var boost::mutex m_mtxErase
   * @brief erase 操作的锁
   */
  nsp_boost::mutex m_mtxErase;
  /** @} */
  
  /**
   * @var KMP m_kmp
   * @brief KMP 算法实体
   */
  KMP m_kmp;
};

template<class _Elem,
  class _Alloc>
Buffer<_Elem, _Alloc>::Buffer(size_t iBufLen, _Alloc *alloc)
  :m_alloc(m_alloc == NULL ? (new nsp_std::allocator<_Elem>()) : alloc),
   m_iChunkSize(iBufLen),
   m_iBegin(0),
   m_aiBufSize(0) {
}

template<class _Elem,
  class _Alloc>
Buffer<_Elem, _Alloc>::~Buffer() {
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::append(const char *szSrc, size_t iSLen) {
#ifdef _TEST_LEVEL_INFO
  assert(m_iBegin <= m_iChunkSize);
#endif //_TEST_LEVEL_INFO
  const char *szWork = szSrc;
  long iRestLen = static_cast<long>(iSLen);
  double tDiff1 = 0;
  
  lockAppend();
  if (!m_vBufs.empty()) {
    char *szBuf = m_vBufs.back();
    size_t iSize = strlen(szBuf);
    if (iSize < m_iChunkSize) {
      if (iRestLen <= m_iChunkSize - iSize) {
        strncat(szBuf + iSize, szWork, static_cast<size_t >(iRestLen));
        iRestLen -= iRestLen;
      } else {
        strncat(szBuf + iSize, szWork, m_iChunkSize - iSize);
        szWork += m_iChunkSize - iSize;
        iRestLen -= (m_iChunkSize - iSize);
      }
    }
  }
  
  while (iRestLen > 0) {
#ifdef _TEST_LEVEL_DEBUG
    clock_t start, end;
    start = clock();
#endif // _TEST_LEVEL_DEBUG
    //char * szNewBuf = new char[m_iChunkSize + 1];
    char *szNewBuf = m_alloc->allocate(m_iChunkSize + 1);
    if (szNewBuf == NULL) {
      break;
    }
#ifdef _TEST_LEVEL_DEBUG
    end = clock();
    tDiff1 += (double)(end - start) / CLOCKS_PER_SEC;
#endif // _TEST_LEVEL_DEBUG
    memset(szNewBuf, 0, m_iChunkSize + 1);
    strncat(szNewBuf, szWork, iRestLen - m_iChunkSize >= 0 ? m_iChunkSize : iRestLen);
    szWork += (iRestLen - m_iChunkSize >= 0 ? m_iChunkSize : iRestLen);
    iRestLen -= (iRestLen - m_iChunkSize >= 0 ? m_iChunkSize : iRestLen);
    m_vBufs.push_back(szNewBuf);
  }
  m_aiBufSize += iSLen;
  unlockAppend();
#ifdef _TEST_LEVEL_DEBUG
  nsp_std::cout << "new op cost: " << tDiff1 << nsp_std::endl;
#endif // _TEST_LEVEL_DEBUG
#ifdef _TEST_LEVEL_INFO
  assert(m_iBegin <= m_iChunkSize);
#endif //_TEST_LEVEL_INFO
  return 1;
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::find(const char *szSrc, size_t iSLen) {
  // FIXME 查找正确应该增加起始位置参数, 因为 m_vBufs 中有效起始位置不一定是0
  long ret = -1;
  lockErase(); // 查找时只允许增加,不允许删
  ret = m_kmp.apply(m_vBufs, m_iChunkSize, szSrc, iSLen);
  unlockErase();
  return ret;
}

template<class _Elem,
  class _Alloc>
long Buffer<_Elem, _Alloc>::erase(size_t iLen) {
#ifdef _TEST_LEVEL_INFO
  assert(m_iBegin <= m_iChunkSize);
#endif //_TEST_LEVEL_INFO
  long iRestDelLen = static_cast<long>(iLen);
  
  size_t iBufSize = size();
  if (iLen > iBufSize) {
    iRestDelLen = iBufSize;
  }
  lockErase();
  while ((iRestDelLen > 0) && (!m_vBufs.empty())) {
    char *pWork = m_vBufs.front();
    if (iRestDelLen >= (m_iChunkSize - m_iBegin)) {
      /* 需要删除一个缓冲区块以上的情况 */
      m_vBufs.pop_front();
      m_alloc->deallocate(pWork, m_iChunkSize + 1);
      m_aiBufSize -= (m_iChunkSize - m_iBegin); // 收缩缓冲区的有效长度
      iRestDelLen -= (m_iChunkSize - m_iBegin);
      m_iBegin = 0;
      continue;
    } else {
      /* 需要删除少于一个缓冲区块的情况 */
      m_iBegin += iRestDelLen;
      m_aiBufSize -= iRestDelLen;
      iRestDelLen -= iRestDelLen;
      if (m_aiBufSize == 0) {
        m_vBufs.pop_front();
        m_alloc->deallocate(pWork, m_iChunkSize + 1);
        m_iBegin = 0;
      }
    }
  }
  unlockErase();
#ifdef _TEST_LEVEL_INFO
  assert(m_iBegin <= m_iChunkSize);
#endif //_TEST_LEVEL_INFO
  return 1;
}

template<class _Elem,
  class _Alloc>
size_t Buffer<_Elem, _Alloc>::size() const {
//  size_t iSize = 0;
//  for (typename nsp_std::deque<_Elem *>::iterator it = m_vBufs.begin();
//       it != m_vBufs.end(); it++) {
//    size_t iBufSize = strlen(*it) - m_iBegin;
//    iSize += iBufSize;
//  }
//  return iSize;
  return m_aiBufSize;
}

template<class _Elem,
  class _Alloc>
bool Buffer<_Elem, _Alloc>::lockErase() {
  m_mtxErase.lock();
  return true;
};

template<class _Elem,
  class _Alloc>
bool Buffer<_Elem, _Alloc>::unlockErase() {
  m_mtxErase.unlock();
  return true;
}

template<class _Elem,
  class _Alloc>
bool Buffer<_Elem, _Alloc>::lockAppend() {
  m_mtxAppend.lock();
  return true;
}

template<class _Elem,
  class _Alloc>
bool Buffer<_Elem, _Alloc>::unlockAppend() {
  m_mtxAppend.unlock();
  return true;
}

/**
 * @class class IoBuffer []
 * @brief Io 缓冲区对象, Buffer<> 全特化派生
 */
class IoBuffer
  : public Buffer<char> {
public:
  explicit IoBuffer(size_t iBufLen);
  
  ~IoBuffer();
};

}

#endif //LIBPUMP_BUFFER_H
