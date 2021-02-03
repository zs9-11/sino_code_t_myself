#include "ntrip_client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
//#include <thread>  // NOLINT.
#include <list>
#include <vector>

#include "ntrip_util.h"
#include "logger.h"


namespace libntrip {

#define TCP_CONNECT_TIMEOUT_S 10
#define TCP_RECV_TIMEOUT_S 10
#define TCP_CLOSE_LINGER_TIME_S 10

// GPGGA format example.
constexpr char gpgga_buffer[] = "$GNGGA,160008.00,3121.0111141,N,12117.5496604,E,4,20,0.5,36.3324,M,0.000,M,08,0009*41\r\n";

//
// Public method.
//

NtripClient::~NtripClient() {
  if (thread_is_running_) {
    Stop();
  }
}

void NtripClient::Init(const std::string &ip, const int &port,
            const std::string &user, const std::string &passwd,
            const std::string &mountpoint) {
        server_ip_ = ip;
        server_port_ = port;
        user_ = user;
        passwd_ = passwd;
        mountpoint_ = mountpoint;
        request_data_ = new char[NTRIP_REQUEST_BUF_SIZE];
        recv_buffer_ = new char[NTRIP_SOCKET_BUF_SIZE];
}
 
bool NtripClient::open_socket(void) {
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        LOG_DEBUG("Create socket fail\n");
        return false;
    }

    // Connect to caster.
    if (connect(socket_fd, reinterpret_cast<struct sockaddr *>(&server_addr),
              sizeof(struct sockaddr_in)) < 0) {
        LOG_DEBUG("Connect caster failed!!!\n");
        return false;
    }

    int flags = fcntl(socket_fd, F_GETFL);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    // TCP socket keepalive.
    int keepalive = 1;  // Enable keepalive attributes.
    int keepidle = 30;  // Time out for starting detection.
    int keepinterval = 5;  // Time interval for sending packets during detection.
    int keepcount = 3;  // Max times for sending packets during detection.
    setsockopt(socket_fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive,
             sizeof(keepalive));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPINTVL, &keepinterval,
             sizeof(keepinterval));
    setsockopt(socket_fd, SOL_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
    socket_fd_ = socket_fd;
    return true;
}

void NtripClient::close_socket(void)
{
	if(socket_fd_ >= 0){
		close(socket_fd_);
        socket_fd_ = -1;
	}
	printf("###close socket###\r\n");
}

bool NtripClient::write_socket(const char*  data, int length)
{
	int ret = send(socket_fd_, (const void*)data, length, 0);
	if(ret < 0)
	{
		printf("*** send fail ***\r\n");
        return false;
	}

	//return ret;
    return true;
}

bool NtripClient::read_socket(char* data, int& length)
{
    int ret = recv(socket_fd_, data, length, 0);
	if(ret > 0)	{
        length = ret;
		//printf("recv data: [%d]%s\r\n", ret, data);
	}else if(ret == 0){
		//printf("*** peer closed ***\r\n");
		//return false;
		//printf("*** read nothing ***\r\n");
        length = ret;
	}else{
		if(!(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)){
			printf("*** error occurs ***\r\n");
		    //return -1;
            return false;
		}else{
			printf("wait for a while\r\n");
		    //return 0;
		}
	}
    //return ret;
    return true;
}

bool NtripClient::login(void){
    char userinfo_raw[48] = {0};
    char userinfo[64] = {0};
  
    snprintf(userinfo_raw, sizeof(userinfo_raw) , "%s:%s",
           user_.c_str(), passwd_.c_str());
    Base64Encode(userinfo_raw, userinfo);
    snprintf(request_data_, NTRIP_SOCKET_BUF_SIZE,
           "GET /%s HTTP/1.1\r\n"
           "User-Agent: %s\r\n"
           "Accept: */*\r\n"
           "Connection: close\r\n"
           "Authorization: Basic %s\r\n"
           "\r\n",
           mountpoint_.c_str(), kClientAgent, userinfo);

    LOG_DEBUG("Send request :%s\n", request_data_);
    // Send request data.
    if (!write_socket(request_data_, strlen(request_data_))) {
        LOG_DEBUG("Send request failed!!!\n");
        //close(socket_fd_);
        return false;
    }

    // Waitting for request to connect caster success.
    int timeout = 10;
    while (timeout--) {
        int readSize = NTRIP_SOCKET_BUF_SIZE;
        bool ret = read_socket(recv_buffer_, readSize);
        if ((ret) &&  readSize >  0 && !strncmp(recv_buffer_, "ICY 200 OK\r\n", 12)) {
        //if ((ret) &&  readSize >  0 && strstr(recv_buffer_, "200 OK")) {
            return true;
        }else if (ret == false) {
            return false;
        }
        sleep(1);
    }

    if (timeout <= 0) {
        return false;
    }
    return true;
}

bool NtripClient::Start(void) {
  thread_ = std::thread(&NtripClient::ThreadHandler, this);
  thread_.detach();
  service_is_running_ = true;
  printf("NtripClient service starting ...\n");
  return true;
}

void NtripClient::Stop(void) {
  service_is_running_ = false;
  thread_is_running_ = false;
  close_socket();
}

void NtripClient::ThreadHandler(void) {
    int ret;
    thread_is_running_ = true;

    fd_set read_fds, write_fds;
    struct timeval	t;

    while (thread_is_running_) {
        close_socket();
        if(!open_socket()){
            continue;
        }
        printf("###ntrip connected###\n");
        
        if(!login()){
            continue;
        }
        LOG_DEBUG("###ntrip logined###\n");
        
        while(true){
	        t.tv_sec = TCP_CONNECT_TIMEOUT_S;
	        t.tv_usec = 0;

	        FD_ZERO(&read_fds);
	        FD_ZERO(&write_fds);

	        FD_SET(socket_fd_, &read_fds);
	        FD_SET(socket_fd_, &write_fds);

	        ret = select(socket_fd_ + 1, &read_fds, &write_fds, NULL, &t);

	        //printf("select ret: %d\r\n", ret);

	        if(ret < 0)	{
		        printf("*** select error ***\r\n");
                //statusCallback
                break;
            }else if( ret == 0 ){
		        printf("*** select timeout ***\r\n");
                continue;
            }else{
	            if(!FD_ISSET(socket_fd_, &read_fds) && !FD_ISSET(socket_fd_, &write_fds))	{
		            printf("*** connect fail ***\r\n");
                    //statusCallback
                    break;
		        }else if(FD_ISSET(socket_fd_, &read_fds) && !FD_ISSET(socket_fd_, &write_fds)){
		            printf("*** connect fail ***\r\n");
                    //statusCallback
                    break;
	            }
                
                if(FD_ISSET(socket_fd_, &write_fds))	{
		            //printf("*** send GGA request:%s***\n", ggaRequestBuffer);
                    //gpgga_buffer
                    //if(!write_socket(gga_buffer_, gga_buffer_.length())){
                    if(!write_socket(gpgga_buffer, strlen(gpgga_buffer))){
                        break;
                    }
                }
                if(FD_ISSET(socket_fd_, &read_fds))	{
                    int readSize  =  NTRIP_SOCKET_BUF_SIZE;
                    if(read_socket(recv_buffer_, readSize)){
                        // dataCallback->cb_rtcmdata(rtcmReadBuffer, ret);
                        if(readSize){
                            callback_(recv_buffer_, readSize);
                        }
                    }else{
                        break;
                    }
	            }
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        }
    }
    close_socket();
    thread_is_running_ = false;
    service_is_running_ = false;
}

}  // namespace libntrip
