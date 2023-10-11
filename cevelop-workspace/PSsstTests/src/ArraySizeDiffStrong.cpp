#include "ArraySizeDiffStrong.h"
#include "cute.h"
#include "pssst.h"

#include <algorithm>

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <iterator>
#include <memory>
#include <vector>



namespace vec {
// make a vector adapter with strongly typed ctors for demonstrating to Peter Dimov what I thought about.
//using namespace pssst;
struct Capacity{ size_t value;};
struct Size{ size_t value;};
namespace detail__{

template<typename T >
struct HasIteratorTraitImpl{
    template<typename I> static double e(...);
    template<typename I> static char e(typename std::iterator_traits<I>::iterator_category*);

static inline constexpr bool value = sizeof(HasIteratorTraitImpl::e<T>(nullptr))==sizeof(char);
};

template<typename T>
constexpr bool  HasIteratorTrait = HasIteratorTraitImpl<T>::value;

static_assert(!HasIteratorTrait<int>);
static_assert(HasIteratorTrait<int*>);



template<typename T, bool=HasIteratorTrait<T>>
constexpr bool isInputIterator = std::is_convertible_v<typename std::iterator_traits<T>::iterator_category,std::input_iterator_tag>;
template<typename T>
constexpr bool isInputIterator<T,false> = false;

template<typename IT, typename V, bool = HasIteratorTrait<IT>>
constexpr bool IteratorHasValueType = false;
template<typename IT, typename V>
constexpr bool IteratorHasValueType<IT,V,true> = std::is_constructible_v<V,typename std::iterator_traits<IT>::value_type>;

}


template<typename T, typename Alloc=std::allocator<T>>
struct Vec: std::vector<T,Alloc>{
  using base=std::vector<T,Alloc>;
  using allocator_type = typename base::allocator_type;
  using value_type = typename base::value_type;

  Vec() noexcept = default;
  explicit Vec(allocator_type const &a) noexcept :base{a}{}
  explicit Vec(Size sz,allocator_type const &a=allocator_type{})
  :base{sz.value,a}{}
  explicit Vec(Capacity cap,allocator_type const &a=allocator_type{})
  :base{a}{base::reserve(cap.value);}
  Vec(Size sz, const value_type& value,
     const allocator_type& a = allocator_type())
  :base{sz.value,value,a}{}
  template<typename _InputIterator>
   Vec(_InputIterator first,
       std::enable_if_t<detail__::isInputIterator<_InputIterator>
       && detail__::IteratorHasValueType<_InputIterator,T>,_InputIterator> last,
       const allocator_type& a = allocator_type())
   : base{first,last,a}{}

};

template<typename T>
Vec(Size,T const &) -> Vec<T>;

template<typename _InputIterator>
 Vec(_InputIterator first,
     std::enable_if_t<detail__::isInputIterator<_InputIterator>,_InputIterator> last) -> Vec<typename std::iterator_traits<_InputIterator>::value_type>;


void demonstrateVecWithCapacity(){
  Vec<int> v{Capacity{100}};
  ASSERT_EQUAL(100u, v.capacity());
  ASSERT_EQUAL(0u, v.size());
}

void demonstrateVecWithSize(){
  Vec v{Size{100},42};
  ASSERT_EQUAL(100u, v.size());
}

void checkOutputIteratorFailsWithVec(){
  std::ostringstream out;
  std::ostream_iterator<int> osi{out};
//  Vec<int> v{osi,osi}; // doesn't compile
}

void checkInputIteratorSameValueType(){
  std::istringstream in{"1 2 3"};
  std::istream_iterator<int> ins{in},eof{};
  static_assert(std::is_same_v<std::input_iterator_tag,std::iterator_traits<decltype(ins)>::iterator_category>);
  static_assert(std::is_convertible_v<std::iterator_traits<decltype(ins)>::iterator_category,std::input_iterator_tag>);
  static_assert(detail__::HasIteratorTrait<decltype(ins)>);
  static_assert(detail__::IteratorHasValueType<decltype(ins),int>);
  static_assert(detail__::isInputIterator<std::istream_iterator<int>>);
  Vec v{ins,eof};
  static_assert(std::is_same_v<Vec<int>, decltype(v)>);
  ASSERT_EQUAL(3u,v.size());
}


}


namespace test {
using namespace pssst;
struct Diff: Linear<std::ptrdiff_t,Diff>{
  ~Diff()=default;
};
struct Size: affine_space_for<Size,Diff> {
	constexpr Size()  = default;
	constexpr Size(size_t val)
	:Size{Diff{static_cast<std::ptrdiff_t>(val)}}{}
	constexpr Size(Diff val)
	:affine_space_for<Size,Diff>{val}{
		if (val < Diff{0})
			throw std::logic_error{"size is too large"};
	}
};

template<typename T>
struct Pointer: ops<Pointer<T>,Order, Inc, Dec> {
	using value_type=T;
	using difference_type=Diff;
	using reference=value_type&;
	using const_reference= value_type const&;
	using pointer=value_type*;
	using const_pointer=const value_type*;
	constexpr reference operator*() const noexcept { return *a; }
	constexpr reference operator[]( difference_type pos ) const noexcept { return a[pos];}
	constexpr Pointer& operator+=( difference_type off ) noexcept { a += value(off); return *this; }
	constexpr Pointer operator+( difference_type off ) const noexcept { return Pointer{ a + value(off) }; }
	friend constexpr Pointer operator+( difference_type off, Pointer p )  noexcept { return Pointer{ p.a + value(off) }; }
	constexpr Pointer& operator-=( difference_type off ) noexcept { a -= value(off); return *this; }
	constexpr Pointer operator-( difference_type off ) const noexcept { return Pointer{ a - value(off) }; }

	constexpr difference_type operator-(Pointer const &right) const noexcept { return difference_type{a - right.a}; }

	constexpr Pointer() noexcept = default;
	explicit constexpr Pointer(T *a) noexcept : a{a}{}
	T* a{};
};

}
namespace std{
template<typename T>
struct iterator_traits< test::Pointer<T> > {
	using Pointer=test::Pointer<T>;
	using difference_type=typename Pointer::difference_type;
	using value_type=typename Pointer::value_type;
	using pointer=Pointer;
	using reference=typename Pointer::reference;
	using iterator_category=std::random_access_iterator_tag;
};
} namespace test {
template <typename T, size_t N>
struct array{
	T a[N];
	using size_type=Size;
	using value_type=T;
	using difference_type=Diff;
	using reference=value_type&;
	using const_reference= value_type const&;
	using pointer=Pointer<value_type>;
	using const_pointer=Pointer<const value_type>;
	using iterator=pointer;
	using const_iterator=const_pointer;
	using reverse_iterator=std::reverse_iterator<iterator>;
	using const_reverse_iterator=std::reverse_iterator<const_iterator>;
	constexpr pointer data() noexcept { return a;}
	constexpr const_pointer data() const noexcept { return a;}
	constexpr reference at( size_type pos ) { if (pos < size()) return a[value(pos)]; throw std::out_of_range{"array::at"};}
	constexpr const_reference at( size_type pos ) const { if (pos < size()) return a[value(value(pos))]; throw std::out_of_range{"array::at"};}
	constexpr reference operator[]( size_type pos ) noexcept { return *(begin()+value(pos));}
	constexpr const_reference operator[]( size_type pos ) const noexcept { return *(begin()+value(pos)); }
	constexpr reference front() noexcept { return *a;}
	constexpr const_reference front() const noexcept { return *a;}
	constexpr reference back() noexcept { return a[N-1];}
	constexpr const_reference back() const noexcept { return a[N-1];}
	constexpr iterator begin() noexcept {return iterator{a};}
	constexpr const_iterator begin() const noexcept {return const_iterator{a};}
	constexpr const_iterator cbegin() const noexcept {return const_iterator{a};}
	constexpr iterator end() noexcept {return iterator{a+N};}
	constexpr const_iterator end() const noexcept {return const_iterator{a+N};}
	constexpr const_iterator cend() const noexcept {return const_iterator{a+N};}
	constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end());}
	constexpr const_reverse_iterator  rbegin() const noexcept { return const_reverse_iterator(end());}
	constexpr const_reverse_iterator  crbegin() const noexcept { return const_reverse_iterator(end());}
	constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin());}
	constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin());}
	constexpr const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin());}
	constexpr bool empty() const noexcept {return N !=0;}
	constexpr size_type size() const noexcept { return size_type{N};}
	constexpr size_type max_size() const noexcept { return size();}
	constexpr void fill( const T& value ) { std::fill(begin(),end(),value);}
	constexpr void swap( array& other ) noexcept(std::is_nothrow_swappable<T>::value){ std::swap_ranges(begin(), end(), other.begin());}
};
template <class T, class... U>
array(T, U...) -> array<T, 1 + sizeof...(U)>;

}

using namespace test;
void TooLargeSizeThrows(){
	ASSERT_THROWS(Size{std::numeric_limits<size_t>::max()}, std::logic_error);
}
void TooLargeSizeMinus1Throws(){
	ASSERT_THROWS(Size{Diff{-1}}, std::logic_error);
}
void TooLargeSizePlus1Throws(){
	ASSERT_THROWS(Size{size_t(std::numeric_limits<std::ptrdiff_t>::max()) +1}, std::logic_error);
}

void TestRangeForWithArray(){
	array<int,5> a{1,2,3,4,5};
	int sum{};
	for(auto elt:a){
		sum += elt;
	}
	ASSERT_EQUAL(15,sum);
}
void TestRangeForWithConstArray(){
	array<int,6> const a{1,2,3,4,5,6};
	int sum{};
	for(auto elt:a){
		sum += elt;
	}
	ASSERT_EQUAL(21,sum);
}

void TestInverseRangeAlgorithmWithArray(){
	array<int,5> a{1,2,3,4,5};
//	auto pos = std::find(a.rbegin(),a.rend(),2); // STL relies on difference_type being integral...
	auto pos = a.rbegin();
	while(pos != a.rend() && *pos != 2) ++pos;
	ASSERT_EQUAL(Diff{2},a.rend()-pos);
}



void TestOOBNegativeAccessThrows(){
	array<int,2> const a{1,2};
	ASSERT_THROWS(a[-1],std::logic_error); // too large
}
void TestOOBLargeAccessThrows(){
	array<int,2> const a{1,2};
	ASSERT_THROWS(a.at(2),std::out_of_range); // too large
}




// TODO: should put Size as vector space and Diff as its affine spaces
// this will provide at least the correct typing for substraction
// but current schema will have Size having the same range as Diff
cute::suite make_suite_ArraySizeDiffStrong() {
	cute::suite s { };
	s.push_back(CUTE(TooLargeSizeThrows));
	s.push_back(CUTE(TooLargeSizeMinus1Throws));
	s.push_back(CUTE(TooLargeSizePlus1Throws));
	s.push_back(CUTE(TestRangeForWithArray));
	s.push_back(CUTE(TestRangeForWithConstArray));
	s.push_back(CUTE(TestOOBNegativeAccessThrows));
	s.push_back(CUTE(TestOOBLargeAccessThrows));
	s.push_back(CUTE(TestInverseRangeAlgorithmWithArray));
	s.push_back(CUTE(vec::demonstrateVecWithCapacity));
	s.push_back(CUTE(vec::demonstrateVecWithSize));
	s.push_back(CUTE(vec::checkInputIteratorSameValueType));
	return s;
}
