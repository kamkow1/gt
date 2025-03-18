#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "gt.h"

#define NCLIENTS    GT_MAX_ENVIRONMENTS
#define PORT        6969
#define MAX_BUFFER  1024

#define USE_READ_WRITE

typedef struct {
    int taken, fd;
} Client;

typedef struct {
    int fd;
    struct sockaddr_in addr;
} Server;

static Client clients[NCLIENTS] = {0};
static Server server;

int client_slot(void)
{
    for (int i = 0; i < NCLIENTS; i++) {
        if (!clients[i].taken) {
            return i;
        }
    }
    return -1;
}

void set_nonblocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void handle_client(void)
{
    Client *client = (Client *)gt_getarg();
    char buffer[MAX_BUFFER];
    for (;;) {
        #ifdef USE_READ_WRITE
            int n = gt_read(client->fd, buffer, sizeof(buffer)-1);
        #else
            int n = gt_recv(client->fd, buffer, sizeof(buffer)-1, 0);
        #endif

        if (n > 0) {
            buffer[n] = '\0';
            printf("thread %d:\n%s\n", gt_getid(), buffer);

            #ifdef USE_READ_WRITE
                gt_write(client->fd, buffer, n);
            #else
                gt_send(client->fd, buffer, n, 0);
            #endif
        } else {
            goto done;
        }
    }

    done:
        printf("disconnecting client with thread %d\n", gt_getid());
        close(client->fd);
        client->taken = false;
        gt_exit();
}

void main1(void)
{
    set_nonblocking(server.fd);

    printf("Listening on %d...\n", PORT);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = gt_accept(server.fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd >= 0) {
            int slot = client_slot();
            if (slot != -1) {
                Client *client = &clients[slot];
                client->taken = true;
                client->fd = client_fd;
                set_nonblocking(client->fd);
                gt_create(&handle_client, client);
            }
        }

        gt_yield();
    }

    gt_exit();
}

int main(void)
{
    if ((server.fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }
    setsockopt(server.fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    server.addr.sin_family = AF_INET;
    server.addr.sin_addr.s_addr = INADDR_ANY;
    server.addr.sin_port = htons(PORT);

    if (bind(server.fd, (struct sockaddr *)&server.addr, sizeof(server.addr)) < 0) {
        close(server.fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server.fd, NCLIENTS) < 0) {
        close(server.fd);
        exit(EXIT_FAILURE);
    }

    gt_init(&main1, NULL);

    close(server.fd);
    return 0;
}
