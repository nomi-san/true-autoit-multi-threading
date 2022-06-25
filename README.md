# true-autoit-multi-threading
Yep, multi-threading in AutoIt.

<br>

> All issues and pull requests are welcome!

<br>

## usage

Download the latest [release](https://github.com/nomi-san/true-autoit-multi-threading/releases) (or build from source with Visual Studio C++).

Library files:
- N.au3 - AutoIt include file
- N.dll - dynamic library
- N64.dll - for 64-bit

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

#### See [test_1.au3](./test_1.au3) and [test_2.au3](./test_2.au3) to learn more.
#### See [N.au3](./N.au3) to get all APIs.

## warning
This is just an experiment.

## how it works?
Please see the C++ source code, I'll update soon 😀

## FAQs

#### Memory leaks after threads done?
After the first thread is created, the memory increases once ~EXE size. And +80kB after each thread done, that isn't memory leaks, they are static members/smart pointers of AutoIt interpreter still alive. They will be freed when process exits.

#### Is its shared object faster than AutoItObject?
Yesh, it is implemented with hash table. I don't know why AutoItObject lookups property name through array sequentially.

#### Why "N" library?
It isn't someone nickname, it's just the first letter I think to.

#### Is there a magic?
Nope.
