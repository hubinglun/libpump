#include <iostream>
#include <vector>
#include "Pump.h"

using namespace boost;
using namespace std;
using namespace PUMP;

///< 测试用例 <1> 带返回值, 带参数值函数
int func1(int i){
	return i;
}

///< 测试用例 <2> 带返回值, 带多参数值函数
char func2(int i, char j){
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
	return new A(a);
}
///< 测试用例 <4> 无返回值, 带引用参数值函数
void modifyA(A & a){
	a.m_a = 10;
}

class cb_modifyA
	: public CbFuncWithoutReturn{
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

class cb_makeA
	: public CbFuncWithReturn{
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

class cb_func1
	:public CbFuncWithReturn {
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

class cb_func2
	:public CbFuncWithReturn {
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

void testcase0(){
	std::vector<PrtCbFn> vfn;

	/**测试用例<1> (通过)*/
	PrtCbFn afn(new cb_func1(3));
	boost::shared_ptr<cb_func1> ptr1 = dynamic_pointer_cast<cb_func1>(afn);
	ptr1->m_fn = bind(func1,_1);
	vfn.push_back(afn);
	(*vfn[0])();

	/**测试用例<2> (通过)*/
	PrtCbFn afn2(new cb_func2(1,65));
	boost::shared_ptr<cb_func2> ptr2 = dynamic_pointer_cast<cb_func2>(afn2);
	ptr2->m_fn = bind(func2,_1,_2);
	vfn.push_back(afn2);
	(*vfn[1])();

	/**测试用例<3> (通过)*/
	PrtCbFn afn3(new cb_makeA(3));
	boost::shared_ptr<cb_makeA> ptr3 = dynamic_pointer_cast<cb_makeA>(afn3);
	ptr3->m_fn = bind(makeA,_1);
	vfn.push_back(afn3);
	(*vfn[2])();

	/**测试用例<4> (通过)*/
	A argA(1);
	PrtCbFn afn4(new cb_modifyA(argA));
	boost::shared_ptr<cb_modifyA> ptr4 = dynamic_pointer_cast<cb_modifyA>(afn4);
	ptr4->m_fn = bind(modifyA,_1);
	vfn.push_back(afn4);
	(*vfn[3])();
}

void testcase1(){
	PUMP::CbFuncList aFnList;

	PrtCbFn afn(new cb_func1(3));
	boost::shared_ptr<cb_func1> ptr1 = dynamic_pointer_cast<cb_func1>(afn);
	ptr1->m_fn = bind(func1,_1);

	/**测试用例<2> (通过)*/
	PrtCbFn afn2(new cb_func2(1,65));
	boost::shared_ptr<cb_func2> ptr2 = dynamic_pointer_cast<cb_func2>(afn2);
	ptr2->m_fn = bind(func2,_1,_2);

	/**测试用例<3> (通过)*/
	PrtCbFn afn3(new cb_makeA(3));
	boost::shared_ptr<cb_makeA> ptr3 = dynamic_pointer_cast<cb_makeA>(afn3);
	ptr3->m_fn = bind(makeA,_1);

	/**测试用例<4> (通过)*/
	A argA(1);
	PrtCbFn afn4(new cb_modifyA(argA));
	boost::shared_ptr<cb_modifyA> ptr4 = dynamic_pointer_cast<cb_modifyA>(afn4);
	ptr4->m_fn = bind(modifyA,_1);

	aFnList.insert(afn);
	aFnList.insert(afn2);
	aFnList.insert(afn3);
	aFnList.insert(afn4);
	aFnList.runAll();
}

int main() {
	testcase1();

	return 0;
}