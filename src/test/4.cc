#include <iostream>

class aa
{
public:
  virtual void foo() {
    std::cout << "aa::foo" << std::endl;
  }
};

class bb
{
public:
  virtual void foo() {
    std::cout << "bb::foo"<< std::endl;
  }
};

class cc : public aa, public bb
{
public:
  virtual void foo() {
    std::cout << "cc::foo" << std::endl;
  }
};

int main()
{
  aa *a = new aa;
  bb *b = new bb;
  cc *c = new cc;
  aa *ac = new cc;
  bb *bc = new cc;
  a->foo();
  b->foo();
  c->foo();
  ac->foo();
  bc->foo();
  return 0;
}
