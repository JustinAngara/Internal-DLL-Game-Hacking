#pragma once
#include <vector>


template <typename Func, typename... Args>
auto executeFlexible(Func function, Args... args) -> decltype(function(args...)) {
	std::cout << "" << std::endl;
	return function(args...);
}

using funcObfuscate = int (*)(int, int);
namespace ObfuscationRegistry {
	extern std::vector<funcObfuscate> methods;  
	void Setup();
}

