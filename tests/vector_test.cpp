#include <gtest/gtest.h>
#include <raw/vector.h>

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T>
bool equal(const raw::vector<T>& v, std::initializer_list<T> ilist)
{
	if (v.size() != ilist.size())
	{
		return false;
	}

	return std::equal(v.begin(), v.end(), ilist.begin());
}

TEST(VectorTest, Constructors)
{
	// Default constructor
	raw::vector<int> v1;
	EXPECT_TRUE(v1.empty());
	EXPECT_EQ(v1.size(), 0);
	EXPECT_EQ(v1.capacity(), 0);

	// Size constructor (value-initialization)
	raw::vector<int> v2(5);
	EXPECT_EQ(v2.size(), 5);
	for (int x : v2)
	{
		EXPECT_EQ(x, 0);
	}

	// Size + value constructor
	raw::vector<int> v3(3, 42);
	EXPECT_TRUE(equal(v3, { 42, 42, 42 }));

	// Range constructor (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::vector<int> v4(src.begin(), src.end());
	EXPECT_TRUE(equal(v4, { 1, 2, 3, 4 }));

	// Initializer list constructor
	raw::vector<int> v5 = { 5, 6, 7 };
	EXPECT_TRUE(equal(v5, { 5, 6, 7 }));

	// Copy constructor
	raw::vector<int> v6(v5);
	EXPECT_TRUE(equal(v6, { 5, 6, 7 }));

	// Move constructor
	raw::vector<int> v7(std::move(v5));
	EXPECT_TRUE(equal(v7, { 5, 6, 7 }));
	EXPECT_TRUE(v5.empty()); // moved-from is valid but unspecified
}

TEST(VectorTest, Assignment)
{
	raw::vector<int> v1 = { 1, 2, 3 };
	raw::vector<int> v2 = { 4, 5 };

	// Copy assignment
	v1 = v2;
	EXPECT_TRUE(equal(v1, { 4, 5 }));
	EXPECT_EQ(v1.size(), 2);

	// Move assignment
	raw::vector<int> v3 = { 6, 7, 8 };
	v1 = std::move(v3);
	EXPECT_TRUE(equal(v1, { 6, 7, 8 }));
	EXPECT_TRUE(v3.empty());

	// Initializer list assignment
	v1 = { 9, 10 };
	EXPECT_TRUE(equal(v1, { 9, 10 }));

	// Self assignment
	v1 = v1;
	EXPECT_TRUE(equal(v1, { 9, 10 }));

	// assign(size, value)
	v1.assign(3, 42);
	EXPECT_TRUE(equal(v1, { 42, 42, 42 }));

	// assign(range)
	std::vector<int> vec = { 1, 2, 3 };
	v1.assign(vec.begin(), vec.end());
	EXPECT_TRUE(equal(v1, { 1, 2, 3 }));

	// assign(initializer_list)
	v1.assign({ 7, 8, 9 });
	EXPECT_TRUE(equal(v1, { 7, 8, 9 }));
}

TEST(VectorTest, ElementAccess)
{
	raw::vector<int> v = { 10, 20, 30, 40, 50 };

	// at (within bounds)
	EXPECT_EQ(v.at(2), 30);
	EXPECT_EQ(v.at(4), 50);

	// at (out of bounds)
	EXPECT_THROW(v.at(5), std::out_of_range);
	const auto& cv = v;
	EXPECT_THROW(cv.at(5), std::out_of_range);

	// operator[]
	EXPECT_EQ(v[1], 20);
	v[1] = 25;
	EXPECT_EQ(v[1], 25);

	// front / back
	EXPECT_EQ(v.front(), 10);
	EXPECT_EQ(v.back(), 50);
	v.front() = 1;
	v.back() = 100;
	EXPECT_EQ(v.front(), 1);
	EXPECT_EQ(v.back(), 100);

	// data()
	int* data = v.data();
	EXPECT_EQ(data[0], 1);
	EXPECT_EQ(data[4], 100);
	const auto& cv2 = v;
	const int* cdata = cv2.data();
	EXPECT_EQ(cdata[2], 30);
}

TEST(VectorTest, Iterators)
{
	raw::vector<int> v = { 1, 2, 3, 4, 5 };

	// Forward iteration
	int sum = 0;
	for (auto it = v.begin(); it != v.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 15);

	// Reverse iteration
	int rsum = 0;
	for (auto it = v.rbegin(); it != v.rend(); ++it)
	{
		rsum += *it;
	}
	EXPECT_EQ(rsum, 15);

	// Const iterators
	const raw::vector<int>& cv = v;
	int csum = 0;
	for (auto it = cv.cbegin(); it != cv.cend(); ++it)
	{
		csum += *it;
	}
	EXPECT_EQ(csum, 15);

	// Iterator difference
	EXPECT_EQ(v.end() - v.begin(), 5);

	// Empty vector iterators
	raw::vector<int> empty;
	EXPECT_EQ(empty.begin(), empty.end());
}

TEST(VectorTest, Capacity)
{
	raw::vector<int> v;
	EXPECT_TRUE(v.empty());
	EXPECT_EQ(v.size(), 0);
	EXPECT_EQ(v.capacity(), 0);

	v.reserve(10);
	EXPECT_GE(v.capacity(), 10);
	EXPECT_EQ(v.size(), 0);
	v.push_back(1);
	EXPECT_EQ(v.size(), 1);
	EXPECT_GE(v.capacity(), 10);

	v.shrink_to_fit();
	EXPECT_EQ(v.capacity(), v.size()); // should be 1

	v = { 1, 2, 3, 4, 5 };
	std::size_t old_cap = v.capacity();
	v.shrink_to_fit();
	EXPECT_EQ(v.capacity(), v.size()); // 5
	EXPECT_LE(v.capacity(), old_cap);

	// max_size is huge (typically)
	EXPECT_GT(v.max_size(), 1000000);
}

TEST(VectorTest, Modifiers)
{
	// push_back / pop_back
	raw::vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.emplace_back(3);
	EXPECT_TRUE(equal(v, { 1, 2, 3 }));
	v.pop_back();
	EXPECT_TRUE(equal(v, { 1, 2 }));
	v.pop_back();
	v.pop_back();
	EXPECT_TRUE(v.empty());

	// clear
	v = { 5, 6, 7 };
	v.clear();
	EXPECT_TRUE(v.empty());

	// insert single element (lvalue)
	v = { 1, 2, 3 };
	auto it = v.insert(v.begin() + 1, 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(v, { 1, 99, 2, 3 }));

	// insert rvalue
	v.insert(v.begin(), 100);
	EXPECT_TRUE(equal(v, { 100, 1, 99, 2, 3 }));

	// insert count copies
	v = { 1, 2, 3 };
	v.insert(v.begin() + 1, 2, 42);
	EXPECT_TRUE(equal(v, { 1, 42, 42, 2, 3 }));

	// force reallocation
	v.reserve(3);
	v = { 1, 2 };
	v.insert(v.begin() + 1, 3, 9);
	EXPECT_TRUE(equal(v, { 1, 9, 9, 9, 2 }));

	// insert range (forward iterator)
	v = { 1, 2, 3 };
	std::vector<int> src = { 10, 20 };
	v.insert(v.begin() + 2, src.begin(), src.end());
	EXPECT_TRUE(equal(v, { 1, 2, 10, 20, 3 }));

	// insert range (input iterator)
	std::istringstream iss("100 200");
	std::istream_iterator<int> iit(iss), iend;
	v.insert(v.end(), iit, iend);
	EXPECT_TRUE(equal(v, { 1, 2, 10, 20, 3, 100, 200 }));

	// insert initializer_list
	v = { 1, 2, 3 };
	v.insert(v.begin() + 1, { 4, 5 });
	EXPECT_TRUE(equal(v, { 1, 4, 5, 2, 3 }));

	// emplace
	v = { 1, 2, 3 };
	it = v.emplace(v.begin() + 1, 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(v, { 1, 99, 2, 3 }));

	// emplace_back
	v.emplace_back(100);
	EXPECT_TRUE(equal(v, { 1, 99, 2, 3, 100 }));

	// erase single
	v.erase(v.begin() + 1); // removes 99
	EXPECT_TRUE(equal(v, { 1, 2, 3, 100 }));

	// erase range
	v.erase(v.begin() + 1, v.begin() + 3); // removes 2, 3
	EXPECT_TRUE(equal(v, { 1, 100 }));

	// resize (default)
	v.resize(5);
	EXPECT_EQ(v.size(), 5);
	EXPECT_EQ(v[2], 0); // value-initialized
	EXPECT_EQ(v[4], 0);
	v.resize(2);
	EXPECT_EQ(v.size(), 2);
	EXPECT_TRUE(equal(v, { 1, 100 }));

	// resize (with value)
	v.resize(4, 42);
	EXPECT_TRUE(equal(v, { 1, 100, 42, 42 }));
}

TEST(VectorTest, Comparisons)
{
	raw::vector<int> v1 = { 1, 2, 3 };
	raw::vector<int> v2 = { 1, 2, 3 };
	raw::vector<int> v3 = { 1, 2, 4 };
	raw::vector<int> v4 = { 1, 2 };

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);
	EXPECT_TRUE(v1 != v3);
	EXPECT_TRUE(v1 < v3);
	EXPECT_TRUE(v3 > v1);
	EXPECT_TRUE(v1 <= v2);
	EXPECT_TRUE(v3 >= v1);
	EXPECT_TRUE(v4 < v1);
}

TEST(VectorTest, NonMemberSwap)
{
	raw::vector<int> v1 = { 1, 2, 3 };
	raw::vector<int> v2 = { 4, 5, 6 };
	swap(v1, v2);
	EXPECT_TRUE(equal(v1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(v2, { 1, 2, 3 }));
}

TEST(VectorTest, NonMemberErase)
{
	raw::vector<int> v = { 1, 2, 3, 2, 4, 2, 5 };
	auto cnt = erase(v, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(v, { 1, 3, 4, 5 }));

	v = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(v, [](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(v, { 1, 3, 5 }));
}
