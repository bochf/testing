#ifndef __BIG_INT_H__
#define __BIG_INT_H__

#include <vector>
#include <string>
#include <ostream>

class BigInt
{
	public:
		enum DIGITS
		{
			MAX_DIGITS_PER_UNIT = 9,
			MAX_NUMBER_PER_UNIT = 1000000000
		};

		BigInt() : _flag(false), _v(1, 0) {}
		BigInt(const std::string s);
		BigInt(const BigInt& rhs);
		BigInt(const int i);
		~BigInt() {}

		// compare with another BigInt
		// if ignoreFlag, compare on absolute value
		// return 1, greater
		// return 0, equal
		// return -1, less
		int compare(const BigInt& rhs, const bool ignoreFlag = false) const;
		int compare(const int i, const bool ignoreFlag = false) const;

		bool operator<(const BigInt& rhs) const {return (compare(rhs) == -1);}
		bool operator<(const int i) const {return compare(i) == -1;}
		bool operator>(const BigInt& rhs) const {return compare(rhs) == 1;}
		bool operator>(const int i) const {return compare(i) == 1;}
		bool operator==(const BigInt& rhs) const {return compare(rhs) == 0;}
		bool operator==(const int i) const {return compare(i) == 0;}
		bool operator!=(const BigInt& rhs) const {return compare(rhs) != 0;}
		bool operator!=(const int i) const {return compare(i) != 0;}
		bool operator<=(const BigInt& rhs) const {return compare(rhs) != 1;}
		bool operator<=(const int i) const {return compare(i) != 1;}
		bool operator>=(const BigInt& rhs) const {return compare(rhs) != -1;}
		bool operator>=(const int i) const {return compare(i) != -1;}

		BigInt operator+(const BigInt& rhs) const;
		BigInt operator+(const int i) const;
		BigInt operator-(const BigInt& rhs) const;
		BigInt operator-(const int i) const;
		BigInt operator*(const BigInt& rhs) const;
		BigInt operator*(const int i) const;

		std::string toString() const;
		friend std::ostream& operator<<(std::ostream& os, const BigInt& bi);
	private:
		bool _flag;
		std::vector<size_t> _v;
		
		// add two non-negative number
		void addValue(const BigInt& a, const BigInt& b, BigInt& result) const;
		// add two ints and carry, return sum and carry
		size_t addInt(const size_t a, const size_t b, size_t& carry) const;
		// subtract one number from another, both are non-negative
		void subValue(const BigInt& a, const BigInt& b, BigInt& result) const;
		// subtract one integer from another, return diff and carry
		size_t subInt(const size_t a, const size_t b, size_t& carry) const;
		// multiplication
		void mulValue(const BigInt& a, const BigInt& b, BigInt& result) const;
		// multiply 2 integers
		size_t mulInt(const size_t a, const size_t b, size_t& carry) const;
		// unset flag if the value is -0
		BigInt& normalize();
};

#endif

