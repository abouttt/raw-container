#include <iostream>

#include "raw/array.h"
#include "raw/vector.h"

int main()
{
	raw::vector<int> v1{ 1, 2, 3, 4, 5 };
	raw::vector<int> v2{ 1, 2, 3 };

	v1.assign(v2.begin(), v2.end());

	std::cout << v1.capacity();

	return 0;
}
