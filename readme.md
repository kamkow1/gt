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

## Example

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

