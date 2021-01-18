#include "BitOperationsTest.h"
#include "cute.h"
#include "pssst.h"
#include <cstdint>

//#define USE_STRONG
using namespace pssst;
struct U_int16 :
#ifdef USE_STRONG
		strong<std::uint16_t,U_int16>,
#endif
				 ops<U_int16, Eq, Out, BitOps, ShiftOps, ShiftOpsSym>
{
#ifndef USE_STRONG
	constexpr explicit U_int16(std::uint16_t val) noexcept :value{val}{}
	constexpr U_int16() noexcept = default;
	std::uint16_t value{};
#endif
};
static_assert(std::is_trivially_copyable_v<U_int16>);

static_assert(static_cast<uint16_t>(~42) == (~ U_int16{42} ).value);

static_assert(10u == (U_int16{42} & U_int16{11}).value);
static_assert(std::is_class_v<U_int16> );
//static_assert( std::is_aggregate_v<U_int16>);
//static_assert(is_strong_v<U_int16>);
//static_assert(43u == (U_int16{42} | U_int16{11}).value);
static_assert(43u == (U_int16{42} | U_int16{11}).value);
static_assert(84u == (U_int16{21} << 2).value);
static_assert(21u == (U_int16{42} >> 1).value);
// fails as it should: static_assert(21u == (U_int16{42} >> 17).value);

struct fail_bitops:strong<int,fail_bitops>,ops<fail_bitops,Eq,BitOps>{
};

//static_assert((fail_bitops{1} | fail_bitops{2} ) == fail_bitops{3});

void testBitNegate(){
	U_int16 ui{42};
	uint16_t ui16{42};
	ASSERT_EQUAL(static_cast<uint16_t>(~ui16),(~ui).value);
}

void testBitAnd(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ASSERT_EQUAL(10u,(ui & eleven).value);
}
void testBitAndAssign(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ui &= eleven;
	ASSERT_EQUAL(10u,ui.value);
}
void testBitOr(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ASSERT_EQUAL(43u,(ui | eleven).value);
}
void testBitOrAssign(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ui |= eleven;
	ASSERT_EQUAL(43u,ui.value);
}

void testBitXor(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ASSERT_EQUAL(33u,(ui ^ eleven).value);
}
void testBitXorAssign(){
	U_int16 ui{42};
	U_int16 const eleven { 11 };
	ui ^= eleven;
	ASSERT_EQUAL(33u,ui.value);
}

void testBitShiftL(){
	U_int16  ui { 42 };
	ASSERT_EQUAL(42u<<5u,(ui << 5u).value);
}
void testBitShiftLAssign(){
	U_int16 ui{42};
	ui <<= 5u;
	ASSERT_EQUAL(42u<<5u,ui.value);
}
void testBitShiftLBits(){
	U_int16  ui { 42 };
	U_int16  bits { 5};
	ASSERT_EQUAL(42u<< bits.value,(ui << bits).value);
}
void testBitShiftLAssignBits(){
	U_int16 ui{42};
	U_int16 const bits { 5};

	ui <<= bits;
	ASSERT_EQUAL(42u<< bits.value,ui.value);
}
void testBitShiftLBeyond(){
	U_int16 const ui { 42 };
	ASSERT_THROWS(ui << 17u, std::logic_error);
}

void testBitShiftR(){
	U_int16 const ui { 42 };
	ASSERT_EQUAL(42u>>5u,(ui >> 5u).value);
}
void testBitShiftRAssign(){
	U_int16 ui{42};
	ui >>= 5u;
	ASSERT_EQUAL(42u>>5u,ui.value);
}
void testBitShiftRBits(){
	U_int16 const ui { 42 };
	U_int16 const bits { 5};
	ASSERT_EQUAL(42u>> bits.value,(ui >> bits).value);
}
void testBitShiftRAssignBits(){
	U_int16 ui{42};
	U_int16 const bits { 5};

	ui >>= bits;
	ASSERT_EQUAL(42u >> bits.value,ui.value);
}
void testBitShiftRBeyond(){
	U_int16 const ui { 42 };
	ASSERT_THROWS(ui >> 17u, std::logic_error);
}




cute::suite make_suite_BitOperationsTest() {
	cute::suite s { };
	s.push_back(CUTE(testBitNegate));
	s.push_back(CUTE(testBitAnd));
	s.push_back(CUTE(testBitAndAssign));
	s.push_back(CUTE(testBitOr));
	s.push_back(CUTE(testBitOrAssign));
	s.push_back(CUTE(testBitXor));
	s.push_back(CUTE(testBitXorAssign));
	s.push_back(CUTE(testBitShiftL));
	s.push_back(CUTE(testBitShiftLAssign));
	s.push_back(CUTE(testBitShiftLBits));
	s.push_back(CUTE(testBitShiftLAssignBits));
	s.push_back(CUTE(testBitShiftR));
	s.push_back(CUTE(testBitShiftRAssign));
	s.push_back(CUTE(testBitShiftRBits));
	s.push_back(CUTE(testBitShiftRAssignBits));
	s.push_back(CUTE(testBitShiftLBeyond));
	s.push_back(CUTE(testBitShiftRBeyond));
	return s;
}
