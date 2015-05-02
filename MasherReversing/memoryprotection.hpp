#pragma once

#include <stdexcept>
#include <windows.h>

class MemoryProtectException : public std::runtime_error
{
public:
	MemoryProtectException( const char* aMsg );
};

class MemoryUnProtectException : public std::runtime_error
{
public:
	MemoryUnProtectException( const char* aMsg );
};

class MemoryProtection
{
public:
	MemoryProtection( void* aAdress, unsigned int aSize );
	~MemoryProtection();
	void UnProtect();
	void Protect();
private:
	void* iAddress;
	unsigned int iSize;
	DWORD iOldFlags;
	bool iProtected;
};
