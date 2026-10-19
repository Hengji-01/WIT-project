#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "device_protocol.h"

//函数1：创建客户端并连接服务器socket + connect 
int create_client(const char *server_ip, int server_port)
{
    //创建套接字
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket 创建失败");
        exit(1);
    }

    struct sockaddr_in server_addr;
    //清理&server_addr和sizeof(server_addr)里面的垃圾数据
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port        = htons(server_port);

    //连接服务端
    if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect失败");
        exit(1);
    }

    printf("client已连接到server: %s:%d\n", server_ip, server_port);
    return client_fd;
}

 //模拟采集一次设备状态
 //这里用随机数模拟替代,后续
void collect_device_status(DeviceStatus *st, uint32_t device_id)
{
    st->device_id = device_id;
    st->cpu_usage = (float)(rand() % 10000) / 100.0f;   // 0.00 ~ 99.99
    st->mem_usage = (float)(rand() % 10000) / 100.0f;
    st->timestamp = (uint64_t)time(NULL);
}

//上报一次设备状态(send+打印)
void report_device_status(int client_fd, const DeviceStatus *st)
{
    ssize_t n = send(client_fd, st, sizeof(*st), 0);
    if (n < 0) {
        perror("send 失败");
        return;
    }

    printf("[客户端] 已上报: 设备ID=%u, CPU=%.2f%%, MEM=%.2f%%, TS=%llu\n",
           st->device_id,
           st->cpu_usage,
           st->mem_usage,
           (unsigned long long)st->timestamp);
}



int main(int argc, char *argv[])
{
    // 允许命令行指定设备 ID，方便一台机器起多个客户端模拟多设备
    // 利用三元运算写
    uint32_t device_id = (argc > 1) ? (uint32_t)atoi(argv[1]) : 1001;
    
/*
也可以写成这种形式
uint32_t device_id;
if (argc > 1) {
    device_id = (uint32_t)atoi(argv[1]);
} else {
    device_id = 1001;
}

*/

    // 让每次运行时的种子不同
    //time(NULL) 保证不同运行时刻种子不同
    //^ device_id 保证同一时刻不同设备种子也不同
    //异或:相同为 0，不同为 1
    //这样写保证运算u是不会n撞数据
    srand((unsigned)time(NULL) ^ device_id);   



    int client_fd = create_client(SERVER_IP, SERVER_PORT);

    /* 每 2 秒上报一次，共上报 5 次，方便演示 */
    for (int i = 0; i < 5; i++) {
        DeviceStatus st;
        collect_device_status(&st, device_id);//采集一次数据
        report_device_status(client_fd, &st);//发送出去
        sleep(2);//睡2秒
    }

    close(client_fd);
    printf("[客户端] 上报完成，连接已关闭\n");
    return 0;
}