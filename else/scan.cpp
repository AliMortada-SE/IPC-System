#include "pipe.h"
#include <iostream>
int main(){
    PIPE pipe(PipeName);
    pipe.init();
    pipe.open();
    SOCKET* cpp = pipe.Socket("cpp");
    SOCKET* go  = pipe.Socket("go");
    cpp->AttachPipe(PipeName);
    go->AttachPipe(PipeName);


    std::cout<<"->";
    SOCKET* socket = nullptr;
    uint8_t byte = 0;
    std::string line;
    std::string Mode;
    std::string Socket;
    std::string Offset;
    while(Mode != "exit"){
        std::cin>>Socket>>Offset;
        socket = pipe.FindSocket(0,Socket);
        if(socket==nullptr){
            std::cout<<"Socket not Found.\n";
            continue;
        }
        byte = socket->GetOffset(Offset);
        if(byte == 255){
            std::cout<<"Error While Reading Byte.\n";
            continue;
        }
        std::cout<<Socket<<"->"<<Offset<<"->"<<(int)byte<<"\n";
        
        std::cout<<"->";
    }
}