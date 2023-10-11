#include "StrongWithConstructor.h"
#include "cute.h"
#include "pssst.h"
#include <stdexcept>
#include <iostream>
#include <ostream>
using namespace pssst;

// possible but not well suited, needs separate thing, because

namespace StrongWithConstructor{
// affine space: degrees (K and C)
struct degrees: strong<double, degrees, Linear_d>{
constexpr degrees(double d={}):strong{d}{}
};

struct Kelvin: affine_space_for<Kelvin,degrees> {
	using base =  affine_space_for;
	constexpr Kelvin(value_type v): base{v} {
		if(v < value_type{} ) throw std::logic_error{"can not have negative K temperature"};
	}
	constexpr Kelvin(vector_space v): base{v} {
		if(v < vector_space{value_type{}}|| v > vector_space{value_type{50}}) {
			std::cerr << "throwing ctor\n" ;

			throw std::logic_error{"can not have negative K temperature"};

		}
	}
};

static_assert(sizeof(Kelvin) == sizeof(Kelvin::vector_space::value_type),"");

void thisIsAKelvinDegreesTest() {
	degrees hotter{20};
	Kelvin spring{15};
	auto x = spring+hotter;
	ASSERT_EQUAL(Kelvin{35},x);
}

void KelvinNegativeThrows() {
	ASSERT_THROWS(Kelvin{-1}, std::logic_error);
}

void KelvinNegativeAfterOperationThrows() {
	Kelvin k{1};
	ASSERT_THROWS((k-degrees{2}), std::logic_error);
}

void KelvinTooPositiveAfterOperationThrows() {
	Kelvin k{1};
	try {
	ASSERT_THROWS((k+=degrees{50}), std::logic_error);
	}catch(...){}
}


}

cute::suite make_suite_StrongWithConstructor() {
	cute::suite s { };
	s.push_back(CUTE(StrongWithConstructor::thisIsAKelvinDegreesTest));
	s.push_back(CUTE(StrongWithConstructor::KelvinNegativeThrows));
	s.push_back(CUTE(StrongWithConstructor::KelvinNegativeAfterOperationThrows));
	s.push_back(CUTE(StrongWithConstructor::KelvinTooPositiveAfterOperationThrows));
	return s;
}
