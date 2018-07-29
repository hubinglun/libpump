//
// Created by yz on 18-7-28.
//

#include <iostream>

using namespace std;

class test_A{
protected:
	int a=1;
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

int main(){
//	int a=0,b=1;
//	int &ra =a, &rb=b;
//	swap(ra,rb);

	std::cout<<"a";
}

