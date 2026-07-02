#pragma once

#include <cstddef>
#include <limits>
#include <memory>
#include <new>

namespace raw::detail
{

template <typename T>
[[nodiscard]] T* allocate(std::size_t count)
{
	if (count == 0)
	{
		return nullptr;
	}

	constexpr std::size_t max_size = std::numeric_limits<std::size_t>::max() / sizeof(T);

	if (count > max_size)
	{
		throw std::bad_array_new_length();
	}

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
void deallocate(T* ptr, std::size_t count) noexcept
{
	if (!ptr)
	{
		return;
	}

	constexpr std::size_t alignment = alignof(T);
	const std::size_t size = count * sizeof(T);

	if constexpr (alignment > alignof(std::max_align_t))
	{
		::operator delete(ptr, size, std::align_val_t{ alignment });
	}
	else
	{
		::operator delete(ptr, size);
	}
}

template <typename T, typename Deleter>
class guard
{
public:
	guard(T* ptr, std::size_t count) noexcept
		: m_ptr(ptr)
		, m_count(count)
	{
	}

	~guard() noexcept
	{
		if (m_ptr)
		{
			Deleter{}(m_ptr, m_count);
		}
	}

	guard(const guard&) = delete;
	guard(guard&&) = delete;

	guard& operator=(const guard&) = delete;
	guard& operator=(guard&&) = delete;

	[[nodiscard]] T* get() const noexcept
	{
		return m_ptr;
	}

	[[nodiscard]] std::size_t count() const noexcept
	{
		return m_count;
	}

	void release() noexcept
	{
		m_ptr = nullptr;
		m_count = 0;
	}

private:
	T* m_ptr;
	std::size_t m_count;
};

template <typename Node>
class node_guard
{
public:
	explicit node_guard(Node* head) noexcept
		: m_head(head)
	{
	}

	~node_guard() noexcept
	{
		while (m_head)
		{
			Node* next = static_cast<Node*>(m_head->next);
			std::destroy_at(m_head);
			deallocate<Node>(m_head, 1);
			m_head = next;
		}
	}

	node_guard(const node_guard&) = delete;
	node_guard(node_guard&&) = delete;

	node_guard& operator=(const node_guard&) = delete;
	node_guard& operator=(node_guard&&) = delete;

	[[nodiscard]] Node* get() const noexcept
	{
		return m_head;
	}

	void release() noexcept
	{
		m_head = nullptr;
	}

private:
	Node* m_head;
};

struct memory_deleter
{
	template <typename T>
	void operator()(T* ptr, std::size_t count) const noexcept
	{
		deallocate(ptr, count);
	}
};

template <typename T>
using memory_guard = guard<T, memory_deleter>;

struct object_deleter
{
	template <typename T>
	void operator()(T* ptr, std::size_t count) const noexcept
	{
		std::destroy_n(ptr, count);
	}
};

template <typename T>
using object_guard = guard<T, object_deleter>;

} // namespace raw::detail
