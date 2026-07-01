#include <algorithm>
#include <compare>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <raw/vector.h>

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
	// Default construction
	raw::vector<int> v1;
	EXPECT_TRUE(v1.empty());
	EXPECT_EQ(v1.size(), 0);
	EXPECT_EQ(v1.capacity(), 0);

	// Size construction (value-initialization)
	raw::vector<int> v2(5);
	EXPECT_EQ(v2.size(), 5);
	EXPECT_GE(v2.capacity(), v2.size());
	for (int x : v2)
	{
		EXPECT_EQ(x, 0);
	}

	// Size + value construction
	raw::vector<int> v3(3, 42);
	EXPECT_TRUE(equal(v3, { 42, 42, 42 }));

	// Range construction (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::vector<int> v4(src.begin(), src.end());
	EXPECT_TRUE(equal(v4, { 1, 2, 3, 4 }));

	// Range construction (input iterator)
	std::istringstream iss("100 200 300");
	std::istream_iterator<int> iit(iss), iend;
	raw::vector<int> v5(iit, iend);
	EXPECT_TRUE(equal(v5, { 100, 200, 300 }));

	// Initializer-list construction
	raw::vector<int> v6 = { 5, 6, 7 };
	EXPECT_TRUE(equal(v6, { 5, 6, 7 }));

	// Copy construction
	raw::vector<int> v7(v6);
	EXPECT_TRUE(equal(v7, { 5, 6, 7 }));

	// Move construction
	raw::vector<int> v8(std::move(v7));
	EXPECT_TRUE(equal(v8, { 5, 6, 7 }));
	EXPECT_TRUE(v7.empty());
}

TEST(VectorTest, Assignment)
{
	raw::vector<int> v1 = { 1, 2, 3 };
	raw::vector<int> v2 = { 4, 5 };

	// Copy assignment
	v1 = v2;
	EXPECT_TRUE(equal(v1, { 4, 5 }));
	EXPECT_TRUE(equal(v2, { 4, 5 }));
	EXPECT_EQ(v1.size(), 2);

	// Move assignment
	raw::vector<int> v3 = { 6, 7, 8 };
	v1 = std::move(v3);
	EXPECT_TRUE(equal(v1, { 6, 7, 8 }));
	EXPECT_TRUE(v3.empty());

	// Initializer-list assignment
	v1 = { 9, 10 };
	EXPECT_TRUE(equal(v1, { 9, 10 }));

	// Self-assignment
	v1 = v1;
	EXPECT_TRUE(equal(v1, { 9, 10 }));

	// assign(count, value)
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
	raw::vector<int> v = { 1, 2, 3, 4 };

	// at (within bounds)
	EXPECT_EQ(v.at(2), 3);

	// at (out of bounds)
	EXPECT_THROW(v.at(4), std::out_of_range);

	// operator[]
	EXPECT_EQ(v[1], 2);
	EXPECT_EQ(v[2], 3);
	v[1] = 20;
	v[2] = 30;
	EXPECT_EQ(v[1], 20);
	EXPECT_EQ(v[2], 30);

	// front / back
	EXPECT_EQ(v.front(), 1);
	EXPECT_EQ(v.back(), 4);
	v.front() = 10;
	v.back() = 40;
	EXPECT_EQ(v.front(), 10);
	EXPECT_EQ(v.back(), 40);

	// const versions
	const auto& cv = v;
	EXPECT_EQ(cv.front(), 10);
	EXPECT_EQ(cv.back(), 40);
	EXPECT_EQ(cv[2], 30);
	EXPECT_EQ(cv.at(2), 30);

	// data()
	int* p = v.data();
	EXPECT_EQ(*p, 10);
	const int* cp = cv.data();
	EXPECT_EQ(cp[2], 30);

	// Empty vector (UB must be avoided when testing)
	raw::vector<int> empty;
	EXPECT_THROW(empty.at(0), std::out_of_range);
}

TEST(VectorTest, Iterators)
{
	raw::vector<int> v = { 1, 2, 3 };

	// Forward iteration
	int sum = 0;
	for (auto it = v.begin(); it != v.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 6);

	// Reverse iteration
	int rsum = 0;
	for (auto it = v.rbegin(); it != v.rend(); ++it)
	{
		rsum += *it;
	}
	EXPECT_EQ(rsum, 6);

	// Random access operations
	auto it = v.begin();
	EXPECT_EQ(it[1], 2);
	it += 2;
	EXPECT_EQ(*it, 3);
	it -= 1;
	EXPECT_EQ(*it, 2);

	// Const iterators
	const auto& cv = v;
	EXPECT_EQ(*cv.cbegin(), 1);
	EXPECT_EQ(*cv.crbegin(), 3);

	// Comparison between mutable and const iterators
	EXPECT_EQ(v.begin(), cv.begin());
	EXPECT_EQ(v.end(), cv.end());
}

TEST(VectorTest, Capacity)
{
	raw::vector<int> v;
	EXPECT_TRUE(v.empty());
	EXPECT_EQ(v.size(), 0);
	EXPECT_EQ(v.capacity(), 0);

	// reserve
	v.reserve(10);
	EXPECT_GE(v.capacity(), 10);
	EXPECT_EQ(v.size(), 0);
	v.push_back(1);
	EXPECT_EQ(v.size(), 1);
	EXPECT_GE(v.capacity(), 10);

	// shrink_to_fit
	v.shrink_to_fit();
	EXPECT_EQ(v.capacity(), v.size());

	// max_size is huge (typically)
	EXPECT_GT(v.max_size(), 1000000u);
}

TEST(VectorTest, Modifiers)
{
	raw::vector<int> v;

	// push_back / pop_back
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	EXPECT_TRUE(equal(v, { 1, 2, 3 }));
	v.pop_back();
	EXPECT_TRUE(equal(v, { 1, 2 }));
	v.pop_back();
	v.pop_back();
	EXPECT_TRUE(v.empty());

	// clear
	v = { 5, 6, 7 };
	auto before_cap = v.capacity();
	v.clear();
	EXPECT_TRUE(v.empty());
	EXPECT_EQ(v.capacity(), before_cap);

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
	v.erase(v.begin() + 1);
	EXPECT_TRUE(equal(v, { 1, 2, 3, 100 }));

	// erase range
	v.erase(v.begin() + 1, v.begin() + 3);
	EXPECT_TRUE(equal(v, { 1, 100 }));

	// resize (default)
	v.resize(5);
	EXPECT_EQ(v.size(), 5);
	EXPECT_EQ(v[2], 0);
	EXPECT_EQ(v[4], 0);
	v.resize(2);
	EXPECT_EQ(v.size(), 2);
	EXPECT_TRUE(equal(v, { 1, 100 }));

	// resize (with value)
	v.resize(4, 42);
	EXPECT_TRUE(equal(v, { 1, 100, 42, 42 }));

	// swap
	raw::vector<int> other = { 7, 8 };
	v.swap(other);
	EXPECT_TRUE(equal(v, { 7, 8 }));
	EXPECT_TRUE(equal(other, { 1, 100, 42, 42 }));

	// self-swap (should be safe)
	v.swap(v);
	EXPECT_TRUE(equal(v, { 7, 8 }));
}

TEST(VectorTest, Comparisons)
{
	raw::vector<int> v1 = { 1, 2, 3 };
	raw::vector<int> v2 = { 1, 2, 3 };
	raw::vector<int> v3 = { 7, 8, 9 };

	// Equality
	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);

	// Relational
	EXPECT_FALSE(v1 < v2);
	EXPECT_TRUE(v1 <= v2);
	EXPECT_FALSE(v1 > v2);
	EXPECT_TRUE(v1 >= v2);

	// Three-way comparison (strong ordering)
	EXPECT_EQ(v1 <=> v2, std::strong_ordering::equal);
	EXPECT_EQ(v1 <=> v3, std::strong_ordering::less);
	EXPECT_EQ(v3 <=> v1, std::strong_ordering::greater);
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
	raw::vector<int> v = { 1, 2, 3, 2, 4, 2 };
	auto cnt = erase(v, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(v, { 1, 3, 4 }));

	v = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(v, [](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(v, { 1, 3, 5 }));
}

TEST(VectorTest, MoveOnly)
{
	struct MoveOnly
	{
		int val;

		explicit MoveOnly(int v)
			: val(v)
		{
		}

		MoveOnly(MoveOnly&& o) noexcept
			: val(o.val)
		{
			o.val = 0;
		}

		MoveOnly(const MoveOnly&) = delete;

		bool operator==(const MoveOnly& rhs) const
		{
			return val == rhs.val;
		}
	};

	raw::vector<MoveOnly> v;
	v.emplace_back(1);
	v.push_back(MoveOnly(2));
	EXPECT_EQ(v.size(), 2);
	EXPECT_EQ(v[0].val, 1);
	EXPECT_EQ(v[1].val, 2);

	v.emplace_back(3);
	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(v[2].val, 3);
}
