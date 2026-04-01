#include <iostream>

#include "raw/array.h"

int main()
{
	raw::array<int, 5> arr{ 1, 2, 3, 4, 5 };

	for (int val : arr)
	{
		std::cout << val << ' ';
	}

	return 0;
}
