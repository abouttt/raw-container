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
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/assert.h"
#include "detail/compare.h"
#include "detail/memory.h"

namespace raw
{

template <typename T>
class list;

namespace detail
{

struct list_node_base
{
	list_node_base* next;
	list_node_base* prev;
};

template <typename T>
struct list_node : list_node_base
{
	T value;

	template <typename... Args>
	explicit list_node(Args&&... args)
		: list_node_base{ nullptr, nullptr }
		, value(std::forward<Args>(args)...)
	{
	}
};

template <typename T>
class list_iterator
{
private:
	using node_base = list_node_base;
	using node = list_node<T>;

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = std::remove_cv_t<T>;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	list_iterator() noexcept
		: m_node(nullptr)
	{
	}

	explicit list_iterator(node_base* node) noexcept
		: m_node(node)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	list_iterator(const list_iterator<U>& other) noexcept
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

	list_iterator& operator++() noexcept
	{
		m_node = m_node->next;
		return *this;
	}

	list_iterator operator++(int) noexcept
	{
		list_iterator tmp = *this;
		++*this;
		return tmp;
	}

	list_iterator& operator--() noexcept
	{
		m_node = m_node->prev;
		return *this;
	}

	list_iterator operator--(int) noexcept
	{
		list_iterator tmp = *this;
		--*this;
		return tmp;
	}

	template <typename U>
	[[nodiscard]] bool operator==(const list_iterator<U>& other) const noexcept
	{
		return m_node == other.m_node;
	}

private:
	template <typename>
	friend class list_iterator;
	friend class list<std::remove_cv_t<T>>;

	node_base* m_node;
};

} // namespace detail

template <typename T>
class list
{
private:
	using node_base = detail::list_node_base;
	using node = detail::list_node<T>;

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

	using iterator = detail::list_iterator<T>;
	using const_iterator = detail::list_iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Constructors / Destructor ---------- //

	list() noexcept
		: m_head{ &m_head, &m_head }
		, m_size(0)
	{
	}

	explicit list(size_type count)
		: list()
	{
		initialize_storage(count);
	}

	list(size_type count, const value_type& value)
		: list()
	{
		initialize_storage(count, value);
	}

	template <std::input_iterator InputIt>
	list(InputIt first, InputIt last)
		: list()
	{
		initialize_storage(first, last);
	}

	list(std::initializer_list<value_type> init)
		: list(init.begin(), init.end())
	{
	}

	list(const list& other)
		: list(other.begin(), other.end())
	{
	}

	list(list&& other) noexcept
		: list()
	{
		swap(other);
	}

	~list() noexcept
	{
		reset_storage();
	}

	// ---------- Assignment ---------- //

	list& operator=(const list& other)
	{
		if (this != &other)
		{
			assign(other.begin(), other.end());
		}

		return *this;
	}

	list& operator=(list&& other) noexcept
	{
		if (this != &other)
		{
			reset_storage();
			swap(other);
		}

		return *this;
	}

	list& operator=(std::initializer_list<value_type> ilist)
	{
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	void assign(size_type count, const value_type& value)
	{
		if (count > max_size())
		{
			throw_length_error();
		}

		node_base* cur = m_head.next;
		size_type assigned = 0;

		while (cur != &m_head && assigned < count)
		{
			static_cast<node*>(cur)->value = value;
			cur = cur->next;
			++assigned;
		}

		if (assigned < count)
		{
			auto [head, tail, created] = create_node_chain(count - assigned, value);
			link_node_chain(&m_head, head, tail);
			m_size += created;
		}

		if (cur != &m_head)
		{
			unlink_node_chain(cur, m_head.prev);
			m_size -= destroy_node_chain(static_cast<node*>(cur));
		}
	}

	template <std::input_iterator InputIt>
	void assign(InputIt first, InputIt last)
	{
		node_base* cur = m_head.next;

		while (cur != &m_head && first != last)
		{
			static_cast<node*>(cur)->value = *first;
			cur = cur->next;
			++first;
		}

		if (first != last)
		{
			auto [head, tail, created] = create_node_chain(first, last);
			link_node_chain(&m_head, head, tail);
			m_size += created;
		}

		if (cur != &m_head)
		{
			unlink_node_chain(cur, m_head.prev);
			m_size -= destroy_node_chain(static_cast<node*>(cur));
		}
	}

	void assign(std::initializer_list<value_type> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	// ---------- Element access ---------- //

	[[nodiscard]] reference front()
	{
		RAW_ASSERT(!empty(), "front() called on empty list");
		return static_cast<node*>(m_head.next)->value;
	}

	[[nodiscard]] const_reference front() const
	{
		RAW_ASSERT(!empty(), "front() called on empty list");
		return static_cast<const node*>(m_head.next)->value;
	}

	[[nodiscard]] reference back()
	{
		RAW_ASSERT(!empty(), "back() called on empty list");
		return static_cast<node*>(m_head.prev)->value;
	}

	[[nodiscard]] const_reference back() const
	{
		RAW_ASSERT(!empty(), "back() called on empty list");
		return static_cast<const node*>(m_head.prev)->value;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(m_head.next);
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(m_head.next);
	}

	[[nodiscard]] const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(&m_head);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(const_cast<node_base*>(&m_head));
	}

	[[nodiscard]] const_iterator cend() const noexcept
	{
		return end();
	}

	[[nodiscard]] reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	[[nodiscard]] reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] bool empty() const noexcept
	{
		return m_head.next == &m_head;
	}

	[[nodiscard]] size_type size() const noexcept
	{
		return m_size;
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

	iterator insert(const_iterator pos, const value_type& value)
	{
		return emplace(pos, value);
	}

	iterator insert(const_iterator pos, value_type&& value)
	{
		return emplace(pos, std::move(value));
	}

	iterator insert(const_iterator pos, size_type count, const value_type& value)
	{
		if (count == 0)
		{
			return iterator(pos.m_node);
		}

		if (count > max_size() - m_size)
		{
			throw_length_error();
		}

		auto [head, tail, created] = create_node_chain(count, value);
		link_node_chain(pos.m_node, head, tail);
		m_size += created;

		return iterator(head);
	}

	template <std::input_iterator InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last)
	{
		if (first == last)
		{
			return iterator(pos.m_node);
		}

		auto [head, tail, created] = create_node_chain(first, last);
		link_node_chain(pos.m_node, head, tail);
		m_size += created;

		return iterator(head);
	}

	iterator insert(const_iterator pos, std::initializer_list<value_type> ilist)
	{
		return insert(pos, ilist.begin(), ilist.end());
	}

	template <typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		if (m_size == max_size())
		{
			throw_length_error();
		}

		node* new_node = create_node(std::forward<Args>(args)...);
		link_node(pos.m_node, new_node);
		++m_size;

		return iterator(new_node);
	}

	iterator erase(const_iterator pos)
	{
		node_base* next = pos.m_node->next;
		unlink_node(pos.m_node);
		destroy_node(static_cast<node*>(pos.m_node));
		--m_size;
		return iterator(next);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == last)
		{
			return iterator(last.m_node);
		}

		unlink_node_chain(first.m_node, last.m_node->prev);
		m_size -= destroy_node_chain(static_cast<node*>(first.m_node));

		return iterator(last.m_node);
	}

	void push_back(const value_type& value)
	{
		emplace_back(value);
	}

	void push_back(value_type&& value)
	{
		emplace_back(std::move(value));
	}

	template <typename... Args>
	reference emplace_back(Args&&... args)
	{
		return *emplace(end(), std::forward<Args>(args)...);
	}

	void pop_back()
	{
		RAW_ASSERT(!empty(), "pop_back() called on empty list");
		erase(std::prev(end()));
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
		return *emplace(begin(), std::forward<Args>(args)...);
	}

	void pop_front()
	{
		RAW_ASSERT(!empty(), "pop_front() called on empty list");
		erase(begin());
	}

	void resize(size_type count)
	{
		if (m_size < count)
		{
			if (count > max_size())
			{
				throw_length_error();
			}

			auto [head, tail, created] = create_node_chain(count - m_size);
			link_node_chain(&m_head, head, tail);
			m_size += created;
		}
		else if (m_size > count)
		{
			erase(std::next(begin(), count), end());
		}
	}

	void resize(size_type count, const value_type& value)
	{
		if (m_size < count)
		{
			if (count > max_size())
			{
				throw_length_error();
			}

			auto [head, tail, created] = create_node_chain(count - m_size, value);
			link_node_chain(&m_head, head, tail);
			m_size += created;
		}
		else if (m_size > count)
		{
			erase(std::next(begin(), count), end());
		}
	}

	void swap(list& other) noexcept
	{
		if (this == std::addressof(other))
		{
			return;
		}

		using std::swap;
		swap(m_head.next, other.m_head.next);
		swap(m_head.prev, other.m_head.prev);
		swap(m_size, other.m_size);

		if (m_head.next == &other.m_head)
		{
			m_head.next = &m_head;
		}
		else
		{
			m_head.next->prev = &m_head;
		}

		if (m_head.prev == &other.m_head)
		{
			m_head.prev = &m_head;
		}
		else
		{
			m_head.prev->next = &m_head;
		}

		if (other.m_head.next == &m_head)
		{
			other.m_head.next = &other.m_head;
		}
		else
		{
			other.m_head.next->prev = &other.m_head;
		}

		if (other.m_head.prev == &m_head)
		{
			other.m_head.prev = &other.m_head;
		}
		else
		{
			other.m_head.prev->next = &other.m_head;
		}
	}

	// ---------- Operations ---------- //

	void merge(list& other)
	{
		merge(other, std::less<>{});
	}

	void merge(list&& other)
	{
		merge(other, std::less<>{});
	}

	template <typename Compare>
	void merge(list& other, Compare comp)
	{
		if (this == std::addressof(other) || other.empty())
		{
			return;
		}

		if (m_size > max_size() - other.m_size)
		{
			throw_length_error();
		}

		iterator it = begin();
		iterator other_it = other.begin();

		while (it != end() && other_it != other.end())
		{
			if (comp(*other_it, *it))
			{
				iterator other_next_it = std::next(other_it);
				splice(it, other, other_it);
				other_it = other_next_it;
			}
			else
			{
				++it;
			}
		}

		if (other_it != other.end())
		{
			splice(end(), other, other_it, other.end());
		}
	}

	template <typename Compare>
	void merge(list&& other, Compare comp)
	{
		merge(other, comp);
	}

	void splice(const_iterator pos, list& other)
	{
		if (this == std::addressof(other) || other.empty())
		{
			return;
		}

		if (m_size > max_size() - other.m_size)
		{
			throw_length_error();
		}

		node_base* head = other.m_head.next;
		node_base* tail = other.m_head.prev;

		other.unlink_node_chain(head, tail);
		link_node_chain(pos.m_node, head, tail);

		m_size += other.m_size;
		other.m_size = 0;
	}

	void splice(const_iterator pos, list&& other)
	{
		splice(pos, other);
	}

	void splice(const_iterator pos, list& other, const_iterator it)
	{
		bool is_same_list = this == std::addressof(other);

		if (is_same_list && (pos.m_node == it.m_node || pos.m_node == it.m_node->next))
		{
			return;
		}

		if (!is_same_list && m_size == max_size())
		{
			throw_length_error();
		}

		other.unlink_node(it.m_node);
		link_node(pos.m_node, it.m_node);

		if (!is_same_list)
		{
			++m_size;
			--other.m_size;
		}
	}

	void splice(const_iterator pos, list&& other, const_iterator it)
	{
		splice(pos, other, it);
	}

	void splice(const_iterator pos, list& other, const_iterator first, const_iterator last)
	{
		if (first == last)
		{
			return;
		}

		if (this != std::addressof(other))
		{
			difference_type length = std::distance(first, last);
			size_type count = static_cast<size_type>(length);

			if (m_size > max_size() - count)
			{
				throw_length_error();
			}

			m_size += count;
			other.m_size -= count;
		}

		node_base* head = first.m_node;
		node_base* tail = last.m_node->prev;

		other.unlink_node_chain(head, tail);
		link_node_chain(pos.m_node, head, tail);
	}

	void splice(const_iterator pos, list&& other, const_iterator first, const_iterator last)
	{
		splice(pos, other, first, last);
	}

	size_type remove(const value_type& value)
	{
		return remove_if([&](const value_type& elem) { return elem == value; });
	}

	template <typename UnaryPred>
	size_type remove_if(UnaryPred p)
	{
		iterator it = begin();
		size_type removed = 0;

		while (it != end())
		{
			if (p(*it))
			{
				it = erase(it);
				++removed;
			}
			else
			{
				++it;
			}
		}

		return removed;
	}

	void reverse() noexcept
	{
		if (m_size <= 1)
		{
			return;
		}

		node_base* cur = &m_head;

		do
		{
			using std::swap;
			swap(cur->next, cur->prev);
			cur = cur->prev;
		} while (cur != &m_head);
	}

	size_type unique()
	{
		return unique(std::equal_to<>{});
	}

	template <typename BinaryPred>
	size_type unique(BinaryPred p)
	{
		if (empty())
		{
			return 0;
		}

		iterator it = begin();
		iterator next_it = std::next(it);
		size_type removed = 0;

		while (next_it != end())
		{
			if (p(*it, *next_it))
			{
				next_it = erase(next_it);
				++removed;
			}
			else
			{
				it = next_it;
				++next_it;
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
		if (m_size <= 1)
		{
			return;
		}

		list carry;
		list bins[64];
		size_type fill = 0;

		while (!empty())
		{
			carry.splice(carry.begin(), *this, begin());

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
	[[noreturn]] static void throw_length_error()
	{
		throw std::length_error("list too long");
	}

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
	[[nodiscard]] std::tuple<node*, node*, size_type> create_node_chain(size_type count, Args&&... args) const
	{
		if (count == 0)
		{
			return { nullptr, nullptr, 0 };
		}

		node* head = create_node(std::forward<Args>(args)...);
		node* tail = head;
		size_type created = 1;
		detail::node_guard guard(head);

		for (; created < count; ++created)
		{
			node* new_node = create_node(std::forward<Args>(args)...);
			tail->next = new_node;
			new_node->prev = tail;
			tail = new_node;
		}

		guard.release();

		return { head, tail, created };
	}

	template <std::input_iterator InputIt>
	[[nodiscard]] std::tuple<node*, node*, size_type> create_node_chain(InputIt first, InputIt last) const
	{
		if (first == last)
		{
			return { nullptr, nullptr, 0 };
		}

		node* head = create_node(*first);
		node* tail = head;
		size_type created = 1;
		detail::node_guard guard(head);

		for (++first; first != last; ++first)
		{
			node* new_node = create_node(*first);
			tail->next = new_node;
			new_node->prev = tail;
			tail = new_node;
			++created;
		}

		guard.release();

		return { head, tail, created };
	}

	void destroy_node(node* node_to_destroy) const noexcept
	{
		std::destroy_at(node_to_destroy);
		detail::deallocate<node>(node_to_destroy, 1);
	}

	size_type destroy_node_chain(node* head) const noexcept
	{
		size_type destroyed = 0;

		while (head)
		{
			node* next = static_cast<node*>(head->next);
			destroy_node(head);
			head = next;
			++destroyed;
		}

		return destroyed;
	}

	void link_node(node_base* pos, node_base* node_to_link) const noexcept
	{
		node_to_link->next = pos;
		node_to_link->prev = pos->prev;
		pos->prev->next = node_to_link;
		pos->prev = node_to_link;
	}

	void link_node_chain(node_base* pos, node_base* head, node_base* tail) const noexcept
	{
		head->prev = pos->prev;
		tail->next = pos;
		pos->prev->next = head;
		pos->prev = tail;
	}

	void unlink_node(node_base* node_to_unlink) const noexcept
	{
		node_to_unlink->prev->next = node_to_unlink->next;
		node_to_unlink->next->prev = node_to_unlink->prev;
		node_to_unlink->next = nullptr;
		node_to_unlink->prev = nullptr;
	}

	void unlink_node_chain(node_base* head, node_base* tail) const noexcept
	{
		head->prev->next = tail->next;
		tail->next->prev = head->prev;
		head->prev = nullptr;
		tail->next = nullptr;
	}

	template <typename... Args>
	void initialize_storage(size_type count, Args&&... args)
	{
		if (count == 0)
		{
			return;
		}

		auto [head, tail, created] = create_node_chain(count, std::forward<Args>(args)...);
		replace_storage(head, tail, created);
	}

	template <std::input_iterator InputIt>
	void initialize_storage(InputIt first, InputIt last)
	{
		if (first == last)
		{
			return;
		}

		auto [head, tail, created] = create_node_chain(first, last);
		replace_storage(head, tail, created);
	}

	void replace_storage(node_base* new_head, node_base* new_tail, size_type new_size) noexcept
	{
		if (m_head.next != &m_head)
		{
			node_base* head = m_head.next;
			unlink_node_chain(head, m_head.prev);
			destroy_node_chain(static_cast<node*>(head));
		}

		if (new_head)
		{
			new_head->prev = &m_head;
			new_tail->next = &m_head;
		}

		m_head.next = new_head ? new_head : &m_head;
		m_head.prev = new_tail ? new_tail : &m_head;
		m_size = new_size;
	}

	void reset_storage() noexcept
	{
		replace_storage(nullptr, nullptr, 0);
	}

	// ---------- Variables ---------- //

	node_base m_head;
	size_type m_size;
};

// ---------- Non-member functions ---------- //

template <typename T>
[[nodiscard]] bool operator==(const list<T>& lhs, const list<T>& rhs)
{
	if (lhs.size() != rhs.size())
	{
		return false;
	}

	return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
[[nodiscard]] detail::synth_three_way_result<T> operator<=>(const list<T>& lhs, const list<T>& rhs)
{
	return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), detail::synth_three_way{});
}

template <typename T>
void swap(list<T>& lhs, list<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
	lhs.swap(rhs);
}

template <typename T, typename U>
typename list<T>::size_type erase(list<T>& c, const U& value)
{
	return c.remove_if([&](const auto& elem) -> bool { return elem == value; });
}

template <typename T, typename Pred>
typename list<T>::size_type erase_if(list<T>& c, Pred pred)
{
	return c.remove_if(pred);
}

// ---------- Deduction guides ---------- //

template <std::input_iterator InputIt>
list(InputIt, InputIt) -> list<std::iter_value_t<InputIt>>;

} // namespace raw
