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

	memory_guard(pointer ptr, size_type count) noexcept
		: _ptr(ptr)
		, _count(count)
	{
	}

	~memory_guard()
	{
		if (_ptr)
		{
			::operator delete(_ptr, _count * sizeof(T), std::align_val_t{ alignof(T) });
		}
	}

	memory_guard(const memory_guard& other) = delete;
	memory_guard(memory_guard&& other) = delete;
	memory_guard& operator=(const memory_guard& other) = delete;
	memory_guard& operator=(memory_guard&& other) = delete;

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
		_count = 0;
	}

private:
	pointer _ptr;
	size_type _count;
};

} // namespace raw::detail
