#include "Degrees.h"
#include "cute.h"
#include "pssst.h"

//#define USE_STRONG
using namespace pssst;
// affine space: degrees (K and C)
struct degrees:
#ifdef USE_STRONG
		strong<double,degrees>,
#endif
		Linear<degrees,double>, ops<degrees,Out>{
#ifndef USE_STRONG
			explicit constexpr
			degrees(double val) noexcept
			:value{val}{}
			constexpr degrees() noexcept = default;
			double value{};
#endif

		};

static_assert(sizeof(double)==sizeof(degrees),"");


struct Kelvin:create_vector_space<Kelvin,degrees>{
#ifndef USE_STRONG
	using base = create_vector_space<Kelvin,degrees>;
	explicit constexpr
				Kelvin(degrees val) noexcept
				:base{val}{}
				explicit constexpr
							Kelvin(double val) noexcept
							:base{degrees{val}}{}
			constexpr Kelvin() noexcept = default;

#endif

};

static_assert(sizeof(double)==sizeof(Kelvin),"");

struct CelsiusZero{
	constexpr degrees operator()() const noexcept{
		return retval<degrees>(273.15);
	}
};

struct Celsius:create_vector_space<Celsius,degrees,CelsiusZero> {
	#ifndef USE_STRONG
	using base=create_vector_space<Celsius,degrees,CelsiusZero>;

	explicit constexpr
				Celsius(degrees val) noexcept
				:base{val}{}
				explicit constexpr
							Celsius(double val) noexcept
							:base{degrees{val}}{}
			constexpr Celsius() noexcept = default;

#endif


};
static_assert(sizeof(degrees)==sizeof(Celsius),"");

constexpr Celsius fromKelvin(Kelvin k) noexcept {
	return Celsius{k.value-(Celsius::origin.value - Kelvin::origin.value)};
}

constexpr Kelvin fromCelsius(Celsius c)noexcept{
	return Kelvin{c.value-(Kelvin::origin.value- Celsius::origin.value)};
}

struct otherdegrees:ops<otherdegrees,Order,Out>{
	double d;
};

degrees x{5};

void thisIsADegreesTest() {
	degrees hotter{20};
	Celsius spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Celsius{35},x);
}
void thisIsAKelvinDegreesTest() {
	degrees hotter{20};
	Kelvin spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Kelvin{35},x);
}
void testCelsiusFromKelvin(){
	Kelvin zero{273.15};
	zero += degrees{20};
	ASSERT_EQUAL(Celsius{20},fromKelvin(zero));
}

void testKelvinFromCelsius(){
	Celsius boiling{100};
	ASSERT_EQUAL(Kelvin{373.15},fromCelsius(boiling));
}

void testConversion(){
	Celsius mild{20};
	Kelvin k{convertTo<Kelvin>(mild)};
	ASSERT_EQUAL(Kelvin{293.15},k);
	ASSERT_EQUAL(mild,convertTo<Celsius>(k));
}



cute::suite make_suite_Degrees() {
	cute::suite s { };
	s.push_back(CUTE(thisIsADegreesTest));
	s.push_back(CUTE(thisIsAKelvinDegreesTest));
	s.push_back(CUTE(testCelsiusFromKelvin));
	s.push_back(CUTE(testKelvinFromCelsius));
	s.push_back(CUTE(testConversion));
	return s;
}
