#include "pssst.h"
namespace pssst {
namespace testing___{

// test Bool properties
static_assert(sizeof(Bool)==sizeof(bool));
static_assert(std::is_trivially_copyable_v<Bool>);
static_assert(std::is_trivially_destructible_v<Bool>);


using ::pssst::underlying_value_type;
static_assert(!detail__::needsbaseinit<int>{},"needsbasinit for built-in");

struct Y {};
struct X:Y{int v;};
static_assert(detail__::needsbaseinit<X>{},"needsbasinit with empty class false");

template <typename U, typename = void>
struct is_vector_space : std::false_type{};
template <typename U>
struct is_vector_space<U, std::void_t<decltype(U::origin)>> : std::true_type{};
template<typename U>
constexpr inline  bool is_vector_space_v=is_vector_space<U>::value;


/// trying anti-compile tests... badly so far
template<typename U, typename=void>
struct doesnt_compile_less_twice:std::true_type{};

struct testless:strong<int,testless,Order>{};

template<typename U>
struct doesnt_compile_less_twice<U,std::void_t<decltype(U{}< U{} < U{})>>:std::false_type{};

#define expr(...) std::void_t<decltype(__VA_ARGS__)>

#define dc(T,...) template<typename U> struct doesnt_compile_less_twice<U,expr(__VA_ARGS__)>:std::false_type{};


static_assert(doesnt_compile_less_twice<testless>{});
//----------


static_assert(!is_vector_space_v<int>,"int is no absolute unit");



struct bla:Linear<int,bla>{};
static_assert(sizeof(bla)==sizeof(int));
static_assert(!is_vector_space_v<bla>,"bla is absolute?");
static_assert(0 == bla{0}.value, "check for subobject warning");
struct blu:create_vector_space<blu,bla>{};
static_assert(sizeof(blu)==sizeof(int));
static_assert(is_vector_space_v<blu>,"blu should be vector space");
//static_assert(bool(blu::origin==blu{0}),"blu origin is zero");
static_assert(blu{42}.value==bla{42}, "rel accessible");
static_assert(std::is_same_v<int,underlying_value_type<bla>>,"..");

constexpr bla x{42};
constexpr bla eightyfour{84};
static_assert(value(-x) == -value(x));
//static_assert(abs(-x) == abs(x)); // std::abs not yet constexpr
static_assert(x == x);
static_assert(x != -x);
static_assert(x + x == eightyfour);
static_assert(x - x == bla{0});
static_assert(x -  -x == eightyfour);
static_assert(x * 2 == eightyfour);
static_assert(2 * x == eightyfour);
static_assert(eightyfour / 2 == x);
static_assert(eightyfour / x == 2);
static_assert(x % 5 == bla{2});

// trait: is_ebo
namespace detail{
template <typename EBase>
struct is_ebo_impl{
	struct non_empty{ char x;};
	struct test:std::conditional_t<std::is_class_v<EBase> && !std::is_final_v<EBase>,EBase,non_empty> {
		char c;
	};
	static_assert(sizeof(non_empty)==sizeof(char),"structs should have optimal size");
	static inline constexpr bool value{sizeof(test)==sizeof(non_empty)};
};
}
template <typename EBase>
struct is_ebo: std::bool_constant<detail::is_ebo_impl<EBase>::value>{};

template <typename U>
constexpr inline bool is_ebo_v=is_ebo<U>::value;

struct dummy{int i;};
static_assert(is_ebo_v<Add<dummy>>,"Add should be EBO enabled");
static_assert(is_ebo_v<Sub<dummy>>,"Sub should be EBO enabled");
static_assert(is_ebo_v<Out<dummy>>,"Out should be EBO enabled");
//static_assert(is_ebo_v<Eq<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Order<dummy>>,"Order should be EBO enabled");
static_assert(is_ebo_v<BitOps<dummy>>,"BitOps should be EBO enabled");
static_assert(is_ebo_v<ShiftOps<dummy>>,"ShiftOps should be EBO enabled");
static_assert(is_ebo_v<Inc<dummy>>,"Inc should be EBO enabled");
static_assert(is_ebo_v<Dec<dummy>>,"Dec should be EBO enabled");
static_assert(is_ebo_v<UPlus<dummy>>,"UPlus should be EBO enabled");
static_assert(is_ebo_v<UMinus<dummy>>,"UMinus should be EBO enabled");
static_assert(is_ebo_v<Value<dummy>>,"Value should be EBO enabled");
static_assert(is_ebo_v<Rounding<dummy>>,"Rounding should be EBO enabled");
static_assert(is_ebo_v<Abs<dummy>>,"Abs should be EBO enabled");
static_assert(is_ebo_v<ExpLog<dummy>>,"ExpLog should be EBO enabled");
static_assert(is_ebo_v<Root<dummy>>,"Root should be EBO enabled");
static_assert(is_ebo_v<Trigonometric<dummy>>,"Trigonometric should be EBO enabled");
static_assert(is_ebo_v<ScalarModulo<dummy,unsigned>>,"ScalarModulo should be EBO enabled");
static_assert(is_ebo_v<ScalarMult<double>::apply<dummy>>,"ScalarModulo should be EBO enabled");

struct dummy_d:ops<dummy,Sub,Add> {
	double v;
};
static_assert(sizeof(double)==sizeof(dummy_d),"dummy_d should be same size as double");

namespace test_ArithModulo{

struct wrong:strong<double,wrong>,ArithMultImpl<wrong,int>, Eq<wrong>{};
static_assert(detail__::has_check_base_v<wrong>);
// fails to compile due to check that multiplication base is same as value type
// static_assert(wrong{1} == wrong{1}/wrong{1});
// static_assert(wrong{1} == wrong{1}*wrong{1});
// static_assert(wrong{1} == wrong{1}%wrong{1});
enum bla:int{};
constexpr bla operator%(bla lhs,bla rhs){ return bla{};}
static_assert(detail__::supports_modulo_v<bla>);

}

struct liter : ops<liter,Additive,Order,Out>{
    // needs ctor to avoid need for extra {} below
    constexpr explicit liter(double lit):l{lit}{};
    double l{};
};
static_assert(sizeof(liter)==sizeof(double)); // ensure empty bases are squashed
static_assert(std::is_trivially_copyable_v<liter>); // ensure efficient argument passing
static_assert(not detail__::needsbaseinit<liter>{},"does liter need base init?");



}

namespace testWithEnum {

enum ue : unsigned {};
constexpr
ue operator |(ue l, ue r){ return ue{unsigned(l)|unsigned(r)}; }

struct sue : strong<ue,sue,BitOps, Value>{};

static_assert(0 == value(sue{} | sue{}));

enum ie : int {};
constexpr
ie operator |(ie l, ie r){ return ie{int(l)|int(r)}; }

struct sie : strong<ie,sie,BitOps, Value>{};

// fails to compile as expected: error: static assertion failed: bitops are only be enabled for unsigned types
//static_assert(0 == value(sie{} | sie{}));

}


}
