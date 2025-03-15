# Green threads for C

## Building

Examples:
```console
chmod +x ./build.sh && ./build.sh
```

If you'd like to include gt into your project, simply copy&paste gt.c gt.h and
gt.S. The default platforms are x86 64 Linux, but support for others may come
in the future ;)

### Platforms

The target platform is defined via `GT_PLATFORM` macro, which can hold the
following values:

- `GT_PLATFORM_LINUX_x86_64`

You can also add a flag to your compiler like so:

```console
gcc -o program -DGT_PLATFORM=GT_PLATFORM_LINUX_x86_64 program.c ...
```

### Configuring

You may want to change the fixed maximum amount of threads or size of threads' stacks.
This is possible via `GT_MAX_ENVIRONMENTS` and `GT_ENVIRONMENT_STACK` macros.

Again, as with the `GT_PLATFORM` macro, you can define it on your commandline like so:

```console
gcc -o program \
    -DGT_PLATFORM=GT_PLATFORM_LINUX_x86_64 \
    -DGT_MAX_ENVIRONMENTS=10 \
    -DGT_ENVIRONMENT_STACK=4096 \
    program.c
    ...
```

By default, you're given 1024 threads to work with and 16KiB of stack memory per thread.

## Examples

Simple counter program:

```c
#include <stdio.h>
#include <stdlib.h>

#include "gt.h"

void count(void)
{
    int N = (int)gt_getarg(); // GCC will complain, but this is OK
    for (int i = 0; i < N; i++) {
        printf("Thread %d, i = %d\n", gt_getid(), i);
        gt_yield();
    }
    printf("THREAD %d IS DONE!\n", gt_getid());
    gt_exit();
}

void main1(void)
{
    // start threads
    gt_create(&count, (void*)10);
    gt_create(&count, (void*)5);
    gt_create(&count, (void*)3);

    while (gt_alive() > 1) {
        gt_yield();
    }
    gt_exit();
}

int main(void)
{
    gt_init(&main1, NULL);

    return 0;
}

```

More real-world example (an echo server):

```c
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
        int n = gt_recv(client->fd, buffer, sizeof(buffer)-1, 0);

        if (n > 0) {
            buffer[n] = '\0';
            printf("thread %d:\n%s\n", gt_getid(), buffer);

            gt_send(client->fd, buffer, n, 0);
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
```

