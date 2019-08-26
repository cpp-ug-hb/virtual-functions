
# Virtual Functions

---

### Syntax

* `virtual` <br/>
  _This member function should use dynamic binding._
* `override` <br/>
  _This virtual member function overrides a base class function._
* `final` <br/>
  _This virtual function must not be overriden by subclasses_
* `= 0`  <br/>
  _This is a pure virtual function (abstract) and must be overridden in subclasses._

Note:
* pure virutal functions can still have an implementation in the base class.

---

### Syntax (2)

```cpp
    virtual ret_t funcName();
    virtual ret_t funcName() = 0;
    ret_t funcName() override;
    ret_t funcName() final;
```

---

### Virtual Functions

```cpp
// setup
    #include <iostream> 

struct Base {
    virtual void f() { std::puts("Base"); }
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
* Functions marked `virtual` will not use static binding.
* Dynamic binding causes the funcitons to be selected at run time.
* Derived functions to not need to be marked `virtual`. 
* Using `override` or `final` is highly recommended.
* Dynamic binding only applies for references or pointers. By-Value causes slicing.

---

### Note: visibility

`virtual` can be applied to any visibility:
* `public`
* `protected`
* `private`

---

### Implementation of Virtual (1)

```cpp
// setup
    #include <cstdio> 

struct BaseStruct {
  using f_func = void (*)(BaseStruct&);
  f_func vptr_f;
};

struct DerivedStruct {
  BaseStruct base;
  operator BaseStruct &() { return base; }
  operator BaseStruct const &()  const { return base; }
};

void Base_f(BaseStruct & self) { std::puts("Base"); }
void Derived_f(DerivedStruct & self) { std::puts("Derived"); }
void Derived_f(BaseStruct & self) { 
    Derived_f(reinterpret_cast<DerivedStruct&>(self));
}

void Base_Constructor(BaseStruct & self) {
  self.vptr_f = &Base_f;
}

void Derived_Constructor(DerivedStruct & self) {
  Base_Constructor(self.base);
  self.base.vptr_f = &Derived_f;
}

/// hide
struct Base {
    virtual void f() { std::puts("Base"); }
};

struct Derived : public Base {
    void f() override { std::puts("Derived"); }
};
/// unhide

int main() {
  /// hide
    Derived d;
    Base & b = d;
    b.f();

  /// unhide
    DerivedStruct ds;
    Derived_Constructor(ds);
    BaseStruct & bs  = ds.base;
    bs.vptr_f(bs);
}
```

Note:
* This reinterpret cast might work, but only because BaseStruct ist the first member of DerivedStruct.
* The Compiler can skip casting, it only has to ensure the corrent `this` pointer is passed to the called function.

---

### Virtual Function Table
The previous approach requires one function pointer per virtual member function. :-(

---

### Virtual Function Table

```cpp
// setup
  #include <cstdio> 

struct BaseStruct;

struct BaseVTable {
  using f_func = void (*)(BaseStruct&);
  f_func f;
};

struct BaseStruct {
  BaseVTable* vtable;
};

struct DerivedStruct {
  BaseStruct base;
  operator BaseStruct &() { return base; }
  operator BaseStruct const &()  const { return base; }
};

void Base_f(BaseStruct & self) { std::puts("Base"); }
void Derived_f(DerivedStruct & self) { std::puts("Derived"); }
void Derived_f(BaseStruct & self) { 
    Derived_f(reinterpret_cast<DerivedStruct&>(self));
}

static BaseVTable s_BaseVTable = {
    &Base_f,
};

void Base_Constructor(BaseStruct & self) {
  self.vtable = &s_BaseVTable;
}


static BaseVTable s_DerivedVTable = {
    &Derived_f,
};

void Derived_Constructor(DerivedStruct & self) {
  Base_Constructor(self.base);
  self.base.vtable = &s_DerivedVTable;
}


struct Base {
    virtual void f() { std::puts("Base"); }
};

struct Derived : public Base {
    void f() override { std::puts("Derived"); }
};

int main() {
    Derived d;
    Base & b = d;
    b.f();
    DerivedStruct ds;
    Derived_Constructor(ds);
    BaseStruct & bs  = ds.base;
    bs.vtable->f(bs);
}

static_assert(sizeof(Base) == sizeof(BaseStruct));
static_assert(sizeof(Derived) == sizeof(DerivedStruct));
```

Note:
* The size of the class no longer increases in the number of virtual functions.
* Only one _global_ vtable per class exists.
* The pointer to the correct vtable is set by the constructor.
* Casting the entries of  vtable to the correct type and calculating the offsets to base classes is better left to the compiler. UB lingers here.

---

### VTables and Multiple Inheritance

```cpp
// setup
  #include <cstdio>

struct Base1 {
    virtual void f() { std::puts("Base1::f()"); }
};

struct Base2 {
    virtual void g() { std::puts("Base2::g()"); }
};

struct Derived : public Base1, public Base2 {
};

int main() {
    Derived d;
    d.f(); d.g();
    /// hide
    static_assert(sizeof(Derived) == 16);
    /// unhide
}
```

Note:
* Both the vtable for Base1 and Base2 needs to be present in Derived.
