#ifndef FS_H
#define FS_H

#include <stddef.h>

#define MAX_NAME_LEN 64

typedef enum {
    NODE_DIR,
    NODE_FILE
} NodeType;

typedef struct FsNode {
    char name[MAX_NAME_LEN];
    NodeType type;

    struct FsNode* parent;
    struct FsNode* first_child;
    struct FsNode* next_sibling;

    
} FsNode;

// Inicializa o sistema de arquivos em memória
void fs_init(void);

// Libera recursos
void fs_shutdown(void);

// Loop principal da "shell" do mini FS
void fs_shell_loop(void);

#endif
