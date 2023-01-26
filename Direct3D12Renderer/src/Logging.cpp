module;
#include <iostream>
#include <string>
#include <windows.h>
module Logging;

import GlobalSettings;

namespace
{
	void PrintToVSDebugConsole(gg::DebugLevel debugLevel, wchar_t const* message)
	{
		wchar_t const* debugLevelStr{ (debugLevel == gg::DebugLevel::Info) ? L"[Info] " : L"[Error] " };
		OutputDebugStringW(debugLevelStr);
		OutputDebugStringW(message);
		OutputDebugStringW(L"\n");
	}
}

namespace gg
{
	void DebugLog(DebugLevel debugLevel, std::wstring const& message)
	{
		DebugLog(debugLevel, message.c_str());
	}

	void DebugLog(DebugLevel debugLevel, wchar_t const* message)
	{
		if constexpr (IsDebug())
		{
			if constexpr (IsWindowsSubSystem())
				PrintToVSDebugConsole(debugLevel, message);
			else
			{
				auto& outStream{ (debugLevel == DebugLevel::Info) ? std::wcout : std::wcerr };
				outStream << message << std::endl;
			}
		}
	}

	std::wstring ToWString(std::string const& str)
	{
		if (str.empty()) return std::wstring{};
		int const size_needed{ MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0) };
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}

} // namespace gg
