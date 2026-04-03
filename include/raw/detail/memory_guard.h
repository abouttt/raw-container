#pragma once

#include <cstddef>
#include <new>

namespace raw::detail
{

template <typename T>
class memory_guard
{
public:
	using size_type = std::size_t;
	using pointer = T*;

	memory_guard(pointer ptr, size_type bytes) noexcept
		: _ptr(ptr)
		, _bytes(bytes)
	{
	}

	~memory_guard()
	{
		if (_ptr)
		{
			::operator delete(_ptr, _bytes, std::align_val_t{ alignof(T) });
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

	[[nodiscard]] size_type bytes() const noexcept
	{
		return _bytes;
	}

	void release() noexcept
	{
		_ptr = nullptr;
		_bytes = 0;
	}

private:
	pointer _ptr;
	size_type _bytes;
};

} // namespace raw::detail
