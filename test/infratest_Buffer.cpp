// KMP_test.cpp : 定义控制台应用程序的入口点。
//

#include <cstdio>
#include <cassert>
#include <ctime>
#include <cmath>
#include <iostream>

/**
 * @bug 若不包含boost库的头文件, 链接时报错
 * @brief 链接错误如下:
 *  libpump.so：对‘boost::system::system_category()’未定义的引用
 *  libpump.so：对‘boost::system::generic_category()’未定义的引用
 *
 * > 原因
 * 推测因为 Buffer 的代码时编译进 libpump 的, libpump库对boost库存在部分依赖,
 * 其中Buffer模块就不依赖, 单独使用Buffer模块时, 由于连接器不会链接boost库, 所以
 * 导致未定义引用错误.
 *
 * > 解决
 * 在使用Buffer模块的代码中包含任何一个libpump中包含了boost头文件的头文件, 如Event.h
 */
#include "Event.h"
#include "Buffer/Buffer.h"

using namespace std;
using namespace PUMP;

/** 测试用例<1> (通过)
 * KMP 算法测试
 */
void test_KMP(){
  PUMP::KMP kmp;
  char str[] = "bacbababadabababacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabacbacbababadababacambabaccambabacaddababacasdsd\0";
  char ptr[] = "abacbac\0";
  clock_t start, end;
  long indx0=-1, indx1=-1;

  start = clock();
  for (int i = 0; i < 1000000; i++)
    indx0 = kmp.apply(str, strlen(str), ptr, strlen(ptr));
  end = clock();
  double tDiff0 = (double)(end - start) / CLOCKS_PER_SEC;

  string strSrc = str, strPatern = ptr;
  start = clock();
  for (int i = 0; i < 1000000; i++)
    indx1 = strSrc.find(strPatern);
  end = clock();
  double tDiff1 = (double)(end - start) / CLOCKS_PER_SEC;

  assert(indx0 == indx1);

  std::cout << "KMP: " << tDiff0 << std::endl;
  std::cout << "find_first_of: " << tDiff1 << std::endl;
}

/** 测试用例<2> (通过)
 * KMP 算法测试
 */
void test_KMP2(){
  char **a = new char*[3];
  a[0] = "abcd";
  a[1] = "efgh";
  a[2] = "ijkl";
  KMP kmp;
  char *ptr = "abcdefghijkl\0";
  std::cout << ptr << kmp.apply(a, 4, 3, ptr, strlen(ptr)) << std::endl;
  ptr = "bcd\0";
  std::cout << ptr << kmp.apply(a, 4, 3, ptr, strlen(ptr)) << std::endl;
  ptr = "cde\0";
  std::cout << ptr << kmp.apply(a, 4, 3, ptr, strlen(ptr)) << std::endl;
  ptr = "def\0";
  std::cout << ptr << kmp.apply(a, 4, 3, ptr, strlen(ptr)) << std::endl;
  ptr = "efg\0";
  std::cout << ptr << kmp.apply(a, 4, 3, ptr, strlen(ptr)) << std::endl;
}

#define BLOCK_SIZE 128
#define BATCH_SIZE 100

/** 测试用例<3> (通过)
 * Buffer 对象测试
 */
void test_Buffer(size_t iBlockSize, size_t iBatchSize){
  allocator<char> aBuffer, aString;
  Buffer<char> buffer1(iBlockSize, &aBuffer);
  Buffer<char> buffer2(iBlockSize, &aBuffer);
  Buffer<char> buffer3(iBlockSize, &aBuffer);
  char first[] = "bacbababadabababacbababadababacambabacbacbababadababacambabacb";
  char second[] = "ddababacbabacambabacasbabacambabacbabacambabacbabacamssssssbabacbabacambabacdsd\0";
  char ptr[] = "cbabacamssssssbabacba\0";
  clock_t start, mid, end;
  long indx0=-1, indx1=-1;
  string strSrc1(aString);
  string strSrc2(aString);
  string strSrc3(aString);
  string strPatern = ptr;
  
  ////////////////////////////////////////////////////////
  start = clock();
  for (int i = 0; i < iBatchSize; i++){
    buffer1.append(first, strlen(first));
    buffer2.append(first, strlen(first));
    buffer3.append(first, strlen(first));
  }
  buffer1.append(second, strlen(second));
  buffer2.append(second, strlen(second));
  buffer3.append(second, strlen(second));
  
  ////////////////////////////////////////////////////////
  mid = clock();
  for (int i = 0; i < 100; i++)
    indx0 = buffer1.find(ptr, strlen(ptr));
  end = clock();
  double tDiff2 = (double)(end - mid) / CLOCKS_PER_SEC;
  
  ////////////////////////////////////////////////////////
  for (int i = 0; i < iBatchSize; i++){
    buffer1.erase(iBlockSize);
    buffer2.erase(iBlockSize);
    buffer3.erase(iBlockSize);
  }
  end = clock();
  double tDiff0 = (double)(end - start) / CLOCKS_PER_SEC - tDiff2;
  
  ////////////////////////////////////////////////////////
  start = clock();
  for (int i = 0; i < iBatchSize; i++){
    strSrc1.append(first, strlen(first));
    strSrc2.append(first, strlen(first));
    strSrc3.append(first, strlen(first));
  }
  strSrc1.append(second, strlen(second));
  strSrc2.append(second, strlen(second));
  strSrc3.append(second, strlen(second));
  
  ////////////////////////////////////////////////////////
  mid = clock();
  for (int i = 0; i < 100; i++)
    indx1 = strSrc1.find(strPatern);
  end = clock();
  double tDiff3 = (double)(end - mid) / CLOCKS_PER_SEC;
  
  ////////////////////////////////////////////////////////
  for (int i = 0; i < iBatchSize; i++){
    strSrc1.erase(0, iBlockSize);
    strSrc2.erase(0, iBlockSize);
    strSrc3.erase(0, iBlockSize);
  }
  end = clock();
  double tDiff1 = (double)(end - start) / CLOCKS_PER_SEC - tDiff3;
  
  assert(indx1 == indx0);
  
  std::cout << iBlockSize << ", " << iBatchSize << std::endl;
  std::cout << "	Buffer append: " << tDiff0 << std::endl;
  std::cout << "	string append: " << tDiff1 << std::endl;
  std::cout << "	Buffer find: " << tDiff2 << std::endl;
  std::cout << "	string find: " << tDiff3 << std::endl;
}

/** 测试用例<4> (通过)
 * Buffer 与 std::string io 性能 及 查找性能对比
 * 结果参考 [buffer&string性能对比.txt]
 */
void cmp_bufferAndstring(){
  int ibatch = 6;
  for (int i = 10; i < pow(10,ibatch); i *= 10){
    test_Buffer(128, i);
  }
  std::cout<<">>> cmp_bufferAndstring complete! <<<"<<std::endl;
}

/** 测试用例<5> (通过)
 * IoBuffer 对象功能测试
 */
void test_IoBuffer(){
  IoBuffer ioBuf(10);
  char tsz[] = "aaaaaaaaaaaa\0";
  ioBuf.append(tsz,strlen(tsz));
  ioBuf.append(tsz,strlen(tsz));
  
  size_t iSize = ioBuf.size();
  
  ioBuf.erase(3);
  ioBuf.erase(3);
  ioBuf.erase(4);
  ioBuf.erase(11);
  ioBuf.append(tsz,strlen(tsz));
  ioBuf.erase(11);
  ioBuf.erase(4);
  std::cout<<"test_IoBuffer complete"<<std::endl;
}

int main(int argc, char* argv[])
{
//  test_KMP();
//  test_KMP2();
//  test_Buffer(BLOCK_SIZE,BATCH_SIZE);
//  cmp_bufferAndstring();
  test_IoBuffer();
  return getchar();
}

