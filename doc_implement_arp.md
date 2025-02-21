# Software Design Description
## For ARP Implementation using Raw Socket

Version 0.2
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
    * 2.2 [Logical](#22-logical)
    * 2.3 [Implementation](#23-implementation)
    * 2.4 [System Model](#24-system-model)
    * 2.5 [Communication Diagram](#25-communication-diagram)
    * 2.6 [Algorithm flowchart](#26-algorithm-flowchart)
    * 2.7 [Features](#27-features)
    * 2.8 [Running the code](#28-running-the-code)

## 1. Introduction
Document này là cung cấp mô tả thiết kế phần mềm cho việc implement giao thức **ARP** trong **userspace** bằng **raw socket**. Document sẽ mô tả các yêu cầu, thiết kế, và các chi tiết implement phần mềm **ARP** => giúp hiểu rõ về cách hệ thống hoạt động và tương tác với mạng.
### 1.1 Document Overview
Tài liệu này được chia thành các phần sau:
- Phần 1: Mục đích tài liệu và mô tả hệ thống.
- Phần 2: Thiết kế phần mềm chi tiết.
### 1.2 Document Purpose
Project này sẽ triển khai giao thức **ARP** trong **userspace**, sử dụng **raw socket** để nhận và gửi các gói **ARP Request** và **ARP Reply**. Các chức năng chính bao gồm:
- Có thể hoạt động trên nhiều **terminal** cùng thiết bị hoặc khác thiết bị (Nhiều **CLIs** giao tiếp với một **Daemon**).
- Gửi và nhận các gói **ARP Request**, **ARP Reply** và quản lý **ARP cache** với **timeout**.
- Hoạt động trực tiếp trên tầng **userspace** mà không thông qua **kernel network stack**.
### 1.3 Keywords
- **ARP**: Address Resolution Protocol, giao thức mapping địa chỉ IP -> địa chỉ MAC thuộc L2 trong cùng 1 network.
- **Raw Socket**: Loại socket nhận data từ L2 trong kernel network stack và bypass lên thẳng userspace.
- **Daemon**: Process chạy ngầm không phụ thuộc vào terminal, sẽ kết thúc khi tắt nguồn.
- **Cache Timeout**: Thời gian sống của một entry trong ARP table để lookup sự mapping giữa địa chỉ IP và địa chỉ MAC, sau thời gian này entry sẽ bị xóa.
- **CLI**: Command-line interface, giao diện dòng lệnh tương tác trực tiếp với người dùng.
- **ioctl**: Hàm tương tác với hardware, trong bối cảnh sử dụng  thì hàm này sẽ dùng để get địa chỉ IP từ 1 NIC cụ thể.
- **IPC**: cơ chế giao tiếp giữa các process với nhau, trong bối cảnh này thì giao tiếp giữa daemon process và CLI process.
- **NIC**: nơi truyền và nhận trực tiếp các gói tin mạng raw chưa qua xử lý, trong bối cảnh này sẽ sử dụng 2 card wifi để tương tác với nhau (trên cùng 1 host).
### 1.4 References
- RFC 826 - "Ethernet Address Resolution Protocol"
- "Unix Network Programming" - Example of Raw Sockets
- Linux Interface Programming guide
- etc... 
## 2. Design
### 2.1 Context
Để các thiết bị có thể liên lạc với nhau trong cùng 1 **LAN**, chúng phải dựa **L2 header** để biết được địa chỉ **MAC** của nhau, vậy làm thé nào biết được địa chỉ **MAC** của nhau khi chúng ta đã có trong tay địa chỉ **IP** => dựa vào **ARP protocol**. **ARP** là một giao thức mạng giúp ánh xạ địa chỉ **IP** sang địa chỉ **MAC** (Media Access Control) trong mạng cục bộ (**LAN**). Ở trong phần mềm này, em sẽ sử dụng 2 loại process: **Daemon process** và **CLI process**. Người dùng nhập vào địa chỉ **IP** cần truy vấn tại chương trình chạy ở **CLI process**, **CLI** sẽ gửi thông báo tới **Daemon** yêu cầu truy vấn ra địa chỉ **MAC** phù hợp. Nếu giả sử tại **Daemon** đã lưu trữ sẵn **MAC** tương ứng với **IP** thì ngay lập tức nó sẽ gửi lại **MAC** cho **CLI** và hiển thị kết quả. Còn nếu không tìm thấy **MAC** phù hợp, **Daemon** sẽ gửi 1 **ARP request** ra ngoài để yêu cầu nhận được **MAC** của thiết bị tương thích và **ARP reply** lại **MAC** đó, cuối cùng là gửi lại kết quả cho **CLI**. Ngoài ra phần mềm còn có những tính năng thao tác và hiển thị **ARP cache table**.
### 2.2 Logical
- **Daemon**: Daemon sẽ lắng nghe thông báo (địa chỉ IP, yêu cầu thao tác với ARP cache) từ các CLI, gửi lại kết quả tương ứng sau khi lấy được thông tin của inteface cụ thể, lưu trữ kết quả vào ARP cache, và thực hiện việc timeout cho ARP cache table.
- **CLI**: CLI cho phép người dùng gửi thông báo tới daemon để nhận lại kết quả.
- **ARP Cache**: Lưu trữ các ánh xạ IP-MAC trong một thời gian nhất định (15 giây) để tái sử dụng.
- **Giao tiếp giữa daemon và CLI**: CLIs gửi thông báo đến daemon thông qua cơ chế IPC (Inter-process communication) và daemon sẽ phản hồi lại kết quả cho từng CLI.
### 2.3 Implementation
- **Daemon**:
  + Tạo daemon: Daemon sẽ chạy dưới dạng một process nền. Nó sẽ lắng nghe các yêu cầu từ CLI thông qua IPC.
  + ARP Request/Reply: Khi nhận được thông báo từ CLI, daemon sẽ sử dụng`ioctl()` để gửi ARP request và nhận ARP reply.
  + Lưu trữ ARP cache: Daemon sẽ lưu các địa chỉ MAC trong ARP cache và mỗi entry sẽ có timeout là 15 giây.
  + Giao tiếp với CLI: Daemon và CLIs giao tiếp qua cơ chế IPC.
- **CLIs**:
  + CLIs sẽ nhận lệnh từ người dùng để thông báo tới daemon.
  + CLIs sẽ hiển thị kết quả mà daemon trả lại.
- **Giao tiếp IPC**:
  + Sử dụng Unix domain socket để có thể giao tiếp Daemon với nhiều CLI thông qua `select()` hoặc `epoll()`.
- **Cơ chế ioctl**:
  + Sử dụng `ioctl` để tương tác với giao diện mạng của hệ thống và thực hiện việc gửi ARP request.
- **Cơ chế nhận packet**:
  + Packet nhận được tại NIC sẽ đẩy thẳng lên userspace và drop tại kernel thông qua `iptables` (chặn packet ở Netfilter (L3-L4)).

### 2.4 System Model

![image](https://github.com/user-attachments/assets/de17fca0-b4f2-4f57-98fe-48356cee1c4e)


- **CLI gửi thông báo**: Người dùng nhập lệnh trong CLI, CLI gửi thông báo qua IPC tới daemon.
- **Daemon nhận thông báo**: Daemon nhận thông báo từ CLI, kiểm tra địa chỉ CLI yêu cầu xem có trong ARP cache không, nếu không thì tiến hành gửi ARP Request tới các interface mạng của hệ thống.
- **Daemon nhận ARP Reply**: Daemon nhận ARP Reply từ thiết bị mạng với IP-MAC cần thiết.
- **Daemon trả kết quả cho CLI**: Daemon gửi kết quả (MAC) về cho CLI.
- **Daemon lưu vào ARP Cache**: Daemon lưu thông tin IP-MAC vào ARP cache theo 1 data structure và gán timeout cho entry đó.
### 2.5 Communication Diagram

![image](https://github.com/user-attachments/assets/53fa10a7-fa45-43e0-b8b7-344429a942cd)

### 2.6 Algorithm flowchart
#### a. Daemon

![ảnh](https://github.com/user-attachments/assets/42a4a3af-e645-40aa-96c9-ef55a042868a)

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

