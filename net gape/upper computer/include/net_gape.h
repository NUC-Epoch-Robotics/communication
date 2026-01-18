#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
class cl{
    private:
    int socket_cl;
    sockaddr_in serv_addr;
    const char* ip;     //上位机IP
    int port;           //端口号（需于下位机相同）
    public:
    cl(const char* ip="192.168.10.32",int port=3000){
        socket_cl=socket(AF_INET, SOCK_STREAM, 0);
        if(socket_cl<0){
            throw std::runtime_error("Socket创建失败");
        }
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
         if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            throw std::runtime_error("IP地址无效");
        }
        std::cout<<"初始化成功"<<std::endl;
    }
    bool connect_cl(){
        if(connect(socket_cl,(const sockaddr*)&serv_addr,sizeof(serv_addr))<0){
            std::cerr << "连接失败! 错误代码: " << errno 
                  << ", 错误信息: " << strerror(errno) << std::endl;
            return false;
            }
        else {
                return true;
            }
        }
    bool sendDoubles(char data[6]) {
        // 发送数据
        if (send(socket_cl, data, 6 * sizeof(char), 0) != 6 * sizeof(char)) {
            return false;
        }
        
        std::cout << "发送数据: ";
        for (int i = 0; i < 6; i++) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
        return true;
    }
    bool receiveDoublesWithTimeout(char (&data)[6], int timeout_seconds = 5) {
    std::cout << "等待接收数据，超时: " << timeout_seconds << "秒" << std::endl;
    
    // 设置超时
    timeval timeout;
    timeout.tv_sec = timeout_seconds;
    timeout.tv_usec = 0;
    setsockopt(socket_cl, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    // 使用阻塞接收
    ssize_t bytes_received = recv(socket_cl, data, 6 * sizeof(char), 0);
    
    // 恢复无超时
    timeout.tv_sec = 0;
    setsockopt(socket_cl, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    if (bytes_received == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            std::cerr << "接收超时 - 下位机没有发送数据" << std::endl;
        } else {
            std::cerr << "接收错误: " << strerror(errno) << std::endl;
        }
        return false;
    } else if (bytes_received == 0) {
        std::cerr << "连接已关闭" << std::endl;
        return false;
    } else if (bytes_received != 6 * sizeof(char)) {
        std::cerr << "数据不完整: " << bytes_received << "/" 
                  << 6 * sizeof(double) << " 字节" << std::endl;
        return false;
    }
    
    std::cout << "成功接收数据: ";
    for (int i = 0; i < 6; i++) {
        std::cout << data[i] << " ";
    }
    std::cout << std::endl;
    return true;
}
    ~cl() {
        if (socket_cl >= 0) {
            close(socket_cl);
        }
    }
};