class MockFoo : public Foo {
 public:
  MOCK_CONST_METHOD0(get,
      int());
};
