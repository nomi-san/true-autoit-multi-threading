#define _CRT_SECURE_NO_WARNINGS

#include <unordered_map>
#include <windows.h>
#include "MemoryModule.h"

void N_InstallHook();
LPDISPATCH N_CreateShared();
LPVOID N_GetBinaryModule(SIZE_T *psize);
typedef int (APIENTRY *WinMain_t)(HINSTANCE, HINSTANCE, LPWSTR, int);

struct Thread
{
    HMEMORYMODULE module;   // MemoryModule
    HANDLE handle;          // Thread handle
    LPDISPATCH local;       // Shared object
    char func[64];          // Function name
};

static DWORD g_mainThreadId = 0;
static std::unordered_map<DWORD, Thread *> g_threadMap{};

LPDISPATCH WINAPI NAPI_Global()
{
    static LPDISPATCH shared;
    // Singleton shared, create new if not yet.
    if (!shared) shared = N_CreateShared();
    return shared;
}

LPDISPATCH WINAPI NAPI_Local()
{
    // Create new shared instance.
    return N_CreateShared();
}

static DWORD CALLBACK N_ThreadRoutine(Thread *thread)
{
    // Execute main entry point.
    WinMain_t main = (WinMain_t)N_MemoryModuleGetEntryPoint(thread->module);
    return main(GetModuleHandleA(NULL), NULL, GetCommandLineW(), 0);
}

DWORD WINAPI NAPI_Run(LPCSTR fn, LPDISPATCH local)
{
    DWORD id;
    Thread *thread = new Thread();

    // Load current EXE with MemoryModule.
    SIZE_T moduleSize;
    LPVOID moduleData = N_GetBinaryModule(&moduleSize);
    thread->module = MemoryLoadLibrary(moduleData, moduleSize);

    // Save thread's function name.
    lstrcpyA(thread->func, fn);
    // Save local shared.
    thread->local = local;
    // Create suspended thread.
    thread->handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)N_ThreadRoutine, (LPVOID)thread, CREATE_SUSPENDED, &id);

    // Insert to map and resume it.
    g_threadMap.insert_or_assign(id, thread);
    ResumeThread(thread->handle);

    return id;
}

BOOL WINAPI NAPI_IsMain()
{
    // Known the current thread is main if it's initialized or not yet.
    return (g_mainThreadId == 0)
        || (GetCurrentThreadId() == g_mainThreadId);
}

void WINAPI NAPI_PrepMain()
{
    // Init once in main thread only.
    if (g_mainThreadId == 0)
    {
        // Just save main thread ID.
        g_mainThreadId = GetCurrentThreadId();
        // Install hook.
        N_InstallHook();
    }
}

LPDISPATCH WINAPI NAPI_PrepSub(LPSTR fn)
{
    DWORD tid = GetCurrentThreadId();
    Thread *thread = g_threadMap.at(tid);

    // Retrieve sub-thread function name and local shared.
    lstrcpyA(fn, thread->func);
    return thread->local;
}

// Called before ExitProcess fires.
BOOL N_ShouldExit()
{
    // Don't exit on sub-thread.
    if (GetCurrentThreadId() != g_mainThreadId)
    {
        DWORD id = GetCurrentThreadId();
        Thread *thread = g_threadMap.at(id);
        HANDLE handle = thread->handle;

        // Free module.
        MemoryFreeLibrary(thread->module);

        // Remove thread from map.
        g_threadMap.erase(id);
        delete thread;

        // Should terminate this thread to prevent unexpected behavior.
        TerminateThread(handle, 0);
        return FALSE;
    }

    return TRUE;
}

void WINAPI NAPI_Wait(DWORD tid)
{
    Thread *thread = g_threadMap.at(tid);
    WaitForSingleObject(thread->handle, INFINITE);
}

void WINAPI NAPI_WaitAll()
{
    DWORD count = (DWORD)g_threadMap.size();
    HANDLE *handles = new HANDLE[count];

    int i = 0;
    for (const auto &pair : g_threadMap)
    {
        handles[i++] = pair.second->handle;
    }

    WaitForMultipleObjects(count, handles, TRUE, INFINITE);
    delete[] handles;
}