#include "pssst.h"

#include <cstddef>
#include <tuple>

#include <utility>

namespace pssstincub {

using namespace pssst;

namespace detail__ {
//taken from: https://stackoverflow.com/a/39784019
struct filler { template< typename type > operator type && (); };

template< typename aggregate, 
          typename index_sequence = std::index_sequence<>, 
          typename = void >
struct aggregate_arity
        : index_sequence
{

};

template< typename aggregate, 
          std::size_t ...indices >
struct aggregate_arity< aggregate, 
                        std::index_sequence< indices... >, 
                        std::void_t< decltype(aggregate{(indices, std::declval< filler >())..., std::declval< filler >()}) > >
    : aggregate_arity< aggregate, 
                       std::index_sequence< indices..., sizeof...(indices) > >
{

};

template<typename T>
concept
a_class_type = std::is_class_v<T>;






template<typename T,typename =void>
struct
is_strong:std::false_type{};

template<typename T>
struct
is_strong<T,std::void_t< decltype([t=std::declval<T>()]{auto [x]=t;return x;})>> : std::true_type{};

template<typename T>
struct
is_strong<T,std::void_t< decltype([t=std::declval<T>()]{auto [x,y]=t;return x;})>> : std::false_type{};


// does not work, even in C++20... lambda body is hard error on instantiation
//template<typename T>
//constexpr
//bool detect<T,decltype([t=std::declval<T>()]{ auto [x] = t;}, true)(true)> = true;

//template<typename T>
//constexpr bool detect = requires { [](T t){ auto const &[x]=t; return x; }; };
//static_assert(!detail__::detect<empty>); // compile error
//static_assert(detail__::detect<one>);

}
struct empty{};
struct one{int v;};
struct two{int a,b;};
struct ctor{ctor(int x):value{x}{} int value;};// fails, not an aggregate, but allows structured binding

static_assert(detail__::aggregate_arity<empty>::size() == 0);
static_assert(detail__::aggregate_arity<one>::size() == 1);
static_assert(detail__::aggregate_arity<two>::size() == 2);
static_assert(detail__::aggregate_arity<ctor>::size() != 1); // meh...

//-----


template<typename T>
constexpr bool
is_strong_v = std::is_class_v<T> && std::is_aggregate_v<T> && detail__::aggregate_arity<T>::size() == 1;

template<typename T>
concept a_strong_type = is_strong_v<T>;

struct emptytype {};
struct valuetype{ int a;};
struct novaluetype{ int a,b;};
class hidden{int i;};
static_assert(! is_strong_v<int>);
static_assert(! is_strong_v<emptytype>);
static_assert(is_strong_v<valuetype>);
static_assert(! is_strong_v<novaluetype>);
static_assert(! is_strong_v<hidden>);
}
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
struct dummy2{int j;}; // needed to avoid double definition of operator==
static_assert(is_ebo_v<Cmp3way<dummy2>>," should be EBO enabled");
static_assert(is_ebo_v<Eq<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<BitOps<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<ShiftOps<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Inc<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Dec<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<UPlus<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<UMinus<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Value<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Rounding<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Abs<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<ExpLog<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Root<dummy>>," should be EBO enabled");
static_assert(is_ebo_v<Trigonometric<dummy>>,"Eq should be EBO enabled");
static_assert(is_ebo_v<ScalarModulo<dummy,int>>,"Eq should be EBO enabled");

struct dummy_d:ops<dummy,Sub,Add> {
	double v;
};
static_assert(sizeof(double)==sizeof(dummy_d),"dummy_d should be same size as double");
}


struct Int:strong<int,Int>,ops<Int,Cmp3way>, Cmp3way<Int,int>{}; // cannot be Cmp3way<int,Int>

static_assert(Int{3}==Int{3});
static_assert(not (Int{3}!=Int{3}));
static_assert(Int{3}>=Int{3});
static_assert(Int{3}<=Int{3});
static_assert(Int{3}<Int{4});
static_assert(Int{4}>Int{3});
static_assert((Int{3} <=> Int{3}) == std::strong_ordering::equal);

static_assert(Int{3}==3);
static_assert(not (Int{3}!= 3 ));
static_assert(3 >= Int{3});
static_assert(Int{3}<=3);
static_assert(3<Int{4});
static_assert(4>Int{3});
static_assert((3 <=> Int{3}) == std::strong_ordering::equal);


}
