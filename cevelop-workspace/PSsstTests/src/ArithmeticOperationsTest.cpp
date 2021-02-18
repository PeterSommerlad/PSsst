#include "ArithmeticOperationsTest.h"
#include "cute.h"
#include "pssst.h"

#include <utility>

using namespace pssst;

namespace test_LinearOps {
//Additive, detail__::bind2<BASE,ScalarMultImpl>::template apply, Order, Value
// Additive=ops<V,UMinus,Abs,Add,Sub>
struct td: Linear<double,td, Out>{};

void testValue(){
  td const x{4.2};
  ASSERT_EQUAL(4.2,value(x));
}

void testValueRef(){
  td x{4.2};
  auto&& ref = value_ref(x);

  static_assert(std::is_lvalue_reference_v<decltype(ref)>);

  ASSERT_EQUAL(reinterpret_cast<intptr_t>(&x.value),reinterpret_cast<intptr_t>(&ref));
}
void testValueConsume(){
  td x{4.2};
  auto&& ref = value_consume(std::move(x));
  static_assert(std::is_rvalue_reference_v<decltype(ref)>);
  ASSERT_EQUAL(reinterpret_cast<intptr_t>(&x.value),reinterpret_cast<intptr_t>(&ref));
}

void testUnaryMinus(){
  td const x{4.2};
  td const y{-x};
  ASSERT_EQUAL(value(y),-value(x));
  ASSERT_EQUAL(-value(y),value(x));
}

void testAbs(){
  td const x{4.2};
  td const y{-x};
  ASSERT_EQUAL(value(abs(y)),value(abs(x)));
  ASSERT_EQUAL(abs(y),abs(x));
}

void testEqualityComparison(){
  td const x{4.2};
  ASSERT(x == x);
  ASSERT(x != td{0});
  ASSERT(td{0} == td{0});
}

void testAddBinary(){
  td const expected{42};
  td const res = td{20.5}+td{21.5};
  ASSERT_EQUAL(value(expected),value(res));
  ASSERT_EQUAL(expected,res); // implicitly tests operator==
}

void testAddAssign(){
  td const expected{4.2};
  td res{2.5};
  res += td{1.7};
  ASSERT_EQUAL(expected,res);
}
void testSubBinary(){
  td const expected{42.2};
  td const res = td{80.7} - td{38.5};
  ASSERT_EQUAL(value(expected),value(res));
  ASSERT_EQUAL(expected,res); // implicitly tests operator==
}

void testSubAssign(){
  td const expected{4.2};
  td res{5.9};
  res -= td{1.7};
  ASSERT_EQUAL(expected,res);
}

void testScalarMultFirst(){
  td const expected{4.2};
  ASSERT_EQUAL(expected,2 * td{2.1});
}
void testScalarMultSecond(){
  td const expected{4.2};
  ASSERT_EQUAL(expected,td{10.5} * 0.4);
}
void testScalarMultBy(){
  td x{100.0};
  x *= 4.2/10;
  ASSERT_EQUAL_DELTA(td{42},x,td{0.000000001}); // auto detection of floating point not working on wrapped
  ASSERT_EQUAL(42., value(x));
}
void testScalarMultDivBy(){
  td const expected{4.2};
  ASSERT_EQUAL(expected,td{10.5}/2.5);
}
void testScalarMultDivByAssign(){
  td x{4.2};
  x /= 0.1;
  ASSERT_EQUAL(td{42},x);
}
void testScalarMultDiv(){
  ASSERT_EQUAL(4.2,td{10.5}/td{2.5});
}

void testLessThan(){
  ASSERT_LESS(td{0},td{0.00000000001});
}
void testGreaterThan(){
  ASSERT_GREATER(td{0.1},td{0.00000000001});
}
void testLessEqualThan(){
  ASSERT_LESS_EQUAL(td{0.0},td{-0.0});
}
void testGreaterEqualThan(){
  ASSERT_GREATER_EQUAL(td{-0.0},td{+0.0});
}

}

namespace test_arithmetic{
struct td: Arithmetic<int,td, Out>{};
void testValue(){
  td const x{42};
  ASSERT_EQUAL(42,value(x));
}

void testValueRef(){
  td x{42};
  auto&& ref = value_ref(x);

  static_assert(std::is_lvalue_reference_v<decltype(ref)>);

  ASSERT_EQUAL(reinterpret_cast<intptr_t>(&x.value),reinterpret_cast<intptr_t>(&ref));
}
void testValueConsume(){
  td x{42};
  auto&& ref = value_consume(std::move(x));
  static_assert(std::is_rvalue_reference_v<decltype(ref)>);
  ASSERT_EQUAL(reinterpret_cast<intptr_t>(&x.value),reinterpret_cast<intptr_t>(&ref));
}

void testUnaryMinus(){
  td const x{42};
  td const y{-x};
  ASSERT_EQUAL(value(y),-value(x));
  ASSERT_EQUAL(-value(y),value(x));
}

void testAbs(){
  td const x{42};
  td const y{-x};
  ASSERT_EQUAL(value(abs(y)),value(abs(x)));
  ASSERT_EQUAL(abs(y),abs(x));
}

void testEqualityComparison(){
  td const x{42};
  ASSERT(x == x);
  ASSERT(x != td{0});
  ASSERT(td{0} == td{0});
}

void testAddBinary(){
  td const expected{42};
  td const res = td{20}+td{22};
  ASSERT_EQUAL(value(expected),value(res));
  ASSERT_EQUAL(expected,res); // implicitly tests operator==
}

void testAddAssign(){
  td const expected{42};
  td res{25};
  res += td{17};
  ASSERT_EQUAL(expected,res);
}
void testSubBinary(){
  td const expected{422};
  td const res = td{807} - td{385};
  ASSERT_EQUAL(value(expected),value(res));
  ASSERT_EQUAL(expected,res); // implicitly tests operator==
}

void testSubAssign(){
  td const expected{42};
  td res{59};
  res -= td{17};
  ASSERT_EQUAL(expected,res);
}

void testMult(){
  td const expected{42};
  ASSERT_EQUAL(expected,td{2} * td{21});
}
void testMultAssign(){
  td x{42};
  x *= td{10};
  ASSERT_EQUAL(td{420},x);
}

void testDiv(){
  td const expected{42};
  ASSERT_EQUAL(expected,td{1050}/td{25});
}
void testDivAssign(){
  td x{42};
  x /= td{10};
  ASSERT_EQUAL(td{4},x);
}
void testModulo(){
  td const expected{25};
  ASSERT_EQUAL(expected,td{1050+25}%td{42});
}
void testModuloAssign(){
  td x{42};
  x %= td{10};
  ASSERT_EQUAL(td{2},x);
}

void testLessThan(){
  ASSERT_LESS(td{0},td{1});
}
void testGreaterThan(){
  ASSERT_GREATER(td{1},td{0});
}
void testLessEqualThan(){
  ASSERT_LESS_EQUAL(td{0},td{0});
}
void testGreaterEqualThan(){
  ASSERT_GREATER_EQUAL(td{0},td{0});
}

}


cute::suite make_suite_ArithmeticOperationsTest() {
	cute::suite s { };
	s.push_back(CUTE(test_LinearOps::testValue));
	s.push_back(CUTE(test_LinearOps::testValueRef));
	s.push_back(CUTE(test_LinearOps::testValueConsume));
	s.push_back(CUTE(test_LinearOps::testUnaryMinus));
	s.push_back(CUTE(test_LinearOps::testAbs));
	s.push_back(CUTE(test_LinearOps::testEqualityComparison));
	s.push_back(CUTE(test_LinearOps::testAddBinary));
	s.push_back(CUTE(test_LinearOps::testAddAssign));
	s.push_back(CUTE(test_LinearOps::testSubBinary));
	s.push_back(CUTE(test_LinearOps::testSubAssign));
	s.push_back(CUTE(test_LinearOps::testScalarMultFirst));
	s.push_back(CUTE(test_LinearOps::testScalarMultSecond));
	s.push_back(CUTE(test_LinearOps::testScalarMultDivBy));
	s.push_back(CUTE(test_LinearOps::testScalarMultDiv));
	s.push_back(CUTE(test_LinearOps::testScalarMultBy));
	s.push_back(CUTE(test_LinearOps::testScalarMultDivByAssign));
	s.push_back(CUTE(test_LinearOps::testLessThan));
	s.push_back(CUTE(test_LinearOps::testGreaterThan));
	s.push_back(CUTE(test_LinearOps::testLessEqualThan));
	s.push_back(CUTE(test_LinearOps::testGreaterEqualThan));
	s.push_back(CUTE(test_arithmetic::testValue));
	s.push_back(CUTE(test_arithmetic::testValueRef));
	s.push_back(CUTE(test_arithmetic::testValueConsume));
	s.push_back(CUTE(test_arithmetic::testUnaryMinus));
	s.push_back(CUTE(test_arithmetic::testAbs));
	s.push_back(CUTE(test_arithmetic::testEqualityComparison));
	s.push_back(CUTE(test_arithmetic::testAddBinary));
	s.push_back(CUTE(test_arithmetic::testAddAssign));
	s.push_back(CUTE(test_arithmetic::testSubBinary));
	s.push_back(CUTE(test_arithmetic::testSubAssign));
	s.push_back(CUTE(test_arithmetic::testMult));
	s.push_back(CUTE(test_arithmetic::testDiv));
	s.push_back(CUTE(test_arithmetic::testDivAssign));
	s.push_back(CUTE(test_arithmetic::testLessThan));
	s.push_back(CUTE(test_arithmetic::testGreaterThan));
	s.push_back(CUTE(test_arithmetic::testLessEqualThan));
	s.push_back(CUTE(test_arithmetic::testGreaterEqualThan));
	s.push_back(CUTE(test_arithmetic::testMultAssign));
	s.push_back(CUTE(test_arithmetic::testModulo));
	s.push_back(CUTE(test_arithmetic::testModuloAssign));
	return s;
}
