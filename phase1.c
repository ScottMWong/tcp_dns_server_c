#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>

#include "helper1.h"

int main(int argc, char* argv[]) {
    //When we start our DNS server we should open our log file and set buffer
    FILE* logfile;
    logfile = fopen("dns_svr.log", "a+");
    //Set and zero out buffer for logfile
    char buffer[BUFSIZ];
    memset(buffer, '\0', sizeof(buffer));
    setvbuf(logfile, buffer, _IOFBF, BUFSIZ);
    //Get packet to process
    char* packet = getpacket();
    //Direct packet to appropriate parsing
    //If packet is query, argv[1] will be "query", otherwise should be "response"
    if (strcmp(argv[1], "query") == 0) 
    {
        //readrequest
        readrequest(packet, logfile);
    }
    else 
    {
        //readresponse
        readresponse(packet, logfile);
    }
    //cleanup
    free(packet);
    return 0;
}

