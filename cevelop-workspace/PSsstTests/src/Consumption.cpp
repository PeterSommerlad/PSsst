#include "Consumption.h"
#include "pssst.h"
#include "cute.h"

#include <sstream>

using namespace pssst;

struct literGas
		: strong<double,literGas,Additive,Order,Out>{
			constexpr static inline auto  suffix=" l";
		};

struct kmDriven:strong<double,kmDriven>
,ScalarMultImpl<kmDriven,double>,Out<kmDriven>{
	constexpr static inline auto  prefix="driven ";
	constexpr static inline auto  suffix=" km";
};

struct literPer100km
		:strong<double,literPer100km,Eq,Out>{
			constexpr static inline auto  suffix=" l/100km";
		};
struct kmpl
		:strong<double,kmpl,Eq,Out>{
			constexpr static inline auto  suffix=" km/l";
};


constexpr
literPer100km operator/(literGas l, kmDriven km){
	return {l.value/(km/100.0).value};
}

constexpr
kmpl operator/(kmDriven km, literGas l){
	return {km.value/l.value};
}


static_assert(sizeof(double)==sizeof(kmDriven));


namespace myliterals {
constexpr literGas operator"" _l(long double value){
	return literGas{static_cast<literGas::value_type>(value)};
}
constexpr literGas operator"" _l(unsigned long long value){
	return literGas{static_cast<literGas::value_type>(value)};
}
constexpr kmDriven operator"" _km(long double value){
	return kmDriven{static_cast<kmDriven::value_type>(value)};
}
constexpr kmDriven operator"" _km(unsigned long long value){
	return kmDriven{static_cast<kmDriven::value_type>(value)};
}
}


literPer100km consumption(literGas l, kmDriven km) {
	return l/km;
}
kmpl efficiency(literGas l, kmDriven km) {
	return km/l;
}
void testConsumptionVSEfficiency(){
	using namespace myliterals;
	literGas const l { 0xA_l };
	auto const km = 200_km;
	ASSERT_EQUAL(100/(l/km).value, (km/l).value);
}


void testConsumption1over1(){
	literGas const l {1} ;
	kmDriven const km { 1 } ;
	ASSERT_EQUAL(literPer100km{100.0},consumption(l,km));
}
void testEfficiency1over1(){
	literGas const l {1} ;
	kmDriven const km { 1 } ;
	ASSERT_EQUAL(kmpl{1.0},efficiency(l, km));
}

void testConsumption40over500(){
	literGas const l { 40 };
	kmDriven const km { 500 };
	ASSERT_EQUAL(literPer100km{8.0},consumption(l,km));
}

void testConsumtionWithLiterals(){
	using namespace myliterals;
	ASSERT_EQUAL(literPer100km{8.0},consumption(40._l,500_km));
}
void testConsumtionWithOutput(){
	using namespace myliterals;
	std::ostringstream out{};

	ASSERT_EQUAL(literPer100km{8.0},consumption(40._l,500_km));
}
void testLiterOutputWithSuffix(){
	literGas const l { 40 };
	std::ostringstream out{};
	out << l;
	ASSERT_EQUAL("40 l",out.str());
}
void testKmOutputWithPrefixAndSuffix(){
	kmDriven const k { 500 };
	std::ostringstream out{};
	out << k;
	ASSERT_EQUAL("driven 500 km",out.str());
}


// try mix-in without strong...



struct liter : ops<liter,Additive,Order,Out>{
	// needs ctor to avoid need for extra {}
	constexpr explicit liter(double lit):l{lit}{};
	double l{};
};
static_assert(sizeof(liter)==sizeof(double)); // ensure empty bases are squashed
static_assert(std::is_trivially_copyable_v<liter>); // ensure efficient argument passing


void testLiterWithoutStrong(){
	liter l1 {43. };
	liter l2 {42.1 };
	l2 += liter{0.8};
	ASSERT_EQUAL_DELTA(l1,l2,liter{0.11});
}


namespace withoutpssst {
struct literGas{
    double value;
};
struct kmDriven{
    double value;
};
struct literper100km{
    double value;
    constexpr
    bool operator==(literper100km const & r)
    const & noexcept { // make it symmetric
      return value == r.value; // shady bc double
    }
    constexpr
    bool operator!=(literper100km const & r)
    const & noexcept {
      return ! (*this == r);
    }
};

literper100km consumption(literGas liter, kmDriven km){
  return {liter.value/(km.value/100)};
  // braces needed (aggregate initialization)
}
void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km{8}, consumption(consumed,distance));
  // error: no match for 'operator=='
}


}

namespace withsimplebool {

template <typename U>
struct Eq{
  friend constexpr
  bool
  operator==(U const &l, U const& r) noexcept {
    auto const &[vl]=l;
    auto const &[vr]=r;
    return vl == vr;
  }
  friend constexpr
  bool
  operator!=(U const &l, U const& r) noexcept {
    return !(l==r);
  }
};


struct literGas{
    double value;
};
struct kmDriven{
    double value;
};
struct literper100km : Eq<literper100km> {
    double value;
};

literper100km consumption(literGas liter, kmDriven km){
  return {{},liter.value/(km.value/100)};
  // braces needed (aggregate initialization)
}
void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL((literper100km{{},8}), consumption(consumed,distance));
  // error: no match for 'operator=='
}
static_assert(std::is_arithmetic_v<bool>);

constexpr literper100km l12{{},12}, l24{{},24};

// for c++now talk. gives warning.
// static_assert(2 * (l12 != l24));
}

cute::suite make_suite_Consumption() {
	cute::suite s { };
	s.push_back(CUTE(testConsumption1over1));
	s.push_back(CUTE(testConsumption40over500));
	s.push_back(CUTE(testLiterWithoutStrong));
	s.push_back(CUTE(testConsumtionWithLiterals));
	s.push_back(CUTE(testLiterOutputWithSuffix));
	s.push_back(CUTE(testKmOutputWithPrefixAndSuffix));
	s.push_back(CUTE(testConsumtionWithOutput));
	s.push_back(CUTE(testEfficiency1over1));
	s.push_back(CUTE(testConsumptionVSEfficiency));
	s.push_back(CUTE(withoutpssst::demonstrateStrongTypeProblem));
	s.push_back(CUTE(withsimplebool::demonstrateStrongTypeProblem));
	return s;
}
