#include "memoryprotection.hpp"
#include <sstream>
#include <windows.h>

MemoryProtectException::MemoryProtectException( const char* aMsg )
: std::runtime_error( aMsg )
{
	// Empty 
}

MemoryUnProtectException::MemoryUnProtectException( const char* aMsg )
: std::runtime_error( aMsg )
{
	// Empty
}

MemoryProtection::MemoryProtection( void* aAddress, unsigned int aSize )
 : iAddress( aAddress ), iSize( aSize ), iOldFlags( 0 ), iProtected( true )
{
	UnProtect();
}

MemoryProtection::~MemoryProtection()
{
	Protect();
}

void MemoryProtection::UnProtect()
{
	if ( iProtected )
	{
		if ( !VirtualProtect( iAddress , iSize, PAGE_EXECUTE_READWRITE, &iOldFlags ) )
		{
			const DWORD gle = GetLastError();
			std::stringstream str;
			str << "Failed to unprotect memory. Error code: " << gle;
			throw MemoryUnProtectException( str.str().c_str() );
		}
		iProtected = false;
	}
}

void MemoryProtection::Protect()
{
	if ( !iProtected )
	{
		DWORD notUsed = 0;
		if ( !VirtualProtect( iAddress , iSize, iOldFlags, &notUsed ) )
		{
			const DWORD gle = GetLastError();
			std::stringstream str;
			str << "Failed to write protect memory. Error code: " << gle;
			throw MemoryProtectException( str.str().c_str() );
		}
		iProtected = true;
	}
}
