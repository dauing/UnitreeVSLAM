#include <vector> 
#include <StereoCameraCommon.hpp>

  
// 从字节流中反序列化点云数据  
std::vector<PCLType> deserializePointCloud(const std::vector<char>& buffer);
  
// 接收点云数据  
std::vector<PCLType> receivePointCloud(const std::string& listen_ip, int listen_port);

// 将点云数据序列化为字节流
std::vector<char> serializePointCloud(const std::vector<PCLType> &pcl_vec);

// 发送点云数据
void sendPointCloud(const std::string &dest_ip, int dest_port, const std::vector<PCLType> &pcl_vec);