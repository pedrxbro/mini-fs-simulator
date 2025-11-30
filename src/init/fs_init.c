#include <stdio.h>
#include "fs.h"
#include "fs_helpers.h"


void fs_init(){
    //Cria o diretório raíz
    fs_root = fs_create_node("/", NODE_DIR, NULL);
    fs_current_dir = fs_root;
}


// Desliga o sistema de arquivos
void fs_shutdown(){
    printf("Desligando sistema de arquivos\n");
    fs_free_tree(fs_root);
    fs_root = NULL;
    fs_current_dir = NULL;
}