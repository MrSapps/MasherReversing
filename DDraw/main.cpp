#include <windows.h>
// Hack so we can call our entry point "DirectDrawCreate"
#define DirectDrawCreate OLD_DirectDrawCreate
#include <ddraw.h>
#undef DirectDrawCreate
#include <string>


// Real direct draw create in the system32 ddraw.dll
typedef HRESULT(WINAPI* DirectDrawCreate_fp)(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter);

// Global function pointer values that point to the real functions in the real ddraw.dll
static DirectDrawCreate_fp OldDirectDrawCreate = NULL;

// This is a global handle to point to the REAL ddraw.dll (in system32 folder)
static HINSTANCE hRealDll = 0;

void LoadRealDDrawDll()
{
    wchar_t systemDir[MAX_PATH];
    GetSystemDirectory(systemDir, MAX_PATH);

    // We then append \ddraw.dll, which makes the string:
    // C:\windows\system32\ddraw.dll
    std::wstring pathToRealDDrawDll(systemDir);
    pathToRealDDrawDll += L"\\ddraw.dll";

    // Load the real ddraw.dll, storing the handle in hRealDll.
    hRealDll = LoadLibrary(pathToRealDDrawDll.c_str());
    if (!hRealDll)
    {
        // Failed to load the real DDraw.Dll
        MessageBox(NULL, L"Can't load or find real DDraw.dll", L"Error", MB_OK | MB_ICONEXCLAMATION);
        exit(-1);
    }
}

void GetFunctionPointersToRealDDrawFunctions()
{
    OldDirectDrawCreate = (DirectDrawCreate_fp)GetProcAddress(hRealDll, "DirectDrawCreate");
    if (!OldDirectDrawCreate)
    {
        // Failed to get a pointer to the real function.
        MessageBox(NULL, L"Can't find DirectDrawCreate function in real dll", L"Error", MB_OK | MB_ICONEXCLAMATION);
        exit(-1);
    }
}

extern "C"
{
    __declspec(dllexport) HRESULT __cdecl DirectDrawCreate(GUID FAR* lpGUID, LPDIRECTDRAW FAR* lplpDD, IUnknown FAR* pUnkOuter)
    {
         // Load the DLL.
         LoadRealDDrawDll();

        // Get the function pointers to real direct draw functions.
        GetFunctionPointersToRealDDrawFunctions();


        HRESULT ret = OldDirectDrawCreate(lpGUID, lplpDD, pUnkOuter);
        
        return ret;
    }
}




BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD     fdwReason,
    _In_ LPVOID    lpvReserved
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {

    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {

    }

    return TRUE;
}
