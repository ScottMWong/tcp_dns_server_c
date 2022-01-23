Assignment for Computer Systems (COMP30023), University of Melbourne, Semester 1 2021
Created May 2021
Relevant documentation below:

Phases
This project should be executed in two phases:
Phase 1 of this project is to design and write as much code as possible without using sockets. This involves
looking up the reference and other resources to understand the format of DNS messages, formatting of log
entries (with timestamps).
Phase 2 is to write the socket code, and integrate it with the code written in Phase 1.

Background
Note: This project will contain some reading of standards as well as writing code. If you ever write
an implementation of a protocol, you will need to read standards such as these. Therefore, becoming
familiar with the format and terminology is an important part of the field of computer systems. You will
be pointed to the relevant sections so that you do not spend your whole time reading the more arcane
parts of the text.
The Domain Name System (DNS) provides, among other things, the mapping between human-meaningful
hostnames like lms.unimelb.edu.au and the numeric IP addresses that indicate where packets should
be sent. DNS consists of a hierarchy of servers, each knowing a portion of the complete mapping.
In this project, you will write a DNS server that accepts requests for IPv6 addresses and serves them
either from its own cache or by querying servers higher up the hierarchy. Each transaction consists of at
most four messages: one from your client to you, one from you to your upstream server, one from your
upstream server to you and one from you to your client. The middle two can be sometimes skipped if
you cache some of the answers.

In a DNS system, the entry mapping a name to an IPv6 address is called a AAAA (or “quad A”) record.
Its “record type” is 28 (QType).
The server will also keep a log of its activities. This is important for reasons such as detecting denial-ofservice
attacks, as well as allowing service upgrades to reflect usage patterns.
For the log, you will need to print a text version of the IPv6 addresses. IPv6 addresses are 128 bits long.
They are represented in text as eight colon-separated strings of 16-bit numbers expressed in hexadecimal.
As a shorthand, a string of consecutive 16-bit numbers that are all zero may be replaced by a single “::”.

Project specification
Accept a DNS “AAAA” query over TCP on port 8053. Forward it to a server whose IPv4 address is
the first command-line argument and whose port is the second command-line argument. (For testing,
use the value in /etc/resolv.conf on your server and port 53). Send the response back to the client
who sent the request, over the same TCP connection. There will be a separate TCP connection for each
query/response with the client. Log these events, as described below.
Note that DNS usually uses UDP, but this project will use TCP because it is a more useful skill for you
to learn. A DNS message over TCP is slightly different from that over UDP: it has a two-byte header
that specify the length (in bytes) of the message, not including the two-byte header itself. This
means that you know the size of the message before you read it, and can malloc() enough space for it.
Assume that there is only one question in the DNS request you receive, although the standard allows
there to be more than one. If there is more than one answer in the reply, then only log the first one,
but always reply to the client with the entire list of answers. If there is no answer in the reply, log the
request line only. If the first answer in the response is not a AAAA field, then do not print a log entry
(for any answer in the response).
The program should be ready to accept another query as soon as it has processed the previous query and
response. (If Non-blocking option is implemented, it must be ready before this too.)
Your server should not shut down by itself. SIGINT (like CTRL-C) will be used to terminate your server
between test cases.
You may notice that a port and interface which has been bound to a socket sometimes cannot be reused
until after a timeout. To make your testing and our marking easier, please override this behaviour by
placing the following lines before the bind() call:
int enable = 1;
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
perror("setsockopt");
exit(1);
}

Testing Phase 1
The phase1 program must read in a packet from stdin, parse it, and then output the log
entry (to the file ./dns_svr.log) that would have been written if you had received the packet over the
network. A command line argument “query” or “response” will be given to indicate where the packet would
have come from. i.e. we will run command ./phase1 [query|response] < <path-to-packet-file>
to invoke your phase1 program.

Assessment criteria

DNS responses
The finished project should be a functional DNS server, for AAAA queries.

Log
Keep a log at ./dns_svr.log (i.e., in current directory) with messages of the following format:
<timestamp> requested <domain_name> – when you receive a request
<timestamp> unimplemented request – if the request type is not AAAA
<timestamp> <domain_name> is at <IP address> – see 2.1
The functions inet_ntop() and inet_pton() are useful for converting between the text version of the
IP address and the 16-byte binary version in the packets. Use man to learn how to use them. Note that
the maximum length of the text version is given by INET6_ADDRSTRLEN, defined in <arpa/inet.h>.

• The field timestamp must not contain spaces. It should be in the ISO8601 format that is generated
by strftime() with the format “%FT%T%z”.
• Fields must be separated by a single space.
• Line endings must be the Unix standard LF (\n) only.
• The log file must contain ASCII characters only. There is no need to perform Punycode conversions.
• Flush the log file after each write (e.g. using fflush(3)). (By default, files on disk are block
buffered, which means that lines written to the log would not appear for a long time. This makes
it difficult for a sysadmin to use the logs to track down problems.)
• Log entries due to cache operation (if implemented) must be printed before those relating to the
cached packet itself.
• Output to stdout and stderr will not be assessed.
• The log file must be readable and writable by the current user (e.g., file permission 0644).

Error handling
When a request is received for something other than a AAAA record, respond with Rcode 4 (“Not Implemented”)
and log “<timestamp> unimplemented request”. Do not forward valid non-AAAA responses
to your client; send nothing except what is needed to send the Rcode.
Assumptions that can be made:
• Requests from client and responses from server will always be well-formed DNS packets
• Clients will always send query (QR 0), Server will always send response (QR 1)
• OPCODE is 0, TC is 0, RD is 1
• The server will always be reachable
You may respond with other Rcodes if any of these assumptions are detected to be broken, though this
is not required nor examined.
Real systems code has to be very robust to errors; a bug in the kernel will crash the entire computer,
which may be supporting many VMs, or running a bank’s entire payment system. However, to keep this
project simple, no further error processing is required here.
