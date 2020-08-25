#include "StrongWithEncapsulation.h"
#include "cute.h"
#include "pssst.h"
using namespace pssst;

template <typename V, typename TAG>
struct Strong { // can not merge ops here, because of initializers required for bases depending on incomplete type
	static_assert(std::is_object_v<V>,"must keep real values - no references or incomplete types allowed");
	using value_type=V;
	Strong(value_type v):val{v}{}

	// providig all overloads of get() breaks encapsulation anyway
	template<size_t N = 0>
	constexpr value_type get() const & noexcept { return val; static_assert(N==0); }
	template<size_t N = 0 >
	constexpr value_type& get() & noexcept { return val;static_assert(N==0);}
	template<size_t N = 0>
	constexpr value_type&& get() && noexcept { return std::move(val);static_assert(N==0);}
	template<size_t N = 0>
	constexpr value_type get() const && noexcept { return std::move(val);static_assert(N==0);}

private:
	V val;
};

// not very useful for any subclass of Strong:

namespace std{
template <typename TAG, typename V >
struct tuple_size<Strong<V,TAG>>:std::integral_constant<size_t,1>{};

template <typename V, typename TAG>
struct tuple_element<0,Strong<V,TAG>> {
	using type = V;
};
}

// nneed at least tuple_size and tuple_element specializtaionts for every strong type
// argh, would require macros, naahhh.
struct TestStrong: Strong<double,TestStrong>, ops<TestStrong,Eq,Out,Additive>{};

// would be nice if we could inject namespace-level type definitions in a foreign namespace for non-functions
namespace std{
template <>
struct tuple_size<TestStrong>:std::integral_constant<size_t,1>{};
template <>
struct tuple_element<0,TestStrong> {
	using type = TestStrong::value_type;
};
}

// is all the scaffolding in place?

template<typename S>
decltype(auto) access(S && s){
	auto &&[x] = s;
	return (x);
}


void thisIsAStrongWithEncapsulationTest() {
	Strong<double,class Somehting> x{42};
	auto&& y = access(x);
	ASSERT_EQUAL(42.0,y);
	y+=1;
	ASSERT_EQUAL(43,y);
	ASSERT_EQUAL(y,x.get());
}
void testTestStrongStructuredBinding(){
	TestStrong x{42};
	auto&&y = access(x);
	ASSERT_EQUAL(42.0,y);
	y+=1;
	ASSERT_EQUAL(43,y);
	ASSERT_EQUAL(y,x.get());
}

void testTestStrongStructuredBindingXValue(){
	TestStrong x{42};
	auto&&y = access(std::move(x));
	ASSERT_EQUAL(42.0,y);
	ASSERT_EQUAL(y,x.get());
	ASSERT_EQUAL(TestStrong{42},x);
}



cute::suite make_suite_StrongWithEncapsulation() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAStrongWithEncapsulationTest));
	s.push_back(CUTE(testTestStrongStructuredBinding));
	s.push_back(CUTE(testTestStrongStructuredBindingXValue));
	return s;
}
