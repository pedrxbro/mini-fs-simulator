#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fs.h"
#include "fcb_helpers.h"


static int next_inode = 1; // contador de inodes

FCB* create_fcb(const char* name, FileType type){
    FCB* fcb = (FCB*)malloc(sizeof(FCB));
    if(!fcb){
        fprintf(stderr, "Erro ao alocar memoria para FCB\n");
        exit(EXIT_FAILURE);
    }

    strncpy(fcb->name, name, MAX_NAME_LEN -1);
    fcb->name[MAX_NAME_LEN -1] = '\0';
    fcb->size = 0;
    fcb->type = type;

    time_t now = time(NULL);
    fcb->created_at = now;
    fcb->modified_at = now;
    fcb->accessed_at = now;

    fcb->inode = next_inode++;
    fcb->permissions = 0644;                   // (rw-r--r--) por enquanto
    fcb->owner = fs_current_user_class;        // proprietário padrão

    fcb->block_count = 0;                      // Nenhum bloco alocado
    for (int i = 0; i < FCB_MAX_BLOCKS; i++)
    {
        fcb->blocks[i] = -1;                     // Inicializa todos os blocos como não alocados
    }
    


    fcb->content = NULL;                       // Conteúdo vazio

    return fcb;
}

void free_fcb(FCB* fcb) {
    if (!fcb) return;

    if (fcb->content) {
        free(fcb->content);
    }
    free(fcb);
}