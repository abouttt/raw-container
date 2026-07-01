#include <algorithm>
#include <compare>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>
#include <raw/array.h>

template <typename T, std::size_t N>
bool equal(const raw::array<T, N>& arr, std::initializer_list<T> ilist)
{
	if (arr.size() != ilist.size())
	{
		return false;
	}

	return std::equal(arr.begin(), arr.end(), ilist.begin());
}

TEST(ArrayTest, Constructors)
{
	// Default construction
	raw::array<int, 3> arr1;
	EXPECT_FALSE(arr1.empty());
	EXPECT_EQ(arr1.size(), 3);

	// Aggregate initialization
	raw::array<int, 3> arr2 = { 1, 2, 3 };
	EXPECT_TRUE(equal(arr2, { 1, 2, 3 }));

	// Copy construction
	raw::array<int, 3> arr3 = arr2;
	EXPECT_TRUE(equal(arr3, { 1, 2, 3 }));

	// Move construction
	raw::array<int, 3> arr4 = std::move(arr2);
	EXPECT_TRUE(equal(arr4, { 1, 2, 3 }));

	// Zero-sized array
	raw::array<int, 0> arr5;
	EXPECT_TRUE(arr5.empty());
	EXPECT_EQ(arr5.size(), 0);

	// Partial initialization (remaining elements zero-initialized)
	raw::array<int, 5> partial = { 1, 2 };
	EXPECT_TRUE(equal(partial, { 1, 2, 0, 0, 0 }));
}

TEST(ArrayTest, Assignment)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 4, 5, 6 };

	// Copy assignment
	arr1 = arr2;
	EXPECT_TRUE(equal(arr1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(arr2, { 4, 5, 6 }));

	// Move assignment
	raw::array<int, 3> arr3 = { 7, 8, 9 };
	arr1 = std::move(arr3);
	EXPECT_TRUE(equal(arr1, { 7, 8, 9 }));

	// Self-assignment
	arr1 = arr1;
	EXPECT_TRUE(equal(arr1, { 7, 8, 9 }));
}

TEST(ArrayTest, ElementAccess)
{
	raw::array<int, 4> arr = { 1, 2, 3, 4 };

	// at (within bounds)
	EXPECT_EQ(arr.at(2), 3);

	// at (out of bounds)
	EXPECT_THROW(arr.at(4), std::out_of_range);

	// operator[]
	EXPECT_EQ(arr[1], 2);
	EXPECT_EQ(arr[2], 3);
	arr[1] = 20;
	arr[2] = 30;
	EXPECT_EQ(arr[1], 20);
	EXPECT_EQ(arr[2], 30);

	// front / back
	EXPECT_EQ(arr.front(), 1);
	EXPECT_EQ(arr.back(), 4);
	arr.front() = 10;
	arr.back() = 40;
	EXPECT_EQ(arr.front(), 10);
	EXPECT_EQ(arr.back(), 40);

	// const versions
	const auto& carr = arr;
	EXPECT_EQ(carr.front(), 10);
	EXPECT_EQ(carr.back(), 40);
	EXPECT_EQ(carr[2], 30);
	EXPECT_EQ(carr.at(2), 30);

	// data()
	int* p = arr.data();
	EXPECT_EQ(*p, 10);
	const int* cp = carr.data();
	EXPECT_EQ(cp[2], 30);

	// Zero-sized array (UB must be avoided when testing)
	raw::array<int, 0> zero;
	EXPECT_THROW(zero.at(0), std::out_of_range);
}

TEST(ArrayTest, Iterators)
{
	raw::array<int, 3> arr = { 1, 2, 3 };

	// Forward iteration
	int sum = 0;
	for (auto it = arr.begin(); it != arr.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 6);

	// Reverse iteration
	int rsum = 0;
	for (auto it = arr.rbegin(); it != arr.rend(); ++it)
	{
		rsum += *it;
	}
	EXPECT_EQ(rsum, 6);

	// Random access operations
	auto it = arr.begin();
	EXPECT_EQ(it[1], 2);
	it += 2;
	EXPECT_EQ(*it, 3);
	it -= 1;
	EXPECT_EQ(*it, 2);

	// Const iterators
	const auto& carr = arr;
	EXPECT_EQ(*carr.cbegin(), 1);
	EXPECT_EQ(*carr.crbegin(), 3);

	// Comparison between mutable and const iterators
	EXPECT_EQ(arr.begin(), carr.begin());
	EXPECT_EQ(arr.end(), carr.end());

	// Zero-sized array
	raw::array<int, 0> zero;
	EXPECT_EQ(zero.begin(), zero.end());
	EXPECT_EQ(zero.end() - zero.begin(), 0);
}

TEST(ArrayTest, Capacity)
{
	raw::array<int, 5> arr = { 1, 2, 3, 4, 5 };
	EXPECT_FALSE(arr.empty());
	EXPECT_EQ(arr.size(), 5);
	EXPECT_EQ(arr.max_size(), 5);

	raw::array<int, 0> zero;
	EXPECT_TRUE(zero.empty());
	EXPECT_EQ(zero.size(), 0);
	EXPECT_EQ(zero.max_size(), 0);
}

TEST(ArrayTest, Operations)
{
	// fill
	raw::array<int, 3> arr1{};
	arr1.fill(9);
	EXPECT_TRUE(equal(arr1, { 9, 9, 9 }));

	// swap
	raw::array<int, 3> arr2 = { 0, 0, 0 };
	arr1.swap(arr2);
	EXPECT_TRUE(equal(arr1, { 0, 0, 0 }));
	EXPECT_TRUE(equal(arr2, { 9, 9, 9 }));

	// self-swap (should be safe)
	arr1.swap(arr1);
	EXPECT_TRUE(equal(arr1, { 0, 0, 0 }));
}

TEST(ArrayTest, Comparisons)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 1, 2, 3 };
	raw::array<int, 3> arr3 = { 7, 8, 9 };

	// Equality
	EXPECT_TRUE(arr1 == arr2);
	EXPECT_FALSE(arr1 != arr2);

	// Relational
	EXPECT_FALSE(arr1 < arr2);
	EXPECT_TRUE(arr1 <= arr2);
	EXPECT_FALSE(arr1 > arr2);
	EXPECT_TRUE(arr1 >= arr2);

	// Three-way comparison (strong ordering)
	EXPECT_EQ(arr1 <=> arr2, std::strong_ordering::equal);
	EXPECT_EQ(arr1 <=> arr3, std::strong_ordering::less);
	EXPECT_EQ(arr3 <=> arr1, std::strong_ordering::greater);

	// Zero-sized array
	raw::array<int, 0> z1, z2;
	EXPECT_TRUE(z1 == z2);
	EXPECT_TRUE(z1 <=> z2 == std::strong_ordering::equal);
}

TEST(ArrayTest, NonMemberSwap)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 4, 5, 6 };
	swap(arr1, arr2);
	EXPECT_TRUE(equal(arr1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(arr2, { 1, 2, 3 }));
}

TEST(ArrayTest, Get)
{
	raw::array<int, 3> arr = { 10, 20, 30 };
	const auto& carr = arr;

	EXPECT_EQ(raw::get<0>(arr), 10);							  // &
	EXPECT_EQ(raw::get<1>(carr), 20);							  // const &
	EXPECT_EQ(raw::get<2>(raw::array<int, 3>{ 40, 50, 60 }), 60); // &&
	const auto&& crarr = raw::array<int, 3>{ 70, 80, 90 };
	EXPECT_EQ(raw::get<0>(std::move(crarr)), 70);				  // const &&
}

TEST(ArrayTest, ToArray)
{
	int carr[] = { 5, 6, 7 };
	const int ccarr[] = { 8, 9 };

	auto arr1 = raw::to_array(carr);			// lvalue
	auto arr2 = raw::to_array(ccarr);			// const lvalue
	auto arr3 = raw::to_array(std::move(carr)); // rvalue

	EXPECT_EQ(arr1[0], 5);
	EXPECT_EQ(arr2[0], 8);
	EXPECT_EQ(arr3[0], 5); // int move is copy

	const int src[2] = { 1, 2 };
	auto arr = raw::to_array(src);
	static_assert(std::is_same_v<decltype(arr), raw::array<int, 2>>);
}

TEST(ArrayTest, StructuredBinding)
{
	raw::array<int, 3> arr = { 10, 20, 30 };
	auto [a, b, c] = arr;
	EXPECT_EQ(a, 10);
	EXPECT_EQ(b, 20);
	EXPECT_EQ(c, 30);
}

TEST(ArrayTest, TupleProtocol)
{
	// std::tuple_size
	static_assert(std::tuple_size_v<raw::array<int, 5>> == 5);
	static_assert(std::tuple_size_v<raw::array<double, 0>> == 0);

	// std::tuple_element
	static_assert(std::is_same_v<std::tuple_element_t<2, raw::array<float, 4>>, float>);
}

TEST(ArrayTest, Constexpr)
{
	constexpr raw::array<int, 3> arr = { 1, 2, 3 };
	constexpr int sum = arr[0] + arr[1] + arr[2];
	static_assert(sum == 6);
	static_assert(!arr.empty());
	static_assert(arr.size() == 3);
	static_assert(arr.max_size() == 3);
	static_assert(arr.front() == 1 && arr.back() == 3);

	// fill in constexpr
	constexpr auto filled = [] {
		raw::array<int, 3> a{};
		a.fill(5);
		return a;
	}();
	static_assert(filled[0] == 5 && filled[2] == 5);

	// swap in constexpr
	constexpr auto swapped = [] {
		raw::array<int, 2> a = { 1, 2 };
		raw::array<int, 2> b = { 3, 4 };
		a.swap(b);
		return std::pair(a, b);
	}();
	static_assert(swapped.first[0] == 3 && swapped.first[1] == 4);
	static_assert(swapped.second[0] == 1 && swapped.second[1] == 2);

	// get / to_array
	constexpr int raw_buf[] = { 7, 8, 9 };
	constexpr auto const_to_arr = raw::to_array(raw_buf);
	static_assert(const_to_arr[0] == 7);
	static_assert(raw::get<1>(const_to_arr) == 8);
}
