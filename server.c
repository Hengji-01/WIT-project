#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "device_protocol.h"


 //函数1：创建服务器套接字,连接服务端socket + setsockopt + bind + listen 
 
int create_server(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket 创建失败");
        exit(1);
    }

    // 设置地址复用，避免重启时 "Address already in use"
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    //调用ipv4
    server_addr.sin_family      = AF_INET;
    // ANY监听所有网路关卡
    server_addr.sin_addr.s_addr = INADDR_ANY;   
    server_addr.sin_port        = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind 失败");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen 失败");
        exit(1);
    }

    printf("[服务器] 启动成功, 监听端口: %d\n", port);
    return server_fd;
}


 // 函数2：解析结构体+打印一条设备状态

void print_device_status(const DeviceStatus *st)
{
    printf("[服务器] 设备状态上报:\n");
    printf("设备ID: %u\n", st->device_id);
    printf("CPU使用率: %.2f%%\n", st->cpu_usage);
    printf("内存使用率: %.2f%%\n", st->mem_usage);
    printf("时间戳: %llu\n", (unsigned long long)st->timestamp);
}

//循环接收结构体信息
void handle_client(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("accept 失败");
        return;
    }

    printf("[服务器] 客户端已连接: IP = %s, Port = %d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    //循环接收：每次尝试收一个完整的 DeviceStatus 
    DeviceStatus st;
    while (1) {
        memset(&st, 0, sizeof(st));

        //把跑通链路先
        ssize_t n = recv(client_fd, &st, sizeof(st), 0);

        if (n == 0) {
            printf("[服务器] 客户端已断开\n");
            break;
        } else if (n < 0) {
            perror("recv 失败");
            break;
        } else if (n != sizeof(st)) {
            printf("[服务器] 收到不完整数据: %zd 字节 (期望 %zu)\n",
                   n, sizeof(st));
            continue;
        }

        print_device_status(&st);
    }

    close(client_fd);
}


int main(void)
{
    int server_fd = create_server(SERVER_PORT);

    //多设备循环
    while (1) {
        handle_client(server_fd);
    }

    close(server_fd);
    return 0;
}