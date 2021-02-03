
#include <thread>
#include <chrono>

#include "EnginePluginAPI.h"

 
int main()
{
    get_ep_interface(nullptr);
    while(true){
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}

