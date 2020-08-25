#include "SafeArithmetic.h"
#include "cute.h"

#include <boost/safe_numerics/safe_integer.hpp>
#include <boost/safe_numerics/exception_policies.hpp>
#include <boost/safe_numerics/automatic.hpp>
#include <cstddef>

// TODO more tests beyond principle

// safe numerics do not save us from the wrong operations... try differently first.
// but we can detect overflowing operations

using safe_diff_t = boost::safe_numerics::safe< std::ptrdiff_t, boost::safe_numerics::automatic >;
using safe_size_t = boost::safe_numerics::safe< size_t >;

void thisIsASafeArithmeticTest() {
	safe_size_t const sz { 0 };
	//static_assert(std::is_signed_v<decltype(sz-1LL)>,"should be signed");
	static_assert(std::is_same_v<decltype(sz-1), safe_size_t>,"staying in the domain?");
	ASSERT_THROWS(sz-1,std::exception);
}
void newTestFunction(){
//	ASSERTM("show more usages with strong types", false);
}


cute::suite make_suite_SafeArithmetic() {
	cute::suite s { };
	s.push_back(CUTE(thisIsASafeArithmeticTest));
	s.push_back(CUTE(newTestFunction));
	return s;
}
