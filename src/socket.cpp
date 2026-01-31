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
bool SOCKET::ConnectToSocket(SOCKET* socket,std::string Header, bool Force){
    if(!socket){
        SOCKET_TAG std::cout<<"Error Finding Socket!\n";
        return 0;
    }
    SOCKET_TAG std::cout<<"Send Connection Request to -> ["<<socket->name<<"] with ID -> ["<<socket->id<<"]\n";
    char CheckID[16];
    char header[25] = {0};
    std::memcpy(header,Header.data(),std::min(Header.size(),sizeof(header)));
    uint64_t ID = SOCKET::id;
    std::string _Send_ID = int64_to_string(ID);
    uint8_t state;
    uint8_t error;
    uint8_t flag;
    uint8_t connection_status;
    if(!ReadBuffer(socket->StartOffset,CheckID,sizeof(CheckID))) return 0;
    if(socket->id!=std::stoull(std::string(CheckID,sizeof(CheckID)))){
        SOCKET_TAG std::cout<<"Socket ID Written in Pipe is not same as ID in RAM or Offset is Wrong.\n Connection Failed.\n";
        std::cout<<"Read ID:"<<std::stoull(std::string(CheckID,sizeof(CheckID)))<<".\n";
        std::cout<<"Stored ID:"<<socket->id<<".\n";
        return 0;
    }
    if(!ReadByte(socket->State(),state)) return 0;
    if(state != ACTIVE){
        SOCKET_TAG std::cout<<"Socket is Not Active, Connection Failed.\n";
        return 0;
    }
    SOCKET_TAG std::cout<<"Peer Socket is  Active.\n";
    if(!ReadByte(socket->Error(),error)) return 0;
    if(error == ERROR){
        SOCKET_TAG std::cout<<"Socket have Errors, Connection Failed.\n";
        return 0;
    }
    if(!ReadByte(socket->Flag(),flag)) return 0;
    if(flag != READY){
        if(!Force){
            SOCKET_TAG std::cout<<"Socket is Busy, Connection Failed.\n";
            return 0;
        }
        while(flag != READY){
            binary.seekg(socket->Flag(),std::ios::beg);
            binary.read(reinterpret_cast<char*>(&flag),1);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    SOCKET_TAG std::cout<<"Peer Socket is Available.\n";
    SOCKET_TAG std::cout<<"Peer ID -> ["<<ID<<"].\n";
    if(!WriteBuffer(socket->PeerID(),_Send_ID.data(),_Send_ID.size())) return 0;
    SOCKET_TAG std::cout<<"Sending ID.\n";
    if(!WriteBuffer(socket->Header(),header,sizeof(header))) return 0;
    SOCKET_TAG std::cout<<"Sending Reason.\n";
    if(!WriteByte(socket->Flag(),READ)) return 0;
    while(flag!=SEEN){
        ReadByte(socket->Flag(),flag);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(!ReadByte(socket->ConnectionStatus(),connection_status)) return 0;
    if(connection_status == REJECTED){
        SOCKET_TAG std::cout<<"Socket Reject Connection, Closing Socket.\n";
        return 0;
    }
    SOCKET_TAG std::cout<<"Peer Socket is Accepted the Connection.\n";
    SOCKET_TAG std::cout<<"Connected to "<<socket->name<<".\n";
    SOCKET::PeerSocket = socket;
    linked = 1;
    return 1;
}
bool SOCKET::Listen(){
    SOCKET_TAG std::cout<<"Listening For Incomming Connections.\n";
    if(!binary.is_open()){
        SOCKET_TAG std::cout<<"Error Socket Listening, File Not in Hand.\n";
        return 0;
    }
    TakeStream();
    uint8_t byte ;
    char _id[16] = {0}; 
    char header_buffer[25] = {0};
    bool AcceptConnection = 1;
    while(byte == 0){
        binary.clear();
        binary.seekg(PeerID(),std::ios::beg);
        binary.read(reinterpret_cast<char*>(&byte),1);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if(binary.fail()){
        SOCKET_TAG std::cout << "Error Binary Reading, Bad Reading.\nFail to Listening.\n";
        ReleaseStream();
        return 0;
    }
    ReleaseStream();
    while(byte != READ){
        ReadByte(Flag(),byte);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(!ReadBuffer(SOCKET::PeerID(),_id,sizeof(_id))) return 0;
    if(!ReadBuffer(SOCKET::Header(),header_buffer,sizeof(header_buffer))){
        SOCKET::PeerSocket = nullptr;
        return 0;
    }    
    std::string _Header (header_buffer,sizeof(header_buffer));
    std::string ID(_id,sizeof(_id));
    if(!scan.Digits(ID) || ID.empty()){
        SOCKET_TAG std::cout<<"Error while Reading Peer ID, Closing Connection.\n";
        return 0;
    }
    SOCKET::PeerSocket = FindSocket(stoull(ID));
    SOCKET_TAG std::cout<<"Socket -> |"<<ID<<"| Trying to Connect.\n";
    //SOCKET_TAG std::cout<<"PEER SOCKET ID ->"<<PeerSocket->id<<"\n";
    if(PeerSocket){
        SOCKET_TAG std::cout<<"Socket Inline.\n";
    }
    else{
        SOCKET_TAG std::cout<<"Failed to get Socket.\n";
        return 0;
    }
    std::string header = CleanBuffer(header_buffer,sizeof(header_buffer));
    if(!WriteByte(ReadingMode(),READ_POCKET)){
        SOCKET::PeerSocket = nullptr;
        return 0;
    }
    if(!AcceptConnection){
        if(!WriteByte(ConnectionStatus(),REJECTED)) {
            SOCKET::PeerSocket = nullptr;
            return 0;
        }
        SOCKET_TAG std::cout<<"Connection Rejected.\n";
    }
    if(!WriteByte(ConnectionStatus(),ACCEPTED)) {
        SOCKET::PeerSocket = nullptr;
        return 0;
    }
    if(!WriteByte(Flag(),SEEN)) return 0;
    SOCKET_TAG std::cout<<"Socket ["<<SOCKET::PeerSocket->name<<"] with ID -> ["<<ID<<"] Connected.\n";
    linked = true;
    return 1;
}
std::string SOCKET::SendMessage(std::string message){
    if(!PeerSocket){
        SOCKET_TAG std::cout<<"Cannot Send Message, There are no Socket in line.\n";
        return 0;
    }
    uint8_t flag ;
    char pocket_buffer [960] = {0};
    while(flag != READY){
        if(!ReadByte(PeerSocket->Flag(),flag)) return 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(flag == READY){
        if(!WriteBuffer(PeerSocket->Pocket(),message.data(),message.size())) return 0;
        if(!WriteByte(PeerSocket->Flag(),READ)) return 0;
    }
    while(flag != SEEN){
        if(!ReadByte(PeerSocket->Flag(),flag)) return 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(flag == SEEN){
        if(!ReadBuffer(Pocket(),pocket_buffer,sizeof(pocket_buffer))) return 0;
        if(!WriteByte(PeerSocket->Flag(),DONE)) return 0;
    }
    ClearBuffer();
    return CleanBuffer(pocket_buffer,sizeof(pocket_buffer));
}
std::string SOCKET::ReceiveMessage(){
    SOCKET_TAG std::cout<<"Sleep Until Message Received.\n";
    if(!PeerSocket || !linked){
        SOCKET_TAG std::cout<<"Cannot Receive Message, There are no Socket in line.\n";
        return 0;
    }
    uint8_t flag;
    char pocket_buffer [960] = {0};
    while(flag != READ){
        if(!ReadByte(Flag(),flag)) return 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(flag == READ){
        if(!ReadBuffer(Pocket(),pocket_buffer,sizeof(pocket_buffer))) return 0;
    }
    SOCKET_TAG std::cout<<"Message Received.\n";
    return CleanBuffer(pocket_buffer,sizeof(pocket_buffer));
}
bool SOCKET::ResponseMessage(std::string response){
    SOCKET_TAG std::cout<<"Preparing For Response.\n";
    if(!PeerSocket || !linked){
        SOCKET_TAG std::cout<<"Cannot Response Message, There are no Socket in line.\n";
        return 0;
    }
    uint8_t flag ;
    char pocket_buffer [960] = {0};
    int counter = 0;
    int timeout = 200000;
    while(flag != READ){
        if(!ReadByte(Flag(),flag)) return 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    if(flag == READ){
        if(!WriteBuffer(PeerSocket->Pocket(),response.data(),response.size())) return 0;
        if(!WriteByte(Flag(),SEEN)) return 0;
    }
    while(flag != DONE && counter<timeout){
        if(!ReadByte(Flag(),flag)) return 0;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        counter++;
    }
    if(flag != DONE){
        std::cout<<"Error Socket Reading Response, Cleaning Pocket.\n";
        if(!WriteBuffer(Pocket(),pocket_buffer,sizeof(pocket_buffer))) return 0;
        return 0;
    }
    ClearBuffer();
    SOCKET_TAG std::cout<<"Response Sent.\n";
    return 1;
}
bool SOCKET::ClearBuffer(){
    char pocketBuffer[960]= {0};
    char headerBuffer[25] = {0};
    char peerIdBuffer[16] =  {0};
    uint8_t byte = 0;
    if(!WriteBuffer(SOCKET::PeerID(),peerIdBuffer,sizeof(peerIdBuffer))) return 0;
    if(!WriteBuffer(SOCKET::Header(),headerBuffer,sizeof(headerBuffer))) return 0;
    if(!WriteBuffer(SOCKET::Pocket(),pocketBuffer,sizeof(pocketBuffer))) return 0;
    SOCKET_TAG std::cout<<"Buffer Cleared.\n";
    return 1;
}
bool SOCKET::ClearPocket(){
    char pocketBuffer[960]= {0};
    char headerBuffer[25] = {0};
    char peerIdBuffer[16] =  {0};
    uint8_t byte = 0;
    if(!WriteByte  (SOCKET::State(),byte)) return 0;
    if(!WriteByte  (SOCKET::Flag(),byte)) return 0;
    if(!WriteByte  (SOCKET::Error(),byte)) return 0;
    if(!WriteByte  (SOCKET::ReadingMode(),byte)) return 0;
    if(!WriteByte  (SOCKET::ConnectionStatus(),byte)) return 0;
    if(!WriteBuffer(SOCKET::PeerID(),peerIdBuffer,sizeof(peerIdBuffer))) return 0;
    if(!WriteBuffer(SOCKET::Header(),headerBuffer,sizeof(headerBuffer))) return 0;
    if(!WriteBuffer(SOCKET::Pocket(),pocketBuffer,sizeof(pocketBuffer))) return 0;
    SOCKET_TAG std::cout<<"Pocket Cleared.\n";
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