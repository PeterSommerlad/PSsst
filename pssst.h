#ifndef SRC_PSSST_H_
#define SRC_PSSST_H_
/* PSSST Peter Sommerlad's Simple Strong Typing framework */
// (c) copyright 2020 Peter Sommerlad : Better Software


#include <cmath>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <limits>

namespace pssst{

namespace detail_ {
template<typename T>
constexpr bool is_class_v = std::is_class<T>::value;
template<typename T>
constexpr bool is_object_v = std::is_object<T>::value;
template<typename T>
constexpr bool is_unsigned_v = std::is_unsigned<T>::value;
template<typename T, typename V>
constexpr bool is_same_v = std::is_same<T,V>::value;
template<typename... T>
using void_t = void;
constexpr inline void
throwing_assert(bool cond, char const *msg) {
  if (not cond) throw std::logic_error(msg);
}
template <typename T,  std::enable_if_t<is_class_v<std::remove_reference_t<T>>,int> = 0>
constexpr auto membertype(T x) {
  return x.value;
}
template <typename T,  std::enable_if_t<not is_class_v<std::remove_reference_t<T>>,int> = 0>
constexpr std::remove_reference_t<T> membertype(T x) {
  return x;
}
// meta-binder for second template argument
template<typename B, template<typename...>class T>
struct bind2{
  template<typename A, typename ...C>
  using apply=T<A,B>;
};
template <typename V, typename TAG>
struct holder {
  static_assert(std::is_object<V>::value, "must keep real values - no references or incomplete types allowed");
  using value_type = V;
  V value { };
};
template <typename U, template <typename ...> class ...BS>
struct internal_ops:private BS<U>...{};


}

#ifndef NDEBUG
#define pssst_assert(cond) detail_::throwing_assert((cond),#cond)
#else
#define pssst_assert(cond)
#endif


// apply multiple operator mix-ins and keep this an aggregate
template <typename U, template <typename ...> class ...BS>
struct ops:detail_::internal_ops<U,BS...>{};

// Either use this as the first base of TAG or nothing
template <typename V, typename TAG, template<typename...>class ...OPS>
struct strong:detail_::holder<V,TAG>,detail_::internal_ops<TAG,OPS...> {
  constexpr strong(V val={}) noexcept :detail_::holder<V,TAG>{val}{}
};


template <typename T>
using underlying_value_type = decltype(detail_::membertype(std::declval<T>()));

// only supports data member in first or last class in hierarchy, not in-between.
// the latter is much too tricky to detect.
// only ever true for aggregates with a single member and empty bases


// construct strong type from wrapped value type
template <typename T, typename S = underlying_value_type<T>>
    constexpr auto retval(S && x) noexcept(std::is_nothrow_constructible<T,S>::value ) {
    return T{std::move(x)}; // value in base
}


    // for linear operations/vector space
template <typename T>
struct default_zero{
  constexpr T operator()() const noexcept(std::is_nothrow_default_constructible<T>::value){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    return retval<T>({});
#pragma GCC diagnostic pop
  }
};

// Operator Mix-in templates. contain operator functions as hidden friends
// rely on structured bindings, so the sole value member must be public
// because friendship is not inherited
// allow for a "strong" boolean type, to avoid integral promotion
// assumes comparisons won't throw as well as constructing Bool

// the following would change with operator<=>, but we target C++17 for now
template <typename U, typename Bool=bool>
struct Eq{
  friend constexpr Bool
  operator==(U const &l, U const& r) noexcept {
    return Bool(l.value == r.value);
  }
  friend constexpr Bool
  operator!=(U const &l, U const& r) noexcept {
    return !(l==r);
  }
};

template <typename U, typename Bool=bool>
struct Order: Eq<U,Bool> {
  friend constexpr Bool
  operator<(U const &l, U const& r) noexcept {
    return Bool(l.value < r.value);
  }
  friend constexpr Bool
  operator>(U const &l, U const& r) noexcept {
    return r < l;
  }
  friend constexpr Bool
  operator<=(U const &l, U const& r) noexcept {
    return !(r < l);
  }
  friend constexpr Bool
  operator>=(U const &l, U const& r) noexcept {
    return !(l < r);
  }
};

// unary plus and minus
// plus only for completeness, because it is a no-op for strong types (no promotion)
template <typename U>
struct UPlus{
  friend constexpr U
  operator+(U const &r) noexcept(noexcept(+std::declval<underlying_value_type<U>>())){
    return retval<U>(+r.value);
  }
};
template <typename U>
struct UMinus{
  friend constexpr U
  operator-(U const &r) noexcept(noexcept(-std::declval<underlying_value_type<U>>())){
    return retval<U>(-r.value);
  }
};


// increment and decrement
template <typename U>
struct Inc{
  friend constexpr auto operator++(U &rv) noexcept(noexcept(++std::declval<underlying_value_type<U>&>())) {
    ++rv.value;
    return rv;
  }
  friend constexpr auto operator++(U &rv,int) noexcept(noexcept(++rv)) {
    auto res=rv;
    ++rv;
    return res;
  }
};
template <typename U>
struct Dec{
  friend constexpr auto operator--(U &rv) noexcept(noexcept(--std::declval<underlying_value_type<U>&>())) {
    --rv.value;
    return rv;
  }
  friend constexpr auto operator--(U &rv,int) noexcept(noexcept(--rv)) {
    auto res=rv;
    --rv;
    return res;
  }
};



// bit operations - noexcept, because no overflow can occur
// only allow them, when wrapped type is an unsigned integer
template <typename R>
struct BitOps {
  friend constexpr R&
  operator|=(R& l, R const &r) noexcept {
    static_assert(detail_::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    l.value |= r.value;
    return l;
  }
  friend constexpr R
  operator|(R l, R const &r) noexcept {
    return l|=r;
  }
  friend constexpr R&
  operator&=(R& l, R const &r) noexcept {
    static_assert(detail_::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    l.value &= r.value;
    return l;
  }
  friend constexpr R
  operator&(R l, R const &r) noexcept {
    return l&=r;
  }
  friend constexpr R&
  operator^=(R& l, R const &r) noexcept {
    static_assert(detail_::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    l.value ^= r.value;
    return l;
  }
  friend constexpr R
  operator^(R l, R const &r) noexcept {
    return l^=r;
  }
  friend constexpr R
  operator~(R const &r) noexcept {
    using TO = underlying_value_type<R>;
    static_assert(detail_::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    return retval<R>({static_cast<TO>(~r.value)});
  }
};


// shifts have common UB, so check it.
// should go to a safe-int lib, but at least do some checks here
// can be turned off with NDEBUG
template<typename R, typename B=unsigned int>
struct ShiftOps{

  friend constexpr R&
  operator<<=(R& l, B r)  {
    using TO = underlying_value_type<R>;
    static_assert(detail_::is_unsigned_v<TO>,
        "bitops are only be enabled for unsigned types");
    static_assert(detail_::is_unsigned_v<B>,"only shift by unsigned types");
    pssst_assert(r <= std::numeric_limits<TO>::digits);
    l.value = static_cast<TO>(l.value << r);
    return l;
  }
  friend constexpr R
  operator<<(R l, B r)  {
    return l<<=r;
  }
  friend constexpr R&
  operator>>=(R& l, B r)  {
    using TO = std::remove_reference_t<decltype(l.value)>;
    static_assert(detail_::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    static_assert(detail_::is_unsigned_v<B>,"only shift by unsigned types");
    pssst_assert(r <= std::numeric_limits<underlying_value_type<R>>::digits);
    l.value = static_cast<TO>(l.value >> r);

    return l;
  }
  friend constexpr R
  operator>>(R l, B r)  {
    return l>>=r;
  }
};


/// adding and subtracting

template <typename R>
struct Add {
#define VT underlying_value_type<R>
  friend constexpr R&
  operator+=(R& l, R const &r) noexcept(noexcept(std::declval<VT&>() += std::declval<VT const &>())) {
    l.value += r.value;
    return l;
  }
  friend constexpr R
  operator+(R l, R const &r) noexcept(noexcept(l+=r))  {
    return l+=r;
  }
#undef VT
};
template <typename U>
struct Sub {
#define VT underlying_value_type<U>
  friend constexpr U&
  operator-=(U& l, U const &r) noexcept(noexcept(std::declval<VT&>() -= std::declval<VT const &>())) {
    l.value -= r.value;
    return l;
  }
  friend constexpr U
  operator-(U l, U const &r) noexcept(noexcept(l-=r))  {
    return l-=r;
  }
#undef VT
};

// provide cmath functions, not really great, but some actually work

// this is how many other std::math functions could be supported
#define MakeUnaryMathFunc(fun) \
    friend constexpr U \
    fun(U const &r) {\
  using std::fun;\
  return retval<U>(fun(r.value)); \
}


template <typename U>
struct Abs{ // is part of Additive, because unit tests need that for comparisons of float like types
  MakeUnaryMathFunc(abs)
};
// convenience, not used yet
template <typename U>
struct Rounding {
  MakeUnaryMathFunc(ceil)
  MakeUnaryMathFunc(floor)
  MakeUnaryMathFunc(trunc)
  MakeUnaryMathFunc(round)
  MakeUnaryMathFunc(nearbyint)
  MakeUnaryMathFunc(rint)
};

// the following should return the underlying type or double, because they are not linear
// and thus the result should not be of the strong type applied to
#define MakeUnaryMathFuncPlain(fun) \
    friend constexpr auto \
    fun(U const &r) {\
  using std::fun;\
  return fun(r.value); \
}

// demo/convenience, not used yet
template <typename U>
struct ExpLog{
  MakeUnaryMathFuncPlain(exp)
  MakeUnaryMathFuncPlain(exp2)
  MakeUnaryMathFuncPlain(expm1)
  MakeUnaryMathFuncPlain(log)
  MakeUnaryMathFuncPlain(log2)
  MakeUnaryMathFuncPlain(log1p)
};
template <typename U>
struct Root{
  MakeUnaryMathFuncPlain(sqrt)
  MakeUnaryMathFuncPlain(cbrt)
};

template <typename U>
struct Trigonometric{
  MakeUnaryMathFuncPlain(sin)
  MakeUnaryMathFuncPlain(cos)
  MakeUnaryMathFuncPlain(tan)
  MakeUnaryMathFuncPlain(asin)
  MakeUnaryMathFuncPlain(acos)
  MakeUnaryMathFuncPlain(atan)
};

#undef MakeUnaryMathFunc
#undef MakeUnaryMathFuncPlain

// shorthand for usual additive operations:
template <typename V>
using Additive=ops<V,UPlus,UMinus,Abs,Add,Sub,Inc,Dec>;

// output is simple. But can be made domain specific by providing
// the static constexpr members prefix and/or suffix in the strong type
// for more complicated things, provide your own << overload

namespace detail_{
// detect prefix and suffix static members for output
template <typename U, typename = detail_::void_t<>>
struct has_prefix : std::false_type{};
template <typename U>
struct has_prefix<U, detail_::void_t<decltype(U::prefix)>> : std::true_type{};
template <typename U, typename = detail_::void_t<>>
struct has_suffix : std::false_type{};
template <typename U>
struct has_suffix<U, detail_::void_t<decltype(U::suffix)>> : std::true_type{};
}
template <typename U>
class Out{
  static
  std::ostream &
  print(std::ostream &out, U const &r, std::true_type, std::true_type)
  {
    return out << U::prefix << r.value << U::suffix;
  }
  static std::ostream&
  print(std::ostream &out, U const &r, std::true_type, std::false_type)
  {
      return out << U::prefix << r.value;
  }
  static std::ostream&
  print(std::ostream &out, U const &r, std::false_type, std::true_type)
  {
    return out << r.value << U::suffix;
  }
  static std::ostream&
  print(std::ostream &out, U const &r, std::false_type, std::false_type)
  {
    return out << r.value;
  }
  friend std::ostream&
  operator<<(std::ostream &out, U const &r) {
    using namespace detail_;
     return print(out,r,has_prefix<U>{},has_suffix<U>{});
  }
};


// prepare for 1-D linear space: scalar multiplicative operations

// modulo operations only allowed if base type is integral
template <typename R, typename BASE, bool=false>
struct ScalarModulo{}; // no operator% for non-integral types

template <typename R, typename BASE>
struct ScalarModulo<R,BASE,true>{
  friend constexpr
  R&
  operator%=(R l, BASE const &r) {
    pssst_assert(r != decltype(r){});
    l.value %= r;
    return l;
  }
  friend constexpr
  R
  operator%(R l, BASE const &r) {
    return l%=r;
  }
};



// multiplicative operations with scalar, provide commutativity of *
template <typename R, typename BASE>
struct ScalarMultImpl : ScalarModulo<R,BASE,std::is_integral<BASE>::value && not detail_::is_same_v<std::decay_t<BASE>,bool>> {
  using scalar_type=BASE;
  friend constexpr R&
  operator*=(R& l, BASE const &r) noexcept {
    l.value *= r;
    return l;
  }
  friend constexpr R
  operator*(R l, BASE const &r) noexcept {
    return l *= r;
  }
  friend constexpr R
  operator*(BASE const & l, R r) noexcept {
    return r *= l;
  }
  friend constexpr R&
  operator/=(R& l, BASE const &r) {
    // need to check if r is 0 and handle error
    pssst_assert(r != decltype(r){});
    l.value /= r; // times 1/r could be more efficient
    return l;
  }
  friend constexpr R
  operator/(R l, BASE const &r) {
    return l /= r;
  }
};

// scalar multiplication must know the scalar type
template<typename TAG,typename BASE=double>
using ScalarMult=detail_::bind2<BASE,ScalarMultImpl>;
// usage: strong<unsigned,TAG,ScalarMult<unsigned>::template apply,Add>
// or for typical value types one below

template<typename TAG>
using ScalarMult_d= ScalarMultImpl<TAG,double>;
template<typename TAG>
using ScalarMult_f= ScalarMultImpl<TAG,float>;
template<typename TAG>
using ScalarMult_ld= ScalarMultImpl<TAG,long double>;
template<typename TAG>
using ScalarMult_i= ScalarMultImpl<TAG,int>;
template<typename TAG>
using ScalarMult_ll= ScalarMultImpl<TAG,long long>;



// a 1-d linear space without origin (or implicit zero)
// first define affine space operations and then define vector space according to affine space
// operations in vector space can be extended if origin is zero
template <typename V, typename BASE> // can not use underlying_value_type, because V is still incomplete
using Linear=ops<V, Additive, ScalarMult<BASE>::template apply, Order, Out>;


// usage: strong<unsigned,TAG,Linear<unsigned>::template apply,Add>
// or by directly inheriting for type TAG: struct TAG: Linear<TAG,unsigned>{ unsigned value;}
// or for typical value types one below for use as strong<> or ops<> template arguments

template<typename TAG>
using Linear_d= Linear<TAG,double>;
template<typename TAG>
using Linear_f= Linear<TAG,float>;
template<typename TAG>
using Linear_ld= Linear<TAG,long double>;
template<typename TAG>
using Linear_i= Linear<TAG,int>;
template<typename TAG>
using Linear_ll= Linear<TAG,long long>;

// 1 dimensional affine space (Points) = Linear + origin(= ZEROFUNC{}())
template <typename POINT, typename VECTOR_SPACE, typename ZEROFUNC=default_zero<VECTOR_SPACE>>
struct affine_space_for : ops<POINT,Order, Out>{
  using vector_space=VECTOR_SPACE;
  using point=POINT;
  using value_type=underlying_value_type<vector_space>;
  static_assert(detail_::is_same_v<vector_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
  // make origin a function to prevent discrepancies in template instantiations of static inline variables across compilers
  static constexpr auto origin {retval<vector_space>(ZEROFUNC{}())};
//  static inline constexpr auto
//  origin() noexcept{ return point{detail_::retval<vector_space>(ZEROFUNC{}())};}
  // the following two constructors are deliberately implicit:
  constexpr affine_space_for(vector_space v=origin()) noexcept:value{v}{}
  constexpr affine_space_for(value_type v) noexcept:value{retval<vector_space>(v)}{}

  vector_space value;

  // need to redefine linear operations to take origin into account.
  // linear
  // point + vector
  friend constexpr point&
  operator+=(point& l, vector_space const &r) noexcept(noexcept(l = point{l.value + r})) {
    l = point{l.value + r};
    return l;
  }
  friend constexpr point
  operator+(point l, vector_space const &r) noexcept(noexcept(l += r)) {
    return l += r;
  }
  friend constexpr point
  operator+(vector_space const & l, point r) noexcept(noexcept(r += l)) {
    return r += l;
  }
  // point - vector // caution check if before origin is allowed overflow check
  friend constexpr point&
  operator-=(point& l, vector_space const &r) noexcept(noexcept(l = point{l.value - r})) {
    l = point{l.value - r};
    return l;
  }
  friend constexpr point
  operator-(point l, vector_space const &r) noexcept(noexcept(l -= r)) {
    return l -= r;
  }
  // point - point = vector
  friend constexpr vector_space
  operator-(point const &l,point const &r) noexcept(noexcept(l.value - r.value)) {
    return l.value - r.value;
  }
};


// must be affine spaces from same vector space, e.g. celsius to kelvin
// NOTODO: conversion with scaling factor, e.g. fahrenheit to celsius -> NO, that is Units!
template<typename TO, typename FROM>
constexpr TO convertTo(FROM from) noexcept{
  static_assert(detail_::is_same_v<
      typename FROM::vector_space
      ,typename TO::vector_space>,"vector space must match for affine spaces conversion");
  using vs = typename FROM::vector_space;
  return retval<TO>((from.value-vs{TO::origin.value-FROM::origin.value}));
  //return retval<TO>((from.value-(value(TO::origin)-value(FROM::origin))));
}
} // pssst


#undef pssst_assert // do not leak macro


#endif /* SRC_PSSST_H_ */
