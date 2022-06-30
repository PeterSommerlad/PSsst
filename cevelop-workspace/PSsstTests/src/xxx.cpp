#include "xxx.h"
#include "cute.h"

void thisIsAxxxTest() {
	ASSERTM("start writing tests", false);	
}

cute::suite make_suite_xxx() {
	cute::suite s { };
	s.push_back(CUTE(thisIsAxxxTest));
	return s;
}
