#include "pssst.h"
namespace pssst {
namespace testing___{
using ::pssst::underlying_value_type;
static_assert(!needsbaseinit<int>{},"needsbasinit for built-in");

struct Y {};
struct X:Y{int v;};
static_assert(needsbaseinit<X>{},"needsbasinit with empty class false");

template <typename U, typename = std::void_t<>>
struct is_vector_space : std::false_type{};
template <typename U>
struct is_vector_space<U, std::void_t<decltype(U::origin)>> : std::true_type{};
template<typename U>
constexpr inline  bool is_vector_space_v=is_vector_space<U>::value;



static_assert(!is_vector_space_v<int>,"int is no absolute unit");



struct bla:strong<int,bla>,Linear<bla,int>{};
static_assert(sizeof(bla)==sizeof(int));
static_assert(!is_vector_space_v<bla>,"bla is absolute?");
static_assert(0 == bla{0}.value, "check for subobject warning");
struct blu:create_vector_space<blu,bla>{};
static_assert(sizeof(blu)==sizeof(int));
static_assert(is_vector_space_v<blu>,"blu should be vector space");
static_assert(blu::origin==blu{0},"blu origin is zero");
static_assert(blu{42}.value==bla{42}, "rel accessible");
static_assert(std::is_same_v<int,underlying_value_type<bla>>,"..");


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
static_assert(is_ebo_v<Sub<dummy>>,"ScalarMult should be EBO enabled");
static_assert(is_ebo_v<Out<dummy>>,"Out should be EBO enabled");
static_assert(is_ebo_v<Order<dummy>>,"Order should be EBO enabled");
static_assert(is_ebo_v<Eq<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<BitOps<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<ShiftOps<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Inc<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Dec<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<UPlus<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<UMinus<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Value<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Rounding<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Abs<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<ExpLog<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Root<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<Trigonometric<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<ScalarModulo<dummy,int>>,"Eq should be EBO enabled");

struct dummy_d:ops<dummy,Sub,Add> {
	double v;
};
static_assert(sizeof(double)==sizeof(dummy_d),"dummy_d should be same size as double");
}


}
