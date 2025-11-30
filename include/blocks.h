#ifndef BLOCKS_H
#define BLOCKS_H

#include <stddef.h>
#include "fs.h"

#define FS_BLOCK_SIZE 16
#define FS_MAX_BLOCKS 256

void blocks_init(void);
void blocks_shutdown(void);

int  blocks_alloc_for_file(FCB* fcb, const char* data, size_t len);
void blocks_free_for_file(FCB* fcb);
void blocks_dump_file(const FCB* fcb);

#endif
