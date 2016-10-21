#ifndef __STACK_Q_H__
#define __STACK_Q_H__

#include <stack>

template<class T>
class StackQ {
	public:
		StackQ() {
			// empty queue
		}

		virtual ~StackQ() {}

		bool empty() const {
			return _in.empty() && _out.empty();
		}

		unsigned int size() const {
			return _in.size() + _out.size();
		}

		T& front() {
			if (_out.empty()) {
				move(_in, _out);
			}
			return _out.top();
		}

		T& back() {
			if (_in.empty()) {
				move(_out, _in);
			}
			return _in.top();
		}

		void push(const T& value) {
			_in.push(value);
		}

		void pop() {
			if (_out.empty()) {
				move(_in, _out);
			}
			_out.pop();
		}

	private:
		std::stack<T> _in;
		std::stack<T> _out;

		void move(std::stack<T>& from, std::stack<T>& to) {
			while (!from.empty()) {
				to.push(from.top());
				from.pop();
			}
		}
};

#endif
