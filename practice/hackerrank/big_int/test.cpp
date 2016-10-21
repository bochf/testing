#include <gtest/gtest.h>

#include <big_int.h>
#include <string>

using namespace std;
using namespace ::testing;


TEST(TestBigInt, create)
{
	{
		// empty
  BigInt bi;
  EXPECT_EQ("0", bi.toString());
	}
	{
		// normal
		BigInt bi(4378829);
		EXPECT_EQ("4378829", bi.toString());
	}
	{
		// negative
		BigInt bi(-343789);
		EXPECT_EQ("-343789", bi.toString());
	}
	{
		// overflow
		BigInt bi(BigInt::MAX_NUMBER_PER_UNIT+1);
		EXPECT_EQ("1000000001", bi.toString());
	}
}


TEST(TestBigInt, createFromStr)
{
  {
    // big positive number
    string s("12345678901234567890467613134814578423186");
    BigInt bi(s);
    EXPECT_EQ(s, bi.toString());
  }
  {
    // big negative number
    string s("-12345678901234567890467613134814578423186");
    BigInt bi(s);
    EXPECT_EQ(s, bi.toString());
  }
  {
    // -0
    BigInt bi("-0");
    EXPECT_EQ("0", bi.toString());
  }
}


TEST(TestBigInt, compare)
{
	{
		// normal
		BigInt a(123);
		BigInt b(445);
		EXPECT_LT(a, b);
		EXPECT_GT(b, a);
	}
	{
		// positive vs negative
		BigInt a(123);
		BigInt b(-456);
		EXPECT_GT(a, b);
		EXPECT_LT(b, a);
		EXPECT_EQ(-1, a.compare(b, true));
	}
	{
		// different size
		BigInt a("12345678901234567890467613134814578423186");
		BigInt b("34378599098902485930299023");
		BigInt c("-12345678901234567890467613134814578423186");
		EXPECT_GT(a, b);
		EXPECT_GT(b, c);
	}
}


TEST(TestBigInt, equal)
{
	BigInt a(123);
	BigInt a1("123");
	EXPECT_EQ(a, a1);
	
	BigInt b(456);
	BigInt b1(-456);
	EXPECT_NE(b, b1);
	
	BigInt c(789);
	BigInt c1(BigInt::MAX_NUMBER_PER_UNIT-789);
	EXPECT_NE(c, c1);
}


TEST(TestBigInt, add)
{
	BigInt a("45678613789431349841364646");
	BigInt b("7864343184312334184431");
	EXPECT_EQ("45686478132615662175549077", (a+b).toString());
	EXPECT_EQ("45678613789431349841364651", (a+5).toString());
}


TEST(TestBigInt, addNegative)
{
	BigInt a(39824353);
	BigInt b(-981044);
	BigInt c = a + b;
	EXPECT_EQ("38843309", (a+b).toString());
	EXPECT_EQ("38843309", (a+(-981044)).toString());
}


TEST(TestBigInt, sub)
{
	BigInt a("398889898909908990824134238210089203483");
	BigInt b("1678431347610000004610047132467331373121341313178312131");
	BigInt c("-1234564634687321347681074411000487301735003141");
	int d = 99999999;
	BigInt e(0);
	BigInt f("-98865457712005458410054510047");
	
	EXPECT_EQ((a-b).toString(), "-1678431347609999605720148222558340548987103103089108648");
	EXPECT_EQ((b-a).toString(), "1678431347609999605720148222558340548987103103089108648");
	EXPECT_EQ((a-c).toString(), "1234565033577220257590065235134725511824206624");
	EXPECT_EQ((c-a).toString(), "-1234565033577220257590065235134725511824206624");
	EXPECT_EQ((a-d).toString(), "398889898909908990824134238209989203484");
	EXPECT_EQ(a-e, a);
	EXPECT_EQ((c-f).toString(), "-1234564634687321248815616698995028891680493094");
}


TEST(TestBigInt, multiply)
{
	BigInt a(0);
	BigInt b(987651234);
	BigInt c("71113700453794213249713165414");
	BigInt d("-75431885220014613499");
	BigInt e(1);
	
	EXPECT_EQ("0", (a*a).toString());
	EXPECT_EQ("0", (a*b).toString());
	EXPECT_EQ("0", (a*c).toString());
	EXPECT_EQ("0", (a*d).toString());
	EXPECT_EQ("0", (a*e).toString());
	EXPECT_EQ((b*a).toString(), (a*b).toString());
	EXPECT_EQ((c*a).toString(), (a*c).toString());
	EXPECT_EQ((d*a).toString(), (a*d).toString());
	EXPECT_EQ((e*a).toString(), (a*e).toString());
	
	EXPECT_EQ(a, e*a);
	EXPECT_EQ(b, e*b);
	EXPECT_EQ(c, e*c);
	EXPECT_EQ(d, e*d);
	EXPECT_EQ(e, e*e);
	
	EXPECT_EQ((b*c).toString(), "70235534007496214698138357967183220876");
	EXPECT_EQ((c*b).toString(), "70235534007496214698138357967183220876");
	EXPECT_EQ((c*d).toString(), "-5364240490201106227343034512713252251173064323586");
	EXPECT_EQ((d*c).toString(), "-5364240490201106227343034512713252251173064323586");
}

TEST(TestBigInt, fibonacci)
{
	std::vector<BigInt> vb;
	vb.push_back(BigInt(1));
	vb.push_back(BigInt(2));

	for (size_t i = 2; i < 20; ++i)
	{
		vb.push_back(vb[i-1]*vb[i-1] + vb[i-2]);
		cout << "t[" << i << "]=" << vb[i] << endl;
	}
}
