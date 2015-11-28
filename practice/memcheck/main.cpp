#include <iostream>
#include <vector>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

using namespace std;

class Singleton {
	public:
		static vector<pthread_t> s_threadPool;
		static bool s_exit;

		static void * run(void * attr) {
			while (!s_exit) {
				usleep(5000);
			}
		}

		static void start(const size_t n) {
			stop();

			s_exit = false;
			for (size_t i = 0; i < n; ++i) {
				pthread_t t;
				s_threadPool.push_back(t);
				pthread_create(&s_threadPool.back(), NULL, &Singleton::run, NULL);

				cout << "thread " << i << " started" << endl;
			}
		}

		static void stop() {
			s_exit = true;
			while (!s_threadPool.empty()) {
				pthread_join(s_threadPool.back(), NULL);
				s_threadPool.pop_back();

				cout << "thread " << s_threadPool.size() << " stopped" << endl;
			}
			vector<pthread_t>(s_threadPool).swap(s_threadPool); // release unused vector space
		}
};

vector<pthread_t> Singleton::s_threadPool;
bool Singleton::s_exit = false;

int main(int argc, char * argv[]) {
	const unsigned int MB = 1024 * 1024;

	size_t bufferSize = 0;
	size_t numberOfThread = 0;

	if (argc > 1) {
		bufferSize = atoi(argv[1]);
	}

	if (argc > 2) {
		numberOfThread = atoi(argv[2]);
	}

	char * buffer = NULL;
	if (bufferSize > 0) {
		buffer = new char[bufferSize * MB];
		cout << bufferSize << "MB memory allocated" << endl;
		cin.get();  // wait for next step
	}

	if (numberOfThread > 0) {
		Singleton::start(numberOfThread);
		cin.get();  // wait for next step
		Singleton::stop();
		cin.get();  // wait for next step
	}

	if (NULL == buffer) {
		delete [] buffer;
	}

  return 0;
}
