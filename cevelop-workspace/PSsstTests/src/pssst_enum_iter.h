#ifndef SRC_PSSST_ENUM_ITER_H_
#define SRC_PSSST_ENUM_ITER_H_
#include "pssst.h"



namespace pssst {

// duplication of macro to avoid leaking it from header
#ifndef NDEBUG
#define pssst_assert(cond) detail__::throwing_assert((cond),#cond)
#else
#define pssst_assert(cond)
#endif

// Increment and Decrement for enumerations
// including iterator range adapter (good idea?)

// define enumerator limit__ one past the end, if you want to use range for
// define enumerator limit__ equal to the last value, if you want wrapping
// define enumerator start__ to be the same value as the first element if its value is not 0
// increment and decrement only make sense, when the enumeration range is contiguous
namespace detail__ {
template <typename E, typename =std::void_t<> >
struct enum_has_limit:std::false_type{};
template <typename E>
struct enum_has_limit< E,
std::void_t<decltype(E::limit__)>
>:std::is_enum<E>{};
template <typename E, typename =std::void_t<> >
struct enum_has_start:std::false_type{};
template <typename E>
struct enum_has_start< E,
std::void_t<decltype(E::start__)>
>:std::is_enum<E>{};

template <typename E>
using ule = std::underlying_type_t<E>;
}
namespace enum_iteration {
template < typename E , typename = std::enable_if_t<std::is_enum_v<E> >>
constexpr E operator++(E &e) noexcept {
	using namespace detail__;
	auto val=static_cast<ule<E>>(e);
	pssst_assert(val < std::numeric_limits<decltype(val)>::max());
	++val;
	if constexpr(enum_has_limit<E>{}){
		if (val > static_cast<ule<E>>(E::limit__)){
			if constexpr (enum_has_start<E>{}){
				return e = E::start__;
			} else {
				val = 0;
			}
		}
	}
	e = static_cast<E>(val);
	return e;
}
template < typename E , typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr E operator++(E & e, int) noexcept {
	auto res=e;
	++e;
	return res;
}
template < typename E , typename = std::enable_if_t<std::is_enum_v<E> >>
constexpr E operator--(E &e) noexcept {
	using namespace detail__;
	auto val=static_cast<ule<E>>(e);
	constexpr E begin = []{
			if constexpr (enum_has_start<E>{})
				return E::start__ ;
			else
				return E{};
	}();

	if (e == begin){ // start is inclusive
		if constexpr(enum_has_limit<E>{}){
			 return e = E::limit__; // limit is supposed to be inclusive
		}
	}
	if constexpr(std::is_signed_v<decltype(val)>) {
			pssst_assert(val > std::numeric_limits<decltype(val)>::min());
	}
	--val;
	e = static_cast<E>(val);
	return e;
}
template < typename E , typename = std::enable_if_t<std::is_enum_v<E>>>
constexpr E operator--(E & e, int) noexcept {
	auto res=e;
	--e;
	return res;
}
namespace inner_detail__{
using ::pssst::enum_iteration::operator++;
// not that easy, needs scaffolding to make it an iterator....
template<typename E, typename = std::enable_if_t<detail__::enum_has_limit<E>{}>>
struct enum_iterator
		:ops<enum_iterator<E>,Value,Inc,Dec,Eq>{
			E val;
constexpr E operator*()noexcept {
	return value(*this);
}
};
}
template<typename E, typename = std::enable_if_t<detail__::enum_has_limit<E>{}>>
constexpr inner_detail__::enum_iterator<E> begin(E e) noexcept {
	return retval<inner_detail__::enum_iterator<E>>(e);
}
template<typename E, typename = std::enable_if_t<detail__::enum_has_limit<E>{}>>
constexpr inner_detail__::enum_iterator<E> end(E ) noexcept {
	return retval<inner_detail__::enum_iterator<E>>(E::limit__);
}
} // enum_iteration
#undef pssst_assert

}


#endif /* SRC_PSSST_ENUM_ITER_H_ */
