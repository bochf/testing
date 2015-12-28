#ifndef __FOO_H__
#define __FOO_H__

#include <iostream>

#include <my_exception.h>

class Foo {
	public:
	Foo(const int n) : d_n(n) {
		std::cout << "create Foo with " << d_n << std::endl;
	}

	virtual ~Foo() {
		std::cout << "delete Foo, n=" << d_n << std::endl;
	}

	public:
	virtual void throwException() {
		throw MyException();
	}

	private:
	int d_n;
};

#endif
