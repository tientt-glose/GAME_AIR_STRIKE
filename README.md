# GAME_AIR_STRIKE

## Cách thêm 1 hàm xử lý và message kèm theo

1. Thêm HEADER mới ở file server
1. Thêm trạng thái client và send_to_server ở file client
1. Thêm trạng thái của server (tương ứng với send_to_server ở file client) ở file server
1. Tái sử dụng hàm mong muốn (nếu có) hoặc viết hàm xử lý mới ở file server
1. Bố cục lại, thêm mới các mã message trả về cho client ở cả hai file client và server
1. [Optional] Nếu muốn gửi kèm thông tin kèm với mã message trả về thì thêm xử lý trong hàm changeFull() ở file server
1. Bố cục lại, thêm mới các plain text message tương ứng với các mã message trong makeFull() ở file client. makeFull() cũng chính là nơi giải mã nếu như server có gửi kèm thông tin ngoài mã message trả về
1. Thêm, sửa điều kiện chạy hàm này trong hàm process ở file server

## Cách thêm message cho hàm sẵn có

1. Bố cục lại, thêm mới các mã message trả về cho client ở cả hai file client và server
1. [Optional] Nếu muốn gửi kèm thông tin kèm với mã message trả về thì thêm xử lý trong hàm changeFull() ở file server
1. Bố cục lại, thêm mới các plain text message tương ứng với các mã message trong makeFull() ở file client. makeFull() cũng chính là nơi giải mã nếu như server có gửi kèm thông tin ngoài mã message trả về

## Một số comment token trong code

1. REFRACTOR: Sẽ dọn dẹp nếu như sau cùng ko dùng đến
1. NOTE: Lưu ý
1. DOTO: Dành cho tiến làm nếu cần
1. TODO: Phải làm
1. Diff: Khác với file mã nguồn gốc của các anh
1. Optional: Có thể thêm nếu cần ko thì xóa vì cx ko ảnh hưởng
1. NGUYEN: Có thể giúp ích cho nguyên trong phần code tiếp theo.

## Một số convention khi code:

* Test kỹ trước khi đẩy pull. Code phải tách nhánh có nghĩa riêng biệt. (ko code trực tiếp trên nhánh master)
* Nên thêm `return "Sequence is wrong"` ở cuối các hàm nếu có thể để biết lỗi đó đang vô định, ko phải là lỗi cụ thể mà mình đã định nghĩa.
* Nhớ reset message noti sau khi dùng hàm GINFO vì đã nhận dc thông tin, đã đọc đc rồi, đã in ra rồi, thì phải reset. (dùng hàm RINFO)

## TEST - DEMO

* Mọi dữ liệu test đều ko có dấu cách khi nhập. Làm mọi động tác từ từ, ko vội vã. Nếu có lỗi thì phải report ngay vào nhóm chat chung.
* Nếu thiếu thư viện tìm kỹ nhất có thể cách cài thêm thư viện đó. File code đã rất cố gắng để sử dụng những thư viện C chuẩn.

** Nếu có gì không hiểu thử search các keyword trong file này ở file code để xem quy trình, ví dụ, cách vận hành, cách làm nhé.