#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

#include "detail/assert.h"
#include "detail/compare.h"
#include "detail/memory.h"

namespace raw
{

template <typename T>
class forward_list;

namespace detail
{

struct flist_node_base
{
	flist_node_base* next;
};

template <typename T>
struct flist_node : flist_node_base
{
	T value;

	template <typename... Args>
	explicit flist_node(Args&&... args)
		: flist_node_base{ nullptr }
		, value(std::forward<Args>(args)...)
	{
	}
};

template <typename T>
class flist_iterator
{
private:
	using node_base = flist_node_base;
	using node = flist_node<T>;

public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = std::remove_cv_t<T>;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	flist_iterator() noexcept
		: m_node(nullptr)
	{
	}

	explicit flist_iterator(node_base* node) noexcept
		: m_node(node)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	flist_iterator(const flist_iterator<U>& other) noexcept
		: m_node(other.m_node)
	{
	}

	[[nodiscard]] reference operator*() const noexcept
	{
		return static_cast<node*>(m_node)->value;
	}

	[[nodiscard]] pointer operator->() const noexcept
	{
		return &static_cast<node*>(m_node)->value;
	}

	flist_iterator& operator++() noexcept
	{
		m_node = m_node->next;
		return *this;
	}

	flist_iterator operator++(int) noexcept
	{
		flist_iterator tmp = *this;
		++*this;
		return tmp;
	}

	template <typename U>
	[[nodiscard]] bool operator==(const flist_iterator<U>& other) const noexcept
	{
		return m_node == other.m_node;
	}

private:
	template <typename>
	friend class flist_iterator;
	friend class forward_list<std::remove_cv_t<T>>;

	node_base* m_node;
};

} // namespace detail

template <typename T>
class forward_list
{
private:
	using node_base = detail::flist_node_base;
	using node = detail::flist_node<T>;

public:
	static_assert(std::is_object_v<T>, "T must be an object type");

	// ---------- Types ---------- //

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using iterator = detail::flist_iterator<T>;
	using const_iterator = detail::flist_iterator<const T>;

	// ---------- Constructors / Destructor ---------- //

	forward_list() noexcept
		: m_head{ nullptr }
	{
	}

	explicit forward_list(size_type count)
		: forward_list()
	{
		initialize_storage(count);
	}

	forward_list(size_type count, const value_type& value)
		: forward_list()
	{
		initialize_storage(count, value);
	}

	template <std::input_iterator InputIt>
	forward_list(InputIt first, InputIt last)
		: forward_list()
	{
		initialize_storage(first, last);
	}

	forward_list(std::initializer_list<value_type> init)
		: forward_list(init.begin(), init.end())
	{
	}

	forward_list(const forward_list& other)
		: forward_list(other.begin(), other.end())
	{
	}

	forward_list(forward_list&& other) noexcept
		: forward_list()
	{
		swap(other);
	}

	~forward_list() noexcept
	{
		reset_storage();
	}

	// ---------- Assignment ---------- //

	forward_list& operator=(const forward_list& other)
	{
		if (this != std::addressof(other))
		{
			assign(other.begin(), other.end());
		}

		return *this;
	}

	forward_list& operator=(forward_list&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			reset_storage();
			swap(other);
		}

		return *this;
	}

	forward_list& operator=(std::initializer_list<value_type> ilist)
	{
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	void assign(size_type count, const value_type& value)
	{
		node_base* prev = &m_head;
		node_base* cur = m_head.next;
		size_type assigned = 0;

		while (cur && assigned < count)
		{
			static_cast<node*>(cur)->value = value;
			prev = cur;
			cur = cur->next;
			++assigned;
		}

		if (assigned < count)
		{
			auto [head, tail] = create_node_chain(count - assigned, value);
			link_node_chain_after(prev, head, tail);
		}
		else if (cur)
		{
			auto [head, tail] = unlink_node_chain_after(prev, nullptr);
			destroy_node_chain(static_cast<node*>(head));
		}
	}

	template <std::input_iterator InputIt>
	void assign(InputIt first, InputIt last)
	{
		node_base* prev = &m_head;
		node_base* cur = m_head.next;

		while (cur && first != last)
		{
			static_cast<node*>(cur)->value = *first;
			prev = cur;
			cur = cur->next;
			++first;
		}

		if (first != last)
		{
			auto [head, tail] = create_node_chain(first, last);
			link_node_chain_after(prev, head, tail);
		}
		else if (cur)
		{
			auto [head, tail] = unlink_node_chain_after(prev, nullptr);
			destroy_node_chain(static_cast<node*>(head));
		}
	}

	void assign(std::initializer_list<value_type> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	// ---------- Element access ---------- //

	[[nodiscard]] reference front()
	{
		RAW_ASSERT(!empty(), "front() called on empty forward_list");
		return static_cast<node*>(m_head.next)->value;
	}

	[[nodiscard]] const_reference front() const
	{
		RAW_ASSERT(!empty(), "front() called on empty forward_list");
		return static_cast<const node*>(m_head.next)->value;
	}

	// ---------- Iterators ---------- //

	iterator before_begin() noexcept
	{
		return iterator(&m_head);
	}

	const_iterator before_begin() const noexcept
	{
		return const_iterator(const_cast<node_base*>(&m_head));
	}

	const_iterator cbefore_begin() const noexcept
	{
		return before_begin();
	}

	iterator begin() noexcept
	{
		return iterator(m_head.next);
	}

	const_iterator begin() const noexcept
	{
		return const_iterator(m_head.next);
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	iterator end() noexcept
	{
		return iterator(nullptr);
	}

	const_iterator end() const noexcept
	{
		return const_iterator(nullptr);
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] bool empty() const noexcept
	{
		return m_head.next == nullptr;
	}

	[[nodiscard]] size_type max_size() const noexcept
	{
		return std::numeric_limits<difference_type>::max();
	}

	// ---------- Modifiers ---------- //

	void clear() noexcept
	{
		reset_storage();
	}

	iterator insert_after(const_iterator pos, const value_type& value)
	{
		return emplace_after(pos, value);
	}

	iterator insert_after(const_iterator pos, value_type&& value)
	{
		return emplace_after(pos, std::move(value));
	}

	iterator insert_after(const_iterator pos, size_type count, const value_type& value)
	{
		if (count == 0)
		{
			return iterator(pos.m_node);
		}

		auto [head, tail] = create_node_chain(count, value);
		link_node_chain_after(pos.m_node, head, tail);

		return iterator(tail);
	}

	template <std::input_iterator InputIt>
	iterator insert_after(const_iterator pos, InputIt first, InputIt last)
	{
		if (first == last)
		{
			return iterator(pos.m_node);
		}

		auto [head, tail] = create_node_chain(first, last);
		link_node_chain_after(pos.m_node, head, tail);

		return iterator(tail);
	}

	iterator insert_after(const_iterator pos, std::initializer_list<value_type> ilist)
	{
		return insert_after(pos, ilist.begin(), ilist.end());
	}

	template <typename... Args>
	iterator emplace_after(const_iterator pos, Args&&... args)
	{
		node* new_node = create_node(std::forward<Args>(args)...);
		link_node_after(pos.m_node, new_node);
		return iterator(new_node);
	}

	iterator erase_after(const_iterator pos)
	{
		node_base* node_to_destroy = pos.m_node->next;

		if (node_to_destroy)
		{
			unlink_node_after(pos.m_node);
			destroy_node(static_cast<node*>(node_to_destroy));
		}

		return iterator(pos.m_node->next);
	}

	iterator erase_after(const_iterator first, const_iterator last)
	{
		if (first == last)
		{
			return iterator(last.m_node);
		}

		auto [head, tail] = unlink_node_chain_after(first.m_node, last.m_node);

		if (head)
		{
			destroy_node_chain(static_cast<node*>(head));
		}

		return iterator(last.m_node);
	}

	void push_front(const value_type& value)
	{
		emplace_front(value);
	}

	void push_front(value_type&& value)
	{
		emplace_front(std::move(value));
	}

	template <typename... Args>
	reference emplace_front(Args&&... args)
	{
		return *emplace_after(before_begin(), std::forward<Args>(args)...);
	}

	void pop_front()
	{
		RAW_ASSERT(!empty(), "pop_front() called on empty forward_list");
		erase_after(before_begin());
	}

	void resize(size_type count)
	{
		node_base* prev = &m_head;
		node_base* cur = m_head.next;
		size_type i = 0;

		while (cur && i < count)
		{
			prev = cur;
			cur = cur->next;
			++i;
		}

		if (cur)
		{
			auto [head, tail] = unlink_node_chain_after(prev, nullptr);
			destroy_node_chain(static_cast<node*>(head));
		}
		else if (i < count)
		{
			auto [head, tail] = create_node_chain(count - i);
			link_node_chain_after(prev, head, tail);
		}
	}

	void resize(size_type count, const value_type& value)
	{
		node_base* prev = &m_head;
		node_base* cur = m_head.next;
		size_type i = 0;

		while (cur && i < count)
		{
			prev = cur;
			cur = cur->next;
			++i;
		}

		if (cur)
		{
			auto [head, tail] = unlink_node_chain_after(prev, nullptr);
			destroy_node_chain(static_cast<node*>(head));
		}
		else if (i < count)
		{
			auto [head, tail] = create_node_chain(count - i, value);
			link_node_chain_after(prev, head, tail);
		}
	}

	void swap(forward_list& other) noexcept
	{
		using std::swap;
		swap(m_head.next, other.m_head.next);
	}

	// ---------- Operations ---------- //

	void merge(forward_list& other)
	{
		merge(other, std::less<>{});
	}

	void merge(forward_list&& other)
	{
		merge(other, std::less<>{});
	}

	template <typename Compare>
	void merge(forward_list& other, Compare comp)
	{
		if (this == std::addressof(other) || other.empty())
		{
			return;
		}

		node_base* prev = &m_head;
		node_base* cur = m_head.next;
		node_base* other_cur = other.m_head.next;

		while (cur && other_cur)
		{
			if (comp(static_cast<node*>(other_cur)->value, static_cast<node*>(cur)->value))
			{
				node_base* other_next = other_cur->next;
				link_node_after(prev, other_cur);
				prev = other_cur;
				other_cur = other_next;
			}
			else
			{
				prev = cur;
				cur = cur->next;
			}
		}

		if (other_cur)
		{
			prev->next = other_cur;
		}

		other.m_head.next = nullptr;
	}

	template <typename Compare>
	void merge(forward_list&& other, Compare comp)
	{
		merge(other, comp);
	}

	void splice_after(const_iterator pos, forward_list& other)
	{
		if (this == std::addressof(other) || other.empty())
		{
			return;
		}

		node_base* tail = other.m_head.next;

		while (tail->next)
		{
			tail = tail->next;
		}

		link_node_chain_after(pos.m_node, other.m_head.next, tail);
		other.m_head.next = nullptr;
	}

	void splice_after(const_iterator pos, forward_list&& other)
	{
		splice_after(pos, other);
	}

	void splice_after(const_iterator pos, forward_list& other, const_iterator it)
	{
		if (this == std::addressof(other) && (pos.m_node == it.m_node || pos.m_node == it.m_node->next))
		{
			return;
		}

		node_base* node_to_move = it.m_node->next;

		if (node_to_move)
		{
			other.unlink_node_after(it.m_node);
			link_node_after(pos.m_node, node_to_move);
		}
	}

	void splice_after(const_iterator pos, forward_list&& other, const_iterator it)
	{
		splice_after(pos, other, it);
	}

	void splice_after(const_iterator pos, forward_list& other, const_iterator first, const_iterator last)
	{
		if (first == last)
		{
			return;
		}

		auto [head, tail] = other.unlink_node_chain_after(first.m_node, last.m_node);

		if (head)
		{
			link_node_chain_after(pos.m_node, head, tail);
		}
	}

	void splice_after(const_iterator pos, forward_list&& other, const_iterator first, const_iterator last)
	{
		splice_after(pos, other, first, last);
	}

	size_type remove(const value_type& value)
	{
		return remove_if([&](const value_type& elem) { return elem == value; });
	}

	template <typename UnaryPred>
	size_type remove_if(UnaryPred p)
	{
		node_base* prev = &m_head;
		node_base* cur = m_head.next;
		size_type removed = 0;

		while (cur)
		{
			if (p(static_cast<node*>(cur)->value))
			{
				node_base* node_to_destroy = cur;
				cur = cur->next;
				unlink_node_after(prev);
				destroy_node(static_cast<node*>(node_to_destroy));
				++removed;
			}
			else
			{
				prev = cur;
				cur = cur->next;
			}
		}

		return removed;
	}

	void reverse() noexcept
	{
		node_base* cur = m_head.next;
		m_head.next = nullptr;

		while (cur)
		{
			node_base* next = cur->next;
			cur->next = m_head.next;
			m_head.next = cur;
			cur = next;
		}
	}

	size_type unique()
	{
		return unique(std::equal_to<>{});
	}

	template <typename BinaryPred>
	size_type unique(BinaryPred p)
	{
		node_base* cur = m_head.next;

		if (!cur)
		{
			return 0;
		}

		size_type removed = 0;

		while (cur->next)
		{
			if (p(static_cast<node*>(cur)->value, static_cast<node*>(cur->next)->value))
			{
				node_base* node_to_destroy = cur->next;
				unlink_node_after(cur);
				destroy_node(static_cast<node*>(node_to_destroy));
				++removed;
			}
			else
			{
				cur = cur->next;
			}
		}

		return removed;
	}

	void sort()
	{
		sort(std::less<>{});
	}

	template <typename Compare>
	void sort(Compare comp)
	{
		if (m_head.next == nullptr || m_head.next->next == nullptr)
		{
			return;
		}

		forward_list carry;
		forward_list bins[64];
		size_type fill = 0;

		while (!empty())
		{
			carry.splice_after(carry.before_begin(), *this, before_begin());

			size_type i = 0;

			while (i < fill && !bins[i].empty())
			{
				bins[i].merge(carry, comp);
				carry.swap(bins[i]);
				++i;
			}

			carry.swap(bins[i]);

			if (i == fill)
			{
				++fill;
			}
		}

		if (fill > 0)
		{
			for (size_type i = 1; i < fill; ++i)
			{
				bins[i].merge(bins[i - 1], comp);
			}

			swap(bins[fill - 1]);
		}
	}

private:
	template <typename... Args>
	[[nodiscard]] node* create_node(Args&&... args) const
	{
		node* new_node = detail::allocate<node>(1);
		detail::memory_guard guard(new_node, 1);
		std::construct_at(new_node, std::forward<Args>(args)...);
		guard.release();
		return new_node;
	}

	template <typename... Args>
	[[nodiscard]] std::pair<node*, node*> create_node_chain(size_type count, Args&&... args) const
	{
		if (count == 0)
		{
			return { nullptr, nullptr };
		}

		node* head = create_node(std::forward<Args>(args)...);
		node* tail = head;
		detail::node_guard guard(head);

		for (size_type i = 1; i < count; ++i)
		{
			node* new_node = create_node(std::forward<Args>(args)...);
			tail->next = new_node;
			tail = new_node;
		}

		guard.release();

		return { head, tail };
	}

	template <std::input_iterator InputIt>
	[[nodiscard]] std::pair<node*, node*> create_node_chain(InputIt first, InputIt last) const
	{
		if (first == last)
		{
			return { nullptr, nullptr };
		}

		node* head = create_node(*first);
		node* tail = head;
		detail::node_guard guard(head);

		for (++first; first != last; ++first)
		{
			node* new_node = create_node(*first);
			tail->next = new_node;
			tail = new_node;
		}

		guard.release();

		return { head, tail };
	}

	void destroy_node(node* node_to_destroy) const noexcept
	{
		std::destroy_at(node_to_destroy);
		detail::deallocate<node>(node_to_destroy, 1);
	}

	void destroy_node_chain(node* head) const noexcept
	{
		while (head)
		{
			node* next = static_cast<node*>(head->next);
			destroy_node(head);
			head = next;
		}
	}

	void link_node_after(node_base* pos, node_base* node_to_link) const noexcept
	{
		node_to_link->next = pos->next;
		pos->next = node_to_link;
	}

	void link_node_chain_after(node_base* pos, node_base* head, node_base* tail) const noexcept
	{
		tail->next = pos->next;
		pos->next = head;
	}

	void unlink_node_after(node_base* pos) const noexcept
	{
		node_base* node_to_unlink = pos->next;
		pos->next = node_to_unlink->next;
		node_to_unlink->next = nullptr;
	}

	std::pair<node_base*, node_base*> unlink_node_chain_after(node_base* pos, node_base* last) const noexcept
	{
		node_base* head = pos->next;

		if (!head || head == last)
		{
			return { nullptr, nullptr };
		}

		node_base* tail = head;

		while (tail->next != last)
		{
			tail = tail->next;
		}

		pos->next = last;
		tail->next = nullptr;

		return { head, tail };
	}

	template <typename... Args>
	void initialize_storage(size_type count, Args&&... args)
	{
		if (count == 0)
		{
			return;
		}

		auto [head, tail] = create_node_chain(count, std::forward<Args>(args)...);
		replace_storage(head);
	}

	template <std::input_iterator InputIt>
	void initialize_storage(InputIt first, InputIt last)
	{
		if (first == last)
		{
			return;
		}

		auto [head, tail] = create_node_chain(first, last);
		replace_storage(head);
	}

	void replace_storage(node_base* new_head) noexcept
	{
		if (m_head.next)
		{
			destroy_node_chain(static_cast<node*>(m_head.next));
		}

		m_head.next = new_head;
	}

	void reset_storage() noexcept
	{
		replace_storage(nullptr);
	}

	// ---------- Variables ---------- //

	node_base m_head;
};

// ---------- Non-member functions ---------- //

template <typename T>
[[nodiscard]] bool operator==(const forward_list<T>& lhs, const forward_list<T>& rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
[[nodiscard]] detail::synth_three_way_result<T> operator<=>(const forward_list<T>& lhs, const forward_list<T>& rhs)
{
	return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), detail::synth_three_way{});
}

template <typename T>
void swap(forward_list<T>& lhs, forward_list<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
	lhs.swap(rhs);
}

template <typename T, typename U>
typename forward_list<T>::size_type erase(forward_list<T>& c, const U& value)
{
	return c.remove_if([&](const auto& elem) -> bool { return elem == value; });
}

template <typename T, typename Pred>
typename forward_list<T>::size_type erase_if(forward_list<T>& c, Pred pred)
{
	return c.remove_if(pred);
}

// ---------- Deduction guides ---------- //

template <std::input_iterator InputIt>
forward_list(InputIt, InputIt) -> forward_list<std::iter_value_t<InputIt>>;

} // namespace raw
