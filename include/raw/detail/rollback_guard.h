#pragma once

#include <cstddef>
#include <memory>

namespace raw::detail
{

template <typename T>
class rollback_guard
{
public:
	using size_type = std::size_t;
	using pointer = T*;

	explicit rollback_guard(pointer begin) noexcept
		: _begin(begin)
		, _end(begin)
	{
	}

	~rollback_guard()
	{
		if (_begin)
		{
			std::destroy(_begin, _end);
		}
	}

	rollback_guard(const rollback_guard& other) = delete;
	rollback_guard(rollback_guard&& other) = delete;
	rollback_guard& operator=(const rollback_guard& other) = delete;
	rollback_guard& operator=(rollback_guard&& other) = delete;

	void set_end(pointer end) noexcept
	{
		_end = end;
	}

	void advance(size_type count) noexcept
	{
		_end += count;
	}

	[[nodiscard]] pointer begin() const noexcept
	{
		return _begin;
	}

	[[nodiscard]] pointer end() const noexcept
	{
		return _end;
	}

	[[nodiscard]] size_type size() const noexcept
	{
		return static_cast<size_type>(_end - _begin);
	}

	void release() noexcept
	{
		_begin = nullptr;
		_end = nullptr;
	}

private:
	pointer _begin;
	pointer _end;
};

} // namespace raw::detail
