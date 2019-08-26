# Classes and Member Functions

---

### A Simple Class

```cpp
// setup
  
class Base 
{
  public:
    Base(int i) 
      : m_i(i), m_j{3}
    {
    }
    int f() const { 
        return m_i * m_j;
    }
  private:
    int m_i;
    int m_j;
};

/// hide
int main() {
    Base b{4};
    b.f();
}
/// unhide
```

---

## How would this look in C?

Note:
* The excercise to convert this to a struct and free functions helps in the following execises.

---

### C: Struct with Functions (*)

```cpp
// setup
  
/// hide
class Base 
{
  public:
    Base(int i) 
      : m_i(i)
      , m_j{3}
    {
    }

    int f() const
    { 
        return m_i * m_j;
    }
  private:
    int m_i;
    int m_j;
};
/// unhide

struct BaseStruct {
  int m_i;
  int m_j;
};

void Base_construct(BaseStruct& self, int i) {
  self.m_i = i;
  self.m_j = 3;
}

int Base_f(BaseStruct const& self) {
  return self.m_i * self.m_j;
}

/// hide
static_assert(sizeof(Base) == sizeof(BaseStruct));

int main() {
    Base b{4};
    b.f();

    BaseStruct bs;
    Base_construct(bs, 4);
    Base_f(bs);
}
/// unhide
```

Note:

* C does not have references
* In C++ struct and class are (almost the same)
* default visibilty for class is private
* default visibilty for struct is public
* Livetime and Constructor calls are different for the two examples.

---

### Inheritance (1)

```cpp
// setup
    #include <cstdio> 

struct Base {
    void f() { std::puts("Base"); }
};

struct Derived : public Base {
    void f() { std::puts("Derived"); }
};

int main() {
    Derived d;
    d.f();
}
```

Note:
* "Derived" is printed, because we call f() on a Derived instance.
* The type of `d` is `Derived`, therefore "Derived" is printed.
* Useful for template code, where the specific type is known and used.
* To forbid this, declare `Base` as `final`.

---

### Derived in C*


```cpp
// setup
    #include <cstdio> 

struct BaseStruct {};
struct DerivedStruct {
  BaseStruct base;
  /// hide
  operator BaseStruct &() { return base; }
  operator BaseStruct const &()  const { return base; }
  /// unhide
};

void Base_f(BaseStruct & self) { std::puts("Base"); }
void Derived_f(DerivedStruct & self) { std::puts("Derived"); }

/// hide
struct Base {
    void f() { std::puts("Base"); }
};

struct Derived : public Base {
    void f() { std::puts("Derived"); }
};

/// unhide
int main() {
    /// hide
    Derived d;
    d.f();
    /// unhide
    DerivedStruct ds;
    Derived_f(ds);
}
```


---

### Inheritance (2)

```cpp
// setup
    #include <iostream> 

struct Base {
    void f() { std::puts("Base"); }
/// hide
  //  Base(Base const&) = delete;
  //  Base& operator= (Base const&) = delete;
/// unhide
};

struct Derived : public Base {
    void f() { std::puts("Derived"); }
};

int main() {
    Derived d;
    Base b = d;
    b.f();
}

```

Note:
* The (default) copy constructor for `b` is used as `Base::Base(const&)` can be called with an instance of `Derived`.
* To avoid accidental slicing delete the copy and move constructors ans operators 
* Alternatively make them protected if subclasses shall provide copy operations;

---

### Derived in C* (2)


```cpp
// setup
  #include <cstdio> 
  //
  struct BaseStruct {};
  struct DerivedStruct {
    BaseStruct base;
    /// hide
    operator BaseStruct &() { return base; }
    operator BaseStruct const &()  const { return base; }
    /// unhide
  };
  void Base_f(BaseStruct & self) { std::puts("Base"); }
  void Derived_f(DerivedStruct & self) { std::puts("Derived"); }
  //
  struct Base {
      void f() { std::puts("Base"); }
  };
  struct Derived : public Base {
      void f() { std::puts("Derived"); }
  };

/// unhide
int main() {
    /// hide
    Derived d;
    Base b = d;
    b.f();

    /// unhide
    DerivedStruct ds;
    BaseStruct bs = ds.base;
    Base_f(bs);
}
```

---

### Inheritance (3)

```cpp
// setup
    #include <iostream> 

struct Base {
    void f() { std::puts("Base"); }
};

struct Derived : public Base {
    void f() { std::puts("Derived"); }
};

int main() {
    Derived d;
    Base & b_ref = d;
    b_ref.f();
}

```

Note: 
* No slicing occurs.
* `Base::f()` is not declared virtual. Only Static binding will be used.
* Either declare `Base::f()` virtual or final to avoid this error.

---

### Derived in C* (3)


```cpp
// setup
  #include <cstdio> 
  //
  struct BaseStruct {};
  struct DerivedStruct {
    BaseStruct base;
    /// hide
    operator BaseStruct &() { return base; }
    operator BaseStruct const &()  const { return base; }
    /// unhide
  };
  void Base_f(BaseStruct & self) { std::puts("Base"); }
  void Derived_f(DerivedStruct & self) { std::puts("Derived"); }
  //
  struct Base {
      void f() { std::puts("Base"); }
  };
  struct Derived : public Base {
      void f() { std::puts("Derived"); }
  };

/// unhide
int main() {
    /// hide
    Derived d;
    Base b = d;
    b.f();
    
    /// unhide
    DerivedStruct ds;
    BaseStruct & bs = ds.base;
    Base_f(bs);
}
```
