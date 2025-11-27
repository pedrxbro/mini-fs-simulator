#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


static FsNode* root = NULL;
static FsNode* current_dir = NULL;

static FsNode* create_node(const char* name, NodeType type, FsNode* parent){
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

    return node;
}

void fs_init(){
    //Cria o diretório raíz
    root = create_node("/", NODE_DIR, NULL);
    current_dir = root;
}

void fs_shutdown(){
    printf("Desligando sistema de arquivos\n"); // Implementação futura
}

static void print_prompt(void) {
    // Estático por enquanto
    printf("/home/user$ ");
    fflush(stdout);
}

static void handle_command(const char* cmd){
    if (strcmp(cmd, "help") == 0) {
        printf("Comandos disponíveis:\n");
        printf("help - Mostra ajuda\n");
        printf("exit - Sai do simulador\n");
    } else if (strcmp(cmd, "exit") == 0) {
        // Quem trata é o loop principal
    } else if (cmd[0] == '\0') {
        // Linha vazia: não faz nada
    } else {
        printf("Comando desconhecido: %s\n", cmd);
        printf("Digite 'help' para ver os comandos disponíveis.\n");
    }
}

void fs_shell_loop(){
    char line[256];

    int running = 1;
    while(running){
        print_prompt();

        if(!fgets(line, sizeof(line), stdin)){
            //Erro ou EOF
            printf("\n");
            break;
        }

        //Remove nova linha
        size_t len = strlen(line);
        if(len > 0 && line[len -1] == '\n'){
            line[len -1] = '\0';
        }

        if(strcmp(line, "exit") == 0){
            running = 0;
        } else {
            handle_command(line);
        }
    }

}