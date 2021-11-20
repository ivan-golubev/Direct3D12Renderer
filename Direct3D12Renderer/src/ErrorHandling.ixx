module;
#include <windows.h>
#include <exception>
#include <string>
export module ErrorHandling;

namespace awesome::errorhandling
{
	export class ComException : public std::exception
	{
	public:
		ComException(HRESULT hr);
		std::string whatString() const;
		char const* what() const override;
	private:
		HRESULT returnCode;
	};

	export void ThrowIfFailed(HRESULT hr);
} // awesome::errorhandling
