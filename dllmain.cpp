// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "powerhouse.h"

// For console functions
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Export the entry points
extern "C" {
    __declspec(dllexport) void __stdcall console() {
        powerhouse::Naruto::console();
    }

    __declspec(dllexport) void __stdcall dat(const char* filePath) {
        powerhouse::Naruto::dat(filePath);
    }
}

