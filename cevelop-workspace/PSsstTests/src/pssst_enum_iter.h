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
template <typename E, typename =void >
struct enum_has_limit:std::false_type{};
template <typename E>
struct enum_has_limit< E,
detail__::void_t<decltype(E::limit__)>
>:std::is_enum<E>{};
template <typename E, typename =void >
struct enum_has_start:std::false_type{};
template <typename E>
struct enum_has_start< E,
detail__::void_t<decltype(E::start__)>
>:std::is_enum<E>{};

template <typename E>
using ule = std::underlying_type_t<E>;


template<typename E>
constexpr E wrap_increment(std::false_type) noexcept{
  return E{}; // assume zero start
}
template<typename E>
constexpr E wrap_increment(std::true_type) noexcept{
  return E::start__; // start__ value
}
template<typename E>
constexpr E wrap_decrement(ule<E> &val,std::false_type) noexcept{
  return E{--val}; // assume zero start
}
template<typename E>
constexpr E wrap_decrement(ule<E> &, std::true_type) noexcept{
  return E::limit__; // start__ value
}


template<typename E>
constexpr E wrapped_value(ule<E> val, std::false_type) noexcept {
  return static_cast<E>(val); // no bounds check and no wrapping to new start
}

template<typename E>
constexpr E wrapped_value(ule<E>val, std::true_type) noexcept {
  if (val > static_cast<ule<E>>(E::limit__)){
    return wrap_increment<E>(typename enum_has_start<E>::type{});
  }
  return static_cast<E>(val); // within bounds
}


}
namespace enum_iteration {
template < typename E , typename = std::enable_if_t<std::is_enum<E>::value >>
constexpr E operator++(E &e) noexcept {
	using namespace detail__;
	auto val=static_cast<ule<E>>(e);
	pssst_assert(val < std::numeric_limits<decltype(val)>::max());
	++val;
	return e = wrapped_value<E>(val,typename enum_has_limit<E>::type{});
//	if constexpr(enum_has_limit<E>{}){
//		if (val > static_cast<ule<E>>(E::limit__)){
//			if constexpr (enum_has_start<E>{}){
//				return e = E::start__;
//			} else {
//				val = 0;
//			}
//		}
//	}
//	e = static_cast<E>(val);
//	return e;
}
template < typename E , typename = std::enable_if_t<std::is_enum<E>::value>>
constexpr E operator++(E & e, int) noexcept {
	auto res=e;
	++e;
	return res;
}
template < typename E , typename = std::enable_if_t<std::is_enum<E>::value >>
constexpr E operator--(E &e) noexcept {
	using namespace detail__;
	auto val=static_cast<ule<E>>(e);
	constexpr E begin = wrap_increment<E>(typename enum_has_start<E>::type{});

	if (enum_has_limit<E>{} && e == begin){ // start is inclusive
	  return e = wrap_decrement<E>(val,typename enum_has_limit<E>::type{});
	}
    pssst_assert(std::is_unsigned<decltype(val)>::value || val > std::numeric_limits<decltype(val)>::min()); // prevent wrapping UB
	--val;
	e = static_cast<E>(val);
	return e;
}
template < typename E , typename = std::enable_if_t<std::is_enum<E>::value>>
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
		:ops<enum_iterator<E>,Inc,Dec,Eq>{
			E value;
			explicit constexpr enum_iterator(E val):value{val}{}
constexpr E operator*() const noexcept {
	return (*this).value;
}
};
}
template<typename E, typename = std::enable_if_t<detail__::enum_has_limit<E>{}>>
constexpr inner_detail__::enum_iterator<E> begin(E e) noexcept {
	return inner_detail__::enum_iterator<E>(e);
}
template<typename E, typename = std::enable_if_t<detail__::enum_has_limit<E>{}>>
constexpr inner_detail__::enum_iterator<E> end(E ) noexcept {
	return inner_detail__::enum_iterator<E>(E::limit__);
}
} // enum_iteration
#undef pssst_assert

}


#endif /* SRC_PSSST_ENUM_ITER_H_ */
