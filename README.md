# :rainbow: True AutoIt Multi-threading

:zap: Just an experiment to achieve multi-threading in AutoIt, as a library.

:clap: All issues and pull requests are welcome! 

:bell: Give a star and subscribe (watch) to get notified about changes.


## Usage

Download the latest [release](https://github.com/nomi-san/true-autoit-multi-threading/releases) (or [build from source](#build-from-source)).

Library files:
- N.au3 - AutoIt include file
- N.dll - 32-bit DLL dependency
- N64.dll - 64-bit DLL (optional if 32-bit only)

Simple example:
```au3
#NoTrayIcon
#include 'N.au3'

func task()
  MsgBox(0, '', "Hello, I'm in another thread.")
endfunc

func main()
  local $t = NRun('task')
  NWait($t)
endfunc

NMain('main')
```

More examples:
- [test_1.au3](./test_1.au3)
- [test_2.au3](./test_2.au3)


See [N.au3](./N.au3) to get all APIs.

## Build from source

Requirements:
- Visual Studio 2017+
- C++ desktop development
- Windows SDK 8.1+

Build steps:
- Open **true-autoit-multi-threading.sln**
- Restore Nuget packages
- Choose **CPU arch** -> x86/x64
- Press build -> N.dll/N64.dll
- Copy above DLLs and N.au3 to your script folder

## FAQs

#### How it works?
Please see the C++ source code, I'll update soon 😀.

#### Does it work with compiled EXE.
Yep, it's designed for both running script in AutoIt.exe and compiled (also upx packed) EXE.

#### Memory leaks after threads done?
After the first thread is created, the memory increases once ~EXE size. And +80kB after each thread done, that isn't memory leaks, they are static members/smart pointers of AutoIt interpreter still alive. They will be freed when process exits.

#### Is its shared object faster than AutoItObject?
Yesh, it is implemented with hash table. I don't know why AutoItObject lookups property name through array sequentially.

#### Is there a magic?
Nope.

#### Thread safe?
Not implemented yet.
