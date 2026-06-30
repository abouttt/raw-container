#include <iostream>

#include "raw/vector.h"

int main()
{
	raw::vector<int> v = { 8, 4, 5, 9 };

	v.push_back(6);
	v.push_back(9);

	v[2] = -1;

	for (int n : v)
	{
		std::cout << n << ' ';
	}
	std::cout << '\n';
}
