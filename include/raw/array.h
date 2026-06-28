#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "detail/assert.h"
#include "detail/compare.h"

namespace raw
{

namespace detail
{

template <typename T>
class array_iterator
{
public:
	using iterator_concept = std::contiguous_iterator_tag;
	using iterator_category = std::random_access_iterator_tag;
	using value_type = std::remove_cv_t<T>;
	using difference_type = std::ptrdiff_t;
	using pointer = T*;
	using reference = T&;

	constexpr array_iterator() noexcept
		: m_ptr(nullptr)
	{
	}

	constexpr explicit array_iterator(pointer ptr) noexcept
		: m_ptr(ptr)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	constexpr array_iterator(const array_iterator<U>& other) noexcept
		: m_ptr(other.m_ptr)
	{
	}

	[[nodiscard]] constexpr reference operator*() const noexcept
	{
		return *m_ptr;
	}

	[[nodiscard]] constexpr pointer operator->() const noexcept
	{
		return m_ptr;
	}

	constexpr array_iterator& operator++() noexcept
	{
		++m_ptr;
		return *this;
	}

	constexpr array_iterator operator++(int) noexcept
	{
		array_iterator tmp = *this;
		++m_ptr;
		return tmp;
	}

	constexpr array_iterator& operator--() noexcept
	{
		--m_ptr;
		return *this;
	}

	constexpr array_iterator operator--(int) noexcept
	{
		array_iterator tmp = *this;
		--m_ptr;
		return tmp;
	}

	constexpr array_iterator& operator+=(difference_type n) noexcept
	{
		m_ptr += n;
		return *this;
	}

	[[nodiscard]] constexpr array_iterator operator+(difference_type n) const noexcept
	{
		return array_iterator(m_ptr + n);
	}

	[[nodiscard]] friend constexpr array_iterator operator+(difference_type n, const array_iterator& it) noexcept
	{
		return array_iterator(it.m_ptr + n);
	}

	constexpr array_iterator& operator-=(difference_type n) noexcept
	{
		m_ptr -= n;
		return *this;
	}

	[[nodiscard]] constexpr array_iterator operator-(difference_type n) const noexcept
	{
		return array_iterator(m_ptr - n);
	}

	template <typename U>
	[[nodiscard]] constexpr difference_type operator-(const array_iterator<U>& other) const noexcept
	{
		return m_ptr - other.m_ptr;
	}

	[[nodiscard]] constexpr reference operator[](difference_type n) const noexcept
	{
		return *(m_ptr + n);
	}

	template <typename U>
	[[nodiscard]] constexpr bool operator==(const array_iterator<U>& other) const noexcept
	{
		return m_ptr == other.m_ptr;
	}

	template <typename U>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const array_iterator<U>& other) const noexcept
	{
		return m_ptr <=> other.m_ptr;
	}

private:
	template <typename>
	friend class array_iterator;

	pointer m_ptr;
};

} // namespace detail

template <typename T, std::size_t N>
struct array
{
	static_assert(std::is_object_v<T>, "T must be an object type");

	// ---------- Types ---------- //

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using iterator = detail::array_iterator<T>;
	using const_iterator = detail::array_iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Element access ---------- //

	[[nodiscard]] constexpr reference at(size_type pos)
	{
		if (pos >= N)
		{
			throw_out_of_range();
		}

		return elems[pos];
	}

	[[nodiscard]] constexpr const_reference at(size_type pos) const
	{
		if (pos >= N)
		{
			throw_out_of_range();
		}

		return elems[pos];
	}

	[[nodiscard]] constexpr reference operator[](size_type pos)
	{
		RAW_ASSERT(pos < N, "array subscript out of range");
		return elems[pos];
	}

	[[nodiscard]] constexpr const_reference operator[](size_type pos) const
	{
		RAW_ASSERT(pos < N, "array subscript out of range");
		return elems[pos];
	}

	[[nodiscard]] constexpr reference front()
	{
		return elems[0];
	}

	[[nodiscard]] constexpr const_reference front() const
	{
		return elems[0];
	}

	[[nodiscard]] constexpr reference back()
	{
		return elems[N - 1];
	}

	[[nodiscard]] constexpr const_reference back() const
	{
		return elems[N - 1];
	}

	[[nodiscard]] constexpr pointer data() noexcept
	{
		return elems;
	}

	[[nodiscard]] constexpr const_pointer data() const noexcept
	{
		return elems;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] constexpr iterator begin() noexcept
	{
		return iterator(elems);
	}

	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator(elems);
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] constexpr iterator end() noexcept
	{
		return iterator(elems + N);
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator(elems + N);
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return end();
	}

	[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	[[nodiscard]] constexpr reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return false;
	}

	[[nodiscard]] constexpr size_type size() const noexcept
	{
		return N;
	}

	[[nodiscard]] constexpr size_type max_size() const noexcept
	{
		return N;
	}

	// ---------- Operations ---------- //

	constexpr void fill(const value_type& value)
	{
		std::fill_n(elems, N, value);
	}

	constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<value_type>)
	{
		if (this != std::addressof(other))
		{
			std::swap_ranges(elems, elems + N, other.elems);
		}
	}

	// ---------- Variables ---------- //

	value_type elems[N];

private:
	[[noreturn]] static constexpr void throw_out_of_range()
	{
		throw std::out_of_range("invalid array subscript");
	}
};

template <typename T>
struct array<T, 0>
{
	static_assert(std::is_object_v<T>, "T must be an object type");

	// ---------- Types ---------- //

	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = T&;
	using const_reference = const T&;
	using pointer = T*;
	using const_pointer = const T*;

	using iterator = detail::array_iterator<T>;
	using const_iterator = detail::array_iterator<const T>;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Element Access ---------- //

	[[nodiscard]] constexpr reference at(size_type)
	{
		throw_out_of_range();
	}

	[[nodiscard]] constexpr const_reference at(size_type) const
	{
		throw_out_of_range();
	}

	[[nodiscard]] constexpr reference operator[](size_type)
	{
		RAW_ASSERT(false, "array subscript out of range");
		return *data();
	}

	[[nodiscard]] constexpr const_reference operator[](size_type) const
	{
		RAW_ASSERT(false, "array subscript out of range");
		return *data();
	}

	[[nodiscard]] constexpr reference front()
	{
		RAW_ASSERT(false, "front() called on empty array");
		return *data();
	}

	[[nodiscard]] constexpr const_reference front() const
	{
		RAW_ASSERT(false, "front() called on empty array");
		return *data();
	}

	[[nodiscard]] constexpr reference back()
	{
		RAW_ASSERT(false, "back() called on empty array");
		return *data();
	}

	[[nodiscard]] constexpr const_reference back() const
	{
		RAW_ASSERT(false, "back() called on empty array");
		return *data();
	}

	[[nodiscard]] constexpr pointer data() noexcept
	{
		return nullptr;
	}

	[[nodiscard]] constexpr const_pointer data() const noexcept
	{
		return nullptr;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] constexpr iterator begin() noexcept
	{
		return iterator{};
	}

	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator{};
	}

	[[nodiscard]] constexpr const_iterator cbegin() const noexcept
	{
		return begin();
	}

	[[nodiscard]] constexpr iterator end() noexcept
	{
		return iterator{};
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{};
	}

	[[nodiscard]] constexpr const_iterator cend() const noexcept
	{
		return end();
	}

	[[nodiscard]] constexpr reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	[[nodiscard]] constexpr reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	// ---------- Capacity ---------- //

	[[nodiscard]] constexpr bool empty() const noexcept
	{
		return true;
	}

	[[nodiscard]] constexpr size_type size() const noexcept
	{
		return 0;
	}

	[[nodiscard]] constexpr size_type max_size() const noexcept
	{
		return 0;
	}

	// ---------- Operations ---------- //

	constexpr void fill(const value_type&)
	{
	}

	constexpr void swap(array&) noexcept
	{
	}

private:
	[[noreturn]] static constexpr void throw_out_of_range()
	{
		throw std::out_of_range("invalid array subscript");
	}
};

// ---------- Non-member functions ---------- //

template <typename T, std::size_t N>
[[nodiscard]] constexpr bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr detail::synth_three_way_result<T> operator<=>(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), detail::synth_three_way{});
}

template <typename T, std::size_t N>
constexpr void swap(array<T, N>& lhs, array<T, N>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
	lhs.swap(rhs);
}

template <std::size_t I, typename T, std::size_t N>
[[nodiscard]] constexpr T& get(array<T, N>& a) noexcept
{
	static_assert(I < N, "array index out of bounds");
	return a[I];
}

template <std::size_t I, typename T, std::size_t N>
[[nodiscard]] constexpr const T& get(const array<T, N>& a) noexcept
{
	static_assert(I < N, "array index out of bounds");
	return a[I];
}

template <std::size_t I, typename T, std::size_t N>
[[nodiscard]] constexpr T&& get(array<T, N>&& a) noexcept
{
	static_assert(I < N, "array index out of bounds");
	return std::move(a[I]);
}

template <std::size_t I, typename T, std::size_t N>
[[nodiscard]] constexpr const T&& get(const array<T, N>&& a) noexcept
{
	static_assert(I < N, "array index out of bounds");
	return std::move(a[I]);
}

namespace detail
{

template <typename T, std::size_t N, std::size_t... I>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array_lvalue_impl(T (&a)[N], std::index_sequence<I...>)
{
	return { {a[I]...} };
}

template <typename T, std::size_t N, std::size_t... I>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array_rvalue_impl(T (&&a)[N], std::index_sequence<I...>)
{
	return { { std::move(a[I])... } };
}

} // namespace detail

template <typename T, std::size_t N>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array(T (&a)[N])
{
	static_assert(!std::is_array_v<T>, "to_array does not accept multidimensional arrays");
	static_assert(std::is_constructible_v<T, T&>, "to_array requires copy constructible elements");
	return detail::to_array_lvalue_impl(a, std::make_index_sequence<N>{});
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array(T (&&a)[N])
{
	static_assert(!std::is_array_v<T>, "to_array does not accept multidimensional arrays");
	static_assert(std::is_move_constructible_v<T>, "to_array requires move constructible elements");
	return detail::to_array_rvalue_impl(std::move(a), std::make_index_sequence<N>{});
}

// ---------- Deduction guides ---------- //

template <typename T, typename... U>
	requires (std::same_as<T, U> && ...)
array(T, U...) -> array<T, 1 + sizeof...(U)>;

} // namespace raw

// ---------- Helper classes ---------- //

namespace std
{

template <typename T, std::size_t N>
struct tuple_size<raw::array<T, N>> : std::integral_constant<std::size_t, N>
{
};

template <std::size_t I, typename T, std::size_t N>
struct tuple_element<I, raw::array<T, N>>
{
	using type = T;
};

} // namespace std
