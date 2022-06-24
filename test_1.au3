#NoTrayIcon ; Should add this to prevent tray icon in sub-thread.
#include 'N.au3'

; This test spawns a thread on every button click.

; Get global shared object.
global $g = NGlobal()

; task 1
func task_1($l)
	; Set last task.
	$g.last = 'task_1'

	; Get message from shared.
	local $msg = $l.message
	MsgBox(0, 'Task 1 (ID: ' & NGetId() & ')', $msg)
endfunc

; task 2
func task_2($l)
	; Set last task.
	$g.last = 'task_2'

	; Get some data.
	local $msg = StringFormat('Sum %d + %d = %d.', $l.a, $l.b, $l.a + $l.b)
	MsgBox(0, 'Task 2 (ID: ' & NGetId() & ')', $msg)
endfunc

; main
func main()
	; Create GUI with 3 buttons.
	GUICreate('Test 1', 300, 180)
	local $btn1 = GUICtrlCreateButton('Task 1', 20, 50, 80, 30)
	local $btn2 = GUICtrlCreateButton('Task 2', 120, 50, 80, 30)
	local $btn3 = GUICtrlCreateButton('Main', 60, 100, 80, 30)
	GUISetState(@SW_SHOW)

	local $count = 0

	; GUI message loop.
	while true
		switch GUIGetMsg()
			; close
			case -3
				exit

			; button 1
			case $btn1
				; Create local shared.
				local $l = NLocal()
				; Set .message prop.
				$l.message = StringFormat('Hi, this is a message (%d).', $count)
				$count += 1
				; Run task_1.
				NRun('task_1', $l)

			; button 2
			case $btn2
				; Create local shared.
				local $l = NLocal()
				; Set some data.
				$l.a = 10
				$l.b = 5
				; Run task_2.
				NRun('task_2', $l)

			; button 3
			case $btn3
				; Get .last from global.
				MsgBox(0, 'Main', 'Last spawned thread: ' & $g.last)
		endswitch
	wend
endfunc

; Execute main entry.
NMain('main')