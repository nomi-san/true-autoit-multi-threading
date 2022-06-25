Đây là thư viện hỗ trợ chạy đa luồng (multi-threading) trong AutoIt, nó không phải là đa tiến trình (multi-process).
Vui lòng tìm hiểu rõ về thread và process trước khi sử dụng.

## sử dụng

Vui lòng tải [file release](https://github.com/nomi-san/true-autoit-multi-threading/releases) mới nhất (hoặc build project này bằng Visual Studio C++).

Các file thư viện bao gồm:
- N.au3 - file include của AutoIt
- N.dll - file DLL đi kèm của thư viện
- N64 - DLL cho 64-bit (nếu dùng 32-bit thì không cần nó)

Ví dụ đơn giản:
```au3
#NoTrayIcon       ; Nên thêm dòng này vào
#include 'N.au3'  ; Thêm thư viện

; Tuyệt đối không đặt code khởi tạo ngoài này

func task()
  ; Hiện hộp thoại
  MsgBox(0, '', "Hello, I'm in another thread.")
endfunc

func main()
  local $t = NRun('task')   ; Chạy task trên luồng mới
  NWait($t)                 ; Chờ cho đến khi luồng thoát
endfunc

; Chạy hàm main
NMain('main')
```

#### Xem [test_1.au3](./test_1.au3) và [test_2.au3](./test_2.au3) để tìm hiểu thêm.

Không có nhiều các ví dụ là do việc đa luồng khá phổ biến và dễ sử dụng, có thể ứng dụng từ các ngôn ngữ khác qua, nhưng ở múc đơn giản nhất là chạy hàm trên luồng khác và giao tiếp với nhau qua dữ liệu.
