#include <gtest/gtest.h>

class MyMem
{
  public:
    MyMem() {};
    virtual ~MyMem() {};

    virtual void * copy(void * dest, const void * src, const size_t len) {
      if (src == dest) {
        return dest;
      }

      if (src > dest) {
        char * a = static_cast<char*>(dest);
        char * b = (char*)(src);
        size_t n = len;
        while(n--) {
          *a = *b;
          a++;
          b++;
        }
      }
      else {
        char * a = static_cast<char*>(dest) + len - 1;
        char * b = (char*)(src) + len - 1;
        size_t n = len;
        while (n--) {
          *a = *b;
          a--;
          b--;
        }
      }

      return dest;
    }
};

TEST(TestMem, copy)
{
  char src[] = "abcdef";
  char dest[100];
	const size_t len = sizeof(src)/sizeof(char);

  MyMem mymem;
  char * result = static_cast<char*>(mymem.copy(dest, src, len));

  EXPECT_EQ(result, dest);
  EXPECT_STREQ(src, dest);

	std::cout << "src=" << src << ", dest=" << dest << std::endl;
}

TEST(TestMem, copyOverlap1)
{
  char src[100] = "abcdef";
	char ori[100] = "\0";
  char * dest = src + 3;
	const size_t len = sizeof(src)/sizeof(char);

  MyMem mymem;
	mymem.copy(ori, src, len);  // backup the src
  char * result = static_cast<char*>(mymem.copy(dest, src, len));

  EXPECT_EQ(result, dest);
  EXPECT_STREQ(ori, dest);
	EXPECT_STREQ(src, "abcabcdef");

	std::cout << "src=" << src << ", dest=" << dest << std::endl;
}

TEST(TestMem, copyOverlap2)
{
	char dest[100];
	char * src = dest + 3;
  char ori[100] = "abcdef";
	const size_t len = sizeof(ori)/sizeof(char);

  MyMem mymem;
	mymem.copy(src, ori, len);  // set src
  char * result = static_cast<char*>(mymem.copy(dest, src, len));

  EXPECT_EQ(result, dest);
  EXPECT_STREQ(ori, dest);
	EXPECT_STREQ(src, "def");

	std::cout << "src=" << src << ", dest=" << dest << std::endl;
}
