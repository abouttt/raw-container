#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "detail/assert.h"
#include "detail/destroy_guard.h"
#include "detail/memory_guard.h"

namespace raw
{

namespace detail
{

template <typename T>
class vector_iterator
{
public:
	using iterator_concept = std::contiguous_iterator_tag;
	using iterator_category = std::random_access_iterator_tag;
	using value_type = std::remove_cv_t<T>;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	constexpr vector_iterator() noexcept
		: _ptr(nullptr)
	{
	}

	constexpr explicit vector_iterator(pointer ptr) noexcept
		: _ptr(ptr)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	constexpr vector_iterator(const vector_iterator<U>& other) noexcept
		: _ptr(other._ptr)
	{
	}

	[[nodiscard]] constexpr reference operator*() const noexcept
	{
		return *_ptr;
	}

	[[nodiscard]] constexpr pointer operator->() const noexcept
	{
		return _ptr;
	}

	constexpr vector_iterator& operator++() noexcept
	{
		++_ptr;
		return *this;
	}

	constexpr vector_iterator operator++(int) noexcept
	{
		vector_iterator tmp = *this;
		++_ptr;
		return tmp;
	}

	constexpr vector_iterator& operator--() noexcept
	{
		--_ptr;
		return *this;
	}

	constexpr vector_iterator operator--(int) noexcept
	{
		vector_iterator tmp = *this;
		--_ptr;
		return tmp;
	}

	constexpr vector_iterator& operator+=(difference_type offset) noexcept
	{
		_ptr += offset;
		return *this;
	}

	[[nodiscard]] constexpr vector_iterator operator+(difference_type offset) const noexcept
	{
		return vector_iterator(_ptr + offset);
	}

	constexpr vector_iterator& operator-=(difference_type offset) noexcept
	{
		_ptr -= offset;
		return *this;
	}

	[[nodiscard]] constexpr vector_iterator operator-(difference_type offset) const noexcept
	{
		return vector_iterator(_ptr - offset);
	}

	template <typename U>
	[[nodiscard]] constexpr difference_type operator-(const vector_iterator<U>& other) const noexcept
	{
		return _ptr - other._ptr;
	}

	[[nodiscard]] constexpr reference operator[](difference_type offset) const noexcept
	{
		return *(_ptr + offset);
	}

	template <typename U>
	[[nodiscard]] constexpr bool operator==(const vector_iterator<U>& other) const noexcept
	{
		return _ptr == other._ptr;
	}

	template <typename U>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const vector_iterator<U>& other) const noexcept
	{
		return _ptr <=> other._ptr;
	}

	[[nodiscard]] friend constexpr vector_iterator operator+(difference_type offset, const vector_iterator& it) noexcept
	{
		return vector_iterator(it._ptr + offset);
	}

private:
	template <typename>
	friend class vector_iterator;

	pointer _ptr;
};

} // namespace detail

template <typename T>
class vector
{
public:
	static_assert(std::is_object_v<T>, "vector element type must be an object");

	// ---------- Types ---------- //

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using iterator = detail::vector_iterator<T>;
	using const_iterator = detail::vector_iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Constructors & Destructor ---------- //

	vector() noexcept
		: _begin(nullptr)
		, _end(nullptr)
		, _cap(nullptr)
	{
	}

	explicit vector(size_type count)
		: vector()
	{
		construct_n(count);
	}

	vector(size_type count, const T& value)
		: vector()
	{
		construct_n(count, value);
	}

	template <std::input_iterator InputIt>
	vector(InputIt first, InputIt last)
		: vector()
	{
		if constexpr (std::forward_iterator<InputIt>)
		{
			const size_type count = range_to_count(first, last);
			construct_n(count, first, last);
		}
		else
		{
			for (; first != last; ++first)
			{
				emplace_back(*first);
			}
		}
	}

	vector(std::initializer_list<T> init)
		: vector(init.begin(), init.end())
	{
	}

	vector(const vector& other)
		: vector(other.begin(), other.end())
	{
	}

	vector(vector&& other) noexcept
		: _begin(std::exchange(other._begin, nullptr))
		, _end(std::exchange(other._end, nullptr))
		, _cap(std::exchange(other._cap, nullptr))
	{
	}

	~vector()
	{
		tidy();
	}

	// ---------- Assignment ---------- //

	vector& operator=(const vector& other)
	{
		if (this != std::addressof(other))
		{
			assign(other.begin(), other.end());
		}

		return *this;
	}

	vector& operator=(vector&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			tidy();

			_begin = std::exchange(other._begin, nullptr);
			_end = std::exchange(other._end, nullptr);
			_cap = std::exchange(other._cap, nullptr);
		}

		return *this;
	}

	vector& operator=(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
		return *this;
	}

	void assign(size_type count, const T& value)
	{
		if (count <= capacity())
		{
			const size_type old_size = size();

			if (count > old_size)
			{
				std::fill_n(_begin, old_size, value);
				std::uninitialized_fill_n(_end, count - old_size, value);
			}
			else
			{
				std::fill_n(_begin, count, value);
				std::destroy_n(_begin + count, old_size - count);
			}

			_end = _begin + count;
		}
		else
		{
			const size_type new_cap = calculate_growth(count);
			pointer new_begin = allocate(new_cap);

			detail::memory_guard<T> mguard(new_begin, new_cap);

			std::uninitialized_fill_n(new_begin, count, value);

			mguard.release();
			change_array(new_begin, count, new_cap);
		}
	}

	template <std::input_iterator InputIt>
	void assign(InputIt first, InputIt last)
	{
		if constexpr (std::forward_iterator<InputIt>)
		{
			const size_type count = range_to_count(first, last);

			if (count <= capacity())
			{
				const size_type old_size = size();

				if (count > old_size)
				{
					InputIt mid = std::next(first, old_size);
					std::copy(first, mid, _begin);
					std::uninitialized_copy(mid, last, _end);
				}
				else
				{
					std::copy(first, last, _begin);
					std::destroy(_begin + count, _end);
				}

				_end = _begin + count;
			}
			else
			{
				const size_type new_cap = calculate_growth(count);
				pointer new_begin = allocate(new_cap);

				detail::memory_guard<T> mguard(new_begin, new_cap);

				std::uninitialized_copy(first, last, new_begin);

				mguard.release();
				change_array(new_begin, count, new_cap);
			}
		}
		else
		{
			vector(first, last).swap(*this);
		}
	}

	void assign(std::initializer_list<T> ilist)
	{
		assign(ilist.begin(), ilist.end());
	}

	// ---------- Element access ---------- //

	[[nodiscard]] constexpr reference at(size_type pos)
	{
		if (pos >= size())
		{
			xrange();
		}

		return _begin[pos];
	}

	[[nodiscard]] constexpr const_reference at(size_type pos) const
	{
		if (pos >= size())
		{
			xrange();
		}

		return _begin[pos];
	}

	[[nodiscard]] constexpr reference operator[](size_type pos)
	{
		RAW_ASSERT(pos < size(), "vector subscript out of range");
		return _begin[pos];
	}

	[[nodiscard]] constexpr const_reference operator[](size_type pos) const
	{
		RAW_ASSERT(pos < size(), "vector subscript out of range");
		return _begin[pos];
	}

	[[nodiscard]] constexpr reference front()
	{
		RAW_ASSERT(!empty(), "front() called on empty vector");
		return *_begin;
	}

	[[nodiscard]] constexpr const_reference front() const
	{
		RAW_ASSERT(!empty(), "front() called on empty vector");
		return *_begin;
	}

	[[nodiscard]] constexpr reference back()
	{
		RAW_ASSERT(!empty(), "back() called on empty vector");
		return *(_end - 1);
	}

	[[nodiscard]] constexpr const_reference back() const
	{
		RAW_ASSERT(!empty(), "back() called on empty vector");
		return *(_end - 1);
	}

	[[nodiscard]] constexpr T* data() noexcept
	{
		return _begin;
	}

	[[nodiscard]] constexpr const T* data() const noexcept
	{
		return _begin;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] constexpr iterator begin() noexcept
	{
		return iterator(_begin);
	}

	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator(_begin);
	}

	[[nodiscard]] constexpr iterator end() noexcept
	{
		return iterator(_end);
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator(_end);
	}

	[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	[[nodiscard]] constexpr reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return end();
	}

	[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return _begin == _end;
	}

	[[nodiscard]] constexpr size_type size() const noexcept
	{
		return static_cast<size_type>(_end - _begin);
	}

	[[nodiscard]] constexpr size_type max_size() const noexcept
	{
		return std::numeric_limits<difference_type>::max() / sizeof(T);
	}

	[[nodiscard]] constexpr size_type capacity() const noexcept
	{
		return static_cast<size_type>(_cap - _begin);
	}

	void reserve(size_type new_cap)
	{
		if (new_cap > capacity())
		{
			if (new_cap > max_size())
			{
				xlength();
			}

			reallocate(new_cap);
		}
	}

	void shrink_to_fit()
	{
		if (_end != _cap)
		{
			if (_begin != _end)
			{
				const size_type new_cap = static_cast<size_type>(_end - _begin);
				reallocate(new_cap);
			}
			else
			{
				tidy();
			}
		}
	}

	// ---------- Modifiers ---------- //

	void clear() noexcept
	{
		std::destroy(_begin, _end);
		_end = _begin;
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
		RAW_ASSERT(pos >= begin() && pos <= end(), "vector insert iterator outside range");

		const difference_type offset = pos - begin();

		if (count == 0)
		{
			return begin() + offset;
		}

		const size_type old_size = size();

		if (old_size + count <= capacity())
		{
			T value_copy = value;
			pointer insert_pos = _begin + offset;
			const size_type elems_after = old_size - offset;

			detail::destroy_guard<T> dguard(_end);

			if (count > elems_after)
			{
				std::uninitialized_fill_n(_end, count - elems_after, value_copy);
				dguard.advance(count - elems_after);

				std::uninitialized_move_n(insert_pos, elems_after, insert_pos + count);
				dguard.advance(elems_after);

				std::fill_n(insert_pos, elems_after, value_copy);
			}
			else
			{
				std::uninitialized_move_n(_end - count, count, _end);
				dguard.advance(count);

				std::move_backward(insert_pos, _end - count, _end);
				std::fill_n(insert_pos, count, value_copy);
			}

			dguard.release();
			_end += count;
		}
		else
		{
			const size_type new_cap = calculate_growth(old_size + count);
			pointer new_begin = allocate(new_cap);
			pointer insert_pos = new_begin + offset;

			detail::memory_guard<T> mguard(new_begin, new_cap);
			detail::destroy_guard<T> dguard(new_begin);
			detail::destroy_guard<T> fill_dguard(insert_pos);

			std::uninitialized_fill_n(insert_pos, count, value);
			fill_dguard.advance(count);

			relocate_n(_begin, offset, new_begin);
			dguard.advance(offset);

			relocate_n(_begin + offset, old_size - offset, insert_pos + count);

			fill_dguard.release();
			dguard.release();
			mguard.release();
			change_array(new_begin, old_size + count, new_cap);
		}

		return begin() + offset;
	}

	template <std::input_iterator InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last)
	{
		RAW_ASSERT(pos >= begin() && pos <= end(), "vector insert iterator outside range");

		if constexpr (std::forward_iterator<InputIt>)
		{
			const difference_type offset = pos - begin();
			const size_type count = range_to_count(first, last);

			if (count == 0)
			{
				return begin() + offset;
			}

			const size_type old_size = size();

			if (old_size + count <= capacity())
			{
				pointer insert_pos = _begin + offset;
				const size_type elems_after = old_size - offset;

				detail::destroy_guard<T> dguard(_end);

				if (count > elems_after)
				{
					InputIt mid = std::next(first, elems_after);

					std::uninitialized_copy(mid, last, _end);
					dguard.advance(count - elems_after);

					std::uninitialized_move(insert_pos, _end, _end + count - elems_after);
					dguard.advance(elems_after);

					std::copy(first, mid, insert_pos);
				}
				else
				{
					std::uninitialized_move(_end - count, _end, _end);
					dguard.advance(count);

					std::move_backward(insert_pos, _end - count, _end);
					std::copy(first, last, insert_pos);
				}

				dguard.release();
				_end += count;
			}
			else
			{
				const size_type new_cap = calculate_growth(old_size + count);
				pointer new_begin = allocate(new_cap);
				pointer insert_pos = new_begin + offset;

				detail::memory_guard<T> mguard(new_begin, new_cap);
				detail::destroy_guard<T> dguard(new_begin);
				detail::destroy_guard<T> copy_dguard(insert_pos);

				std::uninitialized_copy(first, last, insert_pos);
				copy_dguard.advance(count);

				relocate_n(_begin, offset, new_begin);
				dguard.advance(offset);

				relocate_n(_begin + offset, old_size - offset, insert_pos + count);

				copy_dguard.release();
				dguard.release();
				mguard.release();
				change_array(new_begin, old_size + count, new_cap);
			}

			return begin() + offset;
		}
		else
		{
			vector tmp(first, last);
			return insert(pos, tmp.begin(), tmp.end());
		}
	}

	iterator insert(const_iterator pos, std::initializer_list<T> ilist)
	{
		return insert(pos, ilist.begin(), ilist.end());
	}

	template <typename... Args>
	iterator emplace(const_iterator pos, Args&&... args)
	{
		RAW_ASSERT(pos >= begin() && pos <= end(), "vector emplace iterator outside range");

		const difference_type offset = pos - begin();

		if (_end != _cap)
		{
			pointer insert_pos = _begin + offset;

			if (insert_pos == _end)
			{
				std::construct_at(_end, std::forward<Args>(args)...);
				++_end;
			}
			else
			{
				T tmp(std::forward<Args>(args)...);
				std::construct_at(_end, std::move(*(_end - 1)));
				++_end;
				std::move_backward(insert_pos, _end - 2, _end - 1);
				*insert_pos = std::move(tmp);
			}
		}
		else
		{
			const size_type old_size = size();
			const size_type new_cap = calculate_growth(old_size + 1);
			pointer new_begin = allocate(new_cap);
			pointer insert_pos = new_begin + offset;

			detail::memory_guard<T> mguard(new_begin, new_cap);
			detail::destroy_guard<T> dguard(new_begin);
			detail::destroy_guard<T> elem_dguard(insert_pos);

			std::construct_at(insert_pos, std::forward<Args>(args)...);
			elem_dguard.advance(1);

			relocate_n(_begin, offset, new_begin);
			dguard.advance(offset);

			relocate_n(_begin + offset, old_size - offset, insert_pos + 1);

			elem_dguard.release();
			dguard.release();
			mguard.release();
			change_array(new_begin, old_size + 1, new_cap);
		}

		return begin() + offset;
	}

	iterator erase(const_iterator pos)
	{
		return erase(pos, pos + 1);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		RAW_ASSERT(first >= begin() && first <= last && last <= end(), "vector erase iterator outside range");

		const difference_type offset_first = first - cbegin();
		const difference_type offset_last = last - cbegin();
		pointer erase_first = _begin + offset_first;
		pointer erase_last = _begin + offset_last;

		if (erase_first != erase_last)
		{
			pointer new_end = std::move(erase_last, _end, erase_first);
			std::destroy(new_end, _end);
			_end = new_end;
		}

		return iterator(erase_first);
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
		if (_end != _cap)
		{
			std::construct_at(_end, std::forward<Args>(args)...);
			++_end;
		}
		else
		{
			const size_type old_size = size();
			const size_type new_cap = calculate_growth(old_size + 1);
			pointer new_begin = allocate(new_cap);
			pointer insert_pos = new_begin + old_size;

			detail::memory_guard<T> mguard(new_begin, new_cap);
			detail::destroy_guard<T> dguard(insert_pos);

			std::construct_at(insert_pos, std::forward<Args>(args)...);
			dguard.advance(1);

			relocate_n(_begin, old_size, new_begin);

			dguard.release();
			mguard.release();
			change_array(new_begin, old_size + 1, new_cap);
		}

		return back();
	}

	void pop_back()
	{
		RAW_ASSERT(!empty(), "pop_back() called on empty vector");
		--_end;
		std::destroy_at(_end);
	}

	void resize(size_type count)
	{
		const size_type old_size = size();

		if (count < old_size)
		{
			std::destroy_n(_begin + count, old_size - count);
		}
		else if (count > old_size)
		{
			if (count <= capacity())
			{
				std::uninitialized_value_construct_n(_end, count - old_size);
			}
			else
			{
				if (count > max_size())
				{
					xlength();
				}

				const size_type new_cap = calculate_growth(count);
				reallocate(new_cap);
				std::uninitialized_value_construct_n(_end, count - old_size);
			}
		}

		_end = _begin + count;
	}

	void resize(size_type count, const T& value)
	{
		const size_type old_size = size();

		if (count < old_size)
		{
			std::destroy_n(_begin + count, old_size - count);
		}
		else if (count > old_size)
		{
			if (count <= capacity())
			{
				std::uninitialized_fill_n(_end, count - old_size, value);
			}
			else
			{
				if (count > max_size())
				{
					xlength();
				}

				const size_type new_cap = calculate_growth(count);
				pointer new_begin = allocate(new_cap);

				detail::memory_guard<T> mguard(new_begin, new_cap);
				detail::destroy_guard<T> dguard(new_begin);
				detail::destroy_guard<T> fill_dguard(new_begin + old_size);

				std::uninitialized_fill_n(new_begin + old_size, count - old_size, value);
				fill_dguard.advance(count - old_size);

				relocate_n(_begin, old_size, new_begin);
				dguard.advance(old_size);

				fill_dguard.release();
				dguard.release();
				mguard.release();
				change_array(new_begin, count, new_cap);

				return;
			}
		}

		_end = _begin + count;
	}

	void swap(vector& other) noexcept
	{
		if (this != std::addressof(other))
		{
			using std::swap;
			swap(_begin, other._begin);
			swap(_end, other._end);
			swap(_cap, other._cap);
		}
	}

private:
	[[noreturn]] static void xlength()
	{
		throw std::length_error("vector too long");
	}

	[[noreturn]] static void xrange()
	{
		throw std::out_of_range("invalid vector subscript");
	}

	template <std::forward_iterator ForwardIt>
	[[nodiscard]] size_type range_to_count(ForwardIt first, ForwardIt last) const
	{
		const size_type length = static_cast<size_type>(std::distance(first, last));

		if (length > max_size())
		{
			xlength();
		}

		return length;
	}

	[[nodiscard]] size_type calculate_growth(size_type new_size) const
	{
		const size_type old_cap = capacity();
		const size_type max_cap = max_size();

		if (old_cap > max_cap - old_cap / 2)
		{
			return max_cap;
		}

		const size_type geometric = old_cap + old_cap / 2;

		return geometric < new_size ? new_size : geometric;
	}

	[[nodiscard]] pointer allocate(size_type count)
	{
		if (count == 0)
		{
			return nullptr;
		}

		return static_cast<pointer>(::operator new(count * sizeof(T), std::align_val_t{ alignof(T) }));
	}

	void deallocate(pointer ptr, size_type count)
	{
		if (ptr)
		{
			::operator delete(ptr, count * sizeof(T), std::align_val_t{ alignof(T) });
		}
	}

	template <typename... Args>
	void construct_n(size_type count, Args&&... args)
	{
		if (count == 0)
		{
			return;
		}

		pointer new_begin = allocate(count);
		detail::memory_guard<T> mguard(new_begin, count);

		if constexpr (sizeof...(args) == 0)
		{
			std::uninitialized_value_construct_n(new_begin, count);
		}
		else if constexpr (sizeof...(args) == 1)
		{
			std::uninitialized_fill_n(new_begin, count, std::forward<Args>(args)...);
		}
		else if constexpr (sizeof...(args) == 2)
		{
			std::uninitialized_copy(std::forward<Args>(args)..., new_begin);
		}
		else
		{
			static_assert(sizeof...(args) <= 2, "construct_n supports at most 2 arguments");
		}

		mguard.release();
		change_array(new_begin, count, count);
	}

	void relocate_n(pointer first, size_type count, pointer dest)
	{
		if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>)
		{
			std::uninitialized_move_n(first, count, dest);
		}
		else
		{
			std::uninitialized_copy_n(first, count, dest);
		}
	}

	void reallocate(size_type new_cap)
	{
		if (new_cap == 0)
		{
			return;
		}

		pointer new_begin = allocate(new_cap);
		const size_type new_size = std::min(size(), new_cap);

		detail::memory_guard<T> mguard(new_begin, new_cap);

		relocate_n(_begin, new_size, new_begin);

		mguard.release();
		change_array(new_begin, new_size, new_cap);
	}

	void change_array(pointer new_begin, size_type new_size, size_type new_cap)
	{
		if (_begin)
		{
			std::destroy(_begin, _end);
			deallocate(_begin, capacity());
		}

		_begin = new_begin;
		_end = new_begin + new_size;
		_cap = new_begin + new_cap;
	}

	void tidy()
	{
		change_array(nullptr, 0, 0);
	}

	pointer _begin;
	pointer _end;
	pointer _cap;
};

// ---------- Non-member functions ---------- //

template <typename T>
[[nodiscard]] constexpr bool operator==(const vector<T>& lhs, const vector<T>& rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T>
[[nodiscard]] constexpr auto operator<=>(const vector<T>& lhs, const vector<T>& rhs)
{
	return std::lexicographical_compare_three_way(
		lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::compare_three_way{});
}

template <typename T>
constexpr void swap(vector<T>& lhs, vector<T>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
	lhs.swap(rhs);
}

template <typename T, typename U>
constexpr typename vector<T>::size_type erase(vector<T>& c, const U& value)
{
	auto it = std::remove(c.begin(), c.end(), value);
	auto r = c.end() - it;
	c.erase(it, c.end());
	return r;
}

template <typename T, typename Pred>
constexpr typename vector<T>::size_type erase_if(vector<T>& c, Pred pred)
{
	auto it = std::remove_if(c.begin(), c.end(), pred);
	auto r = c.end() - it;
	c.erase(it, c.end());
	return r;
}

// ---------- Deduction guides ---------- //

template <std::input_iterator InputIt>
vector(InputIt, InputIt) -> vector<std::iter_value_t<InputIt>>;

} // namespace raw
