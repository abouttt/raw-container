#include <algorithm>
#include <compare>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <raw/list.h>

template <typename T>
bool equal(const raw::list<T>& lst, std::initializer_list<T> ilist)
{
	if (lst.size() != ilist.size())
	{
		return false;
	}

	return std::equal(lst.begin(), lst.end(), ilist.begin());
}

TEST(ListTest, Constructors)
{
	// Default construction
	raw::list<int> lst1;
	EXPECT_TRUE(lst1.empty());
	EXPECT_EQ(lst1.size(), 0);

	// Size construction (value-initialization)
	raw::list<int> lst2(5);
	EXPECT_EQ(lst2.size(), 5);
	for (int x : lst2)
	{
		EXPECT_EQ(x, 0);
	}

	// Size + value construction
	raw::list<int> lst3(3, 42);
	EXPECT_TRUE(equal(lst3, { 42, 42, 42 }));

	// Range construction (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::list<int> lst4(src.begin(), src.end());
	EXPECT_TRUE(equal(lst4, { 1, 2, 3, 4 }));

	// Range construction (input iterator)
	std::istringstream iss("100 200 300");
	std::istream_iterator<int> iit(iss), iend;
	raw::list<int> lst5(iit, iend);
	EXPECT_TRUE(equal(lst5, { 100, 200, 300 }));

	// Initializer-list construction
	raw::list<int> lst6 = { 5, 6, 7 };
	EXPECT_TRUE(equal(lst6, { 5, 6, 7 }));

	// Copy construction
	raw::list<int> lst7(lst6);
	EXPECT_TRUE(equal(lst7, { 5, 6, 7 }));

	// Move construction
	raw::list<int> lst8(std::move(lst7));
	EXPECT_TRUE(equal(lst8, { 5, 6, 7 }));
	EXPECT_TRUE(lst7.empty());
}

TEST(ListTest, Assignment)
{
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 4, 5 };

	// Copy assignment
	lst1 = lst2;
	EXPECT_TRUE(equal(lst1, { 4, 5 }));
	EXPECT_TRUE(equal(lst2, { 4, 5 }));
	EXPECT_EQ(lst1.size(), 2);

	// Move assignment
	raw::list<int> lst3 = { 6, 7, 8 };
	lst1 = std::move(lst3);
	EXPECT_TRUE(equal(lst1, { 6, 7, 8 }));
	EXPECT_TRUE(lst3.empty());

	// Initializer-list assignment
	lst1 = { 9, 10 };
	EXPECT_TRUE(equal(lst1, { 9, 10 }));

	// Self-assignment
	lst1 = lst1;
	EXPECT_TRUE(equal(lst1, { 9, 10 }));

	// assign(count, value)
	lst1.assign(3, 42);
	EXPECT_TRUE(equal(lst1, { 42, 42, 42 }));

	// assign(range)
	std::vector<int> vec = { 1, 2, 3 };
	lst1.assign(vec.begin(), vec.end());
	EXPECT_TRUE(equal(lst1, { 1, 2, 3 }));

	// assign(initializer_list)
	lst1.assign({ 7, 8, 9 });
	EXPECT_TRUE(equal(lst1, { 7, 8, 9 }));
}

TEST(ListTest, ElementAccess)
{
	raw::list<int> lst = { 1, 2, 3, 4 };

	// front / back
	EXPECT_EQ(lst.front(), 1);
	EXPECT_EQ(lst.back(), 4);
	lst.front() = 10;
	lst.back() = 40;
	EXPECT_EQ(lst.front(), 10);
	EXPECT_EQ(lst.back(), 40);

	// const versions
	const auto& clst = lst;
	EXPECT_EQ(clst.front(), 10);
	EXPECT_EQ(clst.back(), 40);

	// Empty list (UB must be avoided when testing)
	raw::list<int> empty;
	EXPECT_TRUE(empty.empty());
}

TEST(ListTest, Iterators)
{
	raw::list<int> lst = { 1, 2, 3 };

	// Forward iteration
	int sum = 0;
	for (auto it = lst.begin(); it != lst.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 6);

	// Reverse iteration
	int rsum = 0;
	for (auto it = lst.rbegin(); it != lst.rend(); ++it)
	{
		rsum += *it;
	}
	EXPECT_EQ(rsum, 6);

	// Bidirectional operations
	auto it = lst.begin();
	EXPECT_EQ(*it, 1);
	++it;
	EXPECT_EQ(*it, 2);
	--it;
	EXPECT_EQ(*it, 1);

	// Const iterators
	const auto& clst = lst;
	EXPECT_EQ(*clst.cbegin(), 1);
	EXPECT_EQ(*clst.crbegin(), 3);

	// Comparison between mutable and const iterators
	EXPECT_EQ(lst.begin(), clst.begin());
	EXPECT_EQ(lst.end(), clst.end());
}

TEST(ListTest, Capacity)
{
	raw::list<int> lst;
	EXPECT_TRUE(lst.empty());
	EXPECT_EQ(lst.size(), 0);

	lst.push_back(1);
	EXPECT_FALSE(lst.empty());
	EXPECT_EQ(lst.size(), 1);

	// max_size is huge (typically)
	EXPECT_GT(lst.max_size(), 1000000u);
}

TEST(ListTest, Modifiers)
{
	raw::list<int> lst;

	// push_back / pop_back
	lst.push_back(1);
	lst.push_back(2);
	lst.push_back(3);
	EXPECT_TRUE(equal(lst, { 1, 2, 3 }));
	lst.pop_back();
	EXPECT_TRUE(equal(lst, { 1, 2 }));
	lst.pop_back();
	lst.pop_back();
	EXPECT_TRUE(lst.empty());

	// push_front / pop_front
	lst.push_front(10);
	lst.push_front(20);
	lst.push_front(30);
	EXPECT_TRUE(equal(lst, { 30, 20, 10 }));
	lst.pop_front();
	EXPECT_TRUE(equal(lst, { 20, 10 }));
	lst.pop_front();
	lst.pop_front();
	EXPECT_TRUE(lst.empty());

	// clear
	lst = { 5, 6, 7 };
	lst.clear();
	EXPECT_TRUE(lst.empty());

	// insert single element (lvalue)
	lst = { 1, 2, 3 };
	auto it = lst.insert(std::next(lst.begin()), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(lst, { 1, 99, 2, 3 }));

	// insert rvalue
	lst.insert(lst.begin(), 100);
	EXPECT_TRUE(equal(lst, { 100, 1, 99, 2, 3 }));

	// insert count copies
	lst = { 1, 2, 3 };
	lst.insert(std::next(lst.begin()), 2, 42);
	EXPECT_TRUE(equal(lst, { 1, 42, 42, 2, 3 }));

	// insert range (forward iterator)
	lst = { 1, 2, 3 };
	std::vector<int> src = { 10, 20 };
	lst.insert(std::next(lst.begin(), 2), src.begin(), src.end());
	EXPECT_TRUE(equal(lst, { 1, 2, 10, 20, 3 }));

	// insert range (input iterator)
	std::istringstream iss("100 200");
	std::istream_iterator<int> iit(iss), iend;
	lst.insert(lst.end(), iit, iend);
	EXPECT_TRUE(equal(lst, { 1, 2, 10, 20, 3, 100, 200 }));

	// insert initializer_list
	lst = { 1, 2, 3 };
	lst.insert(std::next(lst.begin()), { 4, 5 });
	EXPECT_TRUE(equal(lst, { 1, 4, 5, 2, 3 }));

	// emplace
	lst = { 1, 2, 3 };
	it = lst.emplace(std::next(lst.begin()), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(lst, { 1, 99, 2, 3 }));

	// emplace_back
	lst.emplace_back(100);
	EXPECT_TRUE(equal(lst, { 1, 99, 2, 3, 100 }));

	// emplace_front
	lst.emplace_front(0);
	EXPECT_TRUE(equal(lst, { 0, 1, 99, 2, 3, 100 }));

	// erase single
	lst.erase(std::next(lst.begin(), 2));
	EXPECT_TRUE(equal(lst, { 0, 1, 2, 3, 100 }));

	// erase range
	lst.erase(std::next(lst.begin(), 2), std::prev(lst.end()));
	EXPECT_TRUE(equal(lst, { 0, 1, 100 }));

	// resize (default)
	lst.resize(5);
	EXPECT_EQ(lst.size(), 5);
	EXPECT_EQ(*std::next(lst.begin(), 3), 0);
	EXPECT_EQ(*std::prev(lst.end()), 0);
	lst.resize(2);
	EXPECT_EQ(lst.size(), 2);
	EXPECT_TRUE(equal(lst, { 0, 1 }));

	// resize (with value)
	lst.resize(4, 42);
	EXPECT_TRUE(equal(lst, { 0, 1, 42, 42 }));

	// swap
	raw::list<int> other = { 7, 8 };
	lst.swap(other);
	EXPECT_TRUE(equal(lst, { 7, 8 }));
	EXPECT_TRUE(equal(other, { 0, 1, 42, 42 }));

	// self-swap (should be safe)
	lst.swap(lst);
	EXPECT_TRUE(equal(lst, { 7, 8 }));
}

TEST(ListTest, Operations)
{
	// splice
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 10, 20, 30 };

	lst1.splice(lst1.begin(), lst2);
	EXPECT_TRUE(equal(lst1, { 10, 20, 30, 1, 2, 3 }));
	EXPECT_TRUE(lst2.empty());

	// splice single element
	lst2 = { 100, 200 };
	lst1.splice(std::next(lst1.begin()), lst2, lst2.begin());
	EXPECT_TRUE(equal(lst1, { 10, 100, 20, 30, 1, 2, 3 }));
	EXPECT_EQ(lst2.size(), 1);
	EXPECT_EQ(lst2.front(), 200);

	// splice range
	raw::list<int> lst3 = { 50, 60, 70, 80 };
	auto it = lst3.begin();
	++it;
	auto it2 = it;
	std::advance(it2, 2);
	lst1.splice(lst1.end(), lst3, it, it2);
	EXPECT_TRUE(equal(lst1, { 10, 100, 20, 30, 1, 2, 3, 60, 70 }));
	EXPECT_TRUE(equal(lst3, { 50, 80 }));

	// self‑splice single element (no-op)
	auto sz = lst1.size();
	lst1.splice(lst1.begin(), lst1, std::next(lst1.begin()));
	EXPECT_EQ(lst1.size(), sz);
	EXPECT_EQ(lst1.front(), 100);

	// merge
	raw::list<int> sorted1 = { 1, 3, 5, 7 };
	raw::list<int> sorted2 = { 2, 4, 6, 8 };

	sorted1.merge(sorted2);
	EXPECT_TRUE(equal(sorted1, { 1, 2, 3, 4, 5, 6, 7, 8 }));
	EXPECT_TRUE(sorted2.empty());

	// merge with comparator (descending)
	raw::list<int> sorted3 = { 10, 5, 3 };
	raw::list<int> sorted4 = { 9, 7, 1 };
	sorted3.merge(sorted4, std::greater<>{});
	EXPECT_TRUE(equal(sorted3, { 10, 9, 7, 5, 3, 1 }));
	EXPECT_TRUE(sorted4.empty());

	// merge into empty
	raw::list<int> empty;
	empty.merge(sorted1);
	EXPECT_TRUE(equal(empty, { 1, 2, 3, 4, 5, 6, 7, 8 }));
	EXPECT_TRUE(sorted1.empty());

	// remove / remove_if
	raw::list<int> lst4 = { 1, 2, 3, 2, 4, 2, 5 };
	auto removed = lst4.remove(2);
	EXPECT_EQ(removed, 3);
	EXPECT_TRUE(equal(lst4, { 1, 3, 4, 5 }));

	removed = lst4.remove_if([](int x) { return x % 2 != 0; });
	EXPECT_EQ(removed, 3);
	EXPECT_TRUE(equal(lst4, { 4 }));

	// reverse
	raw::list<int> lst5 = { 1, 2, 3, 4, 5 };
	lst5.reverse();
	EXPECT_TRUE(equal(lst5, { 5, 4, 3, 2, 1 }));

	// empty list reverse
	raw::list<int> empty2;
	empty2.reverse();
	EXPECT_TRUE(empty2.empty());

	// single element reverse
	raw::list<int> single = { 42 };
	single.reverse();
	EXPECT_TRUE(equal(single, { 42 }));

	// unique
	raw::list<int> lst6 = { 1, 1, 2, 3, 3, 3, 4, 5, 5 };
	removed = lst6.unique();
	EXPECT_EQ(removed, 4);
	EXPECT_TRUE(equal(lst6, { 1, 2, 3, 4, 5 }));

	// empty list unique
	raw::list<int> empty3;
	removed = empty3.unique();
	EXPECT_EQ(removed, 0);

	// sort
	raw::list<int> lst7 = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
	lst7.sort();
	EXPECT_TRUE(equal(lst7, { 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 }));

	// sort with comparator (descending)
	lst7 = { 3, 1, 4, 1, 5 };
	lst7.sort(std::greater<>{});
	EXPECT_TRUE(equal(lst7, { 5, 4, 3, 1, 1 }));

	// empty list sort
	raw::list<int> empty4;
	empty4.sort();
	EXPECT_TRUE(empty4.empty());

	// single element sort
	single = { 42 };
	single.sort();
	EXPECT_TRUE(equal(single, { 42 }));
}

TEST(ListTest, Comparisons)
{
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 1, 2, 3 };
	raw::list<int> lst3 = { 7, 8, 9 };

	// Equality
	EXPECT_TRUE(lst1 == lst2);
	EXPECT_FALSE(lst1 != lst2);

	// Relational
	EXPECT_FALSE(lst1 < lst2);
	EXPECT_TRUE(lst1 <= lst2);
	EXPECT_FALSE(lst1 > lst2);
	EXPECT_TRUE(lst1 >= lst2);

	// Three-way comparison (strong ordering)
	EXPECT_EQ(lst1 <=> lst2, std::strong_ordering::equal);
	EXPECT_EQ(lst1 <=> lst3, std::strong_ordering::less);
	EXPECT_EQ(lst3 <=> lst1, std::strong_ordering::greater);
}

TEST(ListTest, NonMemberSwap)
{
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 4, 5, 6 };
	swap(lst1, lst2);
	EXPECT_TRUE(equal(lst1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(lst2, { 1, 2, 3 }));
}

TEST(ListTest, NonMemberErase)
{
	raw::list<int> lst = { 1, 2, 3, 2, 4, 2 };
	auto cnt = erase(lst, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst, { 1, 3, 4 }));

	lst = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(lst, [](int x) {	return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst, { 1, 3, 5 }));
}

TEST(ListTest, MoveOnly)
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

	raw::list<MoveOnly> lst;
	lst.emplace_back(1);
	lst.push_back(MoveOnly(2));
	EXPECT_EQ(lst.size(), 2);
	EXPECT_EQ(lst.front().val, 1);
	EXPECT_EQ(lst.back().val, 2);

	lst.emplace_front(0);
	EXPECT_EQ(lst.size(), 3);
	EXPECT_EQ(lst.front().val, 0);
}
