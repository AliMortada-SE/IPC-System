#pragma once
#ifndef PIPE_H
#define PIPE_H
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <atomic>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <mutex>
#include <memory>
#include <vector>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;
using bit64  = uint64_t;
using bit32  = uint32_t;
using bit16  = uint16_t;
using bit8   = uint8_t;

#define kb  1024
#define mb  (1024 * kb)

constexpr const char* PipeName = "main";

#include "Scanner.h"
#include "socket.h"
class SOCKET;
class PIPE{
        public: 
        PIPE(std::string filename);   
        bool init();
        bool Open();
        bool WriteByte(bit64 _offset,uint8_t byte);
        bool ReadByte(bit64 _offset, uint8_t& out);
        bool WriteBuffer(bit64 _offset, const char* buffer,size_t size);
        bool ReadBuffer(bit64 _offset, char* buffer, size_t size);
        bool Connect(std::unique_ptr<SOCKET> EndpointSocket);
        void TakeStream();
        void ReleaseStream();
        SOCKET* Socket(const std::string name);
        SOCKET* FindSocket(bit64 _ID = 0,const std::string name = "");
        bool RemoveSocket(SOCKET* socket);
        std::vector<std::unique_ptr<SOCKET>> sockets;
        private:
        bit64 filesize;
        bit64 freeOffset = 0;
        int fd = -1;
        char* map = nullptr;
        std::fstream file;
        std::fstream binary;
        std::string existSockets;
        std::string filename;
        std::string SocketsPath = "sockets.pipe";
        std::string RoomsPath = "rooms.pipe";
        std::atomic<bool> isOpen = false;
        std::atomic<bool> Attached = false;
        std::atomic<bool> StreamInUse = false;
        std::mutex streamMutex;
        
    
};


#endif 
