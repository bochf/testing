#include <iostream>
#include <boost/bind.hpp>

#include <gtest/gtest.h>

class Binder {
	public:
		int operator()(int x) {
			std::cout << "functor(" << x << ")" << std::endl;
		}
		int operator()(int x, int y) {
			std::cout << "functor(" << x << ", " << y << ")" << std::endl;
		}
		int f(int x, int y, int z) {
			std::cout << "Binder::f(" << x << ", " << y << ", " << z << ")" << std::endl;
		}
};

int f(int a, int b) {
	std::cout << "f(" << a << ", " << b << ")" << std::endl;
	return 0;
};

void g(int a, int b) {
	std::cout << "g(" << a << ", " << b << ")" << std::endl;
};

TEST(TestBind, Bind) {
	Binder binder;
	std::cout << "Start" << std::endl;
	boost::bind<int>(f, 100, 200)();
	boost::bind(g, 100, 200)();
	boost::bind<int>(binder, 1)();
	boost::bind<int>(binder, 2, 3)();
	boost::bind<int>(&Binder::f, boost::ref(binder), 4, 5, 6)();
	std::cout << "End" << std::endl;
}

GTEST_API_ int main(int argc, char ** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
