#ifndef FS_HELPERS_H
#define FS_HELPERS_H

#include <stddef.h>
#include "fs.h"

void fcb_free(FCB* fcb);

// Cria um novo nó
FsNode* fs_create_node(const char* name, NodeType type, FsNode* parent);

// Procura filho por nome
FsNode* fs_find_child(FsNode* dir, const char* name);

// Adiciona filho a um diretório
void fs_add_child(FsNode* dir, FsNode* child);

// Remove filho específico (e libera memória)
void fs_remove_child(FsNode* dir, FsNode* child);

// Libera árvore inteira
void fs_free_tree(FsNode* node);

// Monta o caminho absoluto de um nó em um buffer
void fs_get_path(FsNode* node, char* buffer, size_t size);

#endif