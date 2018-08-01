//
// Created by yz on 18-8-1.
//

#ifndef LIBPUMP_INFRATEST_CBMAILBOX_H
#define LIBPUMP_INFRATEST_CBMAILBOX_H

#include <cstdio>
#include <iostream>
#include <vector>
#include "Pump.h"

using namespace boost;
using namespace std;
using namespace PUMP;

///< 测试用例 <1> 带返回值, 带参数值函数
int func1(int i){
	printf("func1(int %d) = %d\n", i, i);
	return i;
}

///< 测试用例 <2> 带返回值, 带多参数值函数
char func2(int i, char j){
	printf("func2(int %d, char %c) = %c\n", i, j, i+j);
	return (char)(i+j);
}

class A{
public:
	A(int a){
		m_a = a;
	}
	int m_a;
};

///< 测试用例 <3> 带指针返回值, 带参数值函数
A* makeA(int a){
	printf("makeA(int %d) = A(%d)\n", a, a);
	return new A(a);
}

///< 测试用例 <4> 无返回值, 带引用参数值函数
void modifyA(A & a){
	int t_i = a.m_a;
	a.m_a = 10;
	printf("modifyA(A & a) : a.m_a %d -> %d\n", t_i, a.m_a);
}

/** 回调对象<4>
 * */
class cb_modifyA
	: public CbFnWithoutReturn{
	typedef boost::function<void(A&)> func_t;
public:
	cb_modifyA(A& arg1)
		: m_arg1(arg1){	}
	void operator()() {
		m_fn(m_arg1);
	}
	func_t m_fn;
	A& m_arg1;
};

/** 回调对象<3>
 * */
class cb_makeA
	: public CbFnWithReturn{
	typedef boost::function<A*(int)> func_t;
public:
	cb_makeA(int arg1)
		: m_arg1(arg1){	}
	void operator()() {
		m_ret = m_fn(m_arg1);
	}
	func_t m_fn;
	int m_arg1;
	A *m_ret;
};

/** 回调对象<2>
 * */
class cb_func2
	:public CbFnWithReturn {
private:
	typedef boost::function<char(int,char)> func_t;
public:
	cb_func2(int arg1, char arg2)
		: m_arg1(arg1),
			m_arg2(arg2){}
	void setParm(int arg1, char arg2){
		m_arg1 = arg1;
		m_arg2 = arg2;
	}
	void operator()() {
		m_ret = m_fn(m_arg1,m_arg2);
	}
	func_t m_fn;
	int m_arg1;
	char m_arg2;
	char m_ret;
};

/** 回调对象<1>
 * */
class cb_func1
	:public CbFnWithReturn {
	typedef boost::function<int(int)> func_t;
public:
	cb_func1(int arg1)
		: m_arg1(arg1){}
	void setParm(int arg1){
		m_arg1 = arg1;
	}
	void operator()() {
		m_ret = m_fn(m_arg1);
	}

	func_t m_fn;
	int m_arg1;
	int m_ret;

};

#endif //LIBPUMP_INFRATEST_CBMAILBOX_H
