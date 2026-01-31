#include "pipe.h"
#include "utils.h"

#define PIPE_TAG std::cout<<"["<<PipeName<<"] ";

PIPE::PIPE(std::string filename): filename(filename + ".pipe"){};

bool PIPE::WriteByte(bit64 _offset, uint8_t byte){
    if(!binary.is_open()){
        PIPE_TAG std::cout<<"Error Binary Writing, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekp(_offset, std::ios::beg);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Writing, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.write(reinterpret_cast<const char*>(&byte), 1);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Writing, Write failed.\n";
        ReleaseStream();
        return 0;
    }
    binary.flush();
    ReleaseStream();
    return 1;
}
bool PIPE::ReadByte(bit64 _offset, uint8_t& out) {
    if (!binary.is_open()) {
        PIPE_TAG std::cout << "Error Binary Reading, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear(); 
    binary.seekg(_offset, std::ios::beg);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Reading, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.read(reinterpret_cast<char*>(&out), 1);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Reading, Read failed.\n";
        ReleaseStream();
        return 0;
    }
    ReleaseStream();
    
    return 1;
}
bool PIPE::WriteBuffer(bit64 _offset, const char* buffer,size_t size){
    if(!binary.is_open()){
        PIPE_TAG std::cout<<"Error Binary Writing, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekp(_offset,std::ios::beg);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Writing, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.write(buffer,size);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Writing, Write Failed.\n";
        ReleaseStream();
        return 0;
    }
    binary.flush();
    ReleaseStream();
    return 1;
}
bool PIPE::ReadBuffer(bit64 _offset, char* buffer, size_t size) {
    if (!binary.is_open()) {
        PIPE_TAG std::cout << "Error Binary Reading, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    binary.clear();
    binary.seekg(_offset, std::ios::beg);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Reading, Bad seek.\n";
        ReleaseStream();
        return 0;
    }
    binary.read(buffer, size);
    if (binary.fail()) {
        PIPE_TAG std::cout << "Error Binary Reading, Read Failed.\n";
        ReleaseStream();
        return 0;
    }
    ReleaseStream();
    return 1;
}
void PIPE::TakeStream(){
    streamMutex.lock();
}
void PIPE::ReleaseStream(){
    streamMutex.unlock();
}
bool PIPE::init(){
    std::error_code ec;
    char LOCK = 0;
    if(!fs::exists(filename)){
        std::ofstream create(filename,std::ios::binary);
        TakeStream();
        binary.open(filename,std::ios::in | std::ios::out | std::ios::binary);
        if(!binary.is_open()){
            PIPE_TAG std::cout<<"Error File Opening!\n";
            return 0;
        }    
        binary.seekp(0,std::ios::beg);
        binary.write(&LOCK,1);
        binary.close();
        fs::resize_file(filename,mb*4);
    }    
    if(!fs::exists(PIPE::SocketsPath)){
        file.open("sockets.pipe",std::ios::out);
        file.close();
    }    
    if(!fs::exists(PIPE::RoomsPath)){
        file.open("rooms.pipe",std::ios::out);
        file.close();
    }    
    PIPE::filesize = fs::file_size(filename,ec);
    if(ec){
        PIPE_TAG std::cout<< ec.message();
        return 0;
    }    
    PIPE::file.open(PIPE::SocketsPath,std::ios::in);
    if(PIPE::file.is_open()){
        std::string line;
        bit64 _ID;
        bit64 _Offset;
        while(std::getline(PIPE::file,line)){
            if(line.size()>0){
                std::unique_ptr<SOCKET> TempSocket = std::make_unique<SOCKET>();
                _ID = std::stoull(scan.get("ID",line));
                _Offset = std::stoull(scan.get("StartOffset",line));
                TempSocket->id = _ID;
                TempSocket->StartOffset = _Offset;
                TempSocket->name = scan.get("Name",line);
                TempSocket->EndOffset = TempSocket->StartOffset + 1024;
                if(PIPE::freeOffset  < TempSocket->EndOffset){
                    PIPE::freeOffset = TempSocket->EndOffset;
                }
                PIPE::existSockets +=scan.addValue(TempSocket->name,int64_to_string(TempSocket->id));
                PIPE::sockets.push_back(std::move(TempSocket));
            }
        }
    }
    PIPE::file.close();
    return 1;
}    
bool PIPE::open(){
    if (binary.is_open()){
        binary.close();
    }
    binary.open(filename,std::ios::out | std::ios:: in | std::ios::binary);
    if(!binary.is_open()){
        std::cout<<"Error Connecting to ("<<PipeName<<") Pipe!\n";
        return 0;
    }
    binary.clear();
    binary.seekp(0,std::ios::beg);
    binary.put(static_cast<char>(1));
    binary.flush();
    PIPE_TAG std::cout<<"Pipe "<<filename<<" is Open.\n";
    return 1;
}
SOCKET* PIPE::Socket(const std::string name){
    if(scan.Digits(scan.get(name,existSockets))){
        for(int x=0;x<sockets.size();x++){
            if(name==sockets[x]->name){
                PIPE_TAG std::cout<<"Socket -> "<<sockets[x]->name<<" Attached to pipe.\n";
                return sockets[x].get();
            }
        }
    }
    std::unique_ptr<SOCKET> s = std::make_unique<SOCKET>();
    std::string _StartOffset; 
    s->id = ID16(name);
    s->name = name;
    s->StartOffset = freeOffset;
    if(s->StartOffset == 0) s->StartOffset++;
    s->EndOffset = s->StartOffset + kb;
    //s->owner = this;
    
    freeOffset = s->EndOffset;
    std::string _ID = int64_to_string(s->id);
    TakeStream();
    binary.clear();
    binary.seekp(s->StartOffset,std::ios::beg);
    binary.write(_ID.data(),_ID.size());
    binary.flush();
    ReleaseStream();
    file.open(SocketsPath,std::ios::app);
    if(!file.is_open()){
        PIPE_TAG std::cout<<"Error File Openning!\n";
        return s.get();
    }
    std::string data;
    data += scan.addValue("ID",_ID);
    data += scan.addValue("Name",name);
    data += scan.addValue("StartOffset",int64_to_string(s->StartOffset));
    existSockets += scan.addValue(s->name,int64_to_string(s->id));
    sockets.push_back(std::move(s));
    file << data<<"\n";
    file.close();
    PIPE_TAG std::cout<<"New Socket Created -> "<<name<<".\n";
    return sockets.back().get();
}
bool PIPE::RemoveSocket(SOCKET* socket){
    std::string SocketName = socket->name;
    TakeStream();
    binary.clear();
    binary.seekp(socket->StartOffset,std::ios::beg);
    char buffer[1024] = {0};
    binary.write(buffer,sizeof(buffer));
    binary.flush();
    ReleaseStream();
    std::string NewData;
    for(int x=0; x<sockets.size();x++){
        if(sockets[x]->id == socket->id ){
            sockets.erase(sockets.begin() + x);
            break;
        }
        NewData += scan.addValue("ID",int64_to_string(sockets[x]->id));
        NewData += scan.addValue("Name",sockets[x]->name);
        NewData += scan.addValue("StartOffset",int64_to_string(sockets[x]->StartOffset));
        NewData += '\n';
    }
    file.open(SocketsPath,std::ios::out);
    if(!file.is_open()){
        PIPE_TAG std::cout<<"Error File Opening!\n";
        return 0;
    }
    file<<NewData;
    file.close();
    PIPE_TAG std::cout<<"Socket '"<<SocketName<<"' Deleted.\n";
    return 1;
}
SOCKET* PIPE::FindSocket(bit64 _ID, const std::string name) {
    PIPE_TAG std::cout << "Start Searching For Socket.\n";
    if (_ID == 0 && name.empty()) {
        PIPE_TAG std::cout << "Should Pass ID or Name.\n";
        return nullptr;
    }
    if (_ID != 0) {
        for (auto& s : sockets) {
            if (s->id == _ID) {
                PIPE_TAG std::cout << "Socket Found, ID -> |" << s->id << "|\n";
                return s.get();
            }
        }
    }
    // Fallback: name
    if (!name.empty()) {
        for (auto& s : sockets) {
            if (s->name == name) {
                PIPE_TAG std::cout << "Socket Found, ID -> |" << s->id << "|\n";
                return s.get();
            }
        }
    }
    PIPE_TAG std::cout << "Failed to find Socket.\n";
    return nullptr;
}