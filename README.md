# PSsst
Peter Sommerlad's Simple Strong Typing

```C++
struct Word:strong<std::string,Word, Out, Add, Order>{};
Word operator"" _w(char const * const s, size_t l){
  return Word{std::string{s,l}};
}
void testStringAdditionWorks(){
  Word w{};
  w += "Hello "_w;
  w = w + "World!"_w;
  ASSERT_EQUAL("Hello World!"_w, w);
}
```



Using user-defined types instead of the built-in types can greatly increase the usability and understandability of code.
In addition it provides a compile-time safety net of accidental misuse.
In C++ this is especially important, since the language mechanisms inherited from C allow one to easily mix up usages of numeric types including `bool` without producing compiler warnings.
Such code is error prone due to data loss and can accidentally incur undefined behavior, e.g., operations on `uint16_t` values leading to signed integer overflow situations due to integral promotion to `int`.

On the other hand, writing your own types with useful operations can be tedious as well, even if they just wrap a single value of one of the built-in types. 
For example, while it is beneficial to avoid misuse to limit the set of available operations in comparison to the built-in types, it is tedious to implement corresponding operator overloads to provide useful operations, e.g., comparisons. Having many such type wrappers will also incur code duplication. This has led to other strong typing frameworks and even suggestions for changing the C++ language (https://wg21.link/p0109).

Nevertheless, using a simple `struct` wrapping a single value already provides most of the parameter safety, but it means to compute with such a value requires either to take and replace its guts, uglyfying the code, or to implement the required operator overloads.

Based on the assumption that often such user-defined value types are representing a single value of built-in types and will be replacing the use of `int`, `double` or similar built-in types I created this framework/library to allow the simple creation of domain-specific wrappers for such types. 
Pre-defined mix-in base classes provide a set of consistently-defined operators, e.g., `Eq` for equality comparison, `Order` for relational operators, `Additive` for summing and differences, or `Linear` for types that also allow multiplications with a scalar.
The `Out` mix-in base class provides a simple output operator overload that can be customized by optional static members `prefix` and `postfix` to provide some additional formatting.
However, all those mix-in bases are templates parameterized by your type that they extend. If you intend to use several such mix-ins, spelling your own type name again and again is tedious. 
Therefore, a grouping mechanism is provided by the template `ops<mytype,mixinbase...>`.

To allow such a generic implementation of operators we lose the ability to strictly encapsulate the wrapped value. 
But since this is created to replace usages of plain integers or floating point numbers not much is lost by that, for the gain of additional type safety.

There are two options to provide the type and position of the wrapped value:

1. By using the class template `strong<T,mytype>` as the first base class of your wrapper `mytype`, followed by the operations mix-ins.
   ```C++
   struct literPer100km : strong<double,literPer100km,Eq,Out> { };
   ```
   This defines the type literPer100km with the ability to store a `double` value and having operators `== !=` as well as an `std::ostream` output operator defined.
   A value for that type can be created using `literPer100km{5.6}`.
   
2. By just having a single public non-static member variable and inheriting just from the operations mix-ins.
   ```
   struct literPer100km : ops<literPer100km,Eq,Out> {
       double consuption{};
   };
   ```
   This definition creates a type with the same operators than above, but it needs to be initialized with an extra pair of braces for its base class `literPer100km{{},5.6}`. This need can be eliminated by providing a constructor. However, defining such a constructor might eliminate the ability to detect narrowing conversions, unless unwanted conversion constructor overloads are defined as deleted.

The first option allows the type to remain an aggregate, where the first part of the aggregate is the only one required to be intialized: `literPer100km{8}`

The second option makes using the class a bit of a hassle, because the empty base-class requires an empty pair of braces whenever one constructs a value of the type: `literPer100km{{},8}` 
This extra pair of braces can be eliminated by providing a constructor in your class, but that is already boilerplate code I would like to reduce.

The "trick" to achieve the desired code brevity are the C++17 features: 
   * [Extension to aggregate initialization](https://wg21.link/p0017)
   * [Structured Bindings](https://wg21.link/p0144)
   
For a C++14 version of the framework the lack of these features means that one needs to provide a constructor for each strong type and that to allow operator mix-in implementation that the sole data member must follow a specific naming convention (`.value`). Only the second form from the list above makes sense then.

```C++
// TODO example for C++14 branch
```

### List of Mix-in Bases
The following CRTP class templates provide operators that you can pick and choose for strong types with PSsst. NB, binary operators always include corresponding combined assignment operator:
   * Eq: `==` `!=`
   * Order: Eq and `< <= > >=`
   * UPlus: unary `+` (not useful for wrapped built-in types)
   * UMinus: unary `-`, negation
   * Inc: `++` increment (pre and post)
   * Dec: `--` decrement (pre and post)
   * BitOps: unary `~` binary `| & ^` underlying value type must be unsigned type
   * ShiftOps: `>>` `<<` default for rhs number of bits is `unsigned`
   * Add: binary `+`
   * Sub: binary `-`
   * Abs: calling std::abs or another abs function picked by ADL (allows wrapping a wrapper)
   * Rounding: provides `<cmath>` functions converting floating points to integral values
   * ExpLog: provides `<cmath>` functions for exponentiation and logarithms. result is corresponding floating point value
   * Root: provides `<cmath>` `sqrt` and `cbrt`. result is corresponding floating point value
   * Trigonometric: provides `<cmath>` `sin cos tan asin acos atan` functions, result is corresponding floating point value
   * Out: `std::ostream&` output operator. static members in strong type with names `prefix` and `suffix` can control formatting
   * ScalarMult: binary `* /` with a scalar value, `%` is provided if the scalar type is an integer. 
     It is unchecked, if the scalar matches the type of the member, but highly recommended to do so.
     this is a bit tricky to use in general, because it has two template parameters, the second one for the scalar type. Using it in the list of operations templates looks like `ScalarMult<double>::apply`. Therefore, aliases for the common cases are provided:
      * `ScalarMult_d= ScalarMultImpl<TAG,double>;`
      * `ScalarMult_f= ScalarMultImpl<TAG,float>;`
      * `ScalarMult_ld= ScalarMultImpl<TAG,long double>;`
      * `ScalarMult_i= ScalarMultImpl<TAG,int>;
      * `ScalarMult_ll= ScalarMultImpl<TAG,long long>;` 
   * ScalarModulo: `%` base class of ScalarMult if the scalar type is integral. same issue with ScalarMult about 2nd template argument handled there, no separate aliases for it.
   * Additive: `ops<V,UMinus,Abs,Add,Sub,Inc,Dec>`
   * Linear: similar issue as with ScalarMult, since it needs to promote
   
list of pre-combined aliases:



### A better `Bool` than `bool`