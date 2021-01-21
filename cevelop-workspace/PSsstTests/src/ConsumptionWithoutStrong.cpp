#include "ConsumptionWithoutStrong.h"
#include "pssst.h"
#include "cute.h"

using namespace pssst;
namespace without{

// in C++14 inheritance implies no-aggregate. Therefore the following is moot

#if 0
struct literGas
		: ops<literGas,Additive,Order,Out>{
		double value;
		};


struct literPer100km
		: ops<literPer100km,Eq,Out>{
			double value;
		};

struct X: Eq<X>{
	int value;
	constexpr
	X(int v):value{v}{}
};

static_assert(X{42} != X{43} && X{42} == X{42} ,"");

struct kmDriven:ops<kmDriven,  ScalarMult<double>::apply>{
	double value;
};

static_assert(sizeof(double)==sizeof(kmDriven),"");


namespace myliterals {
constexpr literGas operator"" _l(long double value){
	return literGas{{},static_cast<underlying_value_type<literGas>>(value)};
}
constexpr literGas operator"" _l(unsigned long long value){
	return literGas{{},static_cast<underlying_value_type<literGas>>(value)};
}
constexpr kmDriven operator"" _km(long double value){
	return kmDriven{{},static_cast<underlying_value_type<kmDriven>>(value)};
}
constexpr kmDriven operator"" _km(unsigned long long value){
	return kmDriven{{},static_cast<underlying_value_type<kmDriven>>(value)};
}
}


literPer100km consumption(literGas l, kmDriven km) {
	return {{},(l.value)/(km/100.0).value};
}

void testConsumption1over1(){
	literGas const l {{},1} ;
	kmDriven const km {{}, 1 } ;
	ASSERT_EQUAL((literPer100km{{},100.0}),consumption(l,km));
}

void testConsumption40over500(){
	literGas const l {{}, 40 };
	kmDriven const km {{}, 500 };
	ASSERT_EQUAL((literPer100km{{},8.0}),consumption(l,km));
}

void testConsumtionWithLiterals(){
	using namespace myliterals;
	ASSERT_EQUAL((literPer100km{{},8.0}),consumption(40._l,500_km));
}
#endif
namespace {
// try mix-in without strong...



struct liter : ops<liter,Additive,Order,Out>{
	// needs ctor to avoid need for extra {} below
	constexpr explicit liter(double lit):value{lit}{};
	double value{};
};
static_assert(sizeof(liter)==sizeof(double),""); // ensure empty bases are squashed
static_assert(std::is_trivially_copyable<liter>::value,""); // ensure efficient argument passing
//static_assert(not needsbaseinit<liter>{},"does liter need base init?");


void testLiterWithoutStrong(){
	liter l1 {43. };
	liter l2 {42.1 };
	l2 += liter{0.8};
	ASSERT_EQUAL_DELTA(l1,l2,liter{0.11});
}
}

}
cute::suite make_suite_ConsumptionWithoutStrong() {
	cute::suite s { };
//	s.push_back(CUTE(without::testConsumption1over1));
//	s.push_back(CUTE(without::testConsumption40over500));
	s.push_back(CUTE(without::testLiterWithoutStrong));
//	s.push_back(CUTE(without::testConsumtionWithLiterals));
	return s;
}
