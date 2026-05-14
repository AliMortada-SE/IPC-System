//socket.cpp
#include "socket.h"
#include "utils.h"

uint8_t SOCKET::GetOffset(std::string Target){
    uint64_t offset=0;
    uint8_t byte;
    if(Target == "flag") offset = Flag();
    if(Target == "state") offset = State();
    if(Target == "error") offset = Error();
    if(Target == "peerid") offset = PeerID();
    if(Target == "header") offset = Header();
    if(Target == "reading_mode") offset = ReadingMode();
    if(Target == "ConnectionStatus") offset = ConnectionStatus();
    if(Target == "pocket") offset = Pocket();
    if(ReadByte(offset,byte)){
        return byte;
    }
    return 255;
}
void SOCKET::TakeStream(){
    streamMutex.lock();
}
void SOCKET::ReleaseStream(){
    streamMutex.unlock();
}
bool SOCKET::WriteByte(bit64 _offset, uint8_t byte){
    if(!binary.is_open()){
        SOCKET_TAG std::cout<<"Error Binary Writing, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekp(_offset, std::ios::beg);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Writing, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.write(reinterpret_cast<const char*>(&byte), 1);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Writing, Write failed.\n";
        ReleaseStream();
        return 0;
    }
    binary.flush();
    ReleaseStream();
    return 1;
}
bool SOCKET::ReadByte(bit64 _offset, uint8_t& out) {
    if (!binary.is_open()) {
        SOCKET_TAG std::cout << "Error Binary Reading, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear(); 
    binary.seekg(_offset, std::ios::beg);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Reading, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.read(reinterpret_cast<char*>(&out), 1);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Reading, Read failed.\n";
        ReleaseStream();
        return 0;
    }
    ReleaseStream();
    
    return 1;
}
bool SOCKET::WriteBuffer(bit64 _offset, const char* buffer,size_t size){
    if(!binary.is_open()){
        SOCKET_TAG std::cout<<"Error Binary Writing, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekp(_offset,std::ios::beg);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Writing, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.write(buffer,size);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Writing, Write Failed.\n";
        ReleaseStream();
        return 0;
    }
    binary.flush();
    ReleaseStream();
    return 1;
}
bool SOCKET::ReadBuffer(bit64 _offset, char* buffer, size_t size) {
    if (!binary.is_open()) {
        SOCKET_TAG std::cout << "Error Binary Reading, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekg(_offset, std::ios::beg);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Reading, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.read(buffer, size);
    if (binary.fail()) {
        SOCKET_TAG std::cout << "Error Binary Reading, Read Failed.\n";
        ReleaseStream();
        return 0;
    }
    ReleaseStream();
    return 1;
}
bool SOCKET::AttachPipe(std::string PipeName){
    if(!fs::exists(PipeName+".pipe")){
        SOCKET_TAG std::cout<<"Cannot open '"<<PipeName<<"' Pipe Not Found!\n";
        return 0;
    }
    if(binary.is_open()){
        binary.close();
    }
    binary.open(PipeName+".pipe",std::ios::binary | std::ios::out | std::ios::in);
    if(!binary.is_open()){
        SOCKET_TAG std::cout<<"Error File Opening! '"<<PipeName<<".pipe'\n";
        return 0;
    }
    uint8_t byte;
    if(!ReadByte(0,byte)) return 0;
    if(byte != 1){
        SOCKET_TAG std::cout<<"Pipe '"<<PipeName<<"' Exist but is not open (Locked), Closing Pipe.\n";
        return 0;
    }
    if(!WriteByte(State(),ACTIVE)) return 0;
    if(!WriteByte(Flag(),READY)) return 0;
    if(!WriteByte(Error(),SAFE)) return 0;
    if(!WriteByte(ConnectionStatus(),IDLE)) return 0;
    return 1;
}
SOCKET* SOCKET::FindSocket(bit64 _ID ,const std::string name) {
    if (!sockets) return nullptr;
    SOCKET_TAG std::cout<<"Start Searching For Socket.\n";
    if(_ID == 0 && name ==""){
        SOCKET_TAG std::cout<<"Should Pass ID or Name for Searching.\nFailed Searching for Socket.\n";
    }
    if(_ID != 0 && name != ""){
        for (auto& s : *sockets) {
            if (s->id == _ID){
                SOCKET_TAG std::cout<<"Socket Found, Socket ID -> |"<<s->id<<"|\n";
                return s.get();
            }    
        }   
    }
    if(_ID != 0){
        for (auto& s : *sockets) {
            if (s->id == _ID){
                SOCKET_TAG std::cout<<"Socket Found, Socket ID -> |"<<s->id<<"|\n";
                return s.get();
            }
        }   
    }
    else if(name != ""){
        for (auto& s : *sockets) {
            if (s->name == name){
                SOCKET_TAG std::cout<<"Socket Found, Socket ID -> |"<<s->id<<"|\n";
                return s.get();
            } 
         }
    }
    SOCKET_TAG std::cout<<"Failed to find Socket.\n";
    return nullptr;
}