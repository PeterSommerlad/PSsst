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

namespace detail__ {
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
  using apply=T<A,B,C...>;
};
template <typename V, typename TAG>
struct holder {
  static_assert(std::is_object_v<V>, "must keep real values - no references or incomplete types allowed");
  using value_type = V;
  V value { };
};


}

#ifndef NDEBUG
#define pssst_assert(cond) detail__::throwing_assert((cond),#cond)
#else
#define pssst_assert(cond)
#endif


// apply multiple operator mix-ins and keep this an aggregate
template <typename U, template <typename ...> class ...BS>
struct ops:BS<U>...{};

// Either use this as the first base of TAG or nothing
template <typename V, typename TAG, template<typename...>class ...OPS>
struct strong:detail__::holder<V,TAG>,ops<TAG,OPS...> {
};


template <typename T>
using underlying_value_type = decltype(detail__::membertype(std::declval<T>()));

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
    static_assert(detail__::is_unsigned_v<underlying_value_type<R>>,
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
    static_assert(detail__::is_unsigned_v<underlying_value_type<R>>,
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
    static_assert(detail__::is_unsigned_v<underlying_value_type<R>>,
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
    static_assert(detail__::is_unsigned_v<underlying_value_type<R>>,
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
    static_assert(detail__::is_unsigned_v<TO>,
        "bitops are only be enabled for unsigned types");
    static_assert(detail__::is_unsigned_v<B>,"only shift by unsigned types");
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
    static_assert(detail__::is_unsigned_v<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    static_assert(detail__::is_unsigned_v<B>,"only shift by unsigned types");
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

namespace detail__{
// detect prefix and suffix static members for output
template <typename U, typename = detail__::void_t<>>
struct has_prefixc : std::false_type{};
template <typename U>
struct has_prefixc<U, detail__::void_t<decltype(U::prefix)>> : std::true_type{};
template <typename U, typename = detail__::void_t<>>
struct has_suffixc : std::false_type{};
template <typename U>
struct has_suffixc<U, detail__::void_t<decltype(U::suffix)>> : std::true_type{};
template <typename U>
constexpr bool has_prefix = has_prefixc<U>::value;
template <typename U>
constexpr bool has_suffix = has_suffixc<U>::value;
template<bool b>
using bool_constant = std::conditional_t<b,std::true_type,std::false_type>;
}
template <typename U>
struct Out{
private:
  static
  std::ostream &
  out__(std::ostream &out, U const &r, std::true_type, std::true_type)
  {
    return out << U::prefix << r.value << U::suffix;
  }
  static std::ostream&
  out__(std::ostream &out, U const &r, std::true_type, std::false_type)
  {
      return out << U::prefix << r.value;
  }
  static std::ostream&
  out__(std::ostream &out, U const &r, std::false_type, std::true_type)
  {
    return out << r.value << U::suffix;
  }
  static std::ostream&
  out__(std::ostream &out, U const &r, std::false_type, std::false_type)
  {
    return out << r.value;
  }

  friend std::ostream&
  operator<<(std::ostream &out, U const &r) {
    using namespace detail__;
     return out__(out,r,bool_constant<has_prefix<U>>{},bool_constant<has_suffix<U>>{});
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
struct ScalarMultImpl : ScalarModulo<R,BASE,std::is_integral<BASE>::value && not detail__::is_same_v<std::decay_t<BASE>,bool>> {
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
using ScalarMult=detail__::bind2<BASE,ScalarMultImpl>;
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

// 1 dimensional vector space = Linear + origin(= ZEROFUNC{}())
template <typename ME, typename AFFINE, typename ZEROFUNC=default_zero<AFFINE>>
struct create_vector_space : ops<ME,Order, Out>{
  using affine_space=AFFINE;
  using vector_space=ME;
  using value_type=underlying_value_type<affine_space>;
  static_assert(std::is_same_v<affine_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
  static inline constexpr vector_space origin {retval<affine_space>(ZEROFUNC{}())};
  // the following two constructors are deliberately implicit:
  constexpr create_vector_space(affine_space v=ZEROFUNC{}()):value{v}{}
  constexpr create_vector_space(underlying_value_type<affine_space> v):value{retval<affine_space>(v)}{}

  affine_space value{ZEROFUNC{}()};

  // need to redefine linear operations to take origin into account.
  // linear
  // vs + as
  friend constexpr vector_space&
  operator+=(vector_space& l, affine_space const &r) noexcept(noexcept(l = vector_space{l.value + r})) {
    l = vector_space{l.value + r};
    return l;
  }
  friend constexpr vector_space
  operator+(vector_space l, affine_space const &r) noexcept(noexcept(l += r)) {
    return l += r;
  }
  friend constexpr vector_space
  operator+(affine_space const & l, vector_space r) noexcept(noexcept(r += l)) {
    return r += l;
  }
  // vs - as // caution check if before origin is allowed overflow check
  friend constexpr vector_space&
  operator-=(vector_space& l, affine_space const &r) noexcept(noexcept(l = vector_space{l.value - r})) {
    l = vector_space{l.value - r};
    return l;
  }
  friend constexpr vector_space
  operator-(vector_space l, affine_space const &r) noexcept(noexcept(l -= r)) {
    return l -= r;
  }
  // vs - vs = as
  friend constexpr affine_space
  operator-(vector_space const &l,vector_space const &r) noexcept(noexcept(l.value - r.value)) {
    return l.value - r.value;
  }
  // vs + vs, vs * scalar
  friend constexpr vector_space&
  operator+=(vector_space &l, vector_space const &r) noexcept(noexcept(l = origin + ((l - origin) + (r - origin)))) {
    l = origin + ((l - origin) + (r - origin));
    return l;
  }
  friend constexpr vector_space
  operator+(vector_space l, vector_space const &r) noexcept(noexcept(l+=r)) {
    l += r;
    return l;
  }
  friend constexpr vector_space&
  operator *=(vector_space &l, value_type r) noexcept(noexcept(l = origin + (l - origin) * r)) {
    l = origin + (l - origin) * r ;
    return l;
  }
  friend constexpr vector_space
  operator*(vector_space l, value_type const &r) noexcept(noexcept(l *= r)) {
    return l *= r;
  }
  friend constexpr vector_space
  operator*(value_type const & l, vector_space r) noexcept(noexcept(r *= l)) {
    return r *= l;
  }
  friend constexpr vector_space&
  operator/=(vector_space& l, value_type const &r) {
    // need to check if r is 0 and handle error
    pssst_assert(r != decltype(r){});
    l = vector_space{ l.value / r };
    return l;
  }
  friend constexpr vector_space
  operator/(vector_space l, value_type const &r)  {
    return l /= r;
  }
};
template <typename ME, typename AFFINE, typename ZEROFUNC>
 constexpr typename create_vector_space<ME,AFFINE,ZEROFUNC>::vector_space create_vector_space<ME,AFFINE,ZEROFUNC>::origin;


// must be vector spaces from same affine space, e.g. celsius to kelvin
// TODO: conversion with scaling factor, e.g. fahrenheit to celsius
template<typename TO, typename FROM>
constexpr TO convertTo(FROM from) noexcept{
  static_assert(detail__::is_same_v<
      typename FROM::affine_space
      ,typename TO::affine_space>,"affine spaces must match for conversion");
  return retval<TO>((from.value-(TO::origin.value-FROM::origin.value)));
}

} // pssst


#undef pssst_assert // do not leak macro


#endif /* SRC_PSSST_H_ */
