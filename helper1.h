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

//read packet from stdin
char* getpacket();

//get time for timestamp in log file
char* getformattedtime();

//Get log info from request and print to log file. Call Rcode 4 function if not IPv6.
void readrequest(char* request, FILE* logfile);

//Get log info from result and print to log file
void readresponse(char* result, FILE* logfile);

//Send Rcode 4 (unimplemented request) and print to log file
void unimplementedrequest(char* request, char* formattedtime, FILE* logfile);
