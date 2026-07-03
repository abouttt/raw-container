#include <algorithm>
#include <iostream>

#include "raw/forward_list.h"

int main()
{
	raw::forward_list<int> fl = { 7, 5, 16, 8 };

	fl.push_front(25);
	fl.push_front(13);

	auto it = std::find(fl.begin(), fl.end(), 16);
	if (it != fl.end())
	{
		fl.insert_after(it, 42);
	}

	std::cout << "fl = { ";
	for (int n : fl)
	{
		std::cout << n << ", ";
	}
	std::cout << "};\n";
}
