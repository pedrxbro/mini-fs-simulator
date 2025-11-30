#ifndef FS_H
#define FS_H

#include <stddef.h>
#include <time.h>

#define MAX_NAME_LEN 64
#define PATH_MAX_LEN 1024
#define MAX_TOKENS 32
#define FCB_MAX_BLOCKS 32

typedef enum {
    NODE_DIR,
    NODE_FILE
} NodeType;

typedef enum {
    FILETYPE_TEXT,
    FILETYPE_NUMERIC,
    FILETYPE_BINARY,
    FILETYPE_PROGRAM
} FileType;

// Usuários
typedef enum {
    USER_OWNER,
    USER_GROUP,
    USER_OTHER
} UserClass;

typedef struct FCB {
    char name[MAX_NAME_LEN];
    size_t  size;
    FileType type;

    time_t created_at;          // data/hora de criação
    time_t modified_at;         // data/hora de modificação
    time_t accessed_at;         // data/hora de último acesso

    int inode;                  // Identificador único do nó no sistema de arquivos
    unsigned int permissions;   // Permissões de acesso
    UserClass owner;            // Classe do usuário proprietário

    int blocks[FCB_MAX_BLOCKS]; // Blocos alocados para o arquivo
    int block_count;           // Número de blocos alocados

    char* content;              // Ponteiro para o conteúdo do arquivo na memória
} FCB;

typedef struct FsNode {
    char name[MAX_NAME_LEN];
    NodeType type;

    struct FsNode* parent;       // Ponteiro para o nó pai
    struct FsNode* first_child;  // Ponteiro para o primeiro filho (se for diretório)
    struct FsNode* next_sibling; // Ponteiro para o próximo irmão
        
    FCB* fcb;                    // Ponteiro para o FCB (se for arquivo)
} FsNode;


extern FsNode* fs_root;
extern FsNode* fs_current_dir;
extern UserClass fs_current_user_class;

// Inicializa o sistema de arquivos em memória
void fs_init(void);

// Libera recursos
void fs_shutdown(void);

// Loop principal da "shell" do mini FS
void fs_shell_loop(void);

#endif
