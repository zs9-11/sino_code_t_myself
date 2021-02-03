#ifndef NTRIPLIB_NTRIP_CLIENT_H_
#define NTRIPLIB_NTRIP_CLIENT_H_

#include <string>
#include <thread>  // NOLINT.
#include <functional>

namespace libntrip {

using ClientCallback = std::function<void(const char *, const int &)>;
#define NTRIP_REQUEST_BUF_SIZE   1024
#define NTRIP_SOCKET_BUF_SIZE    2048

class NtripClient {
public:
    NtripClient() = default;
    NtripClient(const NtripClient &) = delete;
    NtripClient& operator=(const NtripClient &) = delete;
    /*
    NtripClient(const std::string &ip, const int &port,
              const std::string &user, const std::string &passwd,
              const std::string &mountpoint) :
      server_ip_(ip),
      server_port_(port),
      user_(user),
      passwd_(passwd),
      mountpoint_(mountpoint) { }
    */
    ~NtripClient();
    
    void Init(const std::string &ip, const int &port,
            const std::string &user, const std::string &passwd,
            const std::string &mountpoint);
 
    // 设置发送的GGA语句.
    void set_gga_buffer(const std::string &gga_buffer) {
        gga_buffer_ = gga_buffer;
    }

    // 设置接收到数据时的回调函数.
    void OnReceived(const ClientCallback &callback) { callback_ = callback; }
    
    bool Start(void);
    
    void Stop(void);
    
    bool service_is_running(void) const {
        return service_is_running_;
    }

private:

    bool open_socket(void); 
  
    void close_socket(void);

    bool write_socket(const char*  data, int length);

    bool read_socket(char* data, int& length);

    bool login(void);

    // Thread handler.
    void ThreadHandler(void);

private:
    bool service_is_running_ = false;
    bool thread_is_running_ = false;
    std::thread thread_;
    std::string server_ip_;
    int server_port_ = -1;
    std::string user_;
    std::string passwd_;
    std::string mountpoint_;
    std::string gga_buffer_;
    int socket_fd_ = -1;

    char *request_data_ = nullptr;
    char *recv_buffer_ = nullptr;

    ClientCallback callback_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_NTRIP_CLIENT_H_
