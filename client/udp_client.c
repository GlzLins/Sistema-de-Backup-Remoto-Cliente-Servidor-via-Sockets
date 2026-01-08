#include "udp_client.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define DISCOVERY_PORT 5555
#define DISCOVERY_MSG  "DISCOVER_BACKUP_SERVER"

int discover_server(char *server_ip, int *server_port) {
    int sock;
    struct sockaddr_in addr, from;
    char buffer[256];
    socklen_t len = sizeof(from);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket");
        return 0;
    }

    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sock, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0,
           (struct sockaddr*)&addr, sizeof(addr));

    int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                     (struct sockaddr*)&from, &len);

    if (n <= 0) {
        close(sock);
        return 0;
    }

    buffer[n] = '\0';

    // Extrai porta do formato BACKUP_SERVER:4443
    if (sscanf(buffer, "BACKUP_SERVER:%d", server_port) != 1) {
        close(sock);
        return 0;
    }

    inet_ntop(AF_INET, &from.sin_addr, server_ip, INET_ADDRSTRLEN);

    close(sock);
    return 1;
}
