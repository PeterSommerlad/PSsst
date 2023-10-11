#ifndef SRC_PSSST_H_
#define SRC_PSSST_H_
/* PSSST Peter Sommerlad's Simple Strong Typing framework */
// (c) copyright 2020 Peter Sommerlad : Better Software


#include <cmath>
#include <type_traits>
#include <utility>
#include <stdexcept>
namespace pssst{

namespace detail_ {
constexpr inline void
throwing_assert(bool cond, char const *msg) {
  if (not cond) throw std::logic_error(msg);
}
template <typename T,  std::enable_if_t<std::is_class_v<std::remove_reference_t<T>>,int> = 0>
constexpr auto membertype(T x) {
  auto [y]=x;
  return y;
}
template <typename T,  std::enable_if_t<not std::is_class_v<std::remove_reference_t<T>>,int> = 0>
constexpr std::remove_reference_t<T> membertype(T x) {
  return x;
}
// meta-binder for second template argument
template<typename B, template<typename...>class T>
struct bind2{
// the following more generic impl only works on gcc... and is illegal
// http://eel.is/c++draft/temp.res.general#6.3
//  template<typename A, typename ...C>
//  using apply=T<A,B,C...>;
// this simpler and sufficient version also supported by msvc and clang
  template<typename A>
  using apply=T<A,B>;
};
template <typename V, typename TAG>
struct holder_thisclassshouldneverbeusedforderivedtobaseconversion {
  static_assert(std::is_object_v<V>, "must keep real values - no references or incomplete types allowed");
  using value_type = V;
  V value { };
};
// make all regular mix-ins private empty bases... indirection keeps aggregateness
// so only three public bases are available for accidental derived-to-base conversion
// the C++20 version with destroying delete in the holder eliminates two of them, except this one
// therefore the ugly name
template <typename TAG, template <typename ...> class ...CRTP>
struct mixinopsthisclassshouldneverbeusedforderivedtobaseconversion:private CRTP<TAG>...{
};
}
#ifndef NDEBUG
#define pssst_assert(cond) ::pssst::detail_::throwing_assert((cond),#cond)
#else
#define pssst_assert(cond)
#endif


// apply multiple operator mix-ins and keep this an aggregate
template <typename TAG, template <typename ...> class ...CRTP>
struct ops:detail_::mixinopsthisclassshouldneverbeusedforderivedtobaseconversion<TAG, CRTP...>{
};

// Either use this as the first base of TAG or nothing
template <typename V, typename TAG, template<typename...>class ...OPS>
struct strong
    :detail_::holder_thisclassshouldneverbeusedforderivedtobaseconversion<V,TAG>
    ,detail_::mixinopsthisclassshouldneverbeusedforderivedtobaseconversion<TAG,OPS...>
{};


template <typename T>
using underlying_value_type = decltype(detail_::membertype(std::declval<T>()));

// only supports data member in first or last class in hierarchy, not in-between.
// the latter is much too tricky to detect.
// only ever true for aggregates with a single member and empty bases
namespace detail_{
template <typename T, typename= void>
struct needsbaseinit:std::false_type{};

template <typename T>
struct needsbaseinit<T,
std::void_t<decltype(T{{},std::declval<underlying_value_type<T>>()})>
>:std::is_aggregate<T>{};

// construct strong type from wrapped value type
template <typename T, typename S = underlying_value_type<T>>
    constexpr auto retval(S && x) noexcept(std::is_nothrow_constructible_v<T,S> ) {
  if constexpr (needsbaseinit<T>{})
        return T{{},std::move(x)}; // value in most derived
  else
    return T{std::move(x)}; // value in base
}

// check for unsigned but allow enums with an unsigned underlying type that are unscoped

template<typename T, bool>
struct safe_underlying_type {
  using type = T;
};
template<typename T>
struct safe_underlying_type<T,true> {
  using type = std::underlying_type_t<T>;
};
template<typename T>
using safe_underlying_type_t = typename safe_underlying_type<T,std::is_enum_v<T>>::type;

template<typename T>
constexpr bool
is_unsigned_like = std::is_unsigned_v<T> || (std::is_unsigned_v<safe_underlying_type_t<T>>);

}

// access wrapped value operation, regardless of its name, copy, ref and rref
template<typename T>
struct Value {
  friend constexpr
  auto const &
  value(T const &x) noexcept {
    auto const & [y]=x;
    return y;
  }
  friend constexpr
  auto &
  value_ref(T &x) noexcept {
    auto & [y]=x;
    return y;
  }
  friend constexpr
  auto &&
  value_consume(T &&x) noexcept {
    auto && [y]=x;
    return std::move(y);
  }
};

    // for linear operations/vector space
template <typename T>
struct default_zero{
  constexpr T operator()() const noexcept(std::is_nothrow_default_constructible_v<T>){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
    return detail_::retval<T>({});
#pragma GCC diagnostic pop
  }
};





// Operator Mix-in templates. contain operator functions as hidden friends
// rely on structured bindings, so the sole value member must be public
// because friendship is not inherited

// the following would change with operator<=>, but we target C++17 for now
template <typename U>
struct Eq{
  friend constexpr bool
  operator==(U const &l, U const& r) noexcept {
    auto const &[vl]=l;
    auto const &[vr]=r;
    return bool(vl == vr);
  }
  friend constexpr bool
  operator!=(U const &l, U const& r) noexcept {
    return !(l==r);
  }
};

template <typename U>
struct Order: Eq<U> {
  friend constexpr bool
  operator<(U const &l, U const& r) noexcept {
    auto const &[vl]=l;
    auto const &[vr]=r;
    return (vl < vr);
  }
  friend constexpr bool
  operator>(U const &l, U const& r) noexcept {
    return r < l;
  }
  friend constexpr bool
  operator<=(U const &l, U const& r) noexcept {
    return !(r < l);
  }
  friend constexpr bool
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
    auto const &[v]=r;
    return detail_::retval<U>(+v);
  }
};
template <typename U>
struct UMinus{
  friend constexpr U
  operator-(U const &r) noexcept(noexcept(-std::declval<underlying_value_type<U>>())){
    auto const &[v]=r;
    return detail_::retval<U>(-v);
  }
};


// increment and decrement
template <typename U>
struct Inc{
  friend constexpr auto operator++(U &rv) noexcept(noexcept(++std::declval<underlying_value_type<U>&>())) {
    auto &[val]=rv;
    ++val;
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
    auto &[val]=rv;
    --val;
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
    auto &[vl]=l;
    static_assert(detail_::is_unsigned_like<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    auto const &[vr] = r;
    vl = vl | vr;
    return l;
  }
  friend constexpr R
  operator|(R l, R const &r) noexcept {
    return l|=r;
  }
  friend constexpr R&
  operator&=(R& l, R const &r) noexcept {
    auto &[vl]=l;
    static_assert(detail_::is_unsigned_like<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    auto const &[vr] = r;
    vl = vl & vr;
    return l;
  }
  friend constexpr R
  operator&(R l, R const &r) noexcept {
    return l&=r;
  }
  friend constexpr R&
  operator^=(R& l, R const &r) noexcept {
    auto &[vl]=l;
    static_assert(detail_::is_unsigned_like<underlying_value_type<R>>,
        "bitops are only be enabled for unsigned types");
    auto const &[vr] = r;
    vl = vl ^ vr;
    return l;
  }
  friend constexpr R
  operator^(R l, R const &r) noexcept {
    return l^=r;
  }
  friend constexpr R
  operator~(R const &r) noexcept {
    auto const &[v]=r;
    using TO = underlying_value_type<R>;
    static_assert(detail_::is_unsigned_like<TO>,
        "bitops are only be enabled for unsigned types");
    return detail_::retval<R>({static_cast<TO>(~v)});
  }
};


// shifts have common UB, so check it.
// should go to a safe-int lib, but at least do some checks here
// can be turned off with NDEBUG
template<typename L, typename B=unsigned int>
struct ShiftOps{

  friend constexpr L&
  operator<<=(L& l, B r)  {
    auto &[vl]=l;
    using TO = underlying_value_type<L>;
    static_assert(detail_::is_unsigned_like<TO>,
        "bitops are only be enabled for unsigned types");
    if constexpr (detail_::is_unsigned_like<B>){
      pssst_assert(r <= std::numeric_limits<TO>::digits);
      vl = static_cast<TO>(vl << r);
    } else {
      auto const &[vr] = r;
      static_assert(detail_::is_unsigned_like<TO>,"shift by unsigned values only");
      pssst_assert(vr <= std::numeric_limits<TO>::digits);
      vl = static_cast<TO>(vl << vr);
    }
    return l;
  }
  friend constexpr L
  operator<<(L l, B r)  {
    return l<<=r;
  }
  friend constexpr L&
  operator>>=(L& l, B r)  {
    auto &[vl]=l;
    using TO = std::remove_reference_t<decltype(vl)>;
    static_assert(detail_::is_unsigned_like<underlying_value_type<L>>,
        "bitops are only be enabled for unsigned types");
    if constexpr (detail_::is_unsigned_like<B>){
      pssst_assert(r <= std::numeric_limits<underlying_value_type<L>>::digits);
      vl = static_cast<TO>(vl >> r);
    } else {
      auto const &[vr] = r;
      static_assert(detail_::is_unsigned_like<underlying_value_type<L>>,"shift by unsigned values only");
      pssst_assert(vr <= std::numeric_limits<std::remove_reference_t<decltype(vl)>>::digits);
      vl = static_cast<TO>(vl >> vr);
    }
    return l;
  }
  friend constexpr L
  operator>>(L l, B r)  {
    return l>>=r;
  }
};

template <typename L>
using ShiftOpsSym = ShiftOps<L,L>;

/// adding and subtracting

template <typename R>
struct Add {
#define VT underlying_value_type<R>
  friend constexpr R&
  operator+=(R& l, R const &r) noexcept(noexcept(std::declval<VT&>() += std::declval<VT const &>())) {
    auto &[vl]=l;
    auto const &[vr] = r;
    vl += vr;
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
    auto &[vl]=l;
    auto const &[vr] = r;
    vl -= vr;
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
  auto const &[v]=r;\
  using std::fun;\
  return detail_::retval<U>(fun(v)); \
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
template <typename U>
struct ExpLog{
  MakeUnaryMathFunc(exp)
  MakeUnaryMathFunc(exp2)
  MakeUnaryMathFunc(expm1)
  MakeUnaryMathFunc(log)
  MakeUnaryMathFunc(log2)
  MakeUnaryMathFunc(log1p)
};
template <typename U>
struct Root{
  MakeUnaryMathFunc(sqrt)
  MakeUnaryMathFunc(cbrt)
};

template <typename U>
struct Trigonometric{
  MakeUnaryMathFunc(sin)
  MakeUnaryMathFunc(cos)
  MakeUnaryMathFunc(tan)
  MakeUnaryMathFunc(asin)
  MakeUnaryMathFunc(acos)
  MakeUnaryMathFunc(atan)
};

// the following should return the underlying type or double, because they are not linear
// and thus the result should not be of the strong type applied to
#define MakeUnaryMathFuncPlain(fun) \
    friend constexpr auto \
    fun(U const &r) {\
  auto const &[v]=r;\
  using std::fun;\
  return fun(v); \
}

// demo/convenience, not used yet
template <typename U>
struct ExpLogPlain{
  MakeUnaryMathFuncPlain(exp)
  MakeUnaryMathFuncPlain(exp2)
  MakeUnaryMathFuncPlain(expm1)
  MakeUnaryMathFuncPlain(log)
  MakeUnaryMathFuncPlain(log2)
  MakeUnaryMathFuncPlain(log1p)
};
template <typename U>
struct RootPlain{
  MakeUnaryMathFuncPlain(sqrt)
  MakeUnaryMathFuncPlain(cbrt)
};

template <typename U>
struct TrigonometricPlain{
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
using Additive=ops<V,UMinus,Abs,Add,Sub>;

// output is simple. But can be made domain specific by providing
// the static constexpr members prefix and/or suffix in the strong type
// for more complicated things, provide your own << overload

namespace detail_{
// detect prefix and suffix static members for output
#if 0
template<typename U, typename = void>
struct has_prefix : std::false_type {};
template<typename U>
struct has_prefix<U, std::void_t<decltype(std::declval<std::ostream&>() << U::prefix)>>
: std::true_type {};
template<typename U, typename = void>
struct has_suffix: std::false_type {};
template<typename U>
struct has_suffix<U, std::void_t<decltype(std::declval<std::ostream&>() << U::suffix)>>
: std::true_type {};
#endif
template<typename U, typename = void>
constexpr bool has_prefix_v {false};
template<typename U>
constexpr bool has_prefix_v<U, std::void_t<decltype(std::declval<std::ostream&>() << U::prefix)>>
{true};
template<typename U, typename = void>
constexpr bool has_suffix_v{false};
template<typename U>
constexpr bool has_suffix_v<U, std::void_t<decltype(std::declval<std::ostream&>() << U::suffix)>>
{true};
}
template <typename U>
struct Out{
  friend std::ostream&
  operator<<(std::ostream &out, U const &r) {
    if constexpr (detail_::has_prefix_v<U>){
      out << U::prefix;
    }
    auto const &[v]=r;
    out << v;
    if constexpr (detail_::has_suffix_v<U>){
      out << U::suffix;
    }
    return out;
  }
};

// Arithmetic multiplicative operations (not always useful, see below for Linear)
namespace detail_{
template <typename R, typename SCALAR> // need to pass base to know if modulo makes sense
struct check_base_impl {
  friend constexpr bool
  check_base(R l){
    auto [vl] = l;
    static_assert( std::is_same_v<decltype(vl),SCALAR>, "You must use the same value type for ArithMultImpl/MultOps");
    return  std::is_same_v<decltype(vl),SCALAR>;
  }
};

template <typename U, typename = std::void_t<>>
struct has_check_base : std::false_type{};
template <typename U>
struct has_check_base<U, std::void_t<decltype(check_base(std::declval<U>()))>> : std::true_type{};

template <typename U>
constexpr bool has_check_base_v = has_check_base<U>{};

template <typename U, typename = std::void_t<>>
struct supports_modulo : std::false_type{};
template <typename U>
struct supports_modulo<U, std::void_t<decltype(std::declval<U>()%std::declval<U>())>> : std::true_type{};

template<typename U>
constexpr bool
supports_modulo_v = supports_modulo<U>{};

}

// modulo operations only allowed if base type is integral
template <typename R,  bool=false>
struct ModuloImpl{}; // no operator% for non-integral types

template <typename R>
struct ModuloImpl<R,true>{
  friend constexpr
  R&
  operator%=(R& l, R const &r) {
    if constexpr (detail_::has_check_base_v<R>) {
       constexpr  auto checked [[maybe_unused]] = check_base(R{});
    }
    auto &[vl]=l;
    auto &[vr]=r;
    pssst_assert(vr != decltype(vr){}); // division by zero
    vl = vl % vr; // support enums
    return l;
  }
  friend constexpr
  R
  operator%(R l, R const &r) {
    return l%=r;
  }
};

template <typename TAG>
using Modulo = ModuloImpl<TAG,true>; // assume integral if explicitly required

template <typename R>
struct Mul {
  friend constexpr R&
  operator*=(R& l, R const &r) noexcept {
    if constexpr (detail_::has_check_base_v<R>) {
       constexpr auto checked [[maybe_unused]] = check_base(R{});
    }
    auto &[vl]=l;
    auto [vr]=r;
    vl = vl * vr;
    return l;
  }
  friend constexpr R
  operator*(R l, R const &r) noexcept {
    return l *= r;
  }
};
template <typename R>
struct Div {
  friend constexpr R&
  operator/=(R& l, R const &r) {
    if constexpr (detail_::has_check_base_v<R>) {
       constexpr  auto  checked[[maybe_unused]] = check_base(R{});
    }
    auto &[vl]=l;
    auto &[vr]=r;
    // need to check if r is 0 and handle error
    pssst_assert(vr != decltype(vr){});
    vl = vl / vr;
    return l;
  }
  friend constexpr R
  operator/(R l, R const &r) {
    return l /= r;
  }
};

// multiplicative operations with scalar, provide commutativity of *
template <typename R, typename SCALAR> // need to pass base to know if modulo makes sense
struct ArithMultImpl
  : detail_::check_base_impl<R,SCALAR>
  , Mul<R>
  , Div<R>
  , ModuloImpl<R,(std::is_integral_v<SCALAR> && not std::is_same_v<std::decay_t<SCALAR>,bool>) ||
                 (std::is_class_v<R> && detail_::supports_modulo_v<SCALAR>) ||
                 (std::is_enum_v<SCALAR> )>
  {
  };



template <typename V, typename SCALAR, template<typename...>class ...OPS> // can not use underlying_value_type, because V is still incomplete
using MultOps = ops<V, detail_::bind2<SCALAR,ArithMultImpl>::template apply, OPS... >;



// A base for multiplicative types, need to add other ops at discretion
template <typename V, typename TAG, template<typename...>class ...OPS>
using Multiplicative=strong<V,TAG,detail_::bind2<V,ArithMultImpl>::template apply, OPS... >;

// for arithmetic types
template <typename V, typename TAG, template<typename...>class ...OPS>
using Arithmetic=Multiplicative<V,TAG,Additive,Order,Value, OPS... >;



// prepare for 1-D linear space: scalar multiplicative operations

// modulo operations only allowed if base type is integral
template <typename R, typename SCALAR, bool=false>
struct ScalarModulo{}; // no operator% for non-integral types

template <typename R, typename SCALAR>
struct ScalarModulo<R,SCALAR,true>{
  friend constexpr
  R&
  operator%=(R& l, SCALAR const &r) {
    static_assert(std::is_integral_v<SCALAR>);
    auto &[vl]=l;
    static_assert(std::is_integral_v<decltype(vl)>);
    pssst_assert(r != decltype(r){}); // division by zero
    vl %= r;
    return l;
  }
  friend constexpr
  R
  operator%(R l, SCALAR const &r) {
    return l%=r;
  }
  friend constexpr
  SCALAR
  operator%(R l, R const &r) {
    auto const &[vr] = r;
    l %= vr; // cheating to have 0 check only once
    auto [vl] = l;
    return vl;
  }
};


// multiplicative operations with scalar, provide commutativity of *
template <typename R, typename SCALAR>
struct ScalarMultImpl : ScalarModulo<R,SCALAR,std::is_integral_v<SCALAR> && not std::is_same_v<std::decay_t<SCALAR>,bool>> {
  using scalar_type=SCALAR;
  friend constexpr R&
  operator*=(R& l, SCALAR const &r) noexcept {
    auto &[vl]=l;
    vl *= r;
    return l;
  }
  friend constexpr R
  operator*(R l, SCALAR const &r) noexcept {
    return l *= r;
  }
  friend constexpr R
  operator*(SCALAR const & l, R r) noexcept {
    return r *= l;
  }
  friend constexpr R&
  operator/=(R& l, SCALAR const &r) {
    auto &[vl]=l;
    // need to check if r is 0 and handle error
    pssst_assert(r != SCALAR{});
    vl /= r;
    return l;
  }
  friend constexpr R
  operator/(R l, SCALAR const &r) {
    return l /= r;
  }
  friend constexpr SCALAR
  operator/(R l, R const &r) {
    auto const &[vr] = r;
    l /= vr; // cheating to have 0 check only once
    auto [vl] = l;
    return vl;
  }

};

// scalar multiplication must know the scalar type
template<typename SCALAR>
using ScalarMult=detail_::bind2<SCALAR,ScalarMultImpl>;
// usage: strong<unsigned,TAG,ScalarMult<unsigned>::apply,Add>
// or for typical value types one below

template<typename TAG>
//using ScalarMult_d= ScalarMult<double>::apply<TAG>;
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

// LinearImpl needs to be a class to be able to pass it as template template argument to bind2
template <typename TAG, typename SCALAR,  template<typename...>class ...OPS>
using LinearOps = ops<TAG,     ScalarMult<SCALAR>::template apply, Additive, Order, Value, OPS... >;
template <typename BASE, typename TAG, template<typename...>class ...OPS>
using Linear=strong<BASE, TAG, ScalarMult<BASE>::template apply, Additive, Order, Value, OPS... >;


// 1 dimensional affine space (Points) = Linear + origin(= ZEROFUNC{}())
template <typename POINT, typename VECTOR_SPACE, typename ZEROFUNC=default_zero<VECTOR_SPACE>>
struct affine_space_for : ops<POINT,Order, Value, Out>{
  using vector_space=VECTOR_SPACE;
  using point=POINT;
  using value_type=underlying_value_type<vector_space>;
  static_assert(std::is_same_v<vector_space,decltype(ZEROFUNC{}())>, "origin must be in domain affine");
  // make origin a function to prevent discrepancies in template instantiations of static inline variables across compilers
  static inline constexpr auto
  origin() noexcept{ return point{detail_::retval<vector_space>(ZEROFUNC{}())};}
  // the following two constructors are deliberately implicit:
  constexpr affine_space_for(vector_space v=origin()) noexcept:value{v}{}
  constexpr affine_space_for(value_type v) noexcept:value{detail_::retval<vector_space>(v)}{}

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
  static_assert(std::is_same_v<
      typename FROM::vector_space
      ,typename TO::vector_space>);
  return detail_::retval<TO>((from.value-(value(TO::origin())-value(FROM::origin()))));
}

} // pssst


#undef pssst_assert // do not leak macro


#endif /* SRC_PSSST_H_ */
