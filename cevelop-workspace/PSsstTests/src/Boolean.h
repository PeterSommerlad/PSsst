#ifndef SRC_BOOLEAN_H_
#define SRC_BOOLEAN_H_

#include "pssst.h"

namespace pssst {
namespace detail__ {
  template<typename T>
  concept non_numeric_convertible_to_bool =
    std::constructible_from<bool,T>
    && (std::is_class_v<std::remove_cv_t<std::remove_reference_t<T>>> || std::is_pointer_v<T>)
    ;
}
// a better bool?
struct Bool {
  friend constexpr Bool operator !(Bool const &l) {
    return Bool{!l.val};
  }
  constexpr Bool operator==(Bool const &other) const {
    return val == other.val;
  }
  // C++20 automatism does not work, because == does not return bool
  constexpr Bool operator!=(Bool const &other) const {
    return !(*this == other);
  }
  constexpr Bool() noexcept = default;
  constexpr Bool(bool const b) noexcept :
      val{b} {
  }
  // nullptr_t is not a pointer
  constexpr Bool(std::nullptr_t) noexcept :Bool{}{}
  // allow contextual conversion to bool for class types and pointers
  template<detail__::non_numeric_convertible_to_bool T>
  constexpr
  Bool(T&& x) noexcept :Bool(static_cast<bool>(x)){}
  // other implicit conversion attempts are not allowed
  // do not auto-convert from numbers or enums,
  // explicit cast to bool required first

  // redundant:
  //  template<typename T>
  //  constexpr Bool(T&&) noexcept = delete;

  constexpr explicit operator bool() const noexcept {
    return val;
  }
private:
  bool val{};
};
static_assert(sizeof(Bool)==sizeof(bool));
static_assert(std::is_trivially_copyable_v<Bool>);
static_assert(std::is_trivially_destructible_v<Bool>);

// comparisons with our non-integral Bool as result
template<typename U>
using OrderB = Order<U,Bool>;
template<typename U>
using EqB = Eq<U,Bool>;

}

#endif /* SRC_BOOLEAN_H_ */
