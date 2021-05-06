#include "BooleanTest.h"
#include "cute.h"
#include <initializer_list>
#include <utility>
#include "pssst.h"

// a better Bool than bool, used for comparisons
struct Bool {
    constexpr Bool() noexcept=default;
    constexpr Bool(bool const b) noexcept :
        val { b } {
    }
    friend constexpr Bool
    operator==(Bool const &l, Bool const& r) noexcept {
        return Bool{l.val == r.val};
    }
    friend constexpr Bool
    operator!=(Bool const &l, Bool const& r) noexcept {
        return !(l==r);
    }
    friend constexpr Bool
    operator !(Bool const &l){
        return Bool{! l.val};
    }
    // convert from pointers
    template <typename T>
    constexpr Bool(T * const x) noexcept :
        val { x!= nullptr }{
        }
    constexpr Bool(std::nullptr_t) noexcept {}
    // other conversion attempts are not allowed
    template <typename T, typename = std::enable_if_t<
                        std::is_constructible<bool,T>::value
                        && std::is_class_v<
                             std::remove_cv_t<std::remove_reference_t<T>>
                             >> >
    constexpr Bool(T const &x) noexcept
    :Bool(static_cast<bool>(x)){}
    constexpr explicit operator bool() const noexcept {
        return val;
    }
    bool val{};
};


using namespace pssst;
struct BoolTest {
  // test Bool properties
  static_assert(sizeof(Bool)==sizeof(bool));
  static_assert(std::is_trivially_copyable_v<Bool>);
  static_assert(std::is_trivially_destructible_v<Bool>);


	constexpr static Bool const t { true };
	constexpr static Bool const f { false };
	void ConvertsTobool() const {
		ASSERTM("true is true",                   t);
	}
    void ConvertsFromStreamIndirectlyOnly() const {
      std::stringstream ss;
      ASSERT(Bool(ss));
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
    void OperatorOrShortCut() const {
      int i{};
      t || ++i;
      ASSERT_EQUAL(0,i);
    }
    void OperatorOrShortCutPass() const {
      int i{};
      f || ++i;
      ASSERT_EQUAL(1,i);
    }
    void OperatorAndShortCut() const {
        int i{};
        f && ++i;
        ASSERT_EQUAL(0,i);
    }
    void OperatorAndShortCutPass() const {
        int i{};
        t && ++i;
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

    struct Num: strong <int,Num,Order>{};

    static_assert(Num{3} == Num{3});
    static_assert(not(Num{3} != Num{3}));
    static_assert(Num{3} <= Num{3});
    static_assert(Num{3} >= Num{3});
    static_assert(!(Num{3} < Num{3}));
    static_assert(!(Num{3} > Num{3}));
	// check for no arithmetic conversion
    template<typename FROM,typename=void >
    struct BoolDoesConvertFrom:std::false_type{};
    template<typename FROM >
    struct BoolDoesConvertFrom<FROM,std::void_t<decltype(Bool{std::declval<FROM>()})>>:std::true_type{};
    template<typename FROM>
    static constexpr bool BoolDoesConvertFrom_v = BoolDoesConvertFrom<FROM>::value;

    static_assert(not BoolDoesConvertFrom_v<char>);
    static_assert(not BoolDoesConvertFrom_v<int>);
    static_assert(not BoolDoesConvertFrom_v<double>);
    static_assert(BoolDoesConvertFrom_v<bool>);
    static_assert(BoolDoesConvertFrom_v<int *>);
    static_assert(BoolDoesConvertFrom_v<std::nullptr_t>);
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
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInOrShortCutWithCast));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInOrShortCutWithCastPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInAndShortCutWithCast));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorBuiltInAndShortCutWithCastPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrShortCut));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorOrShortCutPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndShortCut));
	s.push_back(CUTE_SMEMFUN(BoolTest, OperatorAndShortCutPass));
	s.push_back(CUTE_SMEMFUN(BoolTest, ConvertsFromStreamIndirectlyOnly));
	return s;
}
