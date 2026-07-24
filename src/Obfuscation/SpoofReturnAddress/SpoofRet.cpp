#include <type_traits>
#include <iostream>
#include <Windows.h>
#include <intrin.h>
#include "SpoofRet.h"
#include "sdk/Memory/Memory.h"
#pragma section(".text")
__declspec(allocate(".text")) const unsigned char jmp_rbx[]{ 0xFF, 0x23 };

namespace detail
{
	extern "C" void* _spoofer_stub();

	template <typename Ret, typename... Args>
	static inline auto shellcode_stub_helper(
		const void* shell,
		Args... args
	) -> Ret
	{
		auto fn = (Ret(*)(Args...))(shell);
		return fn(args...);
	}

	template <std::size_t Argc, typename>
	struct argument_remapper
	{
		// At least 5 params
		template<
			typename Ret,
			typename First,
			typename Second,
			typename Third,
			typename Fourth,
			typename... Pack
		>
		static auto do_call(
			const void* shell,
			void* shell_param,
			First first,
			Second second,
			Third third,
			Fourth fourth,
			Pack... pack
		) -> Ret
		{
			return shellcode_stub_helper<
				Ret,
				First,
				Second,
				Third,
				Fourth,
				void*,
				void*,
				Pack...
			>(
				shell,
				first,
				second,
				third,
				fourth,
				shell_param,
				nullptr,
				pack...
			);
		}
	};

	template <std::size_t Argc>
	struct argument_remapper<Argc, std::enable_if_t<Argc <= 4>>
	{
		// 4 or less params
		template<
			typename Ret,
			typename First = void*,
			typename Second = void*,
			typename Third = void*,
			typename Fourth = void*
		>
		static auto do_call(
			const void* shell,
			void* shell_param,
			First first = First{},
			Second second = Second{},
			Third third = Third{},
			Fourth fourth = Fourth{}
		) -> Ret
		{
			return shellcode_stub_helper<
				Ret,
				First,
				Second,
				Third,
				Fourth,
				void*,
				void*
			>(
				shell,
				first,
				second,
				third,
				fourth,
				shell_param,
				nullptr
			);
		}
	};
}

static void* spoof_trampoline = nullptr;


template <typename Ret, typename... Args>
static inline auto spoof_call(
	Ret(*fn)(Args...),
	Args... args
) -> Ret
{
	struct shell_params
	{
		const void* trampoline;
		void* function;
		void* rbx;
	};

	shell_params p{ spoof_trampoline, reinterpret_cast<void*>(fn) };
	using mapper = detail::argument_remapper<sizeof...(Args), void>;
	return mapper::template do_call<Ret, Args...>((const void*)&detail::_spoofer_stub, &p, args...);
}



//////////////////////////////// sooon remove this and refactor somewhere else
//////////////////////////////// currently just tester to test spoof ret



void add_num(int a, int b)
{
	void* ret = _ReturnAddress();

	printf("ret = 0x%p\n", ret);

	printf("a + b = %i\n", (a + b));
}


void SpoofRet::Run()
{

	// jmp rbx
	void* base = (void*)GetModuleHandleA(0);
	printf("base = 0x%p\n", base);

	spoof_trampoline = (void*)jmp_rbx;
	printf("spoof_trampoline = 0x%p\n", spoof_trampoline);
	printf("add_num = 0x%p\n", add_num);

	printf("BEFORE: \n");
	add_num(6, 6);

	system("pause");

	printf("--------------------------\n");
	printf("AFTER:\n");

	spoof_call(add_num, 24, 24);

	while (true) {}
}