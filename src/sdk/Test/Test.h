namespace Test
{
	inline bool g_isAllowedTesting = true;

	namespace Obfuscation
	{
		void Run();
	}
	namespace Utils
	{

		bool IsInValidMemoryRegion(uint64_t return_address);
	}

}