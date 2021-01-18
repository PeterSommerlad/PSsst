#ifndef SRC_BOOLEAN_H_
#define SRC_BOOLEAN_H_

#include "pssst.h"

namespace pssst {
// a better bool?
struct Bool {
  template<typename C> // requires is_invocable_r_v<bool,C const&>
  friend constexpr
  std::enable_if_t<std::is_invocable_r_v<bool, C const&>, Bool> operator ||(
      Bool const &l, C const &r) {
    return l ? l : Bool{static_cast<bool>(r())};
  }
  template<typename C> // requires is_invocable_r_v<bool,C const&>
  friend constexpr
  std::enable_if_t<std::is_invocable_r_v<bool, C const&>, Bool> operator &&(
      Bool const &l, C const &r) {
    return l ? Bool{static_cast<bool>(r())} : l;
  }
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
  // convert from pointers
  template<typename T>
  constexpr Bool(T *const x) noexcept :
      val{x != nullptr} {
  }
  constexpr Bool(std::nullptr_t) noexcept {
  }
  // other implicit conversion attempts are not allowed
  template<typename T>
  constexpr Bool(T&& x) noexcept = delete;
  // do not auto-convert anything else, explicit cast to bool required first

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
