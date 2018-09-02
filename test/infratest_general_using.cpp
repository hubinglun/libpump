//
// Created by yz on 18-7-28.
//

#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>

#include "Logger.h"

using namespace std;
using namespace boost;
using namespace PUMP;

class test_A{
protected:
	int a;
public:
  boost::scoped_ptr<int> pInt;
  test_A() {
    pInt.reset(new int(6));
  }
  ~test_A() {}
};

class test_A_a
	: protected test_A {
public:
	int get(){return a;}
};

void swap(char* &p, char* &q)
{
	char *t = p;
	p = q;
	q = t;
}



void shared_prt_test(){
	shared_ptr<test_A> pA;
	if(pA == NULL) {
		LOG(INFO)<<"pA == NULL";
	}
	else{
		LOG(INFO)<<"pA != NULL";
	}
}

void scoped_ptr_test(){
  test_A a;
}

int main(){
//	int a=0,b=1;
//	int &ra =a, &rb=b;
//	swap(ra,rb);
	
//	shared_prt_test();
  scoped_ptr_test();

	std::cout<<"a";
}

