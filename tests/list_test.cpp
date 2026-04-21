#include <gtest/gtest.h>
#include <raw/list.h>

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

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
	// Default constructor
	raw::list<int> lst1;
	EXPECT_TRUE(lst1.empty());
	EXPECT_EQ(lst1.size(), 0);

	// Size constructor (value-initialization)
	raw::list<int> lst2(5);
	EXPECT_EQ(lst2.size(), 5);
	for (int x : lst2)
	{
		EXPECT_EQ(x, 0);
	}

	// Size + value constructor
	raw::list<int> lst3(3, 42);
	EXPECT_TRUE(equal(lst3, { 42, 42, 42 }));

	// Range constructor (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::list<int> lst4(src.begin(), src.end());
	EXPECT_TRUE(equal(lst4, { 1, 2, 3, 4 }));

	// Range constructor (input iterator)
	std::istringstream iss("10 20 30");
	std::istream_iterator<int> iit(iss), iend;
	raw::list<int> lst5(iit, iend);
	EXPECT_TRUE(equal(lst5, { 10, 20, 30 }));

	// Initializer list constructor
	raw::list<int> lst6 = { 5, 6, 7 };
	EXPECT_TRUE(equal(lst6, { 5, 6, 7 }));

	// Copy constructor
	raw::list<int> lst7(lst6);
	EXPECT_TRUE(equal(lst7, { 5, 6, 7 }));

	// Move constructor
	raw::list<int> lst8(std::move(lst6));
	EXPECT_TRUE(equal(lst8, { 5, 6, 7 }));
	EXPECT_TRUE(lst6.empty()); // moved-from is valid but unspecified
}

TEST(ListTest, Assignment)
{
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 4, 5 };

	// Copy assignment
	lst1 = lst2;
	EXPECT_TRUE(equal(lst1, { 4, 5 }));
	EXPECT_EQ(lst1.size(), 2);

	// Move assignment
	raw::list<int> lst3 = { 6, 7, 8 };
	lst1 = std::move(lst3);
	EXPECT_TRUE(equal(lst1, { 6, 7, 8 }));
	EXPECT_TRUE(lst3.empty());

	// Initializer list assignment
	lst1 = { 9, 10 };
	EXPECT_TRUE(equal(lst1, { 9, 10 }));

	// Self assignment
	lst1 = lst1;
	EXPECT_TRUE(equal(lst1, { 9, 10 }));

	// assign(size, value)
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
	raw::list<int> lst = { 10, 20, 30, 40, 50 };

	// front / back
	EXPECT_EQ(lst.front(), 10);
	EXPECT_EQ(lst.back(), 50);
	lst.front() = 1;
	lst.back() = 100;
	EXPECT_EQ(lst.front(), 1);
	EXPECT_EQ(lst.back(), 100);
}

TEST(ListTest, Iterators)
{
	raw::list<int> lst = { 1, 2, 3, 4, 5 };

	// Forward iteration
	int sum = 0;
	for (auto it = lst.begin(); it != lst.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 15);

	// Reverse iteration
	int rsum = 0;
	for (auto it = lst.rbegin(); it != lst.rend(); ++it)
	{
		rsum += *it;
	}
	EXPECT_EQ(rsum, 15);

	// Const iterators
	const raw::list<int>& clst = lst;
	int csum = 0;
	for (auto it = clst.cbegin(); it != clst.cend(); ++it)
	{
		csum += *it;
	}
	EXPECT_EQ(csum, 15);

	// Empty list iterators
	raw::list<int> empty;
	EXPECT_EQ(empty.begin(), empty.end());
}

TEST(ListTest, Capacity)
{
	raw::list<int> lst;
	EXPECT_TRUE(lst.empty());
	EXPECT_EQ(lst.size(), 0);
	EXPECT_GT(lst.max_size(), 1000000u);

	lst.push_back(1);
	EXPECT_FALSE(lst.empty());
	EXPECT_EQ(lst.size(), 1);
}

TEST(ListTest, Modifiers)
{
	// push_back / pop_back
	raw::list<int> lst;
	lst.push_back(1);
	lst.push_back(2);
	lst.emplace_back(3);
	EXPECT_TRUE(equal(lst, { 1, 2, 3 }));
	lst.pop_back();
	EXPECT_TRUE(equal(lst, { 1, 2 }));
	lst.pop_back();
	lst.pop_back();
	EXPECT_TRUE(lst.empty());

	// push_front / pop_front
	lst.push_front(1);
	lst.push_front(2);
	lst.emplace_front(3);
	EXPECT_TRUE(equal(lst, { 3, 2, 1 }));
	lst.pop_front();
	EXPECT_TRUE(equal(lst, { 2, 1 }));
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

	// erase single
	lst.erase(std::next(lst.begin())); // removes 99
	EXPECT_TRUE(equal(lst, { 1, 2, 3 }));

	// erase range
	lst.erase(std::next(lst.begin()), std::prev(lst.end())); // removes 2
	EXPECT_TRUE(equal(lst, { 1, 3 }));

	// resize (default)
	lst.resize(5);
	EXPECT_EQ(lst.size(), 5);
	EXPECT_TRUE(equal(lst, { 1, 3, 0, 0, 0 }));
	lst.resize(2);
	EXPECT_TRUE(equal(lst, { 1, 3 }));

	// resize (with value)
	lst.resize(4, 42);
	EXPECT_TRUE(equal(lst, { 1, 3, 42, 42 }));
}

TEST(ListTest, Operations)
{
	// merge
	raw::list<int> lst1 = { 1, 3, 5 };
	raw::list<int> lst2 = { 2, 4, 6 };
	lst1.merge(lst2);
	EXPECT_TRUE(equal(lst1, { 1, 2, 3, 4, 5, 6 }));
	EXPECT_TRUE(lst2.empty());

	lst1 = { 6, 4, 2 };
	lst2 = { 5, 3, 1 };
	lst1.merge(lst2, std::greater<int>{});
	EXPECT_TRUE(equal(lst1, { 6, 5, 4, 3, 2, 1 }));

	// splice (whole list)
	lst1 = { 1, 2, 3 };
	lst2 = { 4, 5, 6 };
	lst1.splice(std::next(lst1.begin()), lst2);
	EXPECT_TRUE(equal(lst1, { 1, 4, 5, 6, 2, 3 }));
	EXPECT_TRUE(lst2.empty());

	// splice (single element)
	lst1 = { 1, 2, 3 };
	lst2 = { 4, 5, 6 };
	lst1.splice(lst1.end(), lst2, std::next(lst2.begin()));
	EXPECT_TRUE(equal(lst1, { 1, 2, 3, 5 }));
	EXPECT_TRUE(equal(lst2, { 4, 6 }));

	// splice (range)
	lst1 = { 1, 2, 3 };
	lst2 = { 4, 5, 6, 7 };
	lst1.splice(std::next(lst1.begin()), lst2, std::next(lst2.begin()), std::prev(lst2.end()));
	EXPECT_TRUE(equal(lst1, { 1, 5, 6, 2, 3 }));
	EXPECT_TRUE(equal(lst2, { 4, 7 }));

	// splice (self)
	lst1 = { 1, 2, 3, 4 };
	lst1.splice(lst1.end(), lst1, lst1.begin());
	EXPECT_TRUE(equal(lst1, { 2, 3, 4, 1 }));

	// remove
	lst1 = { 1, 2, 3, 2, 4, 2, 5 };
	auto cnt = lst1.remove(2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst1, { 1, 3, 4, 5 }));

	// remove_if
	lst1 = { 1, 2, 3, 4, 5, 6 };
	cnt = lst1.remove_if([](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst1, { 1, 3, 5 }));

	// reverse
	lst1 = { 1, 2, 3, 4, 5 };
	lst1.reverse();
	EXPECT_TRUE(equal(lst1, { 5, 4, 3, 2, 1 }));

	// unique
	lst1 = { 1, 1, 2, 2, 2, 3, 3, 1, 1 };
	cnt = lst1.unique();
	EXPECT_EQ(cnt, 5);
	EXPECT_TRUE(equal(lst1, { 1, 2, 3, 1 }));

	// sort (ascending)
	lst1 = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
	lst1.sort();
	EXPECT_TRUE(equal(lst1, { 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 }));

	// sort (descending)
	lst1.sort(std::greater<int>{});
	EXPECT_TRUE(equal(lst1, { 9, 6, 5, 5, 5, 4, 3, 3, 2, 1, 1 }));
}

TEST(ListTest, Comparisons)
{
	raw::list<int> lst1 = { 1, 2, 3 };
	raw::list<int> lst2 = { 1, 2, 3 };
	raw::list<int> lst3 = { 1, 2, 4 };
	raw::list<int> lst4 = { 1, 2 };

	EXPECT_TRUE(lst1 == lst2);
	EXPECT_FALSE(lst1 != lst2);
	EXPECT_TRUE(lst1 != lst3);
	EXPECT_TRUE(lst1 < lst3);
	EXPECT_TRUE(lst3 > lst1);
	EXPECT_TRUE(lst1 <= lst2);
	EXPECT_TRUE(lst3 >= lst1);
	EXPECT_TRUE(lst4 < lst1);
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
	raw::list<int> lst = { 1, 2, 3, 2, 4, 2, 5 };
	auto cnt = erase(lst, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst, { 1, 3, 4, 5 }));

	lst = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(lst, [](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(lst, { 1, 3, 5 }));
}
