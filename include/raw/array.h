#pragma once

#include <algorithm>
#include <compare>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace raw
{

namespace detail
{

template <typename T>
class array_iterator
{
public:
	using iterator_concept	= std::contiguous_iterator_tag;
	using iterator_category = std::random_access_iterator_tag;
	using value_type		= std::remove_cv_t<T>;
	using difference_type	= std::ptrdiff_t;
	using pointer			= T*;
	using reference			= T&;

	constexpr array_iterator() noexcept
		: _ptr(nullptr)
	{
	}

	constexpr explicit array_iterator(pointer ptr) noexcept
		: _ptr(ptr)
	{
	}

	template <typename U>
		requires std::convertible_to<U*, pointer>
	constexpr array_iterator(const array_iterator<U>& other) noexcept
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

	constexpr array_iterator& operator++() noexcept
	{
		++_ptr;
		return *this;
	}

	constexpr array_iterator operator++(int) noexcept
	{
		array_iterator tmp = *this;
		++_ptr;
		return tmp;
	}

	constexpr array_iterator& operator--() noexcept
	{
		--_ptr;
		return *this;
	}

	constexpr array_iterator operator--(int) noexcept
	{
		array_iterator tmp = *this;
		--_ptr;
		return tmp;
	}

	constexpr array_iterator& operator+=(difference_type n) noexcept
	{
		_ptr += n;
		return *this;
	}

	[[nodiscard]] constexpr array_iterator operator+(difference_type n) const noexcept
	{
		return array_iterator(_ptr + n);
	}

	[[nodiscard]] friend constexpr array_iterator operator+(difference_type n, const array_iterator& it) noexcept
	{
		return array_iterator(it._ptr + n);
	}

	constexpr array_iterator& operator-=(difference_type n) noexcept
	{
		_ptr -= n;
		return *this;
	}

	[[nodiscard]] constexpr array_iterator operator-(difference_type n) const noexcept
	{
		return array_iterator(_ptr - n);
	}

	template <typename U>
	[[nodiscard]] constexpr difference_type operator-(const array_iterator<U>& other) const noexcept
	{
		return _ptr - other._ptr;
	}

	[[nodiscard]] constexpr reference operator[](difference_type n) const noexcept
	{
		return *(_ptr + n);
	}

	template <typename U>
	[[nodiscard]] constexpr bool operator==(const array_iterator<U>& other) const noexcept
	{
		return _ptr == other._ptr;
	}

	template <typename U>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const array_iterator<U>& other) const noexcept
	{
		return _ptr <=> other._ptr;
	}

private:
	template <typename>
	friend class array_iterator;

	pointer _ptr;
};

} // namespace detail

template <typename T, std::size_t N>
struct array
{
	static_assert(std::is_object_v<T>, "array<T, N> element type must be an object");

	// ---------- Types ---------- //

	using value_type	  = T;
	using size_type		  = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference		  = T&;
	using const_reference = const T&;
	using pointer		  = T*;
	using const_pointer	  = const T*;

	using iterator				 = detail::array_iterator<T>;
	using const_iterator		 = detail::array_iterator<const T>;
	using reverse_iterator		 = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Element Access ---------- //

	[[nodiscard]] constexpr reference at(size_type pos)
	{
		if (pos >= N)
		{
			xrange();
		}

		return _elems[pos];
	}

	[[nodiscard]] constexpr const_reference at(size_type pos) const
	{
		if (pos >= N)
		{
			xrange();
		}

		return _elems[pos];
	}

	[[nodiscard]] constexpr reference operator[](size_type pos)
	{
		return _elems[pos];
	}

	[[nodiscard]] constexpr const_reference operator[](size_type pos) const
	{
		return _elems[pos];
	}

	[[nodiscard]] constexpr reference front()
	{
		return _elems[0];
	}

	[[nodiscard]] constexpr const_reference front() const
	{
		return _elems[0];
	}

	[[nodiscard]] constexpr reference back()
	{
		return _elems[N - 1];
	}

	[[nodiscard]] constexpr const_reference back() const
	{
		return _elems[N - 1];
	}

	[[nodiscard]] constexpr pointer data() noexcept
	{
		return _elems;
	}

	[[nodiscard]] constexpr const_pointer data() const noexcept
	{
		return _elems;
	}

	// ---------- Iterators ---------- //

	[[nodiscard]] constexpr iterator begin() noexcept
	{
		return iterator(_elems);
	}

	[[nodiscard]] constexpr const_iterator begin() const noexcept
	{
		return const_iterator(_elems);
	}

	[[nodiscard]] constexpr iterator end() noexcept
	{
		return iterator(_elems + N);
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator(_elems + N);
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

	constexpr void fill(const T& value)
	{
		std::fill_n(_elems, N, value);
	}

	constexpr void swap(array& other) noexcept(std::is_nothrow_swappable_v<T>)
	{
		std::swap_ranges(_elems, _elems + N, other._elems);
	}

	// ---------- Variables ---------- //

	T _elems[N];

private:
	[[noreturn]] static void xrange()
	{
		throw std::out_of_range("invalid array<T, N> subscript");
	}
};

template <typename T>
struct array<T, 0>
{
	static_assert(std::is_object_v<T>, "array<T, 0> element type must be an object");

	// ---------- Types ---------- //

	using value_type	  = T;
	using size_type		  = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference		  = T&;
	using const_reference = const T&;
	using pointer		  = T*;
	using const_pointer	  = const T*;

	using iterator				 = detail::array_iterator<T>;
	using const_iterator		 = detail::array_iterator<const T>;
	using reverse_iterator		 = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	// ---------- Element Access ---------- //

	[[noreturn]] reference at(size_type /*pos*/)
	{
		xrange();
	}

	[[noreturn]] const_reference at(size_type /*pos*/) const
	{
		xrange();
	}

	[[nodiscard]] reference operator[](size_type /*pos*/)
	{
		return *data();
	}

	[[nodiscard]] const_reference operator[](size_type /*pos*/) const
	{
		return *data();
	}

	[[nodiscard]] reference front()
	{
		return *data();
	}

	[[nodiscard]] const_reference front() const
	{
		return *data();
	}

	[[nodiscard]] reference back()
	{
		return *data();
	}

	[[nodiscard]] const_reference back() const
	{
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

	[[nodiscard]] constexpr iterator end() noexcept
	{
		return iterator{};
	}

	[[nodiscard]] constexpr const_iterator end() const noexcept
	{
		return const_iterator{};
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

	constexpr void fill(const T& /*value*/)
	{
	}

	constexpr void swap(array& /*other*/) noexcept
	{
	}

private:
	[[noreturn]] static void xrange()
	{
		throw std::out_of_range("invalid array<T, 0> subscript");
	}
};

// ---------- Non-member functions ---------- //

template <typename T, std::size_t N>
[[nodiscard]] constexpr bool operator==(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr auto operator<=>(const array<T, N>& lhs, const array<T, N>& rhs)
{
	return std::lexicographical_compare_three_way(
		lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), std::compare_three_way{});
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
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array_impl(T(&a)[N], std::index_sequence<I...>)
{
	return { {a[I]...} };
}

template <typename T, std::size_t N, std::size_t... I>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array_impl(T(&&a)[N], std::index_sequence<I...>)
{
	return { { std::move(a[I])... } };
}

} // namespace detail

template <typename T, std::size_t N>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array(T(&a)[N])
{
	return detail::to_array_impl(a, std::make_index_sequence<N>{});
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr array<std::remove_cv_t<T>, N> to_array(T(&&a)[N])
{
	return detail::to_array_impl(std::move(a), std::make_index_sequence<N>{});
}

// ---------- Deduction guides ---------- //

template <typename T, typename... U>
	requires(std::is_same_v<T, U> && ...)
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
