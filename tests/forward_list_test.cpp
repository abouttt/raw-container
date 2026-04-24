#include <gtest/gtest.h>
#include <raw/forward_list.h>

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

template <typename T>
bool equal(const raw::forward_list<T>& flist, std::initializer_list<T> ilist)
{
	auto it = flist.begin();
	auto il = ilist.begin();

	while (it != flist.end() && il != ilist.end())
	{
		if (*it != *il)
		{
			return false;
		}

		++it;
		++il;
	}

	return it == flist.end() && il == ilist.end();
}

TEST(ForwardListTest, Constructors)
{
	// Default constructor
	raw::forward_list<int> flist1;
	EXPECT_TRUE(flist1.empty());
	EXPECT_EQ(flist1.begin(), flist1.end());

	// Size constructor (value-initialization)
	raw::forward_list<int> flist2(5);
	int count = 0;
	for (auto it = flist2.begin(); it != flist2.end(); ++it)
	{
		++count;
	}

	EXPECT_EQ(count, 5);
	for (int x : flist2)
	{
		EXPECT_EQ(x, 0);
	}

	// Size + value constructor
	raw::forward_list<int> flist3(3, 42);
	EXPECT_TRUE(equal(flist3, { 42, 42, 42 }));

	// Range constructor (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::forward_list<int> flist4(src.begin(), src.end());
	EXPECT_TRUE(equal(flist4, { 1, 2, 3, 4 }));

	// Range constructor (input iterator)
	std::istringstream iss("10 20 30");
	std::istream_iterator<int> iit(iss), iend;
	raw::forward_list<int> flist5(iit, iend);
	EXPECT_TRUE(equal(flist5, { 10, 20, 30 }));

	// Initializer list constructor
	raw::forward_list<int> flist6 = { 5, 6, 7 };
	EXPECT_TRUE(equal(flist6, { 5, 6, 7 }));

	// Copy constructor
	raw::forward_list<int> flist7(flist6);
	EXPECT_TRUE(equal(flist7, { 5, 6, 7 }));

	// Move constructor
	raw::forward_list<int> flist8(std::move(flist6));
	EXPECT_TRUE(equal(flist8, { 5, 6, 7 }));
	EXPECT_TRUE(flist6.empty()); // moved-from is valid but unspecified
}

TEST(ForwardListTest, Assignment)
{
	raw::forward_list<int> flist1 = { 1, 2, 3 };
	raw::forward_list<int> flist2 = { 4, 5 };

	// Copy assignment
	flist1 = flist2;
	EXPECT_TRUE(equal(flist1, { 4, 5 }));

	// Move assignment
	raw::forward_list<int> flist3 = { 6, 7, 8 };
	flist1 = std::move(flist3);
	EXPECT_TRUE(equal(flist1, { 6, 7, 8 }));
	EXPECT_TRUE(flist3.empty());

	// Initializer list assignment
	flist1 = { 9, 10 };
	EXPECT_TRUE(equal(flist1, { 9, 10 }));

	// Self assignment
	flist1 = flist1;
	EXPECT_TRUE(equal(flist1, { 9, 10 }));

	// assign(size, value)
	flist1.assign(3, 42);
	EXPECT_TRUE(equal(flist1, { 42, 42, 42 }));

	// assign(range)
	std::vector<int> vec = { 1, 2, 3 };
	flist1.assign(vec.begin(), vec.end());
	EXPECT_TRUE(equal(flist1, { 1, 2, 3 }));

	// assign(initializer_list)
	flist1.assign({ 7, 8, 9 });
	EXPECT_TRUE(equal(flist1, { 7, 8, 9 }));
}

TEST(ForwardListTest, ElementAccess)
{
	raw::forward_list<int> flist = { 10, 20, 30, 40, 50 };

	// front
	EXPECT_EQ(flist.front(), 10);
	flist.front() = 1;
	EXPECT_EQ(flist.front(), 1);
}

TEST(ForwardListTest, Iterators)
{
	raw::forward_list<int> flist = { 1, 2, 3, 4, 5 };

	// Forward iteration
	int sum = 0;
	for (auto it = flist.begin(); it != flist.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 15);

	// before_begin
	auto it_before = flist.before_begin();
	EXPECT_EQ(++it_before, flist.begin());

	// Const iterators
	const raw::forward_list<int>& cflist = flist;
	int csum = 0;
	for (auto it = cflist.cbegin(); it != cflist.cend(); ++it)
	{
		csum += *it;
	}
	EXPECT_EQ(csum, 15);

	// Empty list iterators
	raw::forward_list<int> empty;
	EXPECT_EQ(empty.begin(), empty.end());
}

TEST(ForwardListTest, Capacity)
{
	raw::forward_list<int> flist;
	EXPECT_TRUE(flist.empty());
	EXPECT_GT(flist.max_size(), 1000000u);

	flist.push_front(1);
	EXPECT_FALSE(flist.empty());
}

TEST(ForwardListTest, Modifiers)
{
	// push_front / pop_front
	raw::forward_list<int> flist;
	flist.push_front(1);
	flist.push_front(2);
	flist.emplace_front(3);
	EXPECT_TRUE(equal(flist, { 3, 2, 1 }));
	flist.pop_front();
	EXPECT_TRUE(equal(flist, { 2, 1 }));
	flist.pop_front();
	flist.pop_front();
	EXPECT_TRUE(flist.empty());

	// clear
	flist = { 5, 6, 7 };
	flist.clear();
	EXPECT_TRUE(flist.empty());

	// insert_after single element (lvalue)
	flist = { 1, 2, 3 };
	auto it = flist.insert_after(flist.before_begin(), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(flist, { 99, 1, 2, 3 }));

	// insert_after rvalue
	it = flist.insert_after(flist.begin(), 100);
	EXPECT_EQ(*it, 100);
	EXPECT_TRUE(equal(flist, { 99, 100, 1, 2, 3 }));

	// insert_after count copies
	flist = { 1, 2, 3 };
	it = flist.insert_after(flist.before_begin(), 2, 42);
	EXPECT_EQ(*it, 42); // last inserted
	EXPECT_TRUE(equal(flist, { 42, 42, 1, 2, 3 }));

	// insert_after range (forward iterator)
	flist = { 1, 2, 3 };
	std::vector<int> src = { 10, 20 };
	it = flist.insert_after(flist.before_begin(), src.begin(), src.end());
	EXPECT_EQ(*it, 20);
	EXPECT_TRUE(equal(flist, { 10, 20, 1, 2, 3 }));

	// insert_after range (input iterator)
	std::istringstream iss("100 200");
	std::istream_iterator<int> iit(iss), iend;
	it = flist.insert_after(flist.before_begin(), iit, iend);
	EXPECT_EQ(*it, 200);
	EXPECT_TRUE(equal(flist, { 100, 200, 10, 20, 1, 2, 3 }));

	// insert_after initializer_list
	flist = { 1, 2, 3 };
	it = flist.insert_after(flist.before_begin(), { 4, 5 });
	EXPECT_EQ(*it, 5);
	EXPECT_TRUE(equal(flist, { 4, 5, 1, 2, 3 }));

	// emplace_after
	flist = { 1, 2, 3 };
	it = flist.emplace_after(flist.before_begin(), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(flist, { 99, 1, 2, 3 }));

	// erase_after single
	flist.erase_after(flist.before_begin()); // removes 99
	EXPECT_TRUE(equal(flist, { 1, 2, 3 }));

	// erase_after range
	auto first = flist.before_begin();
	auto last = std::next(flist.begin(), 2); // points to 3
	flist.erase_after(first, last); // removes 1 and 2
	EXPECT_TRUE(equal(flist, { 3 }));

	// resize (default)
	flist = { 1, 3 };
	flist.resize(5);
	EXPECT_TRUE(equal(flist, { 1, 3, 0, 0, 0 }));
	flist.resize(2);
	EXPECT_TRUE(equal(flist, { 1, 3 }));

	// resize (with value)
	flist.resize(4, 42);
	EXPECT_TRUE(equal(flist, { 1, 3, 42, 42 }));
}

TEST(ForwardListTest, Operations)
{
	// merge
	raw::forward_list<int> flist1 = { 1, 3, 5 };
	raw::forward_list<int> flist2 = { 2, 4, 6 };
	flist1.merge(flist2);
	EXPECT_TRUE(equal(flist1, { 1, 2, 3, 4, 5, 6 }));
	EXPECT_TRUE(flist2.empty());

	flist1 = { 6, 4, 2 };
	flist2 = { 5, 3, 1 };
	flist1.merge(flist2, std::greater<int>{});
	EXPECT_TRUE(equal(flist1, { 6, 5, 4, 3, 2, 1 }));

	// splice_after (whole list)
	flist1 = { 1, 2, 3 };
	flist2 = { 4, 5, 6 };
	flist1.splice_after(flist1.before_begin(), flist2);
	EXPECT_TRUE(equal(flist1, { 4, 5, 6, 1, 2, 3 }));
	EXPECT_TRUE(flist2.empty());

	// splice_after (single element)
	flist1 = { 1, 2, 3 };
	flist2 = { 4, 5, 6 };
	flist1.splice_after(flist1.before_begin(), flist2, flist2.before_begin()); // move 4
	EXPECT_TRUE(equal(flist1, { 4, 1, 2, 3 }));
	EXPECT_TRUE(equal(flist2, { 5, 6 }));

	// splice_after (range)
	flist1 = { 1, 2, 3 };
	flist2 = { 4, 5, 6, 7 };
	auto first2 = flist2.before_begin();
	auto last2 = std::next(flist2.begin(), 2); // after moving 4,5 -> last is node before 6
	flist1.splice_after(flist1.before_begin(), flist2, first2, last2);
	EXPECT_TRUE(equal(flist1, { 4, 5, 1, 2, 3 }));
	EXPECT_TRUE(equal(flist2, { 6, 7 }));

	// splice_after (self)
	flist1 = { 1, 2, 3, 4 };
	// move first element to front (no effect)
	flist1.splice_after(flist1.before_begin(), flist1, flist1.before_begin());
	EXPECT_TRUE(equal(flist1, { 1, 2, 3, 4 }));

	// remove
	flist1 = { 1, 2, 3, 2, 4, 2, 5 };
	auto cnt = flist1.remove(2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(flist1, { 1, 3, 4, 5 }));

	// remove_if
	flist1 = { 1, 2, 3, 4, 5, 6 };
	cnt = flist1.remove_if([](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(flist1, { 1, 3, 5 }));

	// reverse
	flist1 = { 1, 2, 3, 4, 5 };
	flist1.reverse();
	EXPECT_TRUE(equal(flist1, { 5, 4, 3, 2, 1 }));

	// unique
	flist1 = { 1, 1, 2, 2, 2, 3, 3, 1, 1 };
	cnt = flist1.unique();
	EXPECT_EQ(cnt, 5);
	EXPECT_TRUE(equal(flist1, { 1, 2, 3, 1 }));

	// sort (ascending)
	flist1 = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
	flist1.sort();
	EXPECT_TRUE(equal(flist1, { 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 }));

	// sort (descending)
	flist1.sort(std::greater<int>{});
	EXPECT_TRUE(equal(flist1, { 9, 6, 5, 5, 5, 4, 3, 3, 2, 1, 1 }));
}

TEST(ForwardListTest, Comparisons)
{
	raw::forward_list<int> flist1 = { 1, 2, 3 };
	raw::forward_list<int> flist2 = { 1, 2, 3 };
	raw::forward_list<int> flist3 = { 1, 2, 4 };
	raw::forward_list<int> flist4 = { 1, 2 };

	EXPECT_TRUE(flist1 == flist2);
	EXPECT_FALSE(flist1 != flist2);
	EXPECT_TRUE(flist1 != flist3);
	EXPECT_TRUE(flist1 < flist3);
	EXPECT_TRUE(flist3 > flist1);
	EXPECT_TRUE(flist1 <= flist2);
	EXPECT_TRUE(flist3 >= flist1);
	EXPECT_TRUE(flist4 < flist1);
}

TEST(ForwardListTest, NonMemberSwap)
{
	raw::forward_list<int> flist1 = { 1, 2, 3 };
	raw::forward_list<int> flist2 = { 4, 5, 6 };
	swap(flist1, flist2);
	EXPECT_TRUE(equal(flist1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(flist2, { 1, 2, 3 }));
}

TEST(ForwardListTest, NonMemberErase)
{
	raw::forward_list<int> flist = { 1, 2, 3, 2, 4, 2, 5 };
	auto cnt = erase(flist, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(flist, { 1, 3, 4, 5 }));

	flist = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(flist, [](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(flist, { 1, 3, 5 }));
}
