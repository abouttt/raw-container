#pragma once

#include <cstddef>
#include <new>

namespace raw::detail
{

template <typename T>
[[nodiscard]] inline T* allocate(std::size_t count)
{
	constexpr std::size_t alignment = alignof(T);
	const std::size_t size = count * sizeof(T);

	if constexpr (alignment > alignof(std::max_align_t))
	{
		return static_cast<T*>(::operator new(size, std::align_val_t{ alignment }));
	}
	else
	{
		return static_cast<T*>(::operator new(size));
	}
}

template <typename T>
void deallocate(T* ptr, std::size_t sz) noexcept
{
	constexpr std::size_t alignment = alignof(T);
	const std::size_t size = sz * sizeof(T);

	if constexpr (alignment > alignof(std::max_align_t))
	{
		::operator delete(ptr, size, std::align_val_t{ alignment });
	}
	else
	{
		::operator delete(ptr, size);
	}
}

} // namespace raw::detail
