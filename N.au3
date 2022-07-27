; Author: nomi-san (wuuyi123@gmail.com)
; Github: https://github.com/nomi-san/true-autoit-multi-threading
Local $KERNEL32DLL = "kernel32.dll"
local $__nMod = DllCall($KERNEL32DLL, 'handle', 'LoadLibraryW', 'wstr', @AutoItX64 ? 'N64.dll' : 'N.dll')[0]

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
	$IDId = DllCallAddress('dword', $__nPfn_Run, 'str', $fn, 'ptr',  $o)[0]
	return $IDId
endfunc

; Check if running thread is main.
func NIsMain()
	return DllCallAddress('bool', $__nPfn_IsMain)[0]
endfunc

; Execute entry point.
func NMain($ep)
	if NIsMain() then
		DllCallAddress('none', $__nPfn_PrepMain)
		Call($ep)
	else
		; Retrieve function name and local.
		local $s = DllStructCreate('char[64];')
		local $l = DllCallAddress('idispatch', $__nPfn_PrepSub, 'ptr', DllStructGetPtr($s, 1))[0]
		local $fn = DllStructGetData($s, 1)
		Call($fn, $l)
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
	return DllCall($KERNEL32DLL, 'dword', 'GetCurrentThreadId')[0]
endfunc

; Internal function.
func __n_GetProc($i)
	return DllCall($KERNEL32DLL, 'ptr', 'GetProcAddress', 'handle', $__nMod, 'ptr', Ptr($i))[0]
endfunc

Func NTerminate($ThreadID)
	$hThread = _ThreadOpen($ThreadID,0x0001)
	If @error Then Return SetError(3,0,0)
	_ThreadTerminate($hThread)
	If @error Then Return SetError(3,0,0)

	_ThreadCloseHandle($hThread)

	Return True
EndFunc


Func _ThreadOpen($iThreadID,$iAccess,$bInheritHandle = False)
	Local $aRet
	; Special 'get current Thread handle' request? [Full access is implied, and no inheritance - this is a 'pseudo-handle']
	If $iThreadID = -1 Then
		$aRet=DllCall($KERNEL32DLL,"handle","GetCurrentThread")	; usually the constant -2, but we're keeping it future-OS compatible this way
	Else
		$aRet=DllCall($KERNEL32DLL,"handle","OpenThread","dword",$iAccess,"bool",$bInheritHandle,"dword",$iThreadID)
	EndIf
	If @error Then Return SetError(2,@error,0)
	If Not $aRet[0] Then Return SetError(3,0,0)	; 0 = failed!
	Return $aRet[0]
EndFunc

Func _ThreadTerminate($hThread,$iExitCode=0)
	If Not IsPtr($hThread) Then Return SetError(1,0,False)
	Local $aRet=DllCall($KERNEL32DLL,"bool","TerminateThread","handle",$hThread,"int",$iExitCode)
	If @error Then Return SetError(2,@error,False)
	If Not $aRet[0] Then Return SetError(3,0,False)	; False (failure) return
	Return True
EndFunc

Func _ThreadCloseHandle(ByRef $hThread)
	If Not IsPtr($hThread) Then Return SetError(1,0,False)
	Local $aRet=DllCall($KERNEL32DLL,"bool","CloseHandle","handle",$hThread)
	If @error Then Return SetError(2,@error,False)
	If Not $aRet[0] Then Return SetError(3,0,False)
	$hThread = 0	; Invalidate the handle
	Return True
EndFunc
