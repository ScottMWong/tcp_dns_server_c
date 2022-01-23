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


//read packet from stdin. remember to free later!
char* getpacket() 
{
    //get packet size from first 2 bytes
    //packetbytesize is first two bytes of packet indicating size
    char packetbytesize[2];
    //Get file descriptor from filename
    read(STDIN_FILENO, packetbytesize, 2);

    //adapted from https://stackoverflow.com/a/17071529
    int packetsize = (packetbytesize[0]<<8) + packetbytesize[1];
    packetsize += 2;
    //now malloc enough space to store the full packet
    char* packet;
    packet = (char*)malloc(packetsize);

    //write entire packet to pointer location
    packet[0] = packetbytesize[0];
    packet[1] = packetbytesize[1];
    for (int i = 2; i < packetsize; i++) 
    {
        packet[i] = getchar();
    }
    return packet;
    
}

//get time for timestamp in log file
char* getformattedtime()
{
    //Time constructs
    time_t unixtime;
    struct tm* timehelper;
    char timebuffer[100];
    //get unix time, convert to time structure
    //convert to ISO 8601, return malloc pointer
    time(&unixtime);
    timehelper = localtime(&unixtime);
    strftime(timebuffer, 100, "%FT%T%z", timehelper);
    char* timepointer = malloc(sizeof(timebuffer));
    //write timebuffer into timepointer
    strcpy(timepointer, timebuffer);
    return timepointer;
}
//Remember to free after using!

//Get log info from request and print to log file. Call Rcode 4 function if not IPv6.
void readrequest(char* request, FILE* logfile)
{
    //don't need to worry about ID etc. go to first label
    int readposition = 14;

    //read the request name until NULL(0) byte for label
    char currentlabelsize = 0;
    int charswritten = 0;
    char requestname[100];
    //read label size
    currentlabelsize = request[readposition];
    readposition += 1;

    while (true)
    {

        //read for label size, 
        for (int i = 0; i < currentlabelsize; i++)
        {
            requestname[charswritten] = request[readposition];
            readposition += 1;
            charswritten += 1;
        }
        //read label size
        currentlabelsize = request[readposition];
        //if label size is NULL stop reading name and write end of array, add to read position
        if (currentlabelsize == 0)
        {
            requestname[charswritten] = 0;
            readposition += 1;
            break;
        }
        //else add '.' and keep reading
        requestname[charswritten] = '.';
        readposition += 1;
        charswritten += 1;
    }

    //print log line of request to file
    //get time
    char* formattedtime = getformattedtime();
    //print statement
    fprintf(logfile, "%s requested %s\n", formattedtime, requestname);
    fflush(logfile);
    //check if the request is IPv6. (001c)
    if (request[readposition] == 0 && request[readposition + 1] == 28)
    {
        //pass request code on
    }
    //call unimplemented request code
    else
    {
        unimplementedrequest(request, formattedtime, logfile);
    }
    //free information
    free(formattedtime);
}

//Get log info from result and print to log file
void readresponse(char* result, FILE* logfile)
{
    //check how many answers in response packet. If no answers, return early
    int readposition = 8;
    if (result[readposition] == '\0' && result[readposition + 1] == '\0')
    {
        //return early
        return;
    }

    //don't need to worry about ID etc. go to first label
    readposition = 14;

    //read the result name until NULL(0) byte for label
    char currentlabelsize = 0;
    int charswritten = 0;
    char resultname[100];
    //read label size
    currentlabelsize = result[readposition];
    readposition += 1;

    while (true)
    {

        //read for label size, 
        for (int i = 0; i < currentlabelsize; i++)
        {
            resultname[charswritten] = result[readposition];
            readposition += 1;
            charswritten += 1;
        }
        //read label size
        currentlabelsize = result[readposition];
        //if label size is NULL stop reading name and write end of array, add to read position
        if (currentlabelsize == 0)
        {
            resultname[charswritten] = 0;
            readposition += 1;
            break;
        }
        //else add '.' and keep reading
        resultname[charswritten] = '.';
        readposition += 1;
        charswritten += 1;
    }

    //Check if the first answer is IPv6 (001c)
    //If so log first answer
    readposition += 6;
    if (result[readposition] == 0 && result[readposition + 1] == 28)
    {
        //Get length of data
        readposition += 8;
        char addresssize[2];
        addresssize[0] = result[readposition];
        addresssize[1] = result[readposition + 1];
        //adapted from https://stackoverflow.com/a/17071529
        int datalength = (addresssize[0] << 8) + addresssize[1];

        //Read the IPv6 address
        //networkipaddress contains raw bytes, ipaddress contains display format
        readposition += 2;
        char networkipaddress[datalength];
        char ipaddress[INET6_ADDRSTRLEN];
        //Copy out datalength bytes to networkipaddress
        for (int i = 0; i < datalength; i++)
        {
            networkipaddress[i] = result[readposition+i];
        }
        inet_ntop(AF_INET6, networkipaddress, ipaddress, INET6_ADDRSTRLEN);
        //print statement
        char* formattedtime = getformattedtime();
        fprintf(logfile, "%s %s is at %s\n", formattedtime, resultname, ipaddress);
        //Trying to print ipaddress character by character
        fflush(logfile);

        //free formattedtime and ipaddress
        free(formattedtime);
    }
    //If the first answer is not IPv6, print no response at all
}

//Send Rcode 4 (unimplemented request) and print to log file
void unimplementedrequest(char* request, char* formattedtime, FILE* logfile)
{
    fprintf(logfile, "%s unimplemented request\n", formattedtime);
    fflush(logfile);
    //Actually edit packet and send reply
}

