#include <GL/gl.h>
#include <signal.h>
#include <cerrno>
#include <cfenv>
#include <unistd.h>
#include "glViewer/scenewindow.hpp"
#include "../include/UnitreeCameraSDK.hpp"
#include "UdpTrans.hpp"

#define RGB_PCL true ///< Color Point Cloud Enable Flag
const std::string listenIP = "192.168.123.100";
int listenPort = 50001;

void DrawScene(const std::vector<PCLType>& pcl_vec) {
    glBegin(GL_POINTS);
    for (uint i = 0; i < pcl_vec.size(); ++i) {
        PCLType pcl = pcl_vec[i];
        glColor3ub(pcl.clr(2), pcl.clr(1), pcl.clr(0));
        glVertex3f(-pcl.pts(0), -pcl.pts(1), pcl.pts(2));
    }
    glEnd();
}

void DrawScene(const std::vector<cv::Vec3f>& pcl_vec) {
    glBegin(GL_POINTS);
    for (uint i = 0; i < pcl_vec.size(); ++i) {
        cv::Vec3f pcl = pcl_vec[i];
        glColor3ub(255, 255, 0);
        glVertex3f(-pcl(0), -pcl(1), pcl(2));
    }
    glEnd();
}

bool killSignalFlag = false;
void ctrl_c_handler(int s){
    killSignalFlag = true;
    return ; 
}

int main(int argc, char *argv[]){
    
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = ctrl_c_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    
    glwindow::SceneWindow scene(960, 720, "Panorama 3D Scene");
    std::cout << "Glwindow has been created." << std::endl;
    
    while(true){
        
        if(killSignalFlag){
            break;
        }
        
        std::chrono::microseconds t;
#if RGB_PCL
        std::cout << "Prepare to recevie..." << std::endl;       
        std::vector<PCLType> pcl_vec;
        pcl_vec = receivePointCloud(listenIP, listenPort);
        std::cout << "Point cloud received." << std::endl;
        if (pcl_vec.size()==0){
            usleep(1000);
            std::cout << "No data!" << std::endl;
            continue;
        }
        std::cout << "Receive data:" << pcl_vec.size() << std::endl;
#else        
        std::vector<cv::Vec3f> pcl_vec;
        pcl_vec = receivePointCloud(listenIP, listenPort);
#endif

        if (scene.win.alive()) {
            if (scene.start_draw()) {
                DrawScene(pcl_vec);
                scene.finish_draw();
            }
        }
    }
    
    return 0;
}
