#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "fs_helpers.h"
#include "fcb_helpers.h"
#include "blocks.h"



// Cria um novo nó do sistema de arquivos
FsNode* fs_create_node(const char* name, NodeType type, FsNode* parent){
    FsNode* node = (FsNode*)malloc(sizeof(FsNode));

    if(!node){
        fprintf(stderr, "Erro ao alocar memoria para nó do sistema de arquivos\n");
        exit(EXIT_FAILURE);
    }

    strncpy(node->name, name, MAX_NAME_LEN -1);
    node->name[MAX_NAME_LEN -1] = '\0';
    node->type = type;

    node->parent = parent;
    node->first_child = NULL;
    node->next_sibling = NULL;

    node->fcb = NULL; // se for arquivo, vamos atribuir depois

    return node;
}

// Procura filho por nome
FsNode* fs_find_child(FsNode* dir, const char* name){
    if (!dir || dir->type != NODE_DIR) {
        return NULL; // Sem filhos para procurar
    }

    // Aponta o primeiro filho
    FsNode* child = dir->first_child; 


    // Percorre os filhos
    while(child){
        if(strcmp(child->name, name) == 0){ // Se as strings forem iguais. Retorna 0
            return child;
        }
        child = child->next_sibling; // Proximo irmão
    }
    return NULL;
}

// Adiciona um nó filho a um diretório
void fs_add_child(FsNode* dir, FsNode* child){

    if (!dir || dir->type != NODE_DIR) {
        fprintf(stderr, "Erro: Tentativa de adicionar filho a um nó que não é diretório\n");
        return;
    }

    // Define o diretório como pai do novo nó
    child->parent = dir;

    if (!dir->first_child){ // Se nao tiver filho
        dir->first_child = child; // Primeiro filho
    } else {
        // Se já tem filho
        FsNode* last = dir->first_child;
        while(last->next_sibling){
            last = last->next_sibling; // Percorre até chegar no último irmão 
        }
        last->next_sibling = child; // Novo nó vira o próximo irmão 
    }
}

// Remove um filho específico de um diretório e libera memória
void fs_remove_child(FsNode* dir, FsNode* child){
    if (!dir || !child) {
        return; // Nada a fazer
    }

    FsNode* prev = NULL;
    FsNode* curr = dir->first_child;

    // Procura o filho na lista
    while(curr){
        if (curr == child){
            // Encontrou o filho
            if (prev){
                prev->next_sibling = curr->next_sibling; // Remove da lista
            } else {
                dir->first_child = curr->next_sibling; // Atualiza o primeiro filho
            }
            curr->next_sibling = NULL; // Desconecta
            fs_free_tree(curr); // Libera o nó e seus filhos
            return;
        }
        prev = curr;
        curr = curr->next_sibling;
    }
}

static void fs_free_tree_internal(FsNode* node) {
    if (!node) return;

    FsNode* child = node->first_child;
    while (child) {
        FsNode* next = child->next_sibling;
        fs_free_tree_internal(child);
        child = next;
    }

    if (node->fcb) {
        blocks_free_for_file(node->fcb);
        if (node->fcb->content) {
            free(node->fcb->content);
    }
    free(node->fcb);
}

    free(node);
}

void fs_free_tree(FsNode* node) {
    fs_free_tree_internal(node); 
}

void fs_get_path(FsNode* node, char* buffer, size_t size){
    if(!node){
        snprintf(buffer, size, "?"); // Inválido
        return;
    }

    if (node == fs_root){
        snprintf(buffer, size, "/"); // Raíz
        return;
    }

    // Guarda o caminho do pai temporariamente
    char temp[PATH_MAX_LEN];

    // Se tiver pai, chama recursivamente
    if (node->parent){
        fs_get_path(node->parent, temp, sizeof(temp));
    } else {
        snprintf(temp, sizeof(temp), "/"); // Sem pai, é raíz
    }
    // Se o pai for raíz
    if (strcmp(temp, "/") == 0){ 
        snprintf(buffer, size, "/%s", node->name); // Evita duplicar barra
    } else {
        snprintf(buffer, size, "%s/%s", temp, node->name); // Caminho completo
    }
}
