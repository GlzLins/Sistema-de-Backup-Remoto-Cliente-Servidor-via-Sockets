#include "udp_discovery.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define DISCOVERY_PORT 5555
#define DISCOVERY_MSG  "DISCOVER_BACKUP_SERVER"
#define SERVER_PORT    4443

static void *udp_discovery_thread(void *arg) {
    int sock;
    struct sockaddr_in addr, client;
    char buffer[256];
    socklen_t len = sizeof(client);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("UDP socket");
        return NULL;
    }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVERY_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("UDP bind (porta já em uso?)");
        close(sock);
        return NULL;
    }

    printf("🔎 UDP Discovery ativo na porta %d\n", DISCOVERY_PORT);
    fflush(stdout);

    while (1) {
        int n = recvfrom(sock, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr*)&client, &len);
        if (n <= 0)
            continue;

        buffer[n] = '\0';

        if (strcmp(buffer, DISCOVERY_MSG) == 0) {
            char response[64];
            snprintf(response, sizeof(response),
                     "BACKUP_SERVER:%d", SERVER_PORT);

            sendto(sock, response, strlen(response), 0,
                   (struct sockaddr*)&client, len);
        }
    }

    close(sock);
    return NULL;
}

void start_udp_discovery() {
    pthread_t tid;
    if (pthread_create(&tid, NULL, udp_discovery_thread, NULL) == 0) {
        pthread_detach(tid);
    } else {
        perror("pthread_create (UDP discovery)");
    }
}
