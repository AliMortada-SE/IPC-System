//osh.cpp
#include "socket.h"
#include "utils.h"
SOCKET::OSH::OSH(SOCKET* p) : parent(p) {}
SOCKET::SOCKET() : osh(this) {}

bool SOCKET::OSH::Listen(){
    uint8_t byte = NONE;
    uint8_t flag = NONE;
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Error Socket Listening, File Not in Hand.\n";
        return 0;
    }
    std::cout<<"["<<parent->name<<"] Listening... (OSH).\n";
    parent->ReadByte(parent->Flag(),flag);
    while(flag==SEEN){
        parent->ReadByte(parent->Flag(),flag);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(flag == DONE){
        std::cout<<"["<<parent->name<<"] Socket Disconnect, Ready for Other Shots (OSH).\n";
        if(!parent->WriteByte(parent->Flag(),READY)) return 0;
        if(!parent->WriteByte(parent->ConnectionStatus(),IDLE)) return 0;
        flag = READY;
    }
    if(flag == READY){
        std::cout<<"["<<parent->name<<"] Listening For Incomming Shots(OSH).\n";
        while(byte!=RESERVED){
            parent->ReadByte(parent->ConnectionStatus(),byte);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }   
        std::cout<<"["<<parent->name<<"] New Shot Arrived (OSH).\n";
    }
    else if(flag == READ){
        std::cout<<"["<<parent->name<<"] Reading more Data(OSH).\n";
        return 1;
    }
    else if(flag == WAIT){
        std::cout<<"["<<parent->name<<"] Sleep Until Wake(OSH).\n";
        while(byte!=READ){
            parent->ReadByte(parent->Flag(),byte);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }  
        return 1;
    }
    return 1;
}
std::string SOCKET::OSH::ReadHeader(){
    uint8_t byte = NONE;
    char buffer[25] = {0};
    std::string header;
    std::cout<<"["<<parent->name<<"] Reading Header (OSH).\n";
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Error Socket Listening, File Not in Hand.\n";
        return 0;
    }
    while(byte!=READ){
        parent->ReadByte(parent->Flag(),byte);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(!parent->ReadBuffer(parent->Header(),buffer,sizeof(buffer))) return 0;
    header = CleanBuffer(buffer,sizeof(buffer));
    memset(buffer,0,sizeof(buffer));
    std::cout<<"["<<parent->name<<"] Header Read (OSH).\n";
    if(!parent->WriteBuffer(parent->Header(),buffer,sizeof(buffer))) return 0;
    return header;
}
std::string SOCKET::OSH::ReadData(){
    uint8_t byte = NONE;
    char buffer[960] = {0};
    std::string data;
     std::cout<<"["<<parent->name<<"] Reading Pocket (OSH).\n";
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Error Socket Listening, File Not in Hand.\n";
        return 0;
    }
    while(byte!=READ){
        parent->ReadByte(parent->Flag(),byte);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(!parent->ReadBuffer(parent->Pocket(),buffer,sizeof(buffer))) return 0;
    data = CleanBuffer(buffer,sizeof(buffer));
    memset(buffer,0,sizeof(buffer));
    std::cout<<"["<<parent->name<<"] Pocket Read (OSH).\n";
    if(!parent->WriteBuffer(parent->Pocket(),buffer,sizeof(buffer))) return 0;
    if(!parent->WriteByte(parent->Flag(),SEEN)) return 0;
    return data;
}
bool SOCKET::OSH::Connect(SOCKET* socket){
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Connection Failed, File Not in Hand.\n";
        return 0;
    }
    if(!socket){
         std::cout<<"["<<parent->name<<"] Socket not in Range, Connection Failed. (OSH).\n";
        return 0;
    }
    ReceiverSocket = socket;
    uint8_t byte = NONE;
    std::cout<<"["<<parent->name<<"] Connect to Socket (OSH)...\n";
    while(byte!= IDLE){
        parent->ReadByte(ReceiverSocket->ConnectionStatus(),byte);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(!parent->WriteByte(ReceiverSocket->ConnectionStatus(),RESERVED)) return 0;
     std::cout<<"["<<parent->name<<"] Attached, Ready to Send (OSH).\n";
    return 1;
}
bool SOCKET::OSH::Send(const std::string& header,const std::string& data){
    if(header.size()>MAX_HEADER_SIZE){
        std::cout<<"["<<parent->name<<"] Maximum Header Size is ["<<MAX_HEADER_SIZE<<"]. (OSH)\n";
        return 0;
    }
    if(data.size()>MAX_POCKET_SIZE){
        std::cout<<"["<<parent->name<<"] Maximum Header Size is ["<<MAX_POCKET_SIZE<<"]. (OSH)\n";
        return 0;
    }
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Connection Failed, File Not in Hand. (OSH)\n";
        return 0;
    }
    if(!ReceiverSocket){
         std::cout<<"["<<parent->name<<"] Socket not in Hand, Connection Failed. (OSH).\n";
        return 0;
    }
    uint8_t flag = NONE;
    if(!parent->WriteBuffer(ReceiverSocket->Header(),header.data(),header.size())) return 0;
    if(!parent->WriteBuffer(ReceiverSocket->Pocket(),data.data(),data.size()))     return 0;
    if(!parent->WriteByte  (ReceiverSocket->Flag(),READ))                          return 0;
    while(flag!=SEEN){
        parent->ReadByte(ReceiverSocket->Flag(),flag);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    std::cout<<"["<<parent->name<<"] Data Sent and Received. (OSH).\n";
    return 1;
}
bool SOCKET::OSH::Keep(){
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Connection Failed, File Not in Hand.\n";
        return 0;
    }
    if(!ReceiverSocket){
         std::cout<<"["<<parent->name<<"] Socket not in Hand, Connection Failed. (OSH).\n";
        return 0;
    }
    if(!parent->WriteByte(ReceiverSocket->Flag(),WAIT)) return 0;
    std::cout<<"["<<parent->name<<"] Receiver now in Sleep Mode. (OSH).\n";
    return 1;
}
bool SOCKET::OSH::Disconnect(){
    if(parent->fd == -1 || parent->map == MAP_FAILED){
         std::cout<<"["<<parent->name<<"] Connection Failed, File Not in Hand.\n";
        return 0;
    }
    if(!ReceiverSocket){
         std::cout<<"["<<parent->name<<"] Socket not in Hand, Connection Failed. (OSH).\n";
        return 0;
    }
    if(!parent->WriteByte(ReceiverSocket->Flag(),DONE)) return 0;
    ReceiverSocket = nullptr;
    std::cout<<"["<<parent->name<<"] Disconnected. (OSH).\n";
    return 1;
}