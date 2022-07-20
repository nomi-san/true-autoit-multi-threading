; Author: nomi-san (wuuyi123@gmail.com)
; Github: https://github.com/nomi-san/true-autoit-multi-threading

#include-once

local $__nDll = @AutoItX64 ? 'N64.dll' : 'N.dll'
local $__nMod = DllCall('kernel32.dll', 'handle', 'GetModuleHandleW', 'wstr', $__nDll)[0]
if not $__nMod then $__nMod = DllCall('kernel32.dll', 'handle', 'LoadLibraryW', 'wstr', $__nDll)[0]

local $__nPfn_Global 	= __n_GetProc(101)
local $__nPfn_Local 	= __n_GetProc(102)
local $__nPfn_Run 		= __n_GetProc(103)
local $__nPfn_IsMain 	= __n_GetProc(104)
local $__nPfn_PrepMain 	= __n_GetProc(105)
local $__nPfn_PrepSub 	= __n_GetProc(106)
local $__nPfn_Wait 		= __n_GetProc(107)
local $__nPfn_WaitAll 	= __n_GetProc(108)

; Get the global shared object.
func NGlobal()
	return DllCallAddress('idispatch', $__nPfn_Global)[0]
endfunc

; Create new shared object instance (local).
func NLocal()
	return DllCallAddress('idispatch', $__nPfn_Local)[0]
endfunc

; Run a function in new thread.
func NRun($fn, $o = null)
	return DllCallAddress('dword', $__nPfn_Run, 'str', IsFunc($fn) ? FuncName($fn) : $fn, 'ptr',  $o)[0]
endfunc

; Check if running thread is main.
func NIsMain()
	return DllCallAddress('bool', $__nPfn_IsMain)[0]
endfunc

; Execute entry point.
func NMain($ep)
	if NIsMain() then
		DllCallAddress('none', $__nPfn_PrepMain)
		Call(IsFunc($ep) ? FuncName($ep) : $ep)
	else
		; Retrieve function name and local.
		local $s = DllStructCreate('char[64];')
		local $l = DllCallAddress('idispatch', $__nPfn_PrepSub, 'ptr', DllStructGetPtr($s, 1))[0]
		local $fn = DllStructGetData($s, 1)
		Call(IsFunc($fn) ? FuncName($fn) : $fn, $l)
	endif
endfunc

; Wait for a thread.
func NWait($tid)
	DllCallAddress('none', $__nPfn_Wait, 'dword', $tid)
endfunc

; Wait for all threads.
func NWaitAll()
	DllCallAddress('none', $__nPfn_WaitAll)
endfunc

; Get current thread ID.
func NGetId()
	return DllCall('kernel32.dll', 'dword', 'GetCurrentThreadId')[0]
endfunc

; Internal function.
func __n_GetProc($i)
	return DllCall('kernel32.dll', 'ptr', 'GetProcAddress', 'handle', $__nMod, 'ptr', Ptr($i))[0]
endfunc
