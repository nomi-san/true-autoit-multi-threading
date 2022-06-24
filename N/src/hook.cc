#include <windows.h>
#include <detours.h>

BOOL N_ShouldExit();

#if _DEBUG
static decltype(&IsDebuggerPresent) Old_IsDebuggerPresent;
static BOOL WINAPI Hooked_IsDebuggerPresent()
{
    return FALSE;
}
#endif

static decltype(&ExitProcess) Old_ExitProcess;
static void WINAPI Hooked_ExitProcess(int uExitCode)
{
    // Exit only in main thread.
    if (N_ShouldExit())
    {
        Old_ExitProcess(uExitCode);
    }
}

void N_InstallHook()
{
    DetourRestoreAfterWith();

#if _DEBUG
    // For debugging this library.
    (LPVOID &)Old_IsDebuggerPresent = DetourFindFunction("kernel32.dll", "IsDebuggerPresent");
#endif
    (LPVOID &)Old_ExitProcess = DetourFindFunction("kernel32.dll", "ExitProcess");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

#if _DEBUG
    DetourAttach(&(PVOID &)Old_IsDebuggerPresent, Hooked_IsDebuggerPresent);
#endif
    DetourAttach(&(PVOID &)Old_ExitProcess, Hooked_ExitProcess);

    DetourTransactionCommit();
}