#include <big_int.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

BigInt::BigInt(const string s)
{
  if (s.empty()) {
    _flag = false;
    _v.push_back(0);
    return;
  }

  string value(s);
  if (s[0] == '-') {
    _flag = true;
    value.erase(value.begin());
  }
  else {
    _flag = false;
  }

  while (!value.empty()) {
    // get last 9 digits from string
    size_t tmp = 0;
    string digits;
    if (value.size() >= MAX_DIGITS_PER_UNIT) {
      digits.assign(value.end() - MAX_DIGITS_PER_UNIT, value.end());
      value.erase(value.end() - MAX_DIGITS_PER_UNIT, value.end());
    }
    else {
      digits = value;
      value = "";
    }

    size_t n = digits.size();
    for (size_t i = 0; i < n; ++i) {
      char c = digits[i];
      tmp *= 10;
      tmp += int(c - '0');
    }

    _v.push_back(tmp);
  }

  normalize();
}

BigInt::BigInt(const BigInt& rhs)
{
  _flag = rhs._flag;
  _v = rhs._v;
  normalize();
}

BigInt::BigInt(const int i)
{
  size_t tmp = 0;
	if (i < 0)
	{
		_flag = true;
		tmp = 0 - i;
	}
	else
	{
		_flag = false;
		tmp = i;
	}
	
	if (tmp > MAX_NUMBER_PER_UNIT)
	{
	  _v.push_back(tmp % MAX_NUMBER_PER_UNIT);
	  _v.push_back(tmp / MAX_NUMBER_PER_UNIT);
	}
	else
	{
	  _v.push_back(tmp);
	}
}

///////////////////////////////////////////////////////////////////////////////
// compare
int BigInt::compare(const BigInt& rhs, const bool ignoreFlag) const
{
  // compare flag first
  if (!ignoreFlag) {
    if (_flag && !rhs._flag) {
      return -1;
    }
    if (!_flag && rhs._flag) {
      return 1;
    }
  }

  // now both have same flag, compare size
  const size_t n = _v.size();
  const size_t m = rhs._v.size();
  if (n > m) {
    if (!_flag || ignoreFlag) {
      return 1;
    }
    else {
      return -1;
    }
  }

  if (n < m) {
    if (!_flag || ignoreFlag) {
      return -1;
    }
    else {
      return 1;
    }
  }

  // now both have the same size, compare each unit one by one from highest to lowest
  for (size_t i = 0; i < n; ++i) {
    const size_t a = _v[n - i - 1];
    const size_t b = rhs._v[n - i - 1];

    if (a == b) {
      continue;
    }

    if (a > b) {
      if (!_flag || ignoreFlag) {
        return 1;
      }
      else {
        return -1;
      }
    }
    else {
      if (!_flag || ignoreFlag) {
        return -1;
      }
      else {
        return 1;
      }
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// compare
int BigInt::compare(const int i, const bool ignoreFlag) const
{
	BigInt x(i);
	return compare(x, ignoreFlag);
}

///////////////////////////////////////////////////////////////////////////////
// operator +
BigInt BigInt::operator +(const BigInt& rhs) const
{
	BigInt result;
	if (_flag == rhs._flag)
	{
		addValue(*this, rhs, result);
		result._flag = _flag;
	}
	else
	{
	  if (*this > rhs)
	  {
	    subValue(*this, rhs, result);
	  }
	  else
	  {
	    subValue(rhs, *this, result);
	  }
	}
	
	return result.normalize();
}

///////////////////////////////////////////////////////////////////////////////
// operator +
BigInt BigInt::operator +(const int i) const
{
  return *this + BigInt(i);
}

///////////////////////////////////////////////////////////////////////////////
// operator -
BigInt BigInt::operator -(const BigInt& rhs) const
{
  BigInt result;
  if (_flag != rhs._flag)
  {
    // different flag, it is a plus: a - b = a + (-b)
    addValue(*this, rhs, result);
    result._flag = _flag;
  }
  else
  {
    subValue(*this, rhs, result);
    result._flag = (*this < rhs);
  }
  
  return result.normalize();
}

///////////////////////////////////////////////////////////////////////////////
// operator -
BigInt BigInt::operator -(const int i) const
{
  return *this - BigInt(i);
}

///////////////////////////////////////////////////////////////////////////////
// operator *
BigInt BigInt::operator *(const BigInt& rhs) const
{
  BigInt result;

  for (size_t i = _v.size(); i > 0; --i)
  {
    BigInt tmp;
    tmp._v.clear();
    size_t carry = 0;
    for (size_t j = 0; j < rhs._v.size(); ++j)
    {
      tmp._v.push_back(mulInt(_v[i-1], rhs._v[j], carry));
    }
    if (carry)
    {
      tmp._v.push_back(carry);
    }
    result = result + tmp;
    result._v.insert(result._v.begin(), 0);
  }
  
  result._v.erase(result._v.begin());
  result._flag = _flag ^ rhs._flag;
  
  return result.normalize();
}

///////////////////////////////////////////////////////////////////////////////
// toString
string BigInt::toString() const
{
  if (_v.empty()) {
    return "0";
  }

  stringstream ss;
  if (_flag) {
    ss << "-";
  }

  vector<size_t>::const_reverse_iterator crit = _v.rbegin();
  ss << *crit;
  while(_v.rend() != ++crit) {
    ss << setfill('0') << setw(MAX_DIGITS_PER_UNIT) << *crit;
  }

  return ss.str();
}

///////////////////////////////////////////////////////////////////////////////
// private
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// addValue
void BigInt::addValue(const BigInt& a, const BigInt& b, BigInt& result) const
{
	size_t carry = 0;
	const size_t m = a._v.size();
	const size_t n = b._v.size();
	const BigInt& bigger = (m>n) ? a : b;

	size_t i = 0;
	result._v.clear();
	while(i < m && i < n)
	{
		result._v.push_back(addInt(a._v[i], b._v[i], carry));
		++i;
	}
	
	if (m == n)
	{
		if (carry)
		{
			result._v.push_back(carry);
		}
		return;
	}
	else
	{
		while (i < bigger._v.size())
		{
			result._v.push_back(addInt(bigger._v[i], 0, carry));
			++i;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// addInt
size_t BigInt::addInt(const size_t a, const size_t b, size_t& carry) const
{
	size_t sum = a + b + carry;
	if (sum >= MAX_NUMBER_PER_UNIT)
	{
		sum -= MAX_NUMBER_PER_UNIT;
		carry = 1;
	}
	else
	{
		carry = 0;
	}
	
	return sum;
}

///////////////////////////////////////////////////////////////////////////////
// subValue
void BigInt::subValue(const BigInt& a, const BigInt& b, BigInt& result) const
{
	result._v.clear();
	size_t diff = 0;
	size_t carry = 0;
	
	bool aGTb = a.compare(b, true) == 1;  // abs(a) < abs(b)
	const BigInt& bigger = aGTb ? a : b;
	const BigInt& smaller = aGTb ? b : a;
	const size_t m = bigger._v.size();
	const size_t n = smaller._v.size();
	
	// always use bigger - smaller
	size_t i = 0;
	for (; i < n; ++i)
	{
	  result._v.push_back(subInt(bigger._v[i], smaller._v[i], carry));
	}
	
	for (; i < m; ++i)
	{
	  result._v.push_back(subInt(bigger._v[i], 0, carry));
	}
}

///////////////////////////////////////////////////////////////////////////////
// subInt
size_t BigInt::subInt(const size_t a, const size_t b, size_t& carry) const
{
  size_t diff = 0;
	if (a < b + carry)
	{
	  diff = a + MAX_NUMBER_PER_UNIT - b - carry;
	  carry = 1;
	}
	else
	{
	  diff = a - b - carry;
	  carry = 0;
	}
	
	return diff;
}

///////////////////////////////////////////////////////////////////////////////
// mulInt
size_t BigInt::mulInt(const size_t a, const size_t b, size_t& carry) const
{
  uint64_t prod = a * b + carry;
  carry = prod / MAX_NUMBER_PER_UNIT;
  prod %= MAX_NUMBER_PER_UNIT;
}

///////////////////////////////////////////////////////////////////////////////
// normalize
BigInt& BigInt::normalize()
{
  if (_v.empty()) {
    _flag = false;
  }
  
  // remove leading 0
  for (size_t i = _v.size()-1; i > 0 && _v[i] == 0; --i)
  {
    _v.pop_back();
  }
  
  if (_v.size() == 1 && _v[0] == 0) {
    _flag = false;
  }
  
  return *this;
}

///////////////////////////////////////////////////////////////////////////////
// friend
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// operator <<
ostream& operator<<(ostream& os, const BigInt& bi)
{
  os << bi.toString();
  return os;
}