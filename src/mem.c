/*
 * Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "log.h"
#include "btrace.h"
#include "mem.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(n) (sizeof(n) / sizeof(n[0]))
#endif

int mmem_usage = 0;

static pthread_mutex_t alloc_mutex;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

struct mem_block {
    int ID;
    int size;
    struct mem_block *prev, *next;

#ifdef DEBUG
    char magic[32];
#endif

    char content[0];
};

struct mem_block *mb = NULL;

static int msize = 0;

static char *memname[] = {
    "none",
    "IM_BASE",
    "IM_TEMP",
    "IM_CHARARGS",
    "IM_TALK",
    "IM_DRDATA",
    "IM_DRHDR",
    "IM_QUERY",
    "IM_DATABASE",
    "IM_NOTIFY",
    "IM_TIMER",
    "IM_PLAYER",
    "IM_STORE",
    "IM_STORAGE",
    "IM_ZLIB",
    "IM_MYSQL",
    "unknown1",
    "unknown2",
    "unknown3",
    "unknown4"};

static void rem_block(struct mem_block *mem) {
    struct mem_block *prev, *next;

    pthread_mutex_lock(&alloc_mutex);

    msize -= mem->size;
    mmem_usage = msize;

    prev = mem->prev;
    next = mem->next;

    if (prev) prev->next = next;
    else mb = next;

    if (next) next->prev = prev;

    pthread_mutex_unlock(&alloc_mutex);
}

static void add_block(struct mem_block *mem) {
    msize += mem->size;
    if (msize > 100 * 1024 * 1024) kill(getpid(), 11);
    mmem_usage = msize;

    pthread_mutex_lock(&alloc_mutex);

    mem->prev = NULL;
    mem->next = mb;
    if (mb) mb->prev = mem;
    mb = mem;

    pthread_mutex_unlock(&alloc_mutex);
}

void *xmalloc(int size, int ID) {
    struct mem_block *mem;

#ifdef DEBUG
    mem = malloc(size + sizeof(struct mem_block) + 32);
#else
    mem = malloc(size + sizeof(struct mem_block));
#endif
    if (!mem) return NULL;

    mem->ID = ID;
    mem->size = size;

#ifdef DEBUG
    memcpy(mem->magic, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345", 32);
    memcpy(mem->content + size, "abcdefghijklmnopqrstuvwxyz678901", 32);
#endif

    add_block(mem);

    return mem->content;
}

void *xcalloc(int size, int ID) {
    void *ptr;

    ptr = xmalloc(size, ID);
    if (!ptr) return NULL;

    bzero(ptr, size);

    return ptr;
}

void *xrealloc(void *ptr, int size, int ID) {
    struct mem_block *mem;

    if (ptr == NULL) return xcalloc(size, ID);

    mem = (void *)((char *)(ptr) - sizeof(struct mem_block));

    rem_block(mem);

#ifdef DEBUG
    mem = realloc(mem, size + sizeof(struct mem_block) + 32);
#else
    mem = realloc(mem, size + sizeof(struct mem_block));
#endif
    if (!mem) return NULL;

    mem->size = size;
    mem->ID = ID;

#ifdef DEBUG
    memcpy(mem->magic, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345", 32);
    memcpy(mem->content + size, "abcdefghijklmnopqrstuvwxyz678901", 32);
#endif

    add_block(mem);

    return mem->content;
}

void xfree(void *ptr) {
    struct mem_block *mem;

    mem = (void *)((char *)(ptr) - sizeof(struct mem_block));

#ifdef DEBUG
    if (memcmp(mem->magic, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345", 32)) {
        elog("memory fail: top, type=%d, size=%d", mem->ID, mem->size);
        elog("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
             mem->content[0],
             mem->content[1],
             mem->content[2],
             mem->content[3],
             mem->content[4],
             mem->content[5],
             mem->content[6],
             mem->content[7],
             mem->content[8],
             mem->content[9],
             mem->content[10],
             mem->content[11],
             mem->content[12],
             mem->content[13],
             mem->content[14],
             mem->content[15]);
    }
    if (memcmp(mem->content + mem->size, "abcdefghijklmnopqrstuvwxyz678901", 32)) {
        elog("memory fail: bottom, type=%d, size=%d", mem->ID, mem->size);
        elog("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
             mem->content[0],
             mem->content[1],
             mem->content[2],
             mem->content[3],
             mem->content[4],
             mem->content[5],
             mem->content[6],
             mem->content[7],
             mem->content[8],
             mem->content[9],
             mem->content[10],
             mem->content[11],
             mem->content[12],
             mem->content[13],
             mem->content[14],
             mem->content[15]);
    }
    bzero(mem->magic, 32);
    bzero(mem->content + mem->size, 32);
#endif

    rem_block(mem);

    free(mem);
}

void *xstrdup(char *src, int ID) {
    int size;
    char *dst;

    size = strlen(src) + 1;

    dst = xmalloc(size, ID);
    if (!dst) return NULL;

    memcpy(dst, src, size);

    return dst;
}

void list_mem(void) {
    struct mem_block *mem;
    int cnt[100], size[100], n, tcnt = 0, tsize = 0;

    bzero(cnt, sizeof(cnt));
    bzero(size, sizeof(size));

#ifdef DEBUG
    xlog("mem-DEBUG version");
#endif

    pthread_mutex_lock(&alloc_mutex);
    for (mem = mb; mem; mem = mem->next) {
        if (mem->ID > 99 || mem->ID < 1) {
            elog("list_mem found illegal mem ID: %d at %p, giving up.", mem->ID, mem);
            pthread_mutex_unlock(&alloc_mutex);
            return;
        }
#ifdef DEBUG
        if (memcmp(mem->magic, "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345", 32)) {
            elog("memory fail: top, type=%d, size=%d, p=%p", mem->ID, mem->size, mem);
            elog("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                 mem->content[0],
                 mem->content[1],
                 mem->content[2],
                 mem->content[3],
                 mem->content[4],
                 mem->content[5],
                 mem->content[6],
                 mem->content[7],
                 mem->content[8],
                 mem->content[9],
                 mem->content[10],
                 mem->content[11],
                 mem->content[12],
                 mem->content[13],
                 mem->content[14],
                 mem->content[15]);
        }
        if (memcmp(mem->content + mem->size, "abcdefghijklmnopqrstuvwxyz678901", 32)) {
            elog("memory fail: bottom, type=%d, size=%d, p=%p", mem->ID, mem->size, mem);
            elog("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
                 mem->content[0],
                 mem->content[1],
                 mem->content[2],
                 mem->content[3],
                 mem->content[4],
                 mem->content[5],
                 mem->content[6],
                 mem->content[7],
                 mem->content[8],
                 mem->content[9],
                 mem->content[10],
                 mem->content[11],
                 mem->content[12],
                 mem->content[13],
                 mem->content[14],
                 mem->content[15]);
        }
#endif
        cnt[mem->ID]++;
        size[mem->ID] += mem->size;
        tcnt++;
        tsize += mem->size;
    }

    for (n = 0; n < ARRAYSIZE(memname); n++) {
        if (size[n] || cnt[n]) {
            xlog("%-10.10s: %4d blks, %8.2fK",
                 memname[n], cnt[n], size[n] / (1024.0));
        }
    }
    xlog("total: %d blocks, total size: %.2fM", tcnt, tsize / (1024.0 * 1024.0));
    pthread_mutex_unlock(&alloc_mutex);
}

int init_mem(void) {
    if (pthread_mutex_init(&alloc_mutex, NULL)) return 0;
    return 1;
}
