#include "pipe.h"

int main() {
    PIPE pipe(PipeName);
    pipe.init();
    pipe.open();
    SOCKET* cpp = pipe.Socket("cpp");
    SOCKET* go  = pipe.Socket("go");
    cpp->sockets = &pipe.sockets;
    go->sockets = &pipe.sockets;
    cpp->AttachPipe(PipeName);
    go->AttachPipe(PipeName);
    std::thread t1([cpp,go]() {
        if(cpp->osh.Connect(go)){
                cpp->osh.Send("Header:Testing","Data:Ali Mortada");
                std::this_thread::sleep_for(std::chrono::seconds(3));
                cpp->osh.Disconnect();
        }
        std::cout<<"Thread 1 Closed.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    
    std::thread t2([cpp,go]() {
        std::string Header, Data;
        while(go->osh.Listen()){
            Header = go->osh.ReadHeader();
            Data   = go->osh.ReadData();
            std::cout<<Header<<"\n"<<Data<<"\n";
        }
        std::cout<<"Thread 2 Closed.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });
    
    
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // Wait for threads to finish
    t1.join();
    t2.join();
    return 0;
}