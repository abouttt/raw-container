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
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#include "assert.h"
#include "detail/memory.h"
#include "detail/raii.h"

namespace raw
{

template <typename T>
class list;

namespace detail
{

struct list_node_base
{
	list_node_base* next = nullptr;
	list_node_base* prev = nullptr;
};

template <typename T>
struct list_node : list_node_base
{
	T value;

	template <typename... Args>
	list_node(Args&&... args)
		: list_node_base{}
		, value(std::forward<Args>(args)...)
	{
	}
};

template <typename T>
class list_iterator
{
private:
	using node_base = list_node_base;
	using node		= list_node<T>;

public:
	// ---------- Types ---------- //

	using iterator_category = std::bidirectional_iterator_tag;
	using value_type		= std::remove_cv_t<T>;
	using difference_type	= std::ptrdiff_t;
	using pointer			= T*;
	using reference			= T&;

	// ---------- Constructors ---------- //

	list_iterator() noexcept
		: _ptr(nullptr)
	{
	}

	explicit list_iterator(node_base* ptr) noexcept
		: _ptr(ptr)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	list_iterator(const list_iterator<U>& other) noexcept
		: _ptr(other._ptr)
	{
	}

	// ---------- Access ---------- //

	[[nodiscard]] reference operator*() const noexcept
	{
		return static_cast<node*>(_ptr)->value;
	}

	[[nodiscard]] pointer operator->() const noexcept
	{
		return &static_cast<node*>(_ptr)->value;
	}

	// ---------- Increment / Decrement ---------- //

	list_iterator& operator++() noexcept
	{
		_ptr = _ptr->next;
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
		_ptr = _ptr->prev;
		return *this;
	}

	list_iterator operator--(int) noexcept
	{
		list_iterator tmp = *this;
		--*this;
		return tmp;
	}

	// ---------- Comparison ---------- //

	template <typename U>
	[[nodiscard]] bool operator==(const list_iterator<U>& other) const noexcept
	{
		return _ptr == other._ptr;
	}

	template <typename U>
	[[nodiscard]] bool operator!=(const list_iterator<U>& other) const noexcept
	{
		return !(*this == other);
	}

private:
	template <typename>
	friend class list_iterator;
	friend class list<std::remove_cv_t<T>>;

	node_base* _ptr;
};

} // namespace detail

template <typename T>
class list
{
private:
	using node_base = detail::list_node_base;
	using node		= detail::list_node<T>;

public:
	static_assert(std::is_object_v<T>, "list<T> element type must be an object");

	// ---------- Types ---------- //

	using value_type	  = T;
	using size_type		  = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference		  = T&;
	using const_reference = const T&;
	using pointer		  = T*;
	using const_pointer	  = const T*;

	using iterator				 = detail::list_iterator<T>;
	using const_iterator		 = detail::list_iterator<const T>;
	using reverse_iterator		 = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Constructors / Destructor ---------- //

	list() noexcept
		: _sentinel{ &_sentinel, &_sentinel }
		, _size(0)
	{
	}

	explicit list(size_type count)
		: list()
	{
		construct_n(count, T{});
	}

	list(size_type count, const T& value)
		: list()
	{
		construct_n(count, value);
	}

	template <std::input_iterator InputIt>
	list(InputIt first, InputIt last)
		: list()
	{
		construct(first, last);
	}

	list(std::initializer_list<T> init)
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

	~list()
	{
		tidy();
	}

	// ---------- Assignment ---------- //

	list& operator=(const list& other)
	{
		if (this != std::addressof(other))
		{
			list tmp(other);
			swap(tmp);
		}

		return *this;
	}

	list& operator=(list&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			tidy();
			swap(other);
		}

		return *this;
	}

	list& operator=(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	void assign(size_type count, const T& value)
	{
		list tmp(count, value);
		swap(tmp);
	}

	template <std::input_iterator InputIt>
	void assign(InputIt first, InputIt last)
	{
		list tmp(first, last);
		swap(tmp);
	}

	void assign(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	// ---------- Element access ---------- //

	[[nodiscard]] reference front()
	{
		RAW_ASSERT(!empty(), "front() called on empty list");
		return static_cast<node*>(_sentinel.next)->value;
	}

	[[nodiscard]] const_reference front() const
	{
		RAW_ASSERT(!empty(), "front() called on empty list");
		return static_cast<node*>(_sentinel.next)->value;
	}

	[[nodiscard]] reference back()
	{
		RAW_ASSERT(!empty(), "back() called on empty list");
		return static_cast<node*>(_sentinel.prev)->value;
	}

	[[nodiscard]] const_reference back() const
	{
		RAW_ASSERT(!empty(), "back() called on empty list");
		return static_cast<node*>(_sentinel.prev)->value;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] iterator begin() noexcept
	{
		return iterator(_sentinel.next);
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator(_sentinel.next);
	}

	[[nodiscard]] iterator end() noexcept
	{
		return iterator(&_sentinel);
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator(const_cast<node_base*>(&_sentinel));
	}

	[[nodiscard]] reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	[[nodiscard]] reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] const_iterator cend() const noexcept
	{
		return end();
	}

	[[nodiscard]] const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	[[nodiscard]] const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] bool empty() const noexcept
	{
		return _sentinel.next == &_sentinel;
	}

	[[nodiscard]] size_type size() const noexcept
	{
		return _size;
	}

	[[nodiscard]] size_type max_size() const noexcept
	{
		return std::numeric_limits<difference_type>::max();
	}

	// ---------- Modifiers ---------- //

	void clear() noexcept
	{
		tidy();
	}

	iterator insert(const_iterator pos, const T& value)
	{
		return emplace(pos, value);
	}

	iterator insert(const_iterator pos, T&& value)
	{
		return emplace(pos, std::move(value));
	}

	iterator insert(const_iterator pos, size_type count, const T& value)
	{
		if (count == 0)
		{
			return iterator(pos._ptr);
		}

		auto [head, tail, created] = create_node_chain_n(count, value);
		hook_node_chain(pos._ptr, head, tail);
		_size += created;

		return iterator(head);
	}

	template <std::input_iterator InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last)
	{
		if (first == last)
		{
			return iterator(pos._ptr);
		}

		auto [head, tail, created] = create_node_chain(first, last);
		hook_node_chain(pos._ptr, head, tail);
		_size += created;

		return iterator(head);
	}

	iterator insert(const_iterator pos, std::initializer_list<T> ilist)
	{
		return insert(pos, ilist.begin(), ilist.end());
	}

	template <typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		node* new_node = create_node(std::forward<Args>(args)...);
		hook_node(pos._ptr, new_node);
		++_size;
		return iterator(new_node);
	}

	iterator erase(const_iterator pos)
	{
		return erase(pos, std::next(pos));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == last)
		{
			return iterator(last._ptr);
		}

		node_base* head = first._ptr;
		node_base* tail = last._ptr->prev;

		unhook_node_chain(head, tail);
		_size -= destroy_node_chain(static_cast<node*>(head));

		return iterator(last._ptr);
	}

	void push_back(const T& value)
	{
		emplace_back(value);
	}

	void push_back(T&& value)
	{
		emplace_back(std::move(value));
	}

	template <typename... Args>
	reference emplace_back(Args&&... args)
	{
		iterator it = emplace(end(), std::forward<Args>(args)...);
		return *it;
	}

	void pop_back()
	{
		RAW_ASSERT(!empty(), "pop_back() called on empty list");
		erase(std::prev(end()));
	}

	void push_front(const T& value)
	{
		emplace_front(value);
	}

	void push_front(T&& value)
	{
		emplace_front(std::move(value));
	}

	template <typename... Args>
	reference emplace_front(Args&&... args)
	{
		iterator it = emplace(begin(), std::forward<Args>(args)...);
		return *it;
	}

	void pop_front()
	{
		RAW_ASSERT(!empty(), "pop_front() called on empty list");
		erase(begin());
	}

	void resize(size_type count)
	{
		resize(count, T{});
	}

	void resize(size_type count, const T& value)
	{
		if (count < _size)
		{
			iterator it = begin();
			std::advance(it, count);
			erase(it, end());
		}
		else if (count > _size)
		{
			insert(end(), count - _size, value);
		}
	}

	void swap(list& other) noexcept
	{
		if (this == std::addressof(other))
		{
			return;
		}

		node_base* this_head = _sentinel.next;
		node_base* this_tail = _sentinel.prev;
		node_base* other_head = other._sentinel.next;
		node_base* other_tail = other._sentinel.prev;

		if (other_head != &other._sentinel)
		{
			_sentinel.next = other_head;
			_sentinel.prev = other_tail;
			other_head->prev = &_sentinel;
			other_tail->next = &_sentinel;
		}
		else
		{
			_sentinel.next = &_sentinel;
			_sentinel.prev = &_sentinel;
		}

		if (this_head != &_sentinel)
		{
			other._sentinel.next = this_head;
			other._sentinel.prev = this_tail;
			this_head->prev = &other._sentinel;
			this_tail->next = &other._sentinel;
		}
		else
		{
			other._sentinel.next = &other._sentinel;
			other._sentinel.prev = &other._sentinel;
		}

		using std::swap;
		swap(_size, other._size);
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

		iterator it1 = begin();
		iterator it2 = other.begin();

		while (it1 != end() && it2 != other.end())
		{
			if (comp(*it2, *it1))
			{
				iterator next2 = std::next(it2);
				splice(it1, other, it2);
				it2 = next2;
			}
			else
			{
				++it1;
			}
		}

		if (it2 != other.end())
		{
			splice(end(), other, it2, other.end());
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

		splice(pos, other, other.begin(), other.end());
	}

	void splice(const_iterator pos, list&& other)
	{
		splice(pos, other);
	}

	void splice(const_iterator pos, list& other, const_iterator it)
	{
		if (pos == it || pos == std::next(it))
		{
			return;
		}

		splice(pos, other, it, std::next(it));
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

		node_base* head = first._ptr;
		node_base* tail = last._ptr->prev;

		if (this != std::addressof(other))
		{
			size_type count = std::distance(first, last);
			other._size -= count;
			_size += count;
		}

		other.unhook_node_chain(head, tail);
		hook_node_chain(pos._ptr, head, tail);
	}

	void splice(const_iterator pos, list&& other, const_iterator first, const_iterator last)
	{
		splice(pos, other, first, last);
	}

	size_type remove(const T& value)
	{
		return remove_if([&](const T& elem) { return elem == value; });
	}

	template <typename UnaryPred>
	size_type remove_if(UnaryPred p)
	{
		node* cur = static_cast<node*>(_sentinel.next);
		size_type removed = 0;

		while (cur != &_sentinel)
		{
			node* next_node = static_cast<node*>(cur->next);

			if (p(cur->value))
			{
				unhook_node(cur);
				destroy_node(cur);
				--_size;
				++removed;
			}

			cur = next_node;
		}

		return removed;
	}

	void reverse() noexcept
	{
		if (_size <= 1)
		{
			return;
		}

		node_base* cur = _sentinel.next;
		node_base* next_node = nullptr;

		while (cur != &_sentinel)
		{
			next_node = cur->next;
			cur->next = cur->prev;
			cur->prev = next_node;
			cur = next_node;
		}

		using std::swap;
		swap(_sentinel.next, _sentinel.prev);
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

		node* cur = static_cast<node*>(_sentinel.next->next);
		size_type removed = 0;

		while (cur != &_sentinel)
		{
			node* next_node = static_cast<node*>(cur->next);
			node* prev_node = static_cast<node*>(cur->prev);

			if (p(prev_node->value, cur->value))
			{
				unhook_node(cur);
				destroy_node(cur);
				--_size;
				++removed;
			}

			cur = next_node;
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
		if (_size <= 1)
		{
			return;
		}

		list carry;
		list counter[64];
		size_type fill = 0;

		while (!empty())
		{
			carry.splice(carry.begin(), *this, begin());

			size_type i = 0;

			while (i < fill && !counter[i].empty())
			{
				counter[i].merge(carry, comp);
				carry.swap(counter[i++]);
			}

			carry.swap(counter[i]);

			if (i == fill)
			{
				++fill;
			}
		}

		if (fill > 0)
		{
			for (size_type i = 1; i < fill; ++i)
			{
				counter[i].merge(counter[i - 1], comp);
			}

			swap(counter[fill - 1]);
		}
	}

private:
	template <typename... Args>
	[[nodiscard]] node* create_node(Args&&... args)
	{
		node* ptr = detail::allocate<node>(1);
		detail::memory_guard<node> guard(ptr, 1);
		std::construct_at(ptr, std::forward<Args>(args)...);
		guard.release();
		return ptr;
	}

	template <std::input_iterator InputIt>
	std::tuple<node*, node*, size_type> create_node_chain(InputIt first, InputIt last)
	{
		if (first == last)
		{
			return { nullptr, nullptr, 0 };
		}

		node* head = create_node(*first);
		node* tail = head;
		size_type created = 1;

		try
		{
			for (++first; first != last; ++first, ++created)
			{
				node* new_node = create_node(*first);
				tail->next = new_node;
				new_node->prev = tail;
				tail = new_node;
			}
		}
		catch (...)
		{
			destroy_node_chain(head);
			throw;
		}

		return { head, tail, created };
	}

	std::tuple<node*, node*, size_type> create_node_chain_n(size_type count, const T& value)
	{
		if (count == 0)
		{
			return { nullptr, nullptr, 0 };
		}

		node* head = create_node(value);
		node* tail = head;
		size_type created = 1;

		try
		{
			for (; count > 1; --count, ++created)
			{
				node* new_node = create_node(value);
				tail->next = new_node;
				new_node->prev = tail;
				tail = new_node;
			}
		}
		catch (...)
		{
			destroy_node_chain(head);
			throw;
		}

		return { head, tail, created };
	}

	void destroy_node(node* node_to_destroy)
	{
		std::destroy_at(node_to_destroy);
		detail::deallocate(node_to_destroy, 1);
	}

	size_type destroy_node_chain(node* head)
	{
		node* cur = head;
		size_type destroyed = 0;

		while (cur && cur != &_sentinel)
		{
			node* next = static_cast<node*>(cur->next);
			destroy_node(cur);
			cur = next;
			++destroyed;
		}

		return destroyed;
	}

	void hook_node(node_base* pos, node_base* node_to_hook)
	{
		node_to_hook->next = pos;
		node_to_hook->prev = pos->prev;
		pos->prev->next = node_to_hook;
		pos->prev = node_to_hook;
	}

	void hook_node_chain(node_base* pos, node_base* head, node_base* tail)
	{
		head->prev = pos->prev;
		tail->next = pos;
		pos->prev->next = head;
		pos->prev = tail;
	}

	void unhook_node(node_base* node_to_unhook)
	{
		node_to_unhook->prev->next = node_to_unhook->next;
		node_to_unhook->next->prev = node_to_unhook->prev;
	}

	void unhook_node_chain(node_base* head, node_base* tail)
	{
		head->prev->next = tail->next;
		tail->next->prev = head->prev;
		head->prev = nullptr;
		tail->next = nullptr;
	}

	template <std::input_iterator InputIt>
	void construct(InputIt first, InputIt last)
	{
		if (first == last)
		{
			return;
		}

		auto [head, tail, created] = create_node_chain(first, last);
		change_chain(head, tail, created);
	}

	void construct_n(size_type count, const T& value)
	{
		if (count == 0)
		{
			return;
		}

		auto [head, tail, created] = create_node_chain_n(count, value);
		change_chain(head, tail, created);
	}

	void change_chain(node_base* new_head, node_base* new_tail, size_type new_size)
	{
		if (_sentinel.next != &_sentinel)
		{
			destroy_node_chain(static_cast<node*>(_sentinel.next));
		}

		if (new_head)
		{
			_sentinel.next = new_head;
			_sentinel.prev = new_tail;
			new_head->prev = &_sentinel;
			new_tail->next = &_sentinel;
		}
		else
		{
			_sentinel.next = &_sentinel;
			_sentinel.prev = &_sentinel;
		}

		_size = new_size;
	}

	void tidy()
	{
		change_chain(&_sentinel, &_sentinel, 0);
	}

	// ---------- Variables ---------- //

	node_base _sentinel;
	size_type _size;
};

// ---------- Non-member functions ---------- //

template <typename T>
[[nodiscard]] bool operator==(const list<T>& lhs, const list<T>& rhs)
{
	return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T>
[[nodiscard]] auto operator<=>(const list<T>& lhs, const list<T>& rhs)
{
	return std::lexicographical_compare_three_way(
		lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::compare_three_way{});
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
