#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include "memory.h"

namespace raw::detail
{

template <typename T>
class memory_guard
{
public:
	explicit memory_guard(T* ptr, std::size_t count) noexcept
		: _ptr(ptr)
		, _size(count)
	{
	}

	memory_guard(const memory_guard&) = delete;

	memory_guard(memory_guard&& other) noexcept
		: _ptr(std::exchange(other._ptr, nullptr))
		, _size(std::exchange(other._size, 0))
	{
	}

	~memory_guard()
	{
		if (_ptr)
		{
			deallocate(_ptr, _size);
		}
	}

	memory_guard& operator=(const memory_guard&) = delete;

	memory_guard& operator=(memory_guard&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			if (_ptr)
			{
				deallocate(_ptr, _size);
			}

			_ptr = std::exchange(other._ptr, nullptr);
			_size = std::exchange(other._size, 0);
		}

		return *this;
	}

	[[nodiscard]] T* get() const noexcept
	{
		return _ptr;
	}

	[[nodiscard]] std::size_t size() const noexcept
	{
		return _size;
	}

	void release() noexcept
	{
		_ptr = nullptr;
		_size = 0;
	}

private:
	T* _ptr;
	std::size_t _size;
};

template <typename T>
class destroy_guard
{
public:
	explicit destroy_guard(T* first, T* last) noexcept
		: _begin(first)
		, _end(last)
	{
	}

	destroy_guard(const destroy_guard&) = delete;

	destroy_guard(destroy_guard&& other) noexcept
		: _begin(std::exchange(other._begin, nullptr))
		, _end(std::exchange(other._end, nullptr))
	{
	}

	~destroy_guard()
	{
		if (_begin)
		{
			std::destroy(_begin, _end);
		}
	}

	destroy_guard& operator=(const destroy_guard&) = delete;

	destroy_guard& operator=(destroy_guard&& other) noexcept
	{
		if (this != std::addressof(other))
		{
			if (_begin)
			{
				std::destroy(_begin, _end);
			}

			_begin = std::exchange(other._begin, nullptr);
			_end = std::exchange(other._end, nullptr);
		}

		return *this;
	}

	[[nodiscard]] T* begin() const noexcept
	{
		return _begin;
	}

	[[nodiscard]] T* end() const noexcept
	{
		return _end;
	}

	void set_end(T* new_end) noexcept
	{
		_end = new_end;
	}

	void release() noexcept
	{
		_begin = nullptr;
		_end = nullptr;
	}

private:
	T* _begin;
	T* _end;
};

} // namespace raw::detail
