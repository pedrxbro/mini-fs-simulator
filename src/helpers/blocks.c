#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "blocks.h"

static char fs_disk[FS_BLOCK_SIZE * FS_MAX_BLOCKS];
static int fs_block_used[FS_MAX_BLOCKS];


void blocks_init(){
    for (int i = 0; i < FS_MAX_BLOCKS; i++){
        fs_block_used[i] = 0; // Livre
    }
}

void blocks_shutdown(){
    // Nada a fazer por enquanto
}

static int blocks_find_free(int needed, int* out_indices){
    int found = 0;
    
    for (int i = 0; i < FS_MAX_BLOCKS && found < needed; i++){
        if(!fs_block_used[i]){
            out_indices[found++] = i;
        }
    }

    return (found == needed) ? 0 : -1;
}

void blocks_free_for_file(FCB* fcb){
    if(!fcb) return;

    for(int i = 0; i < fcb->block_count; i++){
        int block_index = fcb->blocks[i];
        if(block_index >=0 && block_index < FS_MAX_BLOCKS){
            fs_block_used[block_index] = 0; // libera o bloco
        }
        fcb->blocks[i] = -1; // invalida o índice
    }
    fcb->block_count = 0;
}

int blocks_alloc_for_file(FCB* fcb, const char* data, size_t len){
    if(!fcb) return -1;

    blocks_free_for_file(fcb); // libera blocos existentes

    if(len == 0) return 0; // nada a alocar

    // Calcula quantos blocos são necessários para armazinar os dados
    size_t blocks_needed = (len + FS_BLOCK_SIZE -1) / FS_BLOCK_SIZE;
    if((int)blocks_needed > FCB_MAX_BLOCKS){
        return -1; // arquivo muito grande
    }

    int indexes[FCB_MAX_BLOCKS];
    // Procura por blocos livres
    if(blocks_find_free((int)blocks_needed, indexes) != 0){
        return -1; // espaço insuficiente
    }

    // Marca os blocos encontrados como "usados" e armazena no FCB
    for (int i = 0; i < (int)blocks_needed; i++){
        int idx = indexes[i];
        fs_block_used[idx] = 1;     // marca como usado
        fcb->blocks[i] = idx;       // armazena o índice
    }

    fcb->block_count = (int)blocks_needed; // Registra quantos blocos foram alocados

    // Grava os dados do arquivo dentro dos blocos alocados
    for (int i = 0; i < (int)blocks_needed; i++){
        int idx = fcb->blocks[i];
        size_t offset = i * FS_BLOCK_SIZE;
        size_t remaining = 0;

        // Calcula quantos bytes ainda faltam escrever no arquivo
        if(offset < len){
            remaining = len - offset;
            if (remaining > FS_BLOCK_SIZE){
                remaining = FS_BLOCK_SIZE;
            }
        }

        size_t base = (size_t)idx * FS_BLOCK_SIZE; // Endereço do bloco
        if(remaining > 0){
            memcpy(&fs_disk[base], data + offset, remaining); // Preenche com os dados
        } if(remaining < FS_BLOCK_SIZE){
            memset(&fs_disk[base + remaining], 0, FS_BLOCK_SIZE - remaining); // Zera o restante do bloco
        } else {
            memset(&fs_disk[base], 0, FS_BLOCK_SIZE); // Zera o bloco inteiro
        }
    }
    return 0;
}

void blocks_dump_file(const FCB* fcb) {
    if (!fcb) return;

    printf("blocos: ");
    for (int i = 0; i < fcb->block_count; i++) {
        printf("%d ", fcb->blocks[i]); // Imprime cada bloco alocado ao arquivo
    }
    if (fcb->block_count == 0) {
        printf("(nenhum bloco alocado)");
    }
    printf("\n");
}

void blocks_stats(int* total_blocks, int* used_blocks, int* free_blocks){
    if(total_blocks) { *total_blocks = FS_MAX_BLOCKS; } 

    int used = 0;
    // Conta quantos blocos estão marcados como usados 
    for (int i = 0; i < FS_MAX_BLOCKS; i++){
        if(fs_block_used[i]){
            used++;
        }
    }

    if (used_blocks) { *used_blocks = used; }

    if (free_blocks) { *free_blocks = FS_MAX_BLOCKS - used; }
}