#pragma once
#ifndef SOCKET_H
#define SOCKET_H
#include <fstream>
#include <string>
#include <cstdint>
#include <iostream>
#include <filesystem>
#include <mutex>
#include <chrono>
#include <thread>
#include <cstring>
#include <algorithm>
#include <fcntl.h>
#include "pipe.h"


namespace fs = std::filesystem;
using bit64  = uint64_t;
using bit32  = uint32_t;
using bit16  = uint16_t;
using bit8   = uint8_t;

#define kb  1024
#define mb  (1024 * kb)
#define SOCKET_TAG std::cout<<"["<<SOCKET::name<<"] ";

//Flag
#define READY 0
#define READ  1
#define WAIT  2
#define SEEN  3
#define DONE  4


#define ACTIVE  1
#define INACTIVE 0

#define _true  1
#define _false 0

#define IDLE     0
#define RESERVED 1
#define ACCEPTED 2
#define REJECTED 3

#define SAFE 1
#define ERROR 0

#define READ_POCKET 0
#define READ_ROOM   1
#define READ_FILE   2

#define DISABLED 199
#define NONE 70

#define MAX_HEADER_SIZE 25
#define MAX_POCKET_SIZE 960

// One-Shot Handshake -> OSH

class PIPE;
class SOCKET {
    public:
    ~SOCKET() = default;       // use compiler’s default destructor  
    SOCKET();
    
    SOCKET(const SOCKET&) = delete;            // forbid copying  
    SOCKET& operator=(const SOCKET&) = delete; // forbid copy assignment  
    
    SOCKET(SOCKET&&) = default;                // allow moving  
    SOCKET& operator=(SOCKET&&) = default;     // allow move assignment
    class OSH;
    class OSH{
        SOCKET* parent;
        public:
        explicit OSH(SOCKET* p);
        SOCKET* ReceiverSocket;
        //Receiver
        bool Listen();
        std::string ReadHeader();
        std::string ReadData();
        //Sender
        bool Connect(SOCKET* socket);
        bool Send(const std::string& header,const std::string& data);
        bool Keep();
        bool Disconnect();
    };
    OSH osh;
    void TakeStream();
    void ReleaseStream();
    bool WriteByte(bit64 _offset,uint8_t byte);
    bool ReadByte(bit64 _offset, uint8_t& out);
    bool WriteBuffer(bit64 _offset, const char* buffer,size_t size);
    bool ReadBuffer(bit64 _offset, char* buffer, size_t size);
    bool AttachPipe(std::string PipeName);
    SOCKET* FindSocket(bit64 _ID = 0,const std::string name = "");
    uint8_t GetOffset(std::string Target);

    SOCKET* PeerSocket = nullptr;
    PIPE* owner = nullptr;
    std::string name;
    uint64_t id;
    uint64_t StartOffset;
    uint64_t EndOffset;
    std::fstream binary;
    std::mutex streamMutex;
    std::vector<std::unique_ptr<SOCKET>>* sockets = nullptr;
    
    bool linked = false;
    uint64_t State() const { return StartOffset + 16; }
    uint64_t Flag() const { return StartOffset + 17; }
    uint64_t Error() const { return StartOffset + 18; }
    uint64_t PeerID() const { return StartOffset + 19; }
    uint64_t Header() const { return StartOffset + 35; }
    uint64_t ReadingMode() const { return StartOffset + 62; }
    uint64_t ConnectionStatus() const { return StartOffset + 63; }
    uint64_t Pocket() const { return StartOffset + 64; }
    std::string getName() const { return name; }
    uint64_t getID() const { return id; }
};
#endif