#include "DoubleErsatz.h"
#include "cute.h"

#include "pssst.h"

using namespace pssst;

struct Double:Arithmetic<double,Double,Out>{};

void thisIsADoubleErsatzTest() {
  Double d{42};
  ASSERT_EQUAL(d,Double{6}*Double{7});

}

cute::suite make_suite_DoubleErsatz() {
	cute::suite s { };
	s.push_back(CUTE(thisIsADoubleErsatzTest));
	return s;
}
