module;
#include <windows.h>
#include <string>
#include <format>
#include <cstdint>
module ErrorHandling;

namespace awesome::errorhandling 
{
	ComException::ComException(HRESULT hr) : returnCode{ hr } 
	{}

	std::wstring ComException::whatString() const
	{
		return std::format(L"Failed with HRESULT={}", static_cast<uint32_t>(returnCode));
	}

	char const* ComException::what() const
	{
		static char s_str[64] {};
		sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(returnCode));
		return s_str;
	}

	void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
			throw ComException(hr);
	}

} // awesome::errorhandling
