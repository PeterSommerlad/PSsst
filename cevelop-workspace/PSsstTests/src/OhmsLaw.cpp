#include "OhmsLaw.h"
#include "pssst.h"
#include "cute.h"

using namespace pssst;

struct Voltage:Linear<double,Voltage,Out>{};
struct Current:Linear<double,Current,Out>{};
struct Resistance:Linear<double,Resistance,Out>{};

constexpr Voltage operator""_V(unsigned long long val){
    return Voltage{static_cast<double>(val)};
}
constexpr Voltage operator""_V(long double val){
    return Voltage{static_cast<double>(val)};
}
constexpr Current operator""_A(unsigned long long val){
    return Current{static_cast<double>(val)};
}
constexpr Current operator""_A(long double val){
    return Current{static_cast<double>(val)};
}
constexpr Resistance operator""_Ohm(unsigned long long val){
    return Resistance{static_cast<double>(val)};
}
constexpr Resistance operator""_Ohm(long double val){
    return Resistance{static_cast<double>(val)};
}
constexpr Current operator/(Voltage v, Resistance r){
    auto [vv] = v;
    auto [rr] = r;
    return { vv / rr } ;
}
constexpr Resistance operator/(Voltage v, Current c){
    auto [vv] = v;
    auto [cc] = c;
    return { vv / cc } ;
}
constexpr Voltage operator*(Resistance r, Current c){
    auto [rr] = r;
    auto [cc] = c;
    return { rr * cc } ;
}
constexpr Voltage operator*(Current c, Resistance r){
    auto [rr] = r;
    auto [cc] = c;
    return { rr * cc } ;
}

void thisIsAOhmsLawTest() {
    auto result = 10_V/100_Ohm;
    ASSERT_EQUAL(0.1_A, result);
}
void testVoltageOverResistance(){
    auto result = 10_V/0.1_A;
    ASSERT_EQUAL(100_Ohm, result);
}
void testCurrentTimesResistance(){
    auto result = 100_Ohm * 0.1_A;
    ASSERT_EQUAL(10_V, result);

}



cute::suite make_suite_OhmsLaw() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAOhmsLawTest));
	s.push_back(CUTE(testVoltageOverResistance));
	s.push_back(CUTE(testCurrentTimesResistance));
	return s;
}
