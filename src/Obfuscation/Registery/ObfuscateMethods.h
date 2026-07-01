#pragma once
#include <vector>


template <typename Func, typename... Args>
auto executeFlexible(Func function, Args... args) -> decltype(function(args...)) {
	std::std::cout << "" << std::endl;
	return function(args...);
}

using funcObfuscate = int (*)(int, int);
namespace ObfuscationRegistry
{

	// add the pointers into the vector to obfuscate
	std::vector<funcObfuscate> methods;
	void Setup();
}


