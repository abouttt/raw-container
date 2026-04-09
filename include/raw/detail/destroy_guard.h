#pragma once

#include <cstddef>
#include <memory>

namespace raw::detail
{

template <typename T>
class destroy_guard
{
public:
	using value_type = T;
	using size_type = std::size_t;
	using pointer = T*;

	explicit destroy_guard(pointer ptr, size_type count = 0) noexcept
		: _ptr(ptr)
		, _count(count)
	{
	}

	destroy_guard(const destroy_guard&) = delete;
	destroy_guard& operator=(const destroy_guard&) = delete;

	~destroy_guard()
	{
		if (_ptr)
		{
			std::destroy_n(_ptr, _count);
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

	void set_count(size_type count) noexcept
	{
		_count = count;
	}

	void advance(size_type count) noexcept
	{
		_count += count;
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
