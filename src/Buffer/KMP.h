/**
 * @file KMP.h
 * @brief 定义 libpump 库的 IO 缓冲区模块, 字符串匹配算法
 *
 * @author YangZheng 263693992@qq.com
 * @version 1.0
 * @date 2018.08.12
 */
#pragma once

#ifndef LIBPUMP_KMP_H
#define LIBPUMP_KMP_H

#include <string>
#include <deque>
#include <vector>

namespace nsp_std = std;

namespace PUMP {

class KMP {
public:
  KMP();
  
  ~KMP();
  
  long apply(const char *szSrc, const int iSrcLen, const char *szPattern, const int iPLen);
  
  long apply(const nsp_std::deque<char *> &vBuf, const long iSrcLen, const char *szPattern, const long iPLen);
  
  long apply(char **pszSrc, const long iSLen, const int iSCount, const char *szPattern, const long iPLen);

private:
  void cal_next(const char *szPattern, const long iPLen);
  
  nsp_std::vector<int> m_vNext;
};

}

#endif //LIBPUMP_KMP_H
