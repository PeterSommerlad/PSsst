#ifndef SRC_BOOLEAN_H_
#define SRC_BOOLEAN_H_

#include "pssst.h"


namespace pssst {
// a better bool?
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
	                    && std::is_class<
	                         std::remove_cv_t<std::remove_reference_t<T>>
	                         >::value> >
	constexpr Bool(T const &x) noexcept
	:Bool(static_cast<bool>(x)){}
	constexpr explicit operator bool() const noexcept {
		return val;
	}
	bool val{};
};
static_assert(sizeof(Bool)==sizeof(bool),"");
static_assert(std::is_trivially_copyable<Bool>::value,"");
static_assert(std::is_trivially_destructible<Bool>::value,"");

// comparisons with our non-integral Bool as result
template <typename U>
using OrderB = Order<U,Bool>;
template <typename U>
using EqB = Eq<U,Bool>;

}



#endif /* SRC_BOOLEAN_H_ */
