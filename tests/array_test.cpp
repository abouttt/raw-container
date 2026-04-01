#include <gtest/gtest.h>
#include <raw/array.h>

TEST(ArrayTest, Constructors)
{
	// Aggregate initialization
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	EXPECT_EQ(arr1[0], 1);
	EXPECT_EQ(arr1[1], 2);
	EXPECT_EQ(arr1[2], 3);

	// CTAD (Class Template Argument Deduction)
	raw::array arr2 = { 4, 5, 6, 7 };
	EXPECT_EQ(arr2.size(), 4);
	EXPECT_EQ(arr2[3], 7);

	// Zero-sized array initialization
	raw::array<int, 0> empty_arr{};
	EXPECT_TRUE(empty_arr.empty());

	// Partial initialization (remaining elements zero-initialized)
	raw::array<int, 5> partial = { 1, 2 };
	EXPECT_EQ(partial[2], 0);
	EXPECT_EQ(partial[4], 0);
}

TEST(ArrayTest, Assignment)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 4, 5, 6 };

	// Copy assignment
	arr1 = arr2;
	EXPECT_EQ(arr1[0], 4);
	EXPECT_EQ(arr1[1], 5);
	EXPECT_EQ(arr1[2], 6);
	EXPECT_EQ(arr2[0], 4);
	EXPECT_EQ(arr2[1], 5);
	EXPECT_EQ(arr2[2], 6);

	// Move assignment
	raw::array<int, 3> arr3 = { 7, 8, 9 };
	arr1 = std::move(arr3); // arr3 is valid but unspecified
	EXPECT_EQ(arr1[0], 7);
	EXPECT_EQ(arr1[1], 8);
	EXPECT_EQ(arr1[2], 9);

	// Self assignment
	arr1 = arr1;            // arr1 unchanged
	EXPECT_EQ(arr1[0], 7);
	EXPECT_EQ(arr1[1], 8);
	EXPECT_EQ(arr1[2], 9);
}

TEST(ArrayTest, ElementAccess)
{
	raw::array<int, 4> arr = { 10, 20, 30, 40 };

	EXPECT_EQ(arr.at(2), 30);
	EXPECT_EQ(arr[1], 20);
	EXPECT_EQ(arr.front(), 10);
	EXPECT_EQ(arr.back(), 40);
	EXPECT_EQ(*arr.data(), 10);
	EXPECT_THROW(arr.at(4), std::out_of_range);

	raw::array<int, 0> empty_arr;
	EXPECT_THROW(empty_arr.at(0), std::out_of_range);

	// operator[], front, back, data dereference on zero-size array leads to UB,
	// so we intentionally do not test them.
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

	// Const iterator validation
	const raw::array<int, 3>& carr = arr;
	static_assert(std::is_same_v<decltype(carr.begin()), raw::array<int, 3>::const_iterator>);
	EXPECT_EQ(*carr.cbegin(), 1);
	EXPECT_EQ(*carr.crbegin(), 3);

	// Iterator distance
	EXPECT_EQ((arr.end() - arr.begin()), 3);

	// Zero-sized array iterator equality
	raw::array<int, 0> empty;
	EXPECT_EQ(empty.begin(), empty.end());
}

TEST(ArrayTest, Capacity)
{
	raw::array<int, 5> arr = { 1, 2, 3, 4, 5 };
	EXPECT_EQ(arr.size(), 5);
	EXPECT_EQ(arr.max_size(), 5);
	EXPECT_FALSE(arr.empty());

	raw::array<double, 0> empty_arr;
	EXPECT_EQ(empty_arr.size(), 0);
	EXPECT_EQ(empty_arr.max_size(), 0);
	EXPECT_TRUE(empty_arr.empty());
}

TEST(ArrayTest, Operations)
{
	// fill
	raw::array<int, 3> arr1;
	arr1.fill(9);
	EXPECT_EQ(arr1[0], 9);
	EXPECT_EQ(arr1[1], 9);
	EXPECT_EQ(arr1[2], 9);

	// swap
	raw::array<int, 3> arr2 = { 0, 0, 0 };
	arr1.swap(arr2);
	EXPECT_EQ(arr1[0], 0);
	EXPECT_EQ(arr2[0], 9);
}

TEST(ArrayTest, Comparisons)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 1, 2, 3 };
	raw::array<int, 3> arr3 = { 1, 2, 4 };

	EXPECT_TRUE(arr1 == arr2);
	EXPECT_TRUE(arr1 != arr3);
	EXPECT_TRUE(arr1 < arr3);
	EXPECT_TRUE(arr3 > arr1);
	EXPECT_TRUE(arr1 <= arr2);
	EXPECT_TRUE(arr3 >= arr2);
}

TEST(ArrayTest, NonMemberSwap)
{
	raw::array<int, 3> arr1 = { 1, 2, 3 };
	raw::array<int, 3> arr2 = { 4, 5, 6 };
	swap(arr1, arr2);
	EXPECT_EQ(arr1[0], 4);
	EXPECT_EQ(arr2[0], 1);
}

TEST(ArrayTest, TupleInterface)
{
	using Arr = raw::array<int, 2>;
	static_assert(std::tuple_size_v<Arr> == 2);
	static_assert(std::is_same_v<std::tuple_element_t<0, Arr>, int>);
	static_assert(std::is_same_v<std::tuple_element_t<1, Arr>, int>);

	raw::array<int, 2> arr = { 100, 200 };
	EXPECT_EQ(raw::get<0>(arr), 100);
	EXPECT_EQ(raw::get<1>(arr), 200);
}

TEST(ArrayTest, ToArray)
{
	int c_arr[] = { 1, 2, 3 };
	auto arr1 = raw::to_array(c_arr);
	static_assert(std::is_same_v<decltype(arr1), raw::array<int, 3>>);
	EXPECT_EQ(arr1[0], 1);
	EXPECT_EQ(arr1[1], 2);
	EXPECT_EQ(arr1[2], 3);

	auto arr2 = raw::to_array({ 4, 5, 6 });
	static_assert(std::is_same_v<decltype(arr2), raw::array<int, 3>>);
	EXPECT_EQ(arr2[0], 4);
	EXPECT_EQ(arr2[1], 5);
	EXPECT_EQ(arr2[2], 6);
}

TEST(ArrayTest, StructuredBinding)
{
	raw::array<int, 3> arr = { 10, 20, 30 };
	auto [a, b, c] = arr;
	EXPECT_EQ(a, 10);
	EXPECT_EQ(b, 20);
	EXPECT_EQ(c, 30);
}
