#include <GL/gl.h>
#include <signal.h>
#include <cerrno>
#include <cfenv>
#include <unistd.h>
#include "glViewer/scenewindow.hpp"
#include "../include/UnitreeCameraSDK.hpp"
#include "UdpTrans.hpp"

#define RGB_PCL true ///< Color Point Cloud Enable Flag



bool killSignalFlag = false;
void ctrl_c_handler(int s){
    killSignalFlag = true;
    return ; 
}
const std::string ip = "192.168.123.100";
int port = 50001;

int main(int argc, char *argv[]){
    
    UnitreeCamera cam("stereo_camera_config.yaml");
    if(!cam.isOpened())
        exit(EXIT_FAILURE);

    cam.startCapture();
    cam.startStereoCompute();
    
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrl_c_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    std::cout << cam.getSerialNumber() << " " <<  cam.getPosNumber() << std::endl;
    
    
    while(cam.isOpened()){
        
        if(killSignalFlag){
            break;
        }
        
        std::chrono::microseconds t;
#if RGB_PCL       
        std::vector<PCLType> pcl_vec;
        if(!cam.getPointCloud(pcl_vec, t)){
            usleep(1000);
            sendPointCloud(ip, port, pcl_vec);
            continue;
        }
#else        
        std::vector<cv::Vec3f> pcl_vec;
        if(!cam.getPointCloud(pcl_vec, t)){
            usleep(1000);
            continue;
        }
#endif
    cam.stopStereoCompute();
    cam.stopCapture();
    return 0;
}
