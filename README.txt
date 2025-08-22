## GD32_lwip_libmodbus ##
Modbus-TCP/IP Project. Using FreeRTOS / LWIP / libmodbus
## 移植方案(TCP):
# modbus-tcp.c文件更改
  1、将tcp-pi的相关函数先注释掉，并且将所有 free 和 malloc 函数全部替换成FreeRTOS的 Free 和 Malloc，并#include "Freertos.h"。
  2、文件中带有WIN32的部分都可以全部删掉。头文件<sys/types.h> <sys/ioctl.h>注释，in.h ip.h tcp.h netdb.h 全部换成 lwip/xxx。
  3、modbus_tcp_listen 函数中将以下部分注释掉。服务器测试代码写出来的时候，listen死活不能成功，就因为下面这个东西，给我sockfd关掉。
     // enable = 1;
     // if(setsockopt(new_s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1) {
     //     close(new_s);
     //     return -1;
     // }
     
# modbus-tcp.h文件更改
  删掉WIN32部分。
  
# modbus.c文件更改
  1、将所有 free 和 malloc 函数全部替换成FreeRTOS的 Free 和 Malloc 。添加"Freertos.h"，"task.h"。并且添加如下部分，目的在注释。
     //modbus.c中backend->send本身是调用modbus_backend_t结构体的send。
     //但是现在由于包含了"modbus-private.h" ，这个里面又包含了"sys/socket.h"，导致send等一系列bankend里的函数名全被替换成如lwip_send，而不是bankend->send，导致编译器报错说入口参数少了。
     //但是modbus.c又需要"modbus-private.h"的结构体声明和socket.h里time与fdset等结构体。
     #ifdef send
     #undef send
     #endif
     #ifdef recv
     #undef recv
     #endif
     #ifdef connect
     #undef connect
     #endif
     #ifdef close
     #undef close
     #endif
     #ifdef select
     #undef select
     #endif
   2、_sleep_response_timeout 函数里的内容替换成 vTaskDelay(ctx->response_timeout.tv_sec * 1000 + ctx->response_timeout.tv_usec / 1000);
   
# modbus.h文件更改
  删除WIN32部分。并且注释掉 RTU 的头文件。
  
# modbus_private.h文件更改
  去除 MSC_VER 宏（Microsoft编译器）相关的东西。
  
# modbus-data.c文件更改
  1、去除 `MSC_VER` 宏（Microsoft编译器）相关的东西，并且注释掉有个assert函数。使用microlib，可能 assert 功能是不全的，会编译出assert内部的有个函数没有。如果不使用microlib可以用aseert，但是会出现新的问题如下，并且网口不能正常工作（具体原因还未知）。所以为了避免这些问题，还是使用microlib，并且如果不小心不勾选microlib，那就用ARM V5 编译器编译一遍，再用V6编译器编译一遍就好了。

# modbus-version.h文件更改
  把版本号宏定义改成版本号。在注释头文件时，config.h sys/time type.h都没有，尽情注释掉。
  
# 主从任务
  网口接收任务 ethernetif_input 优先级 > tcp/ip协议栈任务优先级 > APP优先级。  
