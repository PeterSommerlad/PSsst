#include "pssst_enum_iter.h"
#include "EnumOperators.h"
#include "cute.h"
#include <type_traits>

namespace test{
//using pssst::enum_iteration::begin;
//using pssst::enum_iteration::end;
enum class color { black, blue, green, cyan, red, magenta, yellow, white, limit__=white };
enum class coloroff:unsigned { black = 42, start__=black, blue, green, cyan, red, magenta, yellow, white, limit__=white };
enum class nocolor { black, gray, white };
namespace iter {
enum class color { black, blue, green, cyan, red, magenta, yellow, white, limit__ };
enum class coloroff:unsigned { black = 42, start__=black, blue, green, cyan, red, magenta, yellow, white, limit__ };

// must import corresponding functions and operators for ADL
using pssst::enum_iteration::begin;
using pssst::enum_iteration::end;
using pssst::enum_iteration::operator++;
using pssst::enum_iteration::operator--;

}
// only for tests, otherwise too generic
template < typename E >
std::ostream & operator<<(std::ostream &os, E e){
	return os << static_cast<std::underlying_type_t<E>>(e);
}
}

using namespace test;
void thisIsAEnumOperatorsTest() {
	using namespace pssst::enum_iteration;
	color x = color::blue;
	ASSERT_EQUAL(color::blue, x++);
}
void testEnumOperatorsTest() {
	//using namespace pssst::enum_iteration;
	auto x = iter::color::blue;
	ASSERT_EQUAL(iter::color::green, ++x);
}
void testWrappingIncrement(){
	using namespace pssst::enum_iteration;
	color x = color::white;
	++x;
	ASSERT_EQUAL(color::black, x);
}
void testWrappingIncrementWithStart(){
	using namespace pssst::enum_iteration;
	coloroff x = coloroff::white;
	++x;
	ASSERT_EQUAL(coloroff::black, x);
}

void testNonWrappingIncrement(){
	using namespace pssst::enum_iteration;
	nocolor x = nocolor::black;
	x++; x++ ; x++;
	ASSERT_EQUAL(3, static_cast<std::underlying_type_t<nocolor>>(x));
}

void testDecrementSimple(){
	using namespace pssst::enum_iteration;
	color x = color::blue;
	--x;
	ASSERT_EQUAL(color::black, x);
}
void testDecrementWrap(){
	using namespace pssst::enum_iteration;
	auto x = coloroff::black;
	--x;
	ASSERT_EQUAL(coloroff::white,x);
}
void testDecrementWrapSimple(){
	using namespace pssst::enum_iteration;
	auto x = color::black;
	--x;
	ASSERT_EQUAL(color::white,x);
}

void testRangeFor(){
	using pssst::detail__::ule;

	int counter=0;

	auto b = begin(iter::color{});
//	ASSERT_EQUAL(iter::color::black,*b);
	auto const e = end(iter::color{});
//	ASSERT_EQUAL(iter::color::limit__, *e);
	auto it = e;
	do {
		--it;
		counter -= static_cast<ule<decltype(*it)>>(*it);
	} while (it != b);

	for(auto e:iter::color{}){
		counter += static_cast<ule<decltype(e)>>(e);
	}
	ASSERT_EQUAL(0,counter); // (7 * 8)
}





#if 0 // negative compile test for SFINAE
struct fakeenum{
	operator int() const { return 0;}
	fakeenum(int=0 ){}
};
namespace std{ // cheat
template <>
struct underlying_type<fakeenum>{
	using type= int;
};
}

void testThatIncrementOnlyAppliesToEnums(){
	using namespace pssst;
	fakeenum x;
	++x;
}
#endif



cute::suite make_suite_EnumOperators() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAEnumOperatorsTest));
	s.push_back(CUTE(testRangeFor));
	s.push_back(CUTE(testWrappingIncrement));
	s.push_back(CUTE(testNonWrappingIncrement));
	s.push_back(CUTE(testWrappingIncrementWithStart));
	s.push_back(CUTE(testDecrementSimple));
	s.push_back(CUTE(testDecrementWrap));
	s.push_back(CUTE(testDecrementWrapSimple));
	s.push_back(CUTE(testEnumOperatorsTest));
	return s;
}
