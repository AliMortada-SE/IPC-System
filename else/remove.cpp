#include "pipe.h"

int main(){
    PIPE pipe(PipeName);
    pipe.init();
    pipe.open();
    SOCKET* cpp = pipe.Socket("cpp");
    SOCKET* go  = pipe.Socket("go");
    cpp->sockets = &pipe.sockets;

    pipe.RemoveSocket(cpp);
    pipe.RemoveSocket(go);
    return 0;
}