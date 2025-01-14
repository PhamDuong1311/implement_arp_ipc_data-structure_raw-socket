# Software Design Description
## For ARP Implementation using Raw Socket

Version 0.1
Prepared by Pham Hong Duong  
 
Table of Contents
=================
* 1 [Introduction](#1-introduction)
  * 1.1 [Document Purpose](#11-document-purpose)
  * 1.2 [Keyworks](#12-keyworks)
  * 1.3 [References](#13-references)
  * 1.4 [Document Overview](#14-document-overview)
* 2 [Design](#2-overall-description)
    * 2.1 [Context](#21-context)
    * 2.2 [Logical](#22-logical)
    * 2.3 [Implementation](#23-implementation)
    * 2.4 [Flowchart](#24-flowchart)
    * 2.5 [Communication Diagram](#25-communication-diagram)
    * 2.6 [Algorithm flowchart](#26-algorithm-flowchart)
    * 2.7 [The mechanisms used use](#27-the-mechanisms-used-use)
    * 2.8 [Feature](#28-feature)
    * 2.9 [Prerequisites](#29-prerequisites)
    * 2.10 [File Description](#210-file-description)
    * 2.11 [Running the code](#211-running-the-code)

## 1. Introduction
Document này là cung cấp mô tả thiết kế phần mềm cho việc implement giao thức **ARP** trong **userspace** bằng **raw socket**. Document sẽ mô tả các yêu cầu, thiết kế, và các chi tiết implement phần mềm **ARP** => giúp hiểu rõ về cách hệ thống hoạt động và tương tác với mạng.
### 1.1 Document Purpose
Project này sẽ triển khai giao thức **ARP** trong **userspace**, sử dụng **raw socket** để nhận và gửi các gói **ARP Request** và **ARP Reply**. Các chức năng chính bao gồm:
- Quản lý **ARP cache** với **timeout** là `15` giây.
- Gửi và nhận các gói **ARP Request** và **ARP Reply**.
- Hoạt động trên tầng **userspace** mà không thông qua **kernel network stack**.
### 1.2 Keywords
- **ARP**: Address Resolution Protocol, giao thức mapping địa chỉ IP -> địa chỉ MAC thuộc L2 trong cùng 1 network.
- **Raw Socket**: Loại socket nhận data từ L2 trong kernel network stack và bypass lên thẳng userspace.
- **Daemon**: Process chạy ngầm không phụ thuộc vào terminal, sẽ kết thúc khi tắt nguồn.
- **Cache Timeout**: Thời gian sống của một entry trong ARP table để lookup sự mapping giữa địa chỉ IP và địa chỉ MAC, sau thời gian này entry sẽ bị xóa.
- **CLI**: Command-line interface, giao diện dòng lệnh tương tác trực tiếp với người dùng.
- **ioctl**: Hàm tương tác với hardware, trong bối cảnh sử dụng  thì hàm này sẽ dùng để get địa chỉ IP từ 1 NIC cụ thể.
- **IPC**: cơ chế giao tiếp giữa các process với nhau, trong bối cảnh này thì giao tiếp giữa daemon process và CLI process.
- **NIC**: nơi truyền và nhận trực tiếp các gói tin mạng raw chưa qua xử lý, trong bối cảnh này sẽ sử dụng 2 card wifi để tương tác với nhau (trên cùng 1 host).
### 1.3 References
- RFC 826 - "Ethernet Address Resolution Protocol"
- "Unix Network Programming" - Example of Raw Sockets
- Linux Interface Programming guide
- etc... 
### 1.4 Document Overview
Tài liệu này được chia thành các phần sau:
- Phần 1: Mục đích tài liệu và mô tả hệ thống.
- Phần 2: Thiết kế phần mềm chi tiết.
## 2. Design
### 2.1 Context
Để các Host có thể liên lạc với nhau trong cùng 1 LAN, các Host phải dựa L2 header để biết được địa chỉ MAC của nhau, vậy làm thé nào biết được địa chỉ MAC của nhau khi chúng ta đã có trong tay địa chỉ IP => dựa vào ARP protocol. ARP là một giao thức mạng giúp ánh xạ địa chỉ IP sang địa chỉ MAC (Media Access Control) trong mạng cục bộ (LAN). Ở trong sản phầm này, em sẽ sử dụng 2 process: Daemon process và CLI process. Daemon sẽ thực hiện các tác vụ ARP liên tục, trong khi CLI sẽ là công cụ cho người dùng tương tác với daemon để gửi ARP request và nhận ARP reply. Mục tiêu là xây dựng một daemon ARP có thể xử lý ARP request của ứng dụng khác, đồng thời có khả năng gửi ARP request khi có trigger từ CLI tới các interface khác để giao tiếp.
### 2.2 Logical
- **Daemon**: Daemon ARP sẽ lắng nghe yêu cầu ARP request từ CLI, gửi ARP reply sau khi lấy được thông tin của inteface cụ thể, lưu trữ kết quả vào ARP cache, và thực hiện việc timeout cache sau 15 giây.
- **CLI**: CLI cho phép người dùng gửi yêu cầu ARP request tới daemon để nhận lại ARP reply.
- **ARP Cache**: Lưu trữ các ánh xạ IP-MAC trong một thời gian nhất định (15 giây) để tái sử dụng.
- **Giao tiếp giữa daemon và CLI**: CLI gửi yêu cầu ARP request đến daemon thông qua cơ chế IPC (Inter-process communication) và daemon sẽ phản hồi ARP reply.
### 2.3 Implementation
- **Daemon ARP**:
  + Tạo daemon: Daemon sẽ chạy dưới dạng một process nền. Nó sẽ lắng nghe các yêu cầu từ CLI thông qua IPC.
  + ARP Request/Reply: Khi nhận được yêu cầu ARP, daemon sẽ sử dụng`ioctl()` để gửi ARP request và nhận ARP reply.
  + Lưu trữ ARP cache: Daemon sẽ lưu các địa chỉ MAC trong ARP cache và mỗi entry sẽ có thời gian sống là 15 giây.
  + Timeout: Sau 15 giây, các mục trong ARP cache sẽ tự động bị xóa.
  + Giao tiếp với CLI: Daemon và CLI giao tiếp qua cơ chế IPC.
- **CLI**:
  + CLI sẽ nhận lệnh từ người dùng để yêu cầu ARP request từ daemon.
  + CLI sẽ gửi yêu cầu ARP đến daemon qua IPC.
  + CLI sẽ hiển thị kết quả ARP reply mà daemon trả lại.
- **Giao tiếp IPC**:
  + Sử dụng 1 trong các cơ chế IPC (em chưa chọn) để giao tiếp giữa CLI và daemon.
- **Gửi ARP Request khi CLI được trigger**:
  + Daemon có thể gửi ARP request tới các interface của hệ thống để kiểm tra và cập nhật ARP cache của chính nó.
- **Cơ chế ioctl**:
  + Sử dụng `ioctl` để tương tác với giao diện mạng của hệ thống và thực hiện việc gửi ARP request.
### 2.4 Flowchart

![image](https://github.com/user-attachments/assets/de17fca0-b4f2-4f57-98fe-48356cee1c4e)


- **CLI gửi yêu cầu ARP Request**: Người dùng nhập lệnh ARP trong CLI (có địa chỉ IP đích), CLI gửi yêu cầu ARP qua IPC tới daemon.
- **Daemon nhận yêu cầu**: Daemon nhận yêu cầu ARP từ CLI, kiểm tra địa chỉ LCI yêu cầu xem có trong ARP cache không, nếu không thì tiến hành gửi ARP Request tới các interface mạng của hệ thống.
- **Daemon nhận ARP Reply**: Daemon nhận ARP Reply từ thiết bị mạng với IP-MAC cần thiết.
- **Daemon trả kết quả cho CLI**: Daemon gửi thông tin ARP Reply (IP-MAC) về cho CLI.
- **Daemon lưu vào ARP Cache**: Daemon lưu thông tin IP-MAC vào ARP cache theo 1 data structure (linked-list hoặc hash table phù hợp) và gán timeout cho mục đó.
### 2.5 Communication Diagram

![image](https://github.com/user-attachments/assets/53fa10a7-fa45-43e0-b8b7-344429a942cd)

### 2.6 Algorithm flowchart

![image](https://github.com/user-attachments/assets/71c57c00-c2ac-488c-8911-9be350ec2540)


### 2.7 The mechanisms used use
#### a. IPC
Dựa vào lưu đồ thuật toán trên, ta có thể thấy đặc điểm của **ICP** đó là:
- Hai chiều giao tiếp:
   - **CLI** gửi request (IP addr cần query) đến **Daemon**.
   - **Daemon** gửi phản hồi (MAC addr hoặc "failed") lại **CLI**.
- Mở rộng rằng nhiều **CLI** có thể giao tiếp đồng thời với **Daemon**:
   - Mỗi **CLI** có thể gửi yêu cầu riêng và đợi phản hồi từ **Daemon**.
   - **Daemon** xử lý tuần tự hoặc song song từng yêu cầu.

=> Từ những đặc điểm đó, em lựa chọn cơ chế **Message queue** với ưu điểm:
- Phân biệt **CLI** dễ dàng: Mỗi **CLI** có thể sử dụng type để định danh message (vd: type = `PID` của **CLI**).
- Độc lập giữa các **CLI**: Message Queue đảm bảo các **CLI** không truy cập sai thông điệp của nhau.

**Cách triển khai:**
- **CLI**:
   - Gửi request vào Message Queue chính với type là `PID` của **CLI**.
   - Đợi phản hồi với type tương ứng trong queue.
- **Daemon**:
  - Đọc thông điệp từ hàng đợi chung.
  - Xử lý ARP hoặc tìm trong cache.
  - Gửi phản hồi vào hàng đợi với type tương ứng `PID` của **CLI**.
 
**Cấu trúc message:**

```c
struct message {
    long type;          // PID của CLI
    char data[256];     // Dữ liệu (vd: IP addr hoặc MAC addr/failed)
};
```
 
#### b. ARP cache

```c
struct arp_entry {
    char ip_addr[16];        
    char mac_addr[18];       
    time_t timestamp;        
};
```

Lựa chọn Hash map, bởi vì em thấy rằng dữ liệu được lưu trữ phụ thuộc vào IP addr, nên coi IP addr sẽ là key và cặp (MAC addr và timestamp) sẽ là value, dưới đây là mô tả:
- Key: Địa chỉ IP (string hoặc số nguyên sau khi chuyển đổi từ IP).
- Value: Cấu trúc arp_entry chứa MAC addr và timestamp.
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

(Chưa xét tới vấn đề **Độ phức tạp của thuật toán**).
### 2.8 Features
### 2.9 Prerequisites
### 2.10 File Description
### 2.11 Running the code

