#include "infratest_CbMailbox.h"

/** 测试 场景<1> 回调对象 CbFn 能否正常工作
 * */
void testScene0(){
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

/** 测试 场景<2> 回调链表 CbFnList 能否正常工作
 * */
void testScene1(){
	PUMP::CbFnList aFnList;

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

/** 测试 场景<3> 回调邮箱 CbMailbox 能否正常工作
 * */
void testScene2() {
	PUMP::CbQueueMailbox cbMB;

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

	cbMB.insert(EVPRIOR_LEVEL0, afn);
	cbMB.insert(EVPRIOR_LEVEL1,afn2);
	cbMB.insert(EVPRIOR_LEVEL2,afn3);
	cbMB.insert(EVPRIOR_LEVEL0,afn4);

	cbMB.runAll();
}

int main() {
//	testScene1();
	testScene2();
	return 0;
}