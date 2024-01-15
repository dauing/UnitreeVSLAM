#include <sys/socket.h>  
#include <netinet/in.h>  
#include <iostream>  
#include <vector> 
#include <StereoCameraCommon.hpp>
#include <arpa/inet.h>
#include <unistd.h>
#include "UdpTrans.hpp"
  
// 从字节流中反序列化点云数据  
std::vector<PCLType> deserializePointCloud(const std::vector<char>& buffer) {  
    std::vector<PCLType> pcl_vec;  
    pcl_vec.reserve(buffer.size() / sizeof(PCLType));  
  
    for (size_t i = 0; i < buffer.size(); i += sizeof(PCLType)) {  
        PCLType point;  
        std::memcpy(&point, &buffer[i], sizeof(PCLType));  
        pcl_vec.push_back(point);  
    }  
  
    return pcl_vec;  
}  
  
// 接收点云数据  
std::vector<PCLType> receivePointCloud(const std::string& listen_ip, int listen_port) {  
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sockfd < 0) {  
        perror("socket creation failed");  
        exit(EXIT_FAILURE);  
    }  


    int opt = 1;  
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {  
    perror("setsockopt SO_REUSEADDR");  
    // Handle error appropriately  
    }  
  
    // Optionally, set SO_REUSEPORT if needed and supported  
    // Note: SO_REUSEPORT is not always supported and has special semantics  
    // that may not be desired in all use cases.  
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {  
        perror("setsockopt SO_REUSEPORT");  
        // Handle error appropriately  
    }  
  
    struct sockaddr_in servaddr = {};  
    servaddr.sin_family = AF_INET;  
    servaddr.sin_port = htons(listen_port); 
    //初始化一个 sockaddr_in 结构体来表示服务器（实际上是接收方）的地址和端口。

    if (inet_pton(AF_INET, listen_ip.c_str(), &servaddr.sin_addr) <= 0) {  
        perror("invalid address/address not supported");  
        exit(EXIT_FAILURE);  
    }  
    //使用 inet_pton 函数将点分十进制的IP地址转换为二进制格式，并检查其有效性。
  
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    } 
    // 将套接字绑定到指定的IP地址和端口上，并检查是否绑定成功。
  
    char buffer[1024 * 1024 * 1]; // 假设点云数据不会超过1MB，你可以根据需要调整  
    socklen_t len = sizeof(servaddr); 
    //声明一个字符数组 buffer 来存储接收到的数据


    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), MSG_WAITALL, (struct sockaddr *) &servaddr, &len);  
    if (n < 0) {  
        perror("recvfrom failed");  
        exit(EXIT_FAILURE);  
    }  
    //使用 recvfrom 函数接收数据
  
    close(sockfd);  
    return deserializePointCloud(std::vector<char>(buffer, buffer + n));  
}

// 将点云数据序列化为字节流
std::vector<char> serializePointCloud(const std::vector<PCLType> &pcl_vec)
{
    std::vector<char> buffer;
    buffer.reserve(pcl_vec.size() * sizeof(PCLType));

    for (const auto &point : pcl_vec)
    {
        buffer.insert(buffer.end(), reinterpret_cast<const char *>(&point), reinterpret_cast<const char *>(&point) + sizeof(PCLType));
    }

    return buffer;
}

// 发送点云数据
void sendPointCloud(const std::string &dest_ip, int dest_port, const std::vector<PCLType> &pcl_vec)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr = {};
    //声明并初始化了一个sockaddr_in结构体变量servaddr，所有成员都被初始化为0。

    servaddr.sin_family = AF_INET;
    //设置servaddr的sin_family成员为AF_INET，表明这是一个IPv4地址族。

    servaddr.sin_port = htons(dest_port);
    //将目标端口号dest_port从主机字节序转换为网络字节序，并存储在servaddr的sin_port成员中。

    if (inet_pton(AF_INET, dest_ip.c_str(), &servaddr.sin_addr) <= 0)
    {
        perror("invalid address/address not supported");
        exit(EXIT_FAILURE);
    }
    //使用inet_pton函数将点分十进制的IPv4地址字符串dest_ip转换为二进制的网络地址
    //并存储在servaddr.sin_addr中。

    auto serialized_data = serializePointCloud(pcl_vec);
    if (sendto(sockfd, serialized_data.data(), serialized_data.size(), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
    close(sockfd);
}