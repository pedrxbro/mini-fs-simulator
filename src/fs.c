#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


#define PATH_MAX_LEN 1024
#define MAX_TOKENS 16

static FsNode* root = NULL;
static FsNode* current_dir = NULL;

// ====================
// HELPERS DE ESTRUTURA 
// ====================

// Cria um novo nó do sistema de arquivos
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

// Procura filho por nome
static FsNode* find_child(FsNode* dir, const char* name){
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
static void add_child(FsNode* dir, FsNode* child){

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

// Monta o caminho absoluto de um nó 
static void get_path(FsNode* node, char* buffer, size_t size){
    if(!node){
        snprintf(buffer, size, "?"); // Inválido
        return;
    }

    if (node == root){
        snprintf(buffer, size, "/"); // Raíz
        return;
    }

    // Guarda o caminho do pai temporariamente
    char temp[PATH_MAX_LEN];

    // Se tiver pai, chama recursivamente
    if (node->parent){
        get_path(node->parent, temp, sizeof(temp));
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
// ========================
// INICIALIZAÇÃO / SHUTDOWN
// ========================

void fs_init(){
    //Cria o diretório raíz
    root = create_node("/", NODE_DIR, NULL);
    current_dir = root;
}

// Liberar todos os nós recursivamente
static void free_tree(FsNode* node){
    if(!node) return;

    // Libera filhos recursivamente
    FsNode* child = node->first_child;
    while(child){
        FsNode* next = child->next_sibling; // Guarda próximo irmão
        free_tree(child);                   // Libera filho
        child = next;                       // Vai para o próximo irmão
    }

    // Libera o nó atual
    free(node);
} 

// Desliga o sistema de arquivos
void fs_shutdown(){
    printf("Desligando sistema de arquivos\n");
    free_tree(root);
    root = NULL;
    current_dir = NULL;
}

// ==============
// SHELL E PARSER
// ==============

static void print_prompt(void) {
    char path[PATH_MAX_LEN];
    get_path(current_dir, path, sizeof(path));
    printf("%s$ ", path);
    fflush(stdout);
}

// Processa depois de tokenizar
static void handle_command(int argc, char** argv);

// Tokeniza a linha de comando
// Cada token representa um comando ou argumento.
static int parse_line(char* line, char** argv, int max_tokens){
    int argc = 0;

    char* token = strtok(line, " \t");;   // Divide a linha por espaços 
    while (token && argc < max_tokens){
        argv[argc++] = token;           // Guarda o token  
        token = strtok(NULL, " \t");     // Próximo token
    }
    return argc;                        // Número de tokens 
}


void fs_shell_loop(){
    char line[256];
    char* argv[MAX_TOKENS];

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

        // tokenizar
        int argc = parse_line(line, argv, MAX_TOKENS);
        if (argc == 0) {
            // linha vazia
            continue;
        }

        if (strcmp(argv[0], "exit") == 0){
            running = 0; // Sai do loop
        } else {
            handle_command(argc, argv); // Processa o comando
        }
    }

}
// ===========================
// IMPLEMENTAÇÃO DOS COMANDOS
// ===========================

// Listagem de comandos
static void cmd_help(){
    printf("Comandos disponiveis:\n");
    printf("  help            - Mostra comandos disponíveis\n");
    printf("  pwd             - Mostra o caminho do diretorio atual\n");
    printf("  mkdir <dir>     - Cria um novo diretorio no diretório atual\n");
    printf("  exit            - Sai do simulador\n");
}

// Diretório atual 
static void cmd_pwd(){
    char path[PATH_MAX_LEN];
    get_path(current_dir, path, sizeof(path));
    printf("%s\n", path);
}

// Criar diretório
static void cmd_mkdir(int argc, char** argv){
    if (argc < 2){
        printf("Uso: mkdir <nome_diretorio>\n");
        return;
    }

    // Nome do novo diretório
    const char* name = argv[1];

    if (strlen(name) == 0 || strcmp(name, ".") == 0 || strcmp(name, "..") == 0){
        printf("Nome de diretorio invalido\n");
        return;
    }

    if (strchr(name, '/')){
        printf("Nome de diretorio nao pode conter '/'\n");
        return;
    }

    if (find_child(current_dir, name)){
        printf("Erro: Diretorio ou arquivo com esse nome ja existe\n");
        return;
    }

    FsNode* new_dir = create_node(name, NODE_DIR, current_dir); // Cria novo diretório
    add_child(current_dir, new_dir); // Adiciona ao diretório atual
}

static void handle_command(int argc, char** argv){
    const char* cmd = argv[0];

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "pwd") == 0){
        cmd_pwd();
    } else if (strcmp(cmd, "mkdir") == 0){
        cmd_mkdir(argc, argv);
    } else {
        printf("Comando desconhecido: %s\n", cmd);
        printf("Digite 'help' para ver a lista de comandos disponiveis.\n");
    }

}