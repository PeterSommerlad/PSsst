#include "StringOperations.h"
#include "cute.h"
#include "pssst.h"

#include <sstream>

#include <string>

using namespace pssst;


struct Word:strong<std::string,Word, Out, Add, Order>{
};
Word operator"" _w(char const * const s, size_t l){
	return Word{std::string{s,l}};
}
Word operator"" _w(char c){
	return Word{std::string{c}};
}


void thisIsAStringOperationsTest() {
	Word const w { "hello" };
	std::ostringstream out{};
	out << w;
	ASSERT_EQUAL("hello",out.str());
}

void testStringWrapperCompare(){
	Word h{"Hello"};
	Word w{"World"};
	ASSERT_LESS(h,w);

}

void testStringAdditionWorks(){
	Word w{};
	w += "Hello"_w;
	w += ' '_w;
	w += "World!"_w;
	ASSERT_EQUAL("Hello World!"_w, w);
}



cute::suite make_suite_StringOperations() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAStringOperationsTest));
	s.push_back(CUTE(testStringWrapperCompare));
	s.push_back(CUTE(testStringAdditionWorks));
	return s;
}
