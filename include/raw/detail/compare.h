#pragma once

#include <compare>
#include <concepts>
#include <utility>

namespace raw::detail
{

struct synth_three_way
{
	template <typename T, typename U>
		requires std::three_way_comparable_with<T, U>
	[[nodiscard]] constexpr auto operator()(const T& t, const U& u) const noexcept(noexcept(t <=> u))
	{
		return t <=> u;
	}

	template <typename T, typename U>
		requires (!std::three_way_comparable_with<T, U>) && requires(const T& t, const U& u)
	{
		{ t < u } -> std::convertible_to<bool>;
		{ u < t } -> std::convertible_to<bool>;
	}
	[[nodiscard]] constexpr auto operator()(const T& t, const U& u) const noexcept(noexcept(t < u) && noexcept(u < t))
	{
		if (t < u)
		{
			return std::weak_ordering::less;
		}

		if (u < t)
		{
			return std::weak_ordering::greater;
		}

		return std::weak_ordering::equivalent;
	}
};

template <typename T, typename U = T>
using synth_three_way_result = decltype(synth_three_way{}(
	std::declval<const std::remove_cvref_t<T>&>(), std::declval<const std::remove_cvref_t<U>&>()));

} // namespace raw::detail
