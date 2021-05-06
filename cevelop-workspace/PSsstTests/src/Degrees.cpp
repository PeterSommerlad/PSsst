#include "Degrees.h"
#include "cute.h"
#include "pssst.h"

//#define USE_STRONG
using namespace pssst;
// vector space: degrees (K and C)
struct degrees:
#ifdef USE_STRONG
		Linear<double, Out>
#else
		LinearOps<degrees,double, Out>{
			explicit constexpr
			degrees(double val) noexcept
			:value{val}{}
			constexpr degrees() noexcept = default;
			double value{};
#endif

		};

static_assert(sizeof(double)==sizeof(degrees));


struct Kelvin:affine_space_for<Kelvin,degrees>{
#ifndef USE_STRONG
	using base = affine_space_for<Kelvin,degrees>;
	explicit constexpr
				Kelvin(degrees val) noexcept
				:base{val}{}
				explicit constexpr
							Kelvin(double val) noexcept
							:base{degrees{val}}{}
			constexpr Kelvin() noexcept = default;

#endif

};

static_assert(sizeof(double)==sizeof(Kelvin));

struct CelsiusZero{
	constexpr degrees operator()() const noexcept{
		return degrees{273.15};
	}
};

struct Celsius:affine_space_for<Celsius,degrees,CelsiusZero> {
	#ifndef USE_STRONG
	using base=affine_space_for<Celsius,degrees,CelsiusZero>;

	explicit constexpr
				Celsius(degrees val) noexcept
				:base{val}{}
				explicit constexpr
							Celsius(double val) noexcept
							:base{degrees{val}}{}
			constexpr Celsius() noexcept = default;

#endif


};
static_assert(sizeof(degrees)==sizeof(Celsius));

constexpr Celsius fromKelvin(Kelvin k) noexcept {
	return convertTo<Celsius>(k);
}

constexpr Kelvin fromCelsius(Celsius c)noexcept{
	return convertTo<Kelvin>(c);
}

struct otherdegrees:ops<otherdegrees,Order,Out>{
	double d;
};

degrees x{5};

void thisIsADegreesTest() {
	degrees hotter{20};
	Celsius spring{15};
	ASSERT_EQUAL(Celsius{35},spring+hotter);
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
	ASSERT_EQUAL(Celsius{20},convertTo<Celsius>(zero));
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

namespace talk {
// vector space degrees for (K and °C)
struct degrees: Linear<double, degrees, Out>{};

static_assert(sizeof(double)==sizeof(degrees));


struct Kelvin:affine_space_for<Kelvin,degrees>{
static constexpr auto suffix="K";
};

static_assert(sizeof(double)==sizeof(Kelvin));

struct CelsiusZero{
    constexpr degrees operator()() const noexcept{
        return degrees{273.15};
    }
};

struct Celsius:affine_space_for<Celsius,degrees,CelsiusZero> {
  static constexpr auto suffix="°C";
  //Unicode: U+00B0, UTF-8: C2 B0
};
static_assert(sizeof(degrees)==sizeof(Celsius));

constexpr Celsius fromKelvin(Kelvin k) noexcept {
    return convertTo<Celsius>(k);
}

constexpr Kelvin fromCelsius(Celsius c)noexcept{
    return convertTo<Kelvin>(c);
}

void thisIsADegreesTest() {
    degrees hotter{20};
    Celsius spring{15};
    ASSERT_EQUAL(Celsius{35},spring+hotter);
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
    ASSERT_EQUAL(Celsius{20},convertTo<Celsius>(zero));
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

}



cute::suite make_suite_Degrees() {
	cute::suite s { };
    s.push_back(CUTE(thisIsADegreesTest));
    s.push_back(CUTE(thisIsAKelvinDegreesTest));
    s.push_back(CUTE(testCelsiusFromKelvin));
    s.push_back(CUTE(testKelvinFromCelsius));
    s.push_back(CUTE(testConversion));
    s.push_back(CUTE(talk::thisIsADegreesTest));
    s.push_back(CUTE(talk::thisIsAKelvinDegreesTest));
    s.push_back(CUTE(talk::testCelsiusFromKelvin));
    s.push_back(CUTE(talk::testKelvinFromCelsius));
    s.push_back(CUTE(talk::testConversion));
	return s;
}
