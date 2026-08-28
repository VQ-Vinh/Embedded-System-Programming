# Quy ước làm việc với dự án

## STM32CubeMX và mã nguồn sinh tự động

- Khi người dùng yêu cầu chỉnh sửa, trước tiên phải xác định thay đổi đó có thể cấu hình bằng STM32CubeMX hay không.
- Nếu có thể cấu hình bằng CubeMX, ưu tiên hướng dẫn người dùng thực hiện thay đổi trong file `.ioc` bằng CubeMX trước.
- Chỉ bắt đầu viết hoặc chỉnh sửa code sau khi người dùng xác nhận đã hoàn tất việc cấu hình và sinh lại code từ CubeMX.
- Không tự ý sửa trực tiếp phần code do CubeMX quản lý nếu thay đổi tương ứng có thể được thực hiện an toàn trong CubeMX.
- Khi buộc phải thêm code vào file do CubeMX sinh, đặt code trong các vùng `USER CODE BEGIN` / `USER CODE END` phù hợp để tránh bị mất khi sinh lại code.

## Commit và push

- Khi người dùng yêu cầu commit và push, chia thay đổi thành nhiều commit nhỏ, có mục đích rõ ràng và dễ xem lại.
- Không dồn nhiều module hoặc nhiều nhóm thay đổi độc lập vào cùng một commit.
- Mỗi commit phải giữ dự án ở trạng thái hợp lý; ưu tiên tách theo cấu hình CubeMX, module, chức năng, kiểm thử và tài liệu.
- Dùng commit message ngắn gọn nhưng mô tả đúng phạm vi thay đổi.

## Quy ước comment code

- Mỗi module phải có phần comment đầu file mô tả mục đích, trách nhiệm và các phụ thuộc quan trọng của module.
- Mỗi function phải có comment mô tả mục đích, tham số, giá trị trả về và tác động phụ nếu có.
- Comment phải giải thích ý định, ràng buộc thời gian thực, trạng thái, tính đồng bộ hoặc lý do thiết kế; tránh chỉ diễn giải lại từng dòng code.
- Kết hợp cú pháp của extension Better Comments để làm nổi bật nội dung quan trọng:
  - `// !` cho cảnh báo, điều kiện nguy hiểm hoặc ràng buộc bắt buộc.
  - `// ?` cho điểm cần xác minh hoặc quyết định thiết kế chưa chốt.
  - `// TODO:` cho công việc còn lại có phạm vi cụ thể.
  - `// *` cho ghi chú quan trọng hoặc giải thích đáng chú ý.
- Với API và function public, ưu tiên comment kiểu Doxygen (`/** ... */`) để tài liệu có thể được sinh tự động.
- Không để comment Better Comments thay thế phần mô tả đầy đủ của module hoặc function.
