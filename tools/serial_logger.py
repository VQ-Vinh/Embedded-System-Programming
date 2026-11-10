#!/usr/bin/env python3
"""
serial_logger.py - Ghi kết quả cân từ STM32 (USART2, 115200 8N1) ra file CSV.

Cài đặt một lần:
    pip install pyserial

Cách dùng (Windows, cổng COM của mạch USB-UART xem trong Device Manager):
    python tools/serial_logger.py COM5
    python tools/serial_logger.py COM5 -o ket_qua_lan1.csv
    python tools/serial_logger.py --list          # liệt kê cổng COM

Script:
  * In mọi dòng nhận được ra màn hình (kể cả dòng chú thích '#').
  * Ghi các dòng dữ liệu "ID,Weight_g,Target_g,Tolerance_g,Result" vào CSV
    kèm cột thời gian máy tính.
  * Kiểm tra mã lượt (ID): báo nếu trùng hoặc bị nhảy cóc -> phát hiện đếm
    trùng / mất bản ghi theo yêu cầu kiểm thử của đề tài.
  * Nhấn Ctrl+C để dừng; in tổng số PASS / UNDER / OVER để đối chiếu với
    thống kê trên màn hình TFT.
"""
import argparse
import csv
import datetime as dt
import sys

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    sys.exit("Thiếu thư viện pyserial. Chạy: pip install pyserial")

HEADER = ["ID", "Weight_g", "Target_g", "Tolerance_g", "Result"]


def list_ports():
    """In danh sách cổng serial đang có trên máy."""
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("Không thấy cổng serial nào.")
    for p in ports:
        print(f"{p.device:10s} {p.description}")


def main():
    """Đọc cổng serial, ghi CSV và kiểm tra mã lượt cho đến khi nhấn Ctrl+C."""
    ap = argparse.ArgumentParser(description="Ghi kết quả cân STM32 ra CSV")
    ap.add_argument("port", nargs="?", help="Cổng serial, ví dụ COM5 hoặc /dev/ttyUSB0")
    ap.add_argument("-b", "--baud", type=int, default=115200)
    ap.add_argument("-o", "--out", help="Tên file CSV (mặc định ket_qua_<ngay_gio>.csv)")
    ap.add_argument("--list", action="store_true", help="Liệt kê cổng serial rồi thoát")
    args = ap.parse_args()
    # * Tránh lỗi in tiếng Việt trên console Windows dùng bảng mã cũ.
    try:
        sys.stdout.reconfigure(errors="replace")
    except AttributeError:
        pass

    if args.list or not args.port:
        list_ports()
        if not args.port:
            print("\nCần chỉ định cổng, ví dụ: python tools/serial_logger.py COM5")
        return

    out_name = args.out or dt.datetime.now().strftime("ket_qua_%Y%m%d_%H%M%S.csv")
    counts = {"PASS": 0, "UNDER": 0, "OVER": 0}
    last_id = 0
    problems = 0

    with serial.serial_for_url(args.port, args.baud, timeout=1) as ser, \
            open(out_name, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["PC_Time"] + HEADER)
        print(f"Đang nghe {args.port} @ {args.baud}. Ghi vào {out_name}. Ctrl+C để dừng.\n")
        try:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                line = raw.decode("ascii", errors="replace").strip()
                if not line:
                    continue
                print(line)
                if line.startswith("#"):
                    if "STATS CLEARED" in line:
                        last_id = 0
                        counts = {k: 0 for k in counts}
                    continue
                parts = line.split(",")
                if parts == HEADER:
                    # * Kit vừa khởi động lại: thống kê trên kit bắt đầu lại từ 0.
                    last_id = 0
                    counts = {k: 0 for k in counts}
                    continue
                if len(parts) != len(HEADER) or not parts[0].isdigit():
                    print("  ! Dòng không đúng định dạng, bỏ qua")
                    problems += 1
                    continue
                rid = int(parts[0])
                if rid == last_id:
                    print(f"  ! TRÙNG mã lượt {rid}")
                    problems += 1
                elif rid != last_id + 1:
                    print(f"  ! Nhảy mã lượt: {last_id} -> {rid} (mất bản ghi?)")
                    problems += 1
                last_id = rid
                if parts[4] in counts:
                    counts[parts[4]] += 1
                writer.writerow([dt.datetime.now().isoformat(timespec="seconds")] + parts)
                f.flush()
        except KeyboardInterrupt:
            pass

    total = sum(counts.values())
    print("\n--- Tổng kết phiên ---")
    print(f"PASS {counts['PASS']}  UNDER {counts['UNDER']}  OVER {counts['OVER']}  TỔNG {total}")
    print(f"Lỗi định dạng / trùng / mất mã lượt: {problems}")
    print(f"Đã ghi: {out_name}")


if __name__ == "__main__":
    main()
