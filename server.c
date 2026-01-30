#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

void makeSocket(int *sd, char *argv[], struct sockaddr_in *server_addr, int portNumber){
    struct sockaddr_in from_addr; // from address
    socklen_t fromLength = sizeof(from_addr);
    int reuse = 1;
    int rc = 0;
    struct ip_mreq mreq; // multicast structure

    memset(server_addr, 0, sizeof(*server_addr)); // empty the buffer
  
    *sd = socket(AF_INET, SOCK_DGRAM, 0); // create a UDP socket with SOCK_DGRAM
    if (*sd < 0){ // check for socket errors
        perror("socket creation error\n");
        exit(1);
    }

    // allow the same port to be used for multiple sockets
    rc = setsockopt(*sd, SOL_SOCKET, SO_REUSEPORT, (char *)&reuse, sizeof(reuse));
    if (rc < 0){ // check for reuse errors
        perror("setsockopt error");
        exit(1);
    }
    rc = setsockopt(*sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
    if (rc < 0){ // check for reuse errors
        perror("setsockopt error");
        exit(1);
    }

    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(portNumber); // convert portNumber to network order 16 bit

    // set the multicast address to 224.0.0.1
    if (inet_pton(AF_INET, "224.0.0.1", &server_addr->sin_addr) != 1){
        perror("ip error");
        exit(1);
    }

    rc = bind(*sd, (struct sockaddr *)server_addr, sizeof(*server_addr)); // bind the socket to the multicast address and port
    if (rc < 0){ // check for bind errors
        perror ("bind error");
        exit(1);
    }

    memset(&mreq, 0, sizeof(mreq)); // empty the buffer
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    // join multicast group
    if (setsockopt(*sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0){
        perror("setsockopt (IP_ADD_MEMBERSHIP)");
        close(*sd);
        exit (-1);
    }

    while (1){
        int i = 0;
        char buf[256];
        ssize_t bytes = recvfrom(*sd, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from_addr, &fromLength);
        char name[] = "Name";
        char val[] = "Value";
        
        // skip if receive fails
        if (bytes < 0){
            perror("error receiving message");
            continue;
        }
        buf[bytes] = '\0'; // make space for null terminator 
        printf("%-20.20s%-20.20s\n", name, val);

        // parse the key value pairs received
        while (buf[i] != '\0'){
            int j = 0;
            int k = 0;
            char key[256];
            char value[256];

            // skip empty characters (spaces)
            while (isspace((unsigned char) buf[i])){
                i++;
            }
            if (buf[i] == '\0'){
                break;
            }

            // add letters to key character array until ':'
            while (buf[i] != ':' && buf[i] != '\0' && !isspace((unsigned char)buf[i])){
                key[j] = buf[i];
                i++;
                j++;
            }
            key[j] = '\0';
            if (buf[i] == '\0'){
                break;
            }
            i++;
            // skip empty characters (spaces)
            while (isspace((unsigned char)buf[i])){
                i++;
            }

            // add letters and quotes to the value character array
            if (buf[i] == '"'){
                value[k] = buf[i];
                i++;
                k++;
                while (buf[i] != '"' && buf[i] != '\0'){
                    value[k] = buf[i];
                    i++;
                    k++;
                }
                if (buf[i] == '"'){
                    value[k] = buf[i];
                    i++;
                    k++;
                }
            }
            else{
                while (!isspace((unsigned char)buf[i]) && buf[i] != '\0'){
                    value[k] = buf[i];
                    i++;
                    k++;
                }
            }
            value[k] = '\0';

            printf("%-20.20s%-20.20s\n", key, value); // print key value pairs that occupy 20 characters and align left
        }

        // hard code separators for readability 
        printf("\n");
        printf("****************************************\n");
        printf("****************************************\n");
    }
}

int main(int argc, char *argv[]){
    int sd = 0; // socket descriptor 
    int portNumber = 0; 
    struct sockaddr_in server_addr; // addresses structure

    // check for two parameters 
    if (argc < 2){
    printf ("Usage is: server <portNumber>\n");
    exit(1);
    }
    portNumber = atoi(argv[1]);

    // create the server socket 
    makeSocket(&sd, argv, &server_addr, portNumber);
    
    return 0;
}
