#include "BetterSoftwareTalkExamples.h"
#include "cute.h"
#include "pssst.h"

namespace nostrong {
double consumption(double litergas, double kmdriven) {
  return litergas / (kmdriven / 100);
}
double efficiency(double kmdriven, double litergas) {
  return kmdriven / litergas;
}
void demonstrateStrongTypeProblem() {
  ASSERT_EQUAL(8., consumption(40, 500));
//  ASSERT_EQUAL(8., consumption(500, 40)); // which one is correct?
}
void areConsumptionAndefficiencythesame() {
  ASSERT_EQUAL(efficiency(100, 1), 100 / consumption(1, 100));
}
}

void demonstrateBadOperators() {
  ASSERT_EQUAL(42.0, 7. * ((((true << 3) * 3) % 7) << true));
}

namespace strong_with_struct {
struct literGas {
  double value;
};
struct kmDriven {
  double value;
};
double consumption(literGas liter, kmDriven km) {
  return liter.value / (km.value / 100);
}
double efficiency(kmDriven km, literGas liter) {
  return km.value / liter.value;
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(8., consumption(consumed, distance));
//  ASSERT_EQUAL(8., consumption({500}, {40}));
  // could still be a problem which one is correct?
}

}
namespace strong_with_struct1 {
struct literGas {
  double value;
};
struct kmDriven {
  double value;
};
double consumption(literGas liter, kmDriven km) {
  return liter.value / (km.value / 100);
}
double consumption(kmDriven km, literGas liter) { // 2nd overload
  return liter.value / (km.value / 100);
}
double efficiency(kmDriven km, literGas liter) {
  return km.value / liter.value;
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(8., consumption(consumed, distance));
  //	ASSERT_EQUAL(8., consumption({500},{40}));
  // does not compile due to ambiguity
}

void areConsumptionAndefficiencythesame() {
  literGas consumed{40};
  kmDriven distance{500};

  ASSERT_EQUAL(efficiency(distance, consumed),
      100 / consumption(consumed, distance));
}

}

namespace strong_with_struct_equalityoperator {
struct literGas {
  double value;
};
struct kmDriven {
  double value;
};
struct literper100km {
  double value;
  constexpr bool
  operator==(literper100km const&) const noexcept = default;
  // C++20 defaulted equality
};

literper100km consumption(literGas liter, kmDriven km) {
  return {liter.value/(km.value/100)};
  // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km{8.},consumption(consumed, distance));
  // error: no match for 'operator=='
}

void testThatdefaultedequalityalsodefinesinequality() {
  literper100km const one{1};
  literper100km const two{2};
  ASSERT(one != two);
  ASSERT(!(one != one));
  ASSERT_EQUAL((one == two), !(one != two));
  ASSERT_EQUAL((one == one), !(one != one));
}

}

namespace strong_with_struct_comparison {
struct literGas {
  double value;
};
struct kmDriven {
  double value;
};
struct literper100km {
  double value;
  constexpr auto
  operator<=>(literper100km const &) const noexcept = default;
  // C++20 defaulted 3way comparison
};

literper100km consumption(literGas liter, kmDriven km) {
  return {liter.value/(km.value/100)};
  // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km{8.},
      consumption(consumed, distance));
}

void demonstrateLessThanComparison() {
  ASSERT(consumption(literGas{40}, kmDriven{500})
          < consumption(literGas{9}, kmDriven{110}));
}

void demonstrateLessInAssert() {
  ASSERT_LESS(consumption(literGas{40}, kmDriven{500}),
      consumption(literGas{9}, kmDriven{110}));
  // no operator<<(ostream&, strong_with_struct_comparison::literper100km) -- if failed
}

void testThatdefaultedequalityalsodefinesinequality() {
  literper100km const one{1};
  literper100km const two{2};
  ASSERT(one != two);
  ASSERT(!(one != one));
  ASSERT_EQUAL((one == two), !(one != two));
  ASSERT_EQUAL((one == one), !(one != one));
}

}

// manual output operator
namespace strong_with_struct_output {
struct literGas {
  double value;constexpr auto
  operator<=>(literGas const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct kmDriven {
  double value;constexpr auto
  operator<=>(kmDriven const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct literper100km {
  double value;constexpr auto
  operator<=>(literper100km const &) const noexcept = default;
  // C++20 defaulted comparison
};

std::ostream& operator<<(std::ostream &out, literGas const &val) {
  return out << val.value;
}
std::ostream& operator<<(std::ostream &out, kmDriven const &val) {
  return out << val.value;
}
std::ostream& operator<<(std::ostream &out,
    literper100km const &val) {
  return out << val.value;
}

literper100km consumption(literGas liter, kmDriven km) {
  return {liter.value/(km.value/100)}; // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km{8.}, // {8.1} to show problem if base class mix in is used
      consumption(consumed, distance));
  // demonstrateStrongTypeProblem: literper100km{8.1} == consumption(consumed,distance) expected: 8.1 but was: 8
}

void demonstrateLessThanComparison() {
  ASSERT(
      consumption(literGas{40}, kmDriven{500})
          < consumption(literGas{9}, kmDriven{110}));
}

void demonstrateLessInAssert() {
  ASSERT_LESS(consumption(literGas{40}, kmDriven{500}),
      consumption(literGas{9}, kmDriven{110}));
  // demonstrateLessInAssert: consumption(literGas{40},kmDriven{500}) > consumption(literGas{9},kmDriven{110}) left: 8 right: 8.18182
}

}

namespace strong_with_struct_output_generic {
struct literGas {
  double value;constexpr auto
  operator<=>(literGas const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct kmDriven {
  double value;constexpr auto
  operator<=>(kmDriven const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct literper100km {
  double value;constexpr auto
  operator<=>(literper100km const &) const noexcept = default;
  // C++20 defaulted comparison
};

// full generic
template<typename T>
std::ostream& operator<<(std::ostream &out, T const &val) {
  return out << val.value;
}

literper100km consumption(literGas liter, kmDriven km) {
  return {liter.value/(km.value/100)}; // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km{8.},
      consumption(consumed, distance));
  // demonstrateStrongTypeProblem: literper100km{8.1} == consumption(consumed,distance) expected: 8.1 but was: 8
}

void demonstrateLessThanComparison() {
  ASSERT(
      consumption(literGas{40}, kmDriven{500})
          < consumption(literGas{9}, kmDriven{110}));
}

void demonstrateLessInAssert() {
  ASSERT_LESS(consumption(literGas{40}, kmDriven{500}),
      consumption(literGas{9}, kmDriven{110}));
  // demonstrateLessInAssert: consumption(literGas{40},kmDriven{500}) > consumption(literGas{9},kmDriven{110}) left: 8 right: 8.18182
}

}

// mix-in operations (output operator)

namespace strong_with_struct_output_mixin {

template<typename U>
struct Out {
  friend std::ostream&
  operator<<(std::ostream &out, U const &r) { // operator overload as hidden friend
    auto const& [v] = r;
    // structured binding generic access to single data member struct
    out << v;
    return out;
  }
  	constexpr auto
  	operator<=>(Out<U> const &) const noexcept = default; // C++20 defaulted comparison
};
template<typename U>
struct Cmp {
  constexpr auto
  operator<=>(Cmp<U> const &) const noexcept = default;
};

struct literGas: pssst::ops<literGas, pssst::Eq, pssst::Out> {
  double liter; // different names for values
  constexpr auto
  operator<=>(literGas const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct kmDriven: pssst::strong<double, kmDriven>, pssst::ops<
    kmDriven, pssst::Out> {
  constexpr auto
  operator<=>(kmDriven const &) const noexcept = default;
  // C++20 defaulted comparison
};
struct literper100km: Out<literper100km>{ // CRTP Pattern
  double value;
  constexpr bool operator==(literper100km const&) const noexcept = default; // equality still works
  constexpr auto
  operator<=>(literper100km const &r) const noexcept = default;
  // C++20 defaulted comparison fails to compile because empty base class can not be compared
  //	{ return value <=> r.value; }
};

literper100km consumption(literGas liter, kmDriven km) {
  return { {}, liter.liter/(km.value/100)}; // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{{}, 40};
  kmDriven distance{500};
  ASSERT_EQUAL(literper100km({}, 8.),
      consumption(consumed, distance));
  // demonstrateStrongTypeProblem: literper100km{8.1} == consumption(consumed,distance) expected: 8.1 but was: 8
}

void demonstrateLessThanComparison() {
  ASSERT(
      consumption(literGas{{}, 40}, kmDriven{500})
          < consumption(literGas{{}, 9}, kmDriven{110}));
}

void demonstrateLessInAssert() {
  ASSERT_LESS(consumption(literGas{{}, 40}, kmDriven{500}),
      consumption(literGas{{}, 9}, kmDriven{110}));
  // demonstrateLessInAssert: consumption(literGas{40},kmDriven{500}) > consumption(literGas{9},kmDriven{110}) left: 8 right: 8.18182
}

}

namespace strong_with_cmp_output_mixin {

template<typename U>
struct Out {
  friend std::ostream&
  operator<<(std::ostream &out, U const &r) { // operator overload as hidden friend
    auto const& [v] = r;
    // structured binding generic access to single data member struct
    out << v;
    return out;
  }
};
template<typename U>
struct Cmp {
  friend constexpr auto
  operator<=>(Cmp<U> const &, Cmp<U> const &)  noexcept = default;
};

struct literGas: Out<literGas> {
  double liter; // different names for values
};
struct kmDriven: Out<kmDriven> {
  double km;
};
struct literper100km: Cmp<literper100km>, Out<literper100km>{ // CRTP Pattern
  double value;
  // C++20 defaulted comparison fails to compile because empty base class can not be compared
  //    { return value <=> r.value; }
};

literper100km consumption(literGas liter, kmDriven km) {
  return { {}, {} , liter.liter/(km.km/100)}; // braces needed (aggregate initialization)
}

void demonstrateThatEqualityComparisonIsWrong() {
  literGas consumed{{}, 40};
  kmDriven distance{{}, 500};
  ASSERT_EQUAL(literper100km({}, {}, 8.1),
      consumption(consumed, distance));
  // demonstrateStrongTypeProblem: literper100km{8.1} == consumption(consumed,distance) expected: 8.1 but was: 8
}

void demonstrateLessThanComparisonIsWrong() {
  ASSERT(
      not ( // should not fail!
      consumption(literGas{{}, 40}, kmDriven{{},500})
          < consumption(literGas{{}, 9}, kmDriven{{}, 110})));
}

void demonstrateBaseClassDefaulted3wayComparisonIsWrong() {
  // shows that base class comparison is wrong!
  ASSERT_EQUAL(consumption(literGas{{}, 40}, kmDriven{{},500}),
      consumption(literGas{{}, 9}, kmDriven{{},110}));
  // demonstrateLessInAssert: consumption(literGas{40},kmDriven{500}) > consumption(literGas{9},kmDriven{110}) left: 8 right: 8.18182
}

}
namespace strong_with_cmp_mixin_works {

template<typename U>
struct Out {
  friend std::ostream&
  operator<<(std::ostream &out, U const &r) { // operator overload as hidden friend
    auto const& [v] = r;
    // structured binding generic access to single data member struct
    out << v;
    return out;
  }
};
template<typename U>
struct Cmp {
  friend constexpr auto
  operator<=>(U const &l, U const &r)  noexcept {
    auto const &[vl] = l;
    auto const &[vr] = r;
    return vl <=> vr;
  }
  friend constexpr bool
  operator==(U const &l, U const &r)  noexcept {
    auto const &[vl] = l;
    auto const &[vr] = r;
    return vl == vr;
  } // must be resurrected
};

struct literGas: Out<literGas> {
  double liter; // different names for values
};
struct kmDriven: Out<kmDriven> {
  double km;
};
struct literper100km: Cmp<literper100km>, Out<literper100km>{ // CRTP Pattern
  double value;
  // C++20 defaulted comparison fails to compile because empty base class can not be compared
  //    { return value <=> r.value; }
};

literper100km consumption(literGas liter, kmDriven km) {
  return { {}, {} , liter.liter/(km.km/100)}; // braces needed (aggregate initialization)
}

void demonstrateStrongTypeProblem() {
  literGas consumed{{}, 40};
  kmDriven distance{{}, 500};
  ASSERT_EQUAL(literper100km({}, {}, 8.),
      consumption(consumed, distance));
  // demonstrateStrongTypeProblem: literper100km{8.1} == consumption(consumed,distance) expected: 8.1 but was: 8
}

void demonstrateLessThanComparison() {
  ASSERT(
      consumption(literGas{{}, 40}, kmDriven{{},500})
          < consumption(literGas{{}, 9}, kmDriven{{}, 110}));
}

void demonstrateLessInAssert() {
  ASSERT_LESS(consumption(literGas{{}, 40}, kmDriven{{},500}),
      consumption(literGas{{}, 9}, kmDriven{{},110}));
  // demonstrateLessInAssert: consumption(literGas{40},kmDriven{500}) > consumption(literGas{9},kmDriven{110}) left: 8 right: 8.18182
}

}
namespace strong_type_scalarmult {
using namespace pssst;
struct literGas: Out<literGas> {
  double liter; // different names for values
};
struct kmDriven: Out<kmDriven>, ScalarMultImpl<kmDriven,double> {
  double km;
};
struct literper100km: Order<literper100km>, Out<literper100km>{ // CRTP Pattern
  double value;
};

literper100km consumption(literGas liter, kmDriven km) {
  return { {}, {} , liter.liter/(km*0.01).km}; // can multiply km
}

void demonstrateStrongTypeProblem() {
  literGas l{{}, 40};
  kmDriven km{{}, {}, 500};
  ASSERT_EQUAL(literper100km({}, {}, 8.), consumption(l, km));
}


}

cute::suite make_suite_BetterSoftwareTalkExamples() {
  cute::suite s{};
  s.push_back(CUTE(nostrong::demonstrateStrongTypeProblem));
  s.push_back(CUTE(nostrong::areConsumptionAndefficiencythesame));
  s.push_back(
      CUTE(strong_with_struct::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(strong_with_struct1::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct1::areConsumptionAndefficiencythesame));
  s.push_back(
      CUTE(
          strong_with_struct_equalityoperator::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct_equalityoperator::testThatdefaultedequalityalsodefinesinequality));
  s.push_back(
      CUTE(
          strong_with_struct_comparison::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct_comparison::testThatdefaultedequalityalsodefinesinequality));
  s.push_back(
      CUTE(
          strong_with_struct_comparison::demonstrateLessThanComparison));
  s.push_back(
      CUTE(
          strong_with_struct_comparison::demonstrateLessInAssert));
  s.push_back(
      CUTE(
          strong_with_struct_output::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct_output::demonstrateLessThanComparison));
  s.push_back(
      CUTE(strong_with_struct_output::demonstrateLessInAssert));
  s.push_back(
      CUTE(
          strong_with_struct_output_mixin::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct_output_mixin::demonstrateLessThanComparison));
  s.push_back(
      CUTE(
          strong_with_struct_output_mixin::demonstrateLessInAssert));
  s.push_back(
      CUTE(
          strong_with_struct_output_generic::demonstrateStrongTypeProblem));
  s.push_back(
      CUTE(
          strong_with_struct_output_generic::demonstrateLessThanComparison));
  s.push_back(
      CUTE(
          strong_with_struct_output_generic::demonstrateLessInAssert));
  s.push_back(CUTE(demonstrateBadOperators));
	s.push_back(CUTE(strong_with_cmp_output_mixin::demonstrateThatEqualityComparisonIsWrong));
	s.push_back(CUTE(strong_with_cmp_output_mixin::demonstrateLessThanComparisonIsWrong));
	s.push_back(CUTE(strong_with_cmp_output_mixin::demonstrateBaseClassDefaulted3wayComparisonIsWrong));
	s.push_back(CUTE(strong_with_cmp_mixin_works::demonstrateStrongTypeProblem));
	s.push_back(CUTE(strong_with_cmp_mixin_works::demonstrateLessThanComparison));
	s.push_back(CUTE(strong_with_cmp_mixin_works::demonstrateLessInAssert));
	s.push_back(CUTE(strong_type_scalarmult::demonstrateStrongTypeProblem));
  return s;
}
