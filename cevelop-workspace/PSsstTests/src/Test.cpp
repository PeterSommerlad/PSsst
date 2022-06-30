#include "pssst.h"

#include "ArraySizeDiffStrong.h"
#include "Degrees.h"
#include "Consumption.h"
#include "ConsumptionWithoutStrong.h"
#include "StrongWithEncapsulation.h"
#include "StrongWithConstructor.h"
#include "SafeArithmetic.h"
#include "StringOperations.h"
#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"
#include "BooleanTest.h"
#include "BitOperationsTest.h"
#include <string>
#include "EnumOperators.h"
#include "ArithmeticOperationsTest.h"
#include "DoubleErsatz.h"
#include "OhmsLaw.h"
#include "xxx.h"


using namespace pssst;
struct Int: strong<int,Int>,ops<Int,Value,Order,Inc,Add,Out>{
};

namespace {
Int  dummy{42};

int &y = value_ref(dummy);

}
struct Size: strong<unsigned,Size>,ops<Size,Order,Inc,Add,Out> {
};
static_assert(sizeof(Size)==sizeof(unsigned),"no overhead");
static_assert(std::is_trivially_copyable_v<Size>,"should be trivial");

void testSizeworks(){
	Size sz{42};
	//ASSERT_EQUAL(42,sz);// doesn't compile
	//ASSERT_EQUAL(42u,sz);//doesn't compile
	ASSERT_EQUAL(Size{21}+Size{21},sz);
}






	struct uptest:strong<int,uptest>,ops<uptest,UPlus>{};


void testUPlus(){
	uptest one{1};
	ASSERT_EQUAL(one.value,(+one).value);
}
void testUMinus(){
	struct umtest:strong<int,umtest>,ops<umtest,UMinus>{};
	umtest one{1};
	ASSERT_EQUAL(-(one.value),(-one).value);
}
void testUInc(){
	struct uinctest:strong<int,uinctest>,ops<uinctest,Inc>{};
	uinctest var{1};
	ASSERT_EQUAL(2,(++var).value);
	ASSERT_EQUAL(2,(var++).value);
	ASSERT_EQUAL(3,var.value);
}
void testUDec(){
	struct udtest:strong<int,udtest>,ops<udtest,Dec>{};
	udtest var{2};
	ASSERT_EQUAL(1,(--var).value);
	ASSERT_EQUAL(1,(var--).value);
	ASSERT_EQUAL(0,var.value);
}

struct S:strong<std::string,S>,ops<S,Value,Out,Eq>{};

void testWithStringBase(){
	S s{"hello"};
	ASSERT_EQUAL(S{"hello"},s);
}

void testMoveWithStringBase(){
	S s{std::string(1000u,'a')}; // assume beyond SSO
	char const *data = value(s).data();
	ASSERT_EQUAL(1000ul,value(s).size());
	ASSERT_LESS_EQUAL(1000ul,value(s).capacity());
	std::string other{value_consume(std::move(s))};
	ASSERT_EQUAL(1000ul,other.size());
	ASSERT_EQUAL(0ul,value_ref(s).size());
	ASSERT_EQUAL(intptr_t(data),intptr_t(other.data()));
}

struct WaitC:strong<unsigned,WaitC>
            ,ops<WaitC,Eq,Inc,Out>{};
static_assert(sizeof(unsigned)==sizeof(WaitC));
void testWaitCounter(){
	WaitC c{};
	WaitC const one{1};
	ASSERT_EQUAL(WaitC{0},c);
	ASSERT_EQUAL(one,++c);
	ASSERT_EQUAL(one,c++);
	ASSERT_EQUAL(2,c.value);
}

namespace p0109{
// model p0109 energy example

struct energy: Linear<double,energy>{};

struct thermal : energy,  LinearOps<thermal, double>{};

struct kinetic: energy,  LinearOps<kinetic, double>{};

void test_energy_expressions(){
  energy e{1.23}; // okay; explicit
  //double d{e};    // error P0109 explicit
  double d{value(e)}; // pssst version of explicit
  // d = e; //  error: cannot convert 'p0109::energy' to 'double' in assignment
  e = e + e; // okay
  //e = e * e; // error; call to deleted function
  // error: no match for 'operator*' (operand types are 'p0109::energy' and 'p0109::energy')
  e *= 2.71828; // okay

  thermal t{4.56};
  kinetic k{7.89};
  e = t; e = k; // both okay; public allows type adjustment
  //t = e; t = k; // both in error; the adjustment is only unidirectional
  // error: no match for 'operator=' (operand types are 'p0109::thermal' and 'p0109::energy')
  t=t+t; k=k+k; //okay; public implies default trampolines
  e = t + k; // okay; calls the underlying trampoline
  ASSERT_EQUAL(1.23,d); // dummy
}

}

namespace demo_output_crtp {

struct literper100km : Out<literper100km> {
  double value;
  constexpr static inline auto  prefix="consumption ";
  constexpr static inline auto  suffix=" l/100km";
};

void demo_output_crtp(){
  literper100km consumed{{},9.5};
  std::ostringstream out{};
  out << consumed;
  ASSERT_EQUAL("consumption 9.5 l/100km", out.str());
}


}



bool runAllTests(int argc, char const *argv[]) {
	cute::suite s { };
	//TODO add your test here
	s.push_back(CUTE(testWithStringBase));
	s.push_back(CUTE(testSizeworks));
	s.push_back(CUTE(testUPlus));
	s.push_back(CUTE(testUMinus));
	s.push_back(CUTE(testUInc));
	s.push_back(CUTE(testUDec));
	s.push_back(CUTE(testWaitCounter));
	s.push_back(CUTE(testMoveWithStringBase));
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> lis(xmlfile.out);
	auto const runner = cute::makeRunner(lis, argc, argv);
	bool success = runner(s, "AllTests");
  cute::suite DoubleErsatz = make_suite_DoubleErsatz();
  success &= runner(DoubleErsatz, "DoubleErsatz");
	cute::suite BitOperationsTest = make_suite_BitOperationsTest();
	success &= runner(BitOperationsTest, "BitOperationsTest");
	cute::suite ArraySizeDiffStrong = make_suite_ArraySizeDiffStrong();
	success = runner(ArraySizeDiffStrong, "ArraySizeDiffStrong") && success;
	cute::suite Degrees = make_suite_Degrees();
	success = runner(Degrees, "Degrees") && success;
	cute::suite Consumption = make_suite_Consumption();
	success = runner(Consumption, "Consumption") && success;
	cute::suite ConsumptionWithoutStrong = make_suite_ConsumptionWithoutStrong();
	success = runner(ConsumptionWithoutStrong, "ConsumptionWithoutStrong") && success;
	cute::suite StrongWithEncapsulation = make_suite_StrongWithEncapsulation();
	success = runner(StrongWithEncapsulation, "StrongWithEncapsulation") && success;
	cute::suite StrongWithConstructor = make_suite_StrongWithConstructor();
	success = runner(StrongWithConstructor, "make_suite_StrongWithConstructor") && success;
	cute::suite SafeArithmetic = make_suite_SafeArithmetic();
	success = runner(SafeArithmetic, "SafeArithmetic") && success;
	cute::suite StringOperations = make_suite_StringOperations();
	success = runner(StringOperations, "StringOperations") && success;
	cute::suite BooleanTest = make_suite_BooleanTest();
	success &= runner(BooleanTest, "BooleanTest");
	cute::suite EnumOperators = make_suite_EnumOperators();
	success &= runner(EnumOperators, "EnumOperators");
  cute::suite ArithmeticOperationsTest = make_suite_ArithmeticOperationsTest();
	ArithmeticOperationsTest.push_back(CUTE(p0109::test_energy_expressions));
	ArithmeticOperationsTest.push_back(CUTE(demo_output_crtp::demo_output_crtp));
  success &= runner(ArithmeticOperationsTest, "ArithmeticOperationsTest");
    cute::suite OhmsLaw = make_suite_OhmsLaw();
    success &= runner(OhmsLaw, "OhmsLaw");
    cute::suite xxx = make_suite_xxx();
    success &= runner(xxx, "xxx");
	return success;
}

int main(int argc, char const *argv[]) {
    return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
