#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include "raw/array.h"

int main()
{
	raw::array<int, 3> a1{ {1, 2, 3} };
	raw::array<int, 3> a2 = { 1, 2, 3 };

	std::sort(a1.begin(), a1.end());
	std::ranges::reverse_copy(a2, std::ostream_iterator<int>(std::cout, " "));
	std::cout << '\n';

	raw::array<std::string, 2> a3{ "E", "\u018E" };
	for (const auto& s : a3)
	{
		std::cout << s << ' ';
	}
	std::cout << '\n';

	[[maybe_unused]] raw::array a4{ 3.0, 1.0, 4.0 };
	[[maybe_unused]] raw::array<int, 2> a5;
	[[maybe_unused]] raw::array<int, 2> a6{};
	[[maybe_unused]] raw::array<int, 2> a7{ 1 };
}
