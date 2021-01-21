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


using namespace pssst;
struct Int: strong<int,Int>,ops<Int,Order,Inc,Add,Out>{
  constexpr Int(int v={}):strong<int,Int>{v}{}
};

namespace {
Int  dummy{42};

//int &y = value_ref(dummy);

}
struct Size: strong<unsigned,Size>,ops<Size,Order,Inc,Add,Out> {
  constexpr Size(unsigned v):strong<unsigned,Size>{v}{}
};
static_assert(sizeof(Size)==sizeof(unsigned),"no overhead");
static_assert(std::is_trivially_copyable<Size>::value,"should be trivial");

void testSizeworks(){
	Size sz{42};
	//ASSERT_EQUAL(42,sz);// doesn't compile
	//ASSERT_EQUAL(42u,sz);//doesn't compile
	ASSERT_EQUAL(Size{21}+Size{21},sz);
}






	struct uptest:strong<int,uptest>,ops<uptest,UPlus>{
	  uptest(int v={}):strong<int,uptest>{v}{}
	};


void testUPlus(){
	uptest one{1};
	ASSERT_EQUAL(one.value,(+one).value);
}
void testUMinus(){
	struct umtest:strong<int,umtest>,ops<umtest,UMinus>{
	  umtest(int v={}):strong<int,umtest>{v}{}
	};
	umtest one{1};
	ASSERT_EQUAL(-(one.value),(-one).value);
}
void testUInc(){
	struct uinctest:strong<int,uinctest>,ops<uinctest,Inc>{
	  uinctest(int v={}):strong<int,uinctest>{v}{}
	};
	uinctest var{1};
	ASSERT_EQUAL(2,(++var).value);
	ASSERT_EQUAL(2,(var++).value);
	ASSERT_EQUAL(3,var.value);
}
void testUDec(){
	struct udtest:strong<int,udtest>,ops<udtest,Dec>{
	  udtest(int i={}):strong<int,udtest>{i}{}
	};
	udtest var{2};
	ASSERT_EQUAL(1,(--var).value);
	ASSERT_EQUAL(1,(var--).value);
	ASSERT_EQUAL(0,var.value);
}

struct S:strong<std::string,S>,ops<S,Out,Eq>{
  S(std::string v={}):strong<std::string,S>{v}{}
};

void testWithStringBase(){
	S s{"hello"};
	ASSERT_EQUAL(S{"hello"},s);
}

void testMoveWithStringBase(){
	S s{std::string(1000u,'a')}; // assume beyond SSO
	char const *data = (s.value).data();
	ASSERT_EQUAL(1000ul,(s.value).size());
	ASSERT_LESS_EQUAL(1000ul,(s.value).capacity());
	std::string other{(std::move(s.value))};
	ASSERT_EQUAL(1000ul,other.size());
	ASSERT_EQUAL(0ul,(s.value).size());
	ASSERT_EQUAL(intptr_t(data),intptr_t(other.data()));
}

struct WaitC:strong<unsigned,WaitC>
            ,ops<WaitC,Eq,Inc,Out>{
  WaitC(unsigned v={}):strong<unsigned,WaitC>{v}{}
};
static_assert(sizeof(unsigned)==sizeof(WaitC),"");
void testWaitCounter(){
	WaitC c{};
	WaitC const one{1};
	ASSERT_EQUAL(WaitC{0},c);
	ASSERT_EQUAL(one,++c);
	ASSERT_EQUAL(one,c++);
	ASSERT_EQUAL(2,c.value);
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
	return success;
}

int main(int argc, char const *argv[]) {
    return runAllTests(argc, argv) ? EXIT_SUCCESS : EXIT_FAILURE;
}
