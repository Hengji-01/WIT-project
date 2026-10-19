#ifndef DEVICE_PROTOCOL_H
#define DEVICE_PROTOCOL_H

#include <stdint.h>

#define SERVER_PORT   8888
#define SERVER_IP     "127.0.0.1"
#define BUFFER_SIZE   1024


 //用 __attribute__((packed))取消编译器内存对齐填充
typedef struct __attribute__((packed)) {
    //设备ID
    uint32_t device_id;      
    //CPU
    float    cpu_usage;      
    //内存使用率
    float    mem_usage;
     // 采集时间戳 
    uint64_t timestamp;      
} DeviceStatus;

#endif // DEVICE_PROTOCOL_H