# Pitfalls

---

### #1

```cpp
// setup
  #include <cstdint>

struct SomeClass {
    virtual ~SomeClass() = default;

    void a();
    void b();
    /// ...

    uint32_t x;
    uint32_t y;
};

/// hide
struct NonVirtualSomeClass {
    ~NonVirtualSomeClass() = default;

    void a();
    void b();
    /// ...

    uint32_t x;
    uint32_t y;
};

void main() {
  static_assert(sizeof(SomeClass) = 16);
  static_assert(sizeof(NonVirtualSomeClass) = 8);
}
/// unhide
```

Note:
* A virtual method, even the destructor, causes a vtable to be generated
* Usually a Virtual destructor is only needed, when derived objects will be deleted through a pointer to the base class.
* If the class is not meant to be inherited from, it does not need to provide a virtual destructor. 
* If a class does not provide virtual member functions is might not be intended to be derived from.

---

### #2

```cpp
// setup
    #include <cstdio>
    #include <string>
    #include <memory>

struct Base {
    /// hide
    //~Base() {puts("~Base");}
    /// unhide
    virtual void f() { std::puts("Base"); }
};

struct Derived : public Base {
    /// hide
    //~Derived() {puts("~Derived");}
    /// unhide
    void f() override { std::puts(m_hello.c_str()); }
    std::string m_hello{"Hello World!"};
};

int main() {
    std::unique_ptr<Base> base_ptr {new Derived{}};
    base_ptr->f();
}

```
Note:
* Static vs. Dynamic binding also applies to destructors.
* Calling the wrong destructor is undefined behavioud and can cause resouce leaks in practice.

---

### #3

```cpp
// setup
    #include <cstdio>
    #include <memory>

struct Base {
    virtual void f(char) { std::puts("Base::f(char)");}
};

struct Derived : public Base {
    void f(int8_t) { std::puts("Derived::f(int8_t)");}
};

int main() {
    std::unique_ptr<Base> base_ptr {new Derived{}};
    int8_t c = 'A';
    base_ptr->f(c);
}
```

Note:
* The parameters of the virtual functions need to match.
* Otherwise a function overload is defined instead of an overried.
* Use the `overried` or `final` keyword to avoid this.

---

### #4

```cpp
// setup
    #include <cstdio>
    #include <memory>

struct IService {
    virtual ~IService() { stop(); }
    virtual void start() {};
    virtual void stop() {};
};

struct Service : public IService {
    void start() override { std::puts("Service started"); };
    void stop() override { std::puts("Service stopped"); };
};

int main() {
    std::unique_ptr<IService> service_ptr {new Service{}};
    service_ptr->start();
}
```

Note:
* When `~IService()` is called, `~Service` has already finished.
* Therefore members of `Service` can no longer be used.
* Hence `~IService()` can only calls `IService::stop()`.

---

### #5

```cpp
// setup
    #include <cstdio>

struct Base {
    virtual void f() {std::puts("Base::f()"); };
};

struct Derived : public Base {
  private:
    void f() override { std::puts("Derived::f()"); };
};

int main() {
    Derived d;
    d.f();
/// hide
    // Base& b = d;
    // b.f();
/// unhide
}
```

Note:
* Derived classes can change the visibility of methods
* This however breaks the "Liskov substitution principle" 