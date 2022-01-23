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

#include "p2helper.h"

//argv gives server to connect to to solve query, argv[1] is IPv4 address, argv[2] is port
//Don't create input buffers for connections because getpacket handles for us (Need to remember to free though)
int main(int argc, char* argv[]) {
    //When we start our DNS server we should open our log file and set buffer
    FILE* logfile;
    logfile = fopen("dns_svr.log", "a+");
    //Set and zero out buffer for logfile
    char buffer[BUFSIZ];
    memset(buffer, '\0', sizeof(buffer));
    setvbuf(logfile, buffer, _IOFBF, BUFSIZ);

    //Create input socket on port 8053. Also create buffer for this socket.
    //Code heavily taken from (COMP30023)L14 - Socket Flow and TCP Control
    int inputsocketfd = 0;
    int inputconnectionfd = 0;
    struct sockaddr_in inputsocketadd;
    //Create socket
    inputsocketfd = socket(AF_INET, SOCK_STREAM, 0);
    //Initialise server address
    memset(&inputsocketadd, '\0', sizeof(inputsocketadd));
    inputsocketadd.sin_family = AF_INET;
    inputsocketadd.sin_addr.s_addr = htonl(INADDR_ANY);
    inputsocketadd.sin_port = htons(8053);
    //Reuse port if able
    int enable = 1;
    if (setsockopt(inputsocketfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt failed for input");
        exit(1);
    }
    //Bind socket
    bind(inputsocketfd,(struct sockaddr*)&inputsocketadd, sizeof(inputsocketadd));
    //Start listening. 12 max connections in queue
    listen(inputsocketfd, 12);


    //Create connectioninformation to upstream server
    //Code used from week-9-practial and https://www.gnu.org/software/libc/manual/html_node/Inet-Example.html
    // IPv4 address is argv[1] and port is argv[2]
    int serversocketfd = 0;
    struct addrinfo hints;
    struct addrinfo* serverinfo;
    //We need this to iterate through trying to connecting to returned getaddr addresses
    struct addrinfo* rp;
    //Create address
    memset(&hints, '\0', sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //Get addrinfo of server. From man page:
    //The getaddrinfo() function combines the functionality provided by the
    //gethostbyname(3) and getservbyname(3) functions into a single interface
    if (getaddrinfo(argv[1], argv[2], &hints, &serverinfo) > 0) 
    {
        perror("getaddrinfo failed for upstream server");
        exit(1);
    }

    //Since our code will be ended via interrupt, we can trap in while true loop

    //Declare variables we will use
    int requestwritesize;
    int responsewritesize;
    bool isrequestvalid;
    char* requestpacket;
    char* responsepacket;
    while (true) 
    {
        //Get next client connection, taken from (COMP30023)L14 - Socket Flow and TCP Control
        inputconnectionfd = accept(inputsocketfd, (struct sockaddr*)NULL, NULL);
        if (inputconnectionfd < 0) 
        {
            perror("Failed to accept connection");
            exit(1);
        }
        //Get request packet
        requestpacket = getpacket(inputconnectionfd);
        //Parse request packet
        isrequestvalid = readrequest(requestpacket, logfile);
        //Send request to upstream server (if appropriate)
        if (isrequestvalid) 
        {
            // Create socket for upstream connection
            // Connect to first valid result
            // Why are there multiple results? see man page (search 'several reasons')
            // How to search? enter /, then text to search for, press n/N to navigate
            for (rp = serverinfo; rp != NULL; rp = rp->ai_next) {
                serversocketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                if (serversocketfd == -1)
                    continue;

                if (connect(serversocketfd, rp->ai_addr, rp->ai_addrlen) != -1)
                    break; // success

                close(serversocketfd);
            }
            if (rp == NULL) {
                fprintf(stderr, "Failed to connect to upstream server\n");
                exit(1);
            }
            //Send request to upstream server
            requestwritesize = getpacketwritesize(requestpacket);
            write(serversocketfd, requestpacket, requestwritesize);
            //Get response packet from upstream server
            responsepacket = getpacket(serversocketfd);
            //Parse response packet
            readresponse(responsepacket, logfile);
            //Pass on response packet
            responsewritesize = getpacketwritesize(responsepacket);
            write(inputconnectionfd, responsepacket, responsewritesize);
            //Free responsepacket and close serversocketfd
            free(responsepacket);
            close(serversocketfd);
        }
        //Else return Rcode4 packet to sender
        else 
        {
            requestwritesize = getpacketwritesize(requestpacket);
            write(inputconnectionfd, requestpacket, requestwritesize);
        }
        //Clear client connection, taken from (COMP30023)L14 - Socket Flow and TCP Control
        //Free requestpacket for next use
        free(requestpacket);
        close(inputconnectionfd);
    }

    return 0;
}
