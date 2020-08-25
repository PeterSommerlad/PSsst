#include "Boolean.h"
#include "BooleanTest.h"
#include "cute.h"
#include <initializer_list>
#include <utility>

using namespace pssst;
struct BoolTest {
	constexpr static Bool const t { true };
	constexpr static Bool const f { false };
	void ConvertsTobool() const {
		ASSERTM("true is true",                   t);
	}
	void OperatorNot() const {
		ASSERTM("not false is true",             !f);
	}
	void DefaultIsFalse() const {
		ASSERTM("Default Bool{} is false", not Bool{});
	}
	void OperatorNotTrue() const {
		ASSERTM("not true is false",         not !t);
	}
	void OperatorOrTrueFalse() const {
		ASSERTM("true or false is true",     t || f);
	}
	void OperatorOrFalseTrue() const {
		ASSERTM("true or false is true",     t || f);
	}
	void OperatorOrFalse() const {
		ASSERTM("not(false or false)",  not( f || f ));
	}
	void OperatorOrTrueTrue() const {
		ASSERTM("true or true",              t || t);
	}
	void OperatorAndTrueTrue() const {
		ASSERTM("true and true",             t && t);
	}
	void OperatorAndTrueFalse() const {
		ASSERTM("not(true and false)",  not( t && f ));
	}
	void OperatorAndFalseTrue() const {
		ASSERTM("not(false and true)",  not( f && t ));
	}
	void OperatorAndFalseFalse() const {
		ASSERTM("not(false and false)", not( f && f ));
	}
	void OperatorEqFalseFalse() const {
		ASSERTM("false == false",            f == f);
	}
	void OperatorEqTrueTrue() const {
		ASSERTM("true == true",              t == t);
	}
	void OperatorEqTrueFalse() const {
		ASSERTM("not(true == false)",   not( t == f ));
	}
	void OperatorEqFalseTrue() const {
		ASSERTM("not(false == true)",   not( f == t ));
	}
	void OperatorEqNotEqInequality() const {
		for(Bool const l:{f,t})
			for(Bool const r:{f,t})
				ASSERT_EQUAL((l==r), not(l != r));
	}
	void TernaryOperatorWorksWithTrue() const {
		int i{};
		i = t? i+1 : i;
		ASSERT_EQUAL(1,i);
	}
	void TernaryOperatorWorksWithFalse() const {
		int i{};
		i = f? i : i + 1;
		ASSERT_EQUAL(1,i);
	}
	void OperatorOrShortCutWithLambda() const {
		int i{};
		t || [&](){ return (++i);}; // obtain shortcut by passing a lambda returning convertible to bool
		ASSERT_EQUAL(0,i);
	}
	void OperatorOrShortCutWithLambdaPass() const {
		int i{};
		f || [&](){ return (++i);};
		ASSERT_EQUAL(1,i);
	}
	void OperatorAndShortCutWithLambda() const {
		int i{};
		f && [&](){ return (++i);};
		ASSERT_EQUAL(0,i);
	}
	void OperatorAndShortCutWithLambdaPass() const {
		int i{};
		t && [&](){ return (++i);};
		ASSERT_EQUAL(1,i);
	}
	void OperatorBuiltInOrShortCutWithCast() const {
		int i{};
		static_cast<bool>(t) || ++i; // select built-in || with cast to bool.
		ASSERT_EQUAL(0,i);
	}
	void OperatorBuiltInOrShortCutWithCastPass() const {
		int i{};
		static_cast<bool>(f)  || ++i;
		ASSERT_EQUAL(1,i);
	}
	void OperatorBuiltInAndShortCutWithCast() const {
		int i{};
		static_cast<bool>(f)  && ++i;
		ASSERT_EQUAL(0,i);
	}
	void OperatorBuiltInAndShortCutWithCastPass() const {
		int i{};
		static_cast<bool>(t)  && ++i;
		ASSERT_EQUAL(1,i);
	}


	// check for no arithmetic conversion
	template<typename FROM,typename=std::void_t<> >
	struct BoolDoesNotConvertFrom:std::true_type{};
	template<typename FROM >
	struct BoolDoesNotConvertFrom<FROM,std::void_t<decltype(Bool{std::declval<FROM>()})>>:std::false_type{};
	
	static_assert(BoolDoesNotConvertFrom<int>{});
	static_assert(BoolDoesNotConvertFrom<double>{});
	static_assert(not BoolDoesNotConvertFrom<bool>{});
	static_assert(not BoolDoesNotConvertFrom<int *>{});
	static_assert(not BoolDoesNotConvertFrom<nullptr_t>{});

	template<typename WITH,typename=std::void_t<> >
	struct BoolDoesNotAddWith:std::true_type{};
	template<typename WITH >
	struct BoolDoesNotAddWith<WITH,std::void_t<decltype(Bool{} + std::declval<WITH>())>>:std::false_type{};

	static_assert(BoolDoesNotAddWith<int>{});
	static_assert(BoolDoesNotAddWith<bool>{});
	static_assert(BoolDoesNotAddWith<Bool>{});

	template<typename WITH,typename=std::void_t<> >
	struct boolDoesNotAddWith:std::true_type{};
	template<typename WITH >
	struct boolDoesNotAddWith<WITH,std::void_t<decltype(bool{} + std::declval<WITH>())>>:std::false_type{};

	static_assert(not boolDoesNotAddWith<int>{});
	static_assert(not boolDoesNotAddWith<bool>{});
	static_assert(boolDoesNotAddWith<void>{});
	static_assert(boolDoesNotAddWith<Bool>{});

};





cute::suite make_suite_BooleanTest() {
	cute::suite s { };
	s.push_back(CUTE_SMEMFUN(BoolTest, ConvertsTobool));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorNot));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorNotTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrTrueTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndTrueTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrTrueFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrFalseTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndTrueFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndFalseTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndFalseFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorEqFalseFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorEqTrueTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorEqTrueFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorEqFalseTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorEqNotEqInequality));
	s.push_back(CUTE_SMEMFUN(BoolTest, DefaultIsFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, TernaryOperatorWorksWithFalse));
	s.push_back(CUTE_SMEMFUN(BoolTest, TernaryOperatorWorksWithTrue));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrShortCutWithLambda));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndShortCutWithLambda));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrShortCutWithLambdaPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndShortCutWithLambdaPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInOrShortCutWithCast));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInOrShortCutWithCastPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInAndShortCutWithCast));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInAndShortCutWithCastPass));
	return s;
}
