#pragma once

#include "memoryprotection.hpp"
#include <windows.h>
#include <type_traits>

template<class T>
class JmpHookedFunction
{
public:
    JmpHookedFunction(void* aOldFunc, void* aNewFunc)
        : iOldFunc( (T)aOldFunc ), iNewFunc( (T)aNewFunc ), iHooked( false )
	{
		// Copy its intro
		CopyOrignalFunctionBytes();

		// Hook it
		JmpHookedFunction::Hook();
	}

	~JmpHookedFunction()
	{
		UnHook();
	}

    T OrignalFunction()
	{
		return iOldFunc;
	}

    void UnHook()
	{
		{
            MemoryProtection memProt( (void*)iOldFunc, 0x100 );

			if ( iNewFunc && iHooked == true )
			{
				for ( int i=0; i<sizeof(iOldFuncBytes); ++i )
				{
					*((unsigned char *) ((unsigned int)iOldFunc+i)) = iOldFuncBytes[i];
				}
				iHooked = false;
			}
		}
		FlushInstructionCache( GetCurrentProcess(), (void*)iOldFunc, 6 );
	}

    void Hook()
	{
		{
            MemoryProtection memProt( (void*)iOldFunc, 0x100 );
			if ( iNewFunc && iHooked == false )
			{
			
				// JMP
				*((unsigned char *) ((unsigned int)iOldFunc+0)) = 0xE9;

				// New func addr
                unsigned int addr = (unsigned int)iNewFunc;
                addr = addr - 5;

                unsigned int tmp = 0;
                if (unsigned int(iOldFunc) > addr)
                {
                    tmp = ((unsigned int)iOldFunc) - addr;
                }
                else
                {
                    tmp = addr - ((unsigned int)iOldFunc);
                }

                *((unsigned int *)((unsigned int)iOldFunc + 1)) = (unsigned int)tmp;

				iHooked = true;
			}
		}
		FlushInstructionCache( GetCurrentProcess(), (void*)iOldFunc, 6 );
	}


private:

	void CopyOrignalFunctionBytes()
	{
		for ( int i=0; i<sizeof(iOldFuncBytes)/sizeof(iOldFuncBytes[0]); ++i )
		{
			iOldFuncBytes[i] = *((unsigned char *) ((unsigned int)iOldFunc+i));
		}
	}

private:
	T iOldFunc;
	T iNewFunc;
	
	bool iHooked;

	// JMP + Target
	unsigned char iOldFuncBytes[5];
};

template < class T >
class CallOriginalJmpHookedFunction
{
public:
	CallOriginalJmpHookedFunction( JmpHookedFunction<T>& aFunc )
		: iFunc( aFunc )
	{
		iFunc.UnHook();
	}

	~CallOriginalJmpHookedFunction()
	{
		iFunc.Hook();
	}

private:
	 JmpHookedFunction<T>& iFunc;
};

