# Software Design Description
## For ARP Implementation using Raw Socket

Version 0.3
Prepared by Pham Hong Duong  
 
Table of Contents
=================
* 1 [Introduction](#1-introduction)
  * 1.1 [Document Overview](#11-document-overview)
  * 1.2 [Document Purpose](#12-document-purpose)
  * 1.3 [Keyworks](#13-keyworks)
  * 1.4 [References](#14-references)
* 2 [Design](#2-overall-description)
    * 2.1 [Context](#21-context)
    * 2.2 [Implementation](#22-implementation)
    * 2.4 [System Model](#24-system-model)
    * 2.5 [Communication Diagram](#25-communication-diagram)
    * 2.6 [Algorithm flowchart](#26-algorithm-flowchart)
    * 2.7 [Features](#27-features)
    * 2.8 [Running the code](#28-running-the-code)

## 1. Introduction
Document này là cung cấp mô tả thiết kế phần mềm cho việc implement bộ giao thức **ARP** tương tự trên **Linux OS**. Document sẽ mô tả các yêu cầu, thiết kế, và các chi tiết implement phần mềm **ARP** => giúp hiểu rõ về cách hệ thống hoạt động và tương tác với mạng.
### 1.1 Document Overview
Tài liệu này được chia thành các phần sau:
- Phần 1: Mục đích tài liệu và mô tả hệ thống.
- Phần 2: Thiết kế phần mềm chi tiết.
### 1.2 Document Purpose
Project này sẽ triển khai giao thức **ARP** trong **userspace**, sử dụng **raw socket** để nhận và gửi các gói **ARP Request** và **ARP Reply**. Các feature chính bao gồm:
- Truy vấn trong **ARP cache** tự định nghĩa (add, delete, show, find...).
- Gửi và nhận các gói **ARP Request**, **ARP Reply** và quản lý **ARP cache** với **timeout**.
- Hoạt động trực tiếp trên tầng **userspace** mà không thông qua **kernel network stack**.
### 1.3 Keywordsfeature
- **ARP**: Address Resolution Protocol, giao thức mapping địa chỉ IP -> địa chỉ MAC thuộc L2 trong cùng 1 network.
- **Raw Socket**: Loại socket nhận data từ L2 trong kernel network stack và bypass lên thẳng userspace.
- **Daemon**: Process chạy ngầm không phụ thuộc vào terminal, sẽ kết thúc khi tắt nguồn.
- **Cache Timeout**: Thời gian sống của một entry trong ARP table để lookup sự mapping giữa địa chỉ IP và địa chỉ MAC, sau thời gian này entry sẽ bị xóa.
- **CLI**: Command-line interface, giao diện dòng lệnh tương tác trực tiếp với người dùng.
- **ioctl**: Hàm tương tác với hardware, trong bối cảnh sử dụng  thì hàm này sẽ dùng để get địa chỉ IP từ 1 NIC cụ thể.
- **IPC**: cơ chế giao tiếp giữa các process với nhau, trong bối cảnh này thì giao tiếp giữa daemon process và CLI process.
- **NIC**: nơi truyền và nhận trực tiếp các gói tin mạng raw chưa qua xử lý, trong bối cảnh này sẽ sử dụng 2 card wifi để tương tác với nhau (trên cùng 1 host).
- etc...
### 1.4 References
- RFC 826 - "Ethernet Address Resolution Protocol"
- "Unix Network Programming" - Example of Raw Sockets
- Linux Interface Programming guide
- etc... 
## 2. Design
### 2.1 Context
Hệ thống bao gồm một **daemon** chạy nền chịu trách nhiệm quản lý **ARP cache** và xử lý các yêu cầu gửi, nhận **ARP Request** và **ARP reply**. **CLI** được thiết kế để tương tác với **daemon** thông qua **IPC**, cho phép thực hiện các thao tác trên **ARP cache** như **add**, **delete**, **show** và **find** các mục trong **cache**. **CLI** cung cấp các lệnh như `add <IP> <MAC>` để thêm một cặp **IP-MAC** vào **ARP** **cache**, `delete <IP>` để xóa một mục khỏi **cache**, `show` để hiển thị toàn bộ **ARP cache** và `find <IP>` để kiểm tra xem một **IP** có trong **cache** hay không. Khi nhận lệnh tìm kiếm một **IP** chưa có trong **cache**, **daemon** sẽ gửi **ARP Request** lên mạng và cập nhật **cache** khi nhận được **ARP Reply**. Giao tiếp giữa **CLI** và **daemon** đảm bảo **CLI** có thể truy vấn **ARP cache** nhanh chóng và **daemon** có thể tự động cập nhật địa chỉ **MAC** khi cần thiết. Để tránh xung đột với xử lý **ARP** của **kernel**, hệ thống sẽ vô hiệu hóa **ARP** trên **interface mạng** bằng lệnh `ip link set dev <interface> arp off`, đảm bảo mọi yêu cầu **ARP** đều do **daemon** quản lý. Ngoài ra, **daemon** cũng có thể nhận các **ARP Request** từ bên ngoài và **ARP Reply** với địa chỉ **MAC** tương ứng của nó, thay thế hoàn toàn vai trò của **kernel**.
### 2.2 Implementation
- **Daemon**:
  + Chạy nền dưới dạng process.
  + Cho phép cấu hình user (iface, timeout ARP cache) trước khi chạy.
  + Lấy địa chỉ IP, MAC của iface được chọn.
  + Tắt xử lý ARP tại kernel network stack.
  + Giao tiếp với CLI qua IPC.
  + Gửi và nhận ARP request/reply.
  + Lưu trữ ARP cache có timeout.
  + Xuất file lưu trữ ARP cache.
- **CLIs**:
  + Nhận lệnh từ người dùng để thông báo tới daemon.
  + Hiển thị kết quả mà daemon trả lại.

### 2.4 System Model

![image](https://github.com/user-attachments/assets/82c75231-33b8-4edd-8a35-f2e15bf08485)


Mô tả quá trình hoạt động:
    Tắt xử lý ARP trong Kernel:
        Lệnh ip link set dev <interface> arp off được sử dụng để tắt chức năng xử lý ARP của nhân Linux.
        Điều này ngăn Kernel tự động phản hồi hoặc gửi ARP request.

    Daemon trong Userspace thay thế Kernel xử lý ARP:
        CLI (giao diện dòng lệnh) tương tác với Daemon thông qua IPC (Inter-Process Communication).
        Daemon thực hiện hai tác vụ chính với ARP Cache:
            POST: Lưu các mục ARP mới vào cache.
            GET: Truy xuất địa chỉ MAC từ ARP cache.
        Daemon có thể xuất ARP cache ra file để lưu trữ.

    Trao đổi ARP giữa Host A và các Host khác:
        Khi một tiến trình trên Host B (ví dụ CLI thực hiện lệnh ping) cần biết địa chỉ MAC của Host A, nó sẽ gửi ARP Request.
        Host A không để Kernel phản hồi mà Daemon trong Userspace sẽ tiếp nhận ARP Request, tìm địa chỉ MAC trong ARP cache, và nếu có, sẽ gửi ARP Reply.
        Nếu Daemon không tìm thấy trong cache, nó có thể gửi ARP Request tới các Host khác để tìm kiếm địa chỉ MAC phù hợp.
### 2.5 Communication Diagram
chưa sửa

![image](https://github.com/user-attachments/assets/53fa10a7-fa45-43e0-b8b7-344429a942cd)

### 2.6 Algorithm flowchart
#### a. Daemon

**![ảnh](https://github.com/user-attachments/assets/42a4a3af-e645-40aa-96c9-ef55a042868a)
**
#### b. CLI

![ảnh](https://github.com/user-attachments/assets/d1c37ecc-81ac-485a-adac-b79dab919368)

#### c. Giao tiếp ngoại 

![ảnh](https://github.com/user-attachments/assets/bc28a836-25ec-43c4-a9dd-017c204d50c4)

### 2.7 Features
#### a. Giao tiếp giữa Daemon và CLI
Sử dụng **Unix domain socket** vì:
- **Buffer trên Kernel (Backlog)**:
 - Khi nhiều **CLI** gửi đến cùng lúc, các kết nối được xếp vào **queue** (backlog) của `listen()`.
 - Mặc định, chỉ khi **queue** này đầy, các kết nối mới bị từ chối.
- **Kết nối độc lập**:
 - Mỗi khi **Daemon** `accept()`, mỗi yêu cầu của **CLI** sẽ được gán một **socket** riêng (**file descriptor** riêng).
 - Các kết nối hoạt động độc lập, dữ liệu từ **CLI A** không bao giờ lẫn với **CLI B**.
- Sử dụng `select()` hoặc `epoll()` để **daemon** có thể xử lý nhiều **CLI**.

#### b. ARP cache

```c
struct arp_entry {
    char ip_addr[16];        
    char mac_addr[18];       
    time_t timestamp;        
};
```

Lựa chọn **Hash map**, bởi vì em thấy rằng dữ liệu được lưu trữ phụ thuộc vào **IP addr**, nên coi **IP addr** sẽ là **key** và cặp **(MAC addr và timestamp)** sẽ là **value**, dưới đây là mô tả:
- **Key**: Địa chỉ IP (string hoặc số nguyên sau khi chuyển đổi từ IP).
- **Value**: Cấu trúc arp_entry chứa MAC addr và timestamp.
- Sử dụng hash function để ánh xạ IP addr thành vị trí trong bảng.

```c
struct arp_entry {
    char ip_addr[16];      // Địa chỉ IP (key)
    char mac_addr[18];     // Địa chỉ MAC
    time_t timestamp;      // Thời điểm lưu entry
    UT_hash_handle hh;     // Cấu trúc hỗ trợ trong uthash
};
```

**Ưu điểm**:
- Dễ dàng kiểm tra xem IP addr đã tồn tại hay chưa.
- Phù hợp với ARP cache vì số lượng phần tử không quá lớn.

### 2.8 Running the code
- Chạy **Daemon process** ở Terminal 1:
  - $ cd Code_Daemon
  - $ make
  - sudo ./daemon
  - Ở trong **arp cache table** đã lưu trữ sẵn địa chỉ `(192.168.1.1, 11:22:33:44:55:66)` để test
- Chạy **CLI process** ở Terminal 2:
  - $ cd Code_CLI
  - $ make
  - $ ./cli
  - Nhập input là địa chỉ IP đích cần truy vấn (giả sử muốn test với địa chỉ IP lưu sẵn là `192.168.1.1`)
 
=> Input ở Terminal 2 là địa chỉ MAC đích.

- Vấn đề chưa giải quyết: Em đã test hết các module ở trong Daemon và đã chạy thành công, chỉ có module `(arp.c, arp.h)` là chưa test được vì em chưa cài linux để bắt bản tin arp giữa 2 máy => em lưu trữ sẵn 1 cặp địa chỉ IP, MAC vào **arp cache table** sẵn để test => đã hoàn thành.

