#define _POSIX_C_SOURCE 200112L
#include "stun_gestion.h"


// RFC 5389 Section 6 STUN Message Structure (request)
struct STUNMessageHeader
{
    // Message Type (Binding Request / Response)
    uint16_t type;
    
    // Payload length of this message
    uint16_t length;
    
    // Magic Cookie
    uint32_t cookie;
    
    // Unique Transaction ID
    uint32_t identifier[3];
};



// RFC 5389 Section 15 STUN Attributes (header)
struct STUNAttributeHeader {
    // Attibute Type
    uint16_t type;
    
    // Payload length of this attribute
    uint16_t length;
};

// RFC 5389 Section 15.2 XOR-MAPPED-ADDRESS (answer)
struct STUNXORMappedIPv4Address
{
    uint8_t reserved;
    
    uint8_t family;
    
    uint16_t port;
    
    uint32_t address;
};


int getPublicIPv4Address(struct STUNServer server, char address[16])
{
    srand((unsigned int) time(NULL));

    int socketd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in localAddress;
    
    memset(&localAddress,0,sizeof(struct sockaddr_in));
    
    localAddress.sin_family = AF_INET;
    
    localAddress.sin_addr.s_addr = INADDR_ANY; //why?
    
    localAddress.sin_port = htons(0); //port 0 -> port au hasard
    
    if (bind(socketd, (struct sockaddr*) &localAddress, sizeof(struct sockaddr_in)) < 0) {
        
        close(socketd);
        
        return -1;
    }
    
    // Remote Address
    // First resolve the STUN server address
    struct addrinfo* results = NULL;
    
    struct addrinfo hints;
    
    memset(&hints,0, sizeof(struct addrinfo));
    
    hints.ai_family = AF_INET;
    
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(server.address, NULL, &hints, &results) != 0) {
        close(socketd);
        return -2;
    }

    struct in_addr stunaddr;
    struct addrinfo *p;
    
    for(p = results; p != NULL; p = p->ai_next) {
        stunaddr = ((struct sockaddr_in*) p->ai_addr)->sin_addr;
    
        // Create the remote address
        struct sockaddr_in remoteAddress;
    
        memset(&remoteAddress,0, sizeof(struct sockaddr_in));
    
        remoteAddress.sin_family = AF_INET;
        remoteAddress.sin_addr = stunaddr;
        remoteAddress.sin_port = htons(server.port);
    
        // Construct a STUN request
        struct STUNMessageHeader request;
    
        request.type = htons(0x0001); 
    
        request.length = htons(0x0000); 
    
        request.cookie = htonl(STUN_COOKIE);
    
        for (int index = 0; index < 3; index++) {
            request.identifier[index] = rand();
        }
    
        // Send the request
        if (sendto(socketd, &request, sizeof(struct STUNMessageHeader), 0, (struct sockaddr*) &remoteAddress, sizeof(struct sockaddr_in)) == -1) {
            continue;
        }
    
        // Set the timeout
        struct timeval tv = {1, 0}; //time out of 1 second
    
        setsockopt(socketd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    
        // Read the response
        char* buffer = malloc(sizeof(char)*512);
        if(buffer==NULL) continue;
        memset(buffer,0, 512);
    
        long length = read(socketd, buffer, 512);
    
        if (length < 0) {
            free(buffer);
            continue;
        }
    
        char* pointer = buffer;
    
        struct STUNMessageHeader* response = (struct STUNMessageHeader*) buffer;
    
        if (response->type == htons(0x0101)) {
            // Check the identifer
            for (int index = 0; index < 3; index++) {
                if (request.identifier[index] != response->identifier[index]) {
                    free(buffer);
                    continue;
                }
            }
            pointer += sizeof(struct STUNMessageHeader);
            while (pointer < buffer + length) {
                struct STUNAttributeHeader* header = (struct STUNAttributeHeader*) pointer;
            
                if (header->type == htons(XOR_MAPPED_ADDRESS_TYPE)) {
                    pointer += sizeof(struct STUNAttributeHeader);
                    struct STUNXORMappedIPv4Address* xorAddress = (struct STUNXORMappedIPv4Address*) pointer;
                    if(xorAddress->family!=IPv4_ADDRESS_FAMILY) {
                        free(buffer);
                        continue;
                    }
                    uint32_t num_addr = xorAddress->address ^ htonl(STUN_COOKIE);
                    memset(address,0,10);
                    memset(address+10,255,2);
                    memcpy(address+12,&num_addr,4);
                    freeaddrinfo(results);
                    free(buffer);
                    close(socketd);
                
                    return 0;
                    
                }
            
                pointer += (sizeof(struct STUNAttributeHeader) + ntohs(header->length));
            }
        }
        free(buffer);
    
    }
    freeaddrinfo(results);
    close(socketd);
    return 1;
    
}