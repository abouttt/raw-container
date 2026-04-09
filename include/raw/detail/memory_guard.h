#pragma once

#include <cstddef>
#include <new>

namespace raw::detail
{

template <typename T>
class memory_guard
{
public:
	using value_type = T;
	using size_type = std::size_t;
	using pointer = T*;

	explicit memory_guard(pointer ptr, size_type count) noexcept
		: _ptr(ptr)
		, _count(count)
	{
	}

	memory_guard(const memory_guard& other) = delete;
	memory_guard& operator=(const memory_guard& other) = delete;

	~memory_guard()
	{
		if (_ptr)
		{
			::operator delete(_ptr, _count * sizeof(T), std::align_val_t{ alignof(T) });
		}
	}

	[[nodiscard]] pointer get() const noexcept
	{
		return _ptr;
	}

	[[nodiscard]] size_type count() const noexcept
	{
		return _count;
	}

	void release() noexcept
	{
		_ptr = nullptr;
	}

private:
	pointer _ptr;
	const size_type _count;
};

} // namespace raw::detail
