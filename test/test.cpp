template <typename T>
class A {
  public:
  template <typename U>
    void hello(){}

    void foo() {}
    void bar34() {}
};


void foo2() {}
void bar2() {}
int main() {
  A<int> a;
  a.hello<int>();
  a.foo();
}