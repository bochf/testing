#ifndef __MY_EXCEPTION_H__
#define __MY_EXCEPTION_H__

#include <exception>

class MyException : public std::exception {
	public:
	virtual const char * what() {
		return "my exception happened";
	}
};

#endif
