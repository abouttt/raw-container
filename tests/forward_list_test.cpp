#include <algorithm>
#include <compare>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include <raw/forward_list.h>

template <typename T>
bool equal(const raw::forward_list<T>& fl, std::initializer_list<T> ilist)
{
	return std::equal(fl.begin(), fl.end(), ilist.begin(), ilist.end());
}

TEST(ForwardListTest, Constructors)
{
	// Default construction
	raw::forward_list<int> fl1;
	EXPECT_TRUE(fl1.empty());

	// Size construction (value-initialization)
	raw::forward_list<int> fl2(5);
	int count = 0;
	for (int x : fl2)
	{
		EXPECT_EQ(x, 0);
		++count;
	}
	EXPECT_EQ(count, 5);

	// Size + value construction
	raw::forward_list<int> fl3(3, 42);
	EXPECT_TRUE(equal(fl3, { 42, 42, 42 }));

	// Range construction (forward iterator)
	std::vector<int> src = { 1, 2, 3, 4 };
	raw::forward_list<int> fl4(src.begin(), src.end());
	EXPECT_TRUE(equal(fl4, { 1, 2, 3, 4 }));

	// Range construction (input iterator)
	std::istringstream iss("100 200 300");
	std::istream_iterator<int> iit(iss), iend;
	raw::forward_list<int> fl5(iit, iend);
	EXPECT_TRUE(equal(fl5, { 100, 200, 300 }));

	// Initializer-list construction
	raw::forward_list<int> fl6 = { 5, 6, 7 };
	EXPECT_TRUE(equal(fl6, { 5, 6, 7 }));

	// Copy construction
	raw::forward_list<int> fl7(fl6);
	EXPECT_TRUE(equal(fl7, { 5, 6, 7 }));

	// Move construction
	raw::forward_list<int> fl8(std::move(fl7));
	EXPECT_TRUE(equal(fl8, { 5, 6, 7 }));
	EXPECT_TRUE(fl7.empty());
}

TEST(ForwardListTest, Assignment)
{
	raw::forward_list<int> fl1 = { 1, 2, 3 };
	raw::forward_list<int> fl2 = { 4, 5 };

	// Copy assignment
	fl1 = fl2;
	EXPECT_TRUE(equal(fl1, { 4, 5 }));
	EXPECT_TRUE(equal(fl2, { 4, 5 }));

	// Move assignment
	raw::forward_list<int> fl3 = { 6, 7, 8 };
	fl1 = std::move(fl3);
	EXPECT_TRUE(equal(fl1, { 6, 7, 8 }));
	EXPECT_TRUE(fl3.empty());

	// Initializer-list assignment
	fl1 = { 9, 10 };
	EXPECT_TRUE(equal(fl1, { 9, 10 }));

	// Self-assignment
	fl1 = fl1;
	EXPECT_TRUE(equal(fl1, { 9, 10 }));

	// assign(count, value)
	fl1.assign(3, 42);
	EXPECT_TRUE(equal(fl1, { 42, 42, 42 }));

	// assign(range)
	std::vector<int> vec = { 1, 2, 3 };
	fl1.assign(vec.begin(), vec.end());
	EXPECT_TRUE(equal(fl1, { 1, 2, 3 }));

	// assign(initializer_list)
	fl1.assign({ 7, 8, 9 });
	EXPECT_TRUE(equal(fl1, { 7, 8, 9 }));
}

TEST(ForwardListTest, ElementAccess)
{
	raw::forward_list<int> fl = { 1, 2, 3, 4 };

	// front
	EXPECT_EQ(fl.front(), 1);
	fl.front() = 10;
	EXPECT_EQ(fl.front(), 10);

	// const versions
	const auto& cfl = fl;
	EXPECT_EQ(cfl.front(), 10);

	// Empty forward_list (UB must be avoided when testing)
	raw::forward_list<int> empty;
	EXPECT_TRUE(empty.empty());
}

TEST(ForwardListTest, Iterators)
{
	raw::forward_list<int> fl = { 1, 2, 3 };

	// Forward iteration
	int sum = 0;
	for (auto it = fl.begin(); it != fl.end(); ++it)
	{
		sum += *it;
	}
	EXPECT_EQ(sum, 6);

	// before_begin / begin / end
	auto it_bb = fl.before_begin();
	++it_bb;
	EXPECT_EQ(*it_bb, 1);
	EXPECT_EQ(fl.begin(), it_bb);
	EXPECT_EQ(fl.end(), ++(++(++fl.begin())));

	// Const iterators
	const auto& cfl = fl;
	EXPECT_EQ(*cfl.cbegin(), 1);
	auto it_cbb = cfl.cbefore_begin();
	++it_cbb;
	EXPECT_EQ(it_cbb, cfl.begin());
	EXPECT_EQ(cfl.cend(), ++(++(++cfl.cbegin())));

	// Comparison between mutable and const iterators
	EXPECT_EQ(fl.before_begin(), cfl.cbefore_begin());
	EXPECT_EQ(fl.begin(), cfl.begin());
	EXPECT_EQ(fl.end(), cfl.end());
}

TEST(ForwardListTest, Capacity)
{
	raw::forward_list<int> fl;
	EXPECT_TRUE(fl.empty());

	fl.push_front(1);
	EXPECT_FALSE(fl.empty());

	// max_size is huge (typically)
	EXPECT_GT(fl.max_size(), 1000000u);
}

TEST(ForwardListTest, Modifiers)
{
	raw::forward_list<int> fl;

	// push_front / pop_front
	fl.push_front(10);
	fl.push_front(20);
	fl.push_front(30);
	EXPECT_TRUE(equal(fl, { 30, 20, 10 }));
	fl.pop_front();
	EXPECT_TRUE(equal(fl, { 20, 10 }));
	fl.pop_front();
	fl.pop_front();
	EXPECT_TRUE(fl.empty());

	// clear
	fl = { 5, 6, 7 };
	fl.clear();
	EXPECT_TRUE(fl.empty());

	// insert_after single element (lvalue)
	fl = { 1, 2, 3 };
	auto it = fl.insert_after(fl.begin(), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(fl, { 1, 99, 2, 3 }));

	// insert_after rvalue
	fl.insert_after(fl.before_begin(), 100);
	EXPECT_TRUE(equal(fl, { 100, 1, 99, 2, 3 }));

	// insert_after count copies
	fl = { 1, 2, 3 };
	fl.insert_after(fl.begin(), 2, 42);
	EXPECT_TRUE(equal(fl, { 1, 42, 42, 2, 3 }));

	// insert_after range (forward iterator)
	fl = { 1, 2, 3 };
	std::vector<int> src = { 10, 20 };
	fl.insert_after(std::next(fl.begin()), src.begin(), src.end());
	EXPECT_TRUE(equal(fl, { 1, 2, 10, 20, 3 }));

	// insert_after range (input iterator)
	std::istringstream iss("100 200");
	std::istream_iterator<int> iit(iss), iend;
	fl.insert_after(fl.before_begin(), iit, iend);
	EXPECT_TRUE(equal(fl, { 100, 200, 1, 2, 10, 20, 3 }));

	// insert_after initializer_list
	fl = { 1, 2, 3 };
	fl.insert_after(fl.begin(), { 4, 5 });
	EXPECT_TRUE(equal(fl, { 1, 4, 5, 2, 3 }));

	// emplace_after
	fl = { 1, 2, 3 };
	it = fl.emplace_after(fl.begin(), 99);
	EXPECT_EQ(*it, 99);
	EXPECT_TRUE(equal(fl, { 1, 99, 2, 3 }));

	// emplace_front
	fl.emplace_front(100);
	EXPECT_TRUE(equal(fl, { 100, 1, 99, 2, 3 }));

	// erase_after single
	fl.erase_after(fl.begin());
	EXPECT_TRUE(equal(fl, { 100, 99, 2, 3 }));

	// erase_after range
	fl.erase_after(fl.begin(), std::next(fl.begin(), 3));
	EXPECT_TRUE(equal(fl, { 100, 3 }));

	// resize (default)
	fl.resize(5);
	int cnt = 0;
	for (auto it = fl.begin(); it != fl.end(); ++it)
	{
		++cnt;
	}
	EXPECT_EQ(cnt, 5);
	auto it_end = fl.begin();
	std::advance(it_end, 2);
	EXPECT_EQ(*it_end, 0);
	fl.resize(2);
	EXPECT_TRUE(equal(fl, { 100, 3 }));

	// resize (with value)
	fl.resize(4, 42);
	EXPECT_TRUE(equal(fl, { 100, 3, 42, 42 }));

	// swap
	raw::forward_list<int> other = { 7, 8 };
	fl.swap(other);
	EXPECT_TRUE(equal(fl, { 7, 8 }));
	EXPECT_TRUE(equal(other, { 100, 3, 42, 42 }));

	// self-swap (should be safe)
	fl.swap(fl);
	EXPECT_TRUE(equal(fl, { 7, 8 }));
}

TEST(ForwardListTest, Operations)
{
	// splice_after
	raw::forward_list<int> fl1 = { 1, 2, 3 };
	raw::forward_list<int> fl2 = { 10, 20, 30 };

	fl1.splice_after(fl1.before_begin(), fl2);
	EXPECT_TRUE(equal(fl1, { 10, 20, 30, 1, 2, 3 }));
	EXPECT_TRUE(fl2.empty());

	// splice_after single element
	fl2 = { 100, 200 };
	fl1.splice_after(fl1.begin(), fl2, fl2.before_begin());
	EXPECT_TRUE(equal(fl1, { 10, 100, 20, 30, 1, 2, 3 }));
	EXPECT_TRUE(equal(fl2, { 200 }));

	// splice_after range
	raw::forward_list<int> fl3 = { 50, 60, 70, 80 };
	auto it_first = fl3.begin();
	auto it_last = std::next(it_first, 3);
	fl1.splice_after(fl1.before_begin(), fl3, it_first, it_last);
	EXPECT_TRUE(equal(fl1, { 60, 70, 10, 100, 20, 30, 1, 2, 3 }));
	EXPECT_TRUE(equal(fl3, { 50, 80 }));

	// self-splice_after single element (no-op)
	auto pos_self = fl1.begin();
	fl1.splice_after(pos_self, fl1, fl1.begin());
	EXPECT_TRUE(equal(fl1, { 60, 70, 10, 100, 20, 30, 1, 2, 3 }));

	// merge
	raw::forward_list<int> sorted1 = { 1, 3, 5, 7 };
	raw::forward_list<int> sorted2 = { 2, 4, 6, 8 };

	sorted1.merge(sorted2);
	EXPECT_TRUE(equal(sorted1, { 1, 2, 3, 4, 5, 6, 7, 8 }));
	EXPECT_TRUE(sorted2.empty());

	// merge with comparator (descending)
	raw::forward_list<int> sorted3 = { 10, 5, 3 };
	raw::forward_list<int> sorted4 = { 9, 7, 1 };
	sorted3.merge(sorted4, std::greater<>{});
	EXPECT_TRUE(equal(sorted3, { 10, 9, 7, 5, 3, 1 }));
	EXPECT_TRUE(sorted4.empty());

	// merge into empty
	raw::forward_list<int> empty;
	empty.merge(sorted1);
	EXPECT_TRUE(equal(empty, { 1, 2, 3, 4, 5, 6, 7, 8 }));
	EXPECT_TRUE(sorted1.empty());

	// remove / remove_if
	raw::forward_list<int> fl4 = { 1, 2, 3, 2, 4, 2, 5 };
	auto removed = fl4.remove(2);
	EXPECT_EQ(removed, 3);
	EXPECT_TRUE(equal(fl4, { 1, 3, 4, 5 }));

	removed = fl4.remove_if([](int x) { return x % 2 != 0; });
	EXPECT_EQ(removed, 3);
	EXPECT_TRUE(equal(fl4, { 4 }));

	// reverse
	raw::forward_list<int> fl5 = { 1, 2, 3, 4, 5 };
	fl5.reverse();
	EXPECT_TRUE(equal(fl5, { 5, 4, 3, 2, 1 }));

	// empty list reverse
	raw::forward_list<int> empty2;
	empty2.reverse();
	EXPECT_TRUE(empty2.empty());

	// single element reverse
	raw::forward_list<int> single = { 42 };
	single.reverse();
	EXPECT_TRUE(equal(single, { 42 }));

	// unique
	raw::forward_list<int> fl6 = { 1, 1, 2, 3, 3, 3, 4, 5, 5 };
	removed = fl6.unique();
	EXPECT_EQ(removed, 4);
	EXPECT_TRUE(equal(fl6, { 1, 2, 3, 4, 5 }));

	// empty list unique
	raw::forward_list<int> empty3;
	removed = empty3.unique();
	EXPECT_EQ(removed, 0);

	// sort
	raw::forward_list<int> fl7 = { 3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5 };
	fl7.sort();
	EXPECT_TRUE(equal(fl7, { 1, 1, 2, 3, 3, 4, 5, 5, 5, 6, 9 }));

	// sort with comparator (descending)
	fl7 = { 3, 1, 4, 1, 5 };
	fl7.sort(std::greater<>{});
	EXPECT_TRUE(equal(fl7, { 5, 4, 3, 1, 1 }));

	// empty list sort
	raw::forward_list<int> empty4;
	empty4.sort();
	EXPECT_TRUE(empty4.empty());

	// single element sort
	single = { 42 };
	single.sort();
	EXPECT_TRUE(equal(single, { 42 }));
}

TEST(ForwardListTest, Comparisons)
{
	raw::forward_list<int> fl1 = { 1, 2, 3 };
	raw::forward_list<int> fl2 = { 1, 2, 3 };
	raw::forward_list<int> fl3 = { 7, 8, 9 };

	// Equality
	EXPECT_TRUE(fl1 == fl2);
	EXPECT_FALSE(fl1 != fl2);

	// Relational
	EXPECT_FALSE(fl1 < fl2);
	EXPECT_TRUE(fl1 <= fl2);
	EXPECT_FALSE(fl1 > fl2);
	EXPECT_TRUE(fl1 >= fl2);

	// Three-way comparison (strong ordering)
	EXPECT_EQ(fl1 <=> fl2, std::strong_ordering::equal);
	EXPECT_EQ(fl1 <=> fl3, std::strong_ordering::less);
	EXPECT_EQ(fl3 <=> fl1, std::strong_ordering::greater);
}

TEST(ForwardListTest, NonMemberSwap)
{
	raw::forward_list<int> fl1 = { 1, 2, 3 };
	raw::forward_list<int> fl2 = { 4, 5, 6 };
	swap(fl1, fl2);
	EXPECT_TRUE(equal(fl1, { 4, 5, 6 }));
	EXPECT_TRUE(equal(fl2, { 1, 2, 3 }));
}

TEST(ForwardListTest, NonMemberErase)
{
	raw::forward_list<int> fl = { 1, 2, 3, 2, 4, 2 };
	auto cnt = erase(fl, 2);
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(fl, { 1, 3, 4 }));

	fl = { 1, 2, 3, 4, 5, 6 };
	cnt = erase_if(fl, [](int x) { return x % 2 == 0; });
	EXPECT_EQ(cnt, 3);
	EXPECT_TRUE(equal(fl, { 1, 3, 5 }));
}

TEST(ForwardListTest, MoveOnly)
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

	raw::forward_list<MoveOnly> fl;
	fl.emplace_front(1);
	fl.push_front(MoveOnly(2));
	EXPECT_EQ(fl.front().val, 2);
	int count = 0;
	for (auto it = fl.begin(); it != fl.end(); ++it)
	{
		++count;
	}
	EXPECT_EQ(count, 2);

	fl.emplace_front(3);
	EXPECT_EQ(fl.front().val, 3);
	count = 0;
	for (auto it = fl.begin(); it != fl.end(); ++it)
	{
		++count;
	}
	EXPECT_EQ(count, 3);
}
