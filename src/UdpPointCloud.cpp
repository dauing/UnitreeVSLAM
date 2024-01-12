#include "UdpPointCloud.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <vector>
#include <StereoCameraCommon.hpp>

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
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip.c_str(), &servaddr.sin_addr) <= 0)
    {
        perror("invalid address/address not supported");
        exit(EXIT_FAILURE);
    }

    auto serialized_data = serializePointCloud(pcl_vec);
    if (sendto(sockfd, serialized_data.data(), serialized_data.size(), MSG_CONFIRM, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }

    close(sockfd);
}
