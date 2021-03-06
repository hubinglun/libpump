/**
 * @file KMP.h
 * @brief 定义 libpump 库的 IO 缓冲区模块, 字符串匹配算法
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.08.12
 */

#ifndef LIBPUMP_KMP_H
#define LIBPUMP_KMP_H

#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#include <string>
#include <deque>
#include <vector>

#include "Logger.h"

namespace nsp_std = std;

namespace Pump {

class KMP {
public:
  KMP();
  
  ~KMP();
  
  long apply(const char *szSrc, long iSrcLen, const char *szPattern, long iPLen);
  
  long apply(const nsp_std::deque<char *> &vBuf, long iSrcLen, const char *szPattern, long iPLen);
  
  long apply(char **pszSrc, long iSLen, int iSCount, const char *szPattern, long iPLen);

private:
  void cal_next(const char *szPattern, long iPLen);
  
  nsp_std::vector<int> m_vNext;
};

}
#endif //LIBPUMP_KMP_H
