#include "ArraySizeDiffStrong.h"
#include "cute.h"
#include "pssst.h"

#include <algorithm>

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <iterator>

namespace test {
using namespace pssst;
struct Diff: strong<std::ptrdiff_t,Diff>,Linear<Diff, std::ptrdiff_t>{
	constexpr Diff()noexcept = default;
	constexpr Diff(std::ptrdiff_t val):strong<std::ptrdiff_t,Diff>{val}{}
};
struct Size: create_vector_space<Size,Diff> {
	constexpr Size() noexcept = default;
	constexpr Size(size_t val)
	:Size{Diff(val)}{}
	constexpr Size(Diff val)
	:create_vector_space<Size,Diff>{val}{
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
	constexpr reference operator*() const noexcept { return *value; }
	constexpr reference operator[]( difference_type pos ) const noexcept { return value[pos];}
	constexpr Pointer& operator+=( difference_type off ) noexcept { value += (off.value); return *this; }
	constexpr Pointer operator+( difference_type off ) const noexcept { return Pointer{ value + (off.value) }; }
	friend constexpr Pointer operator+( difference_type off, Pointer p )  noexcept { return Pointer{ p.value + (off.value) }; }
	constexpr Pointer& operator-=( difference_type off ) noexcept { value -= (off.value); return *this; }
	constexpr Pointer operator-( difference_type off ) const noexcept { return Pointer{ value - (off.value) }; }

	constexpr difference_type operator-(Pointer const &right) const noexcept { return difference_type{value - right.value}; }

	constexpr Pointer() noexcept = default;
	explicit constexpr Pointer(T *value) noexcept : value{value}{}
	T* value{};
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
	constexpr reference at( size_type pos ) { if (pos < size()) return a[pos.value.value]; throw std::out_of_range{"array::at"};}
	constexpr const_reference at( size_type pos ) const { if (pos < size()) return a[pos.value.value]; throw std::out_of_range{"array::at"};}
	constexpr reference operator[]( size_type pos ) noexcept { return *(begin()+pos.value);}
	constexpr const_reference operator[]( size_type pos ) const noexcept { return *(begin()+pos.value); }
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
	constexpr void swap( array& other ) noexcept/*(std::is_nothrow_swappable<T>::value)*/{ std::swap_ranges(begin(), end(), other.begin());}
};
//template <class T, class... U>
//array(T, U...) -> array<T, 1 + sizeof...(U)>;

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
	return s;
}
