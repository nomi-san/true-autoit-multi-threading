#NoTrayIcon ; Should add this to prevent tray icon in sub-thread.
#include 'N.au3'

; This test create a new thread worker that changes GUI title randomly.

; task
func task($state)
	local $gui = HWnd($state.hwnd)	; .hwnd is casted to int, just convert it to HWND
	while true
		; Change GUI title when ready.
		if $state.ready then
			local $n = Random(0xFF, 0xFFFFFF, 1)
			WinSetTitle($gui, '', 'Random: ' & $n)
		endif
		Sleep(16)
	wend
endfunc

; main
func main()
	; Create GUI with 3 buttons.
	local $gui = GUICreate('Test 2', 320, 150)
	local $btn = GUICtrlCreateButton('Start', 20, 50, 80, 30)
	GUISetState(@SW_SHOW)

	; Create shared state.
	local $state = NLocal()
	$state.hwnd = $gui		; GUI handle
	$state.ready = false	; ready state

	; Run task with state.
	NRun('task', $state)

	; GUI message loop.
	while true
		switch GUIGetMsg()
			; close
			case -3
				exit

			; button 1
			case $btn
				if $state.ready then
					$state.ready = false
					GUICtrlSetData($btn, 'Start')
				else
					$state.ready = true
					GUICtrlSetData($btn, 'Stop')
				endif
		endswitch
	wend
endfunc

; Execute main entry.
NMain('main')