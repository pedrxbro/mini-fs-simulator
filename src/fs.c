#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


#define PATH_MAX_LEN 1024
#define MAX_TOKENS 32

static FsNode* root = NULL;
static FsNode* current_dir = NULL;
static int next_inode = 1; 

// ===============
// HELPERS DE FCB
// ===============

static FCB* create_fcb(const char* name, FileType type){
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
    fcb->permissions = 0644;        // (rw-r--r--) por enquanto
    fcb->content = NULL;            // Conteúdo vazio

    return fcb;
}


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

    node->fcb = NULL; // se for arquivo, vamos atribuir depois

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

// Remove um filho específico de um diretório e libera memória
static void remove_child(FsNode* dir, FsNode* child);

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

    if (node->fcb) { // Se for arquivo, libera o FCB
        if (node->fcb->content) {
            free(node->fcb->content);
        }
        free(node->fcb);
    }

    // Libera o nó atual
    free(node);
} 

static void remove_child(FsNode* dir, FsNode* child){
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
            free_tree(curr); // Libera o nó e seus filhos
            return;
        }
        prev = curr;
        curr = curr->next_sibling;
    }
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
    printf("  help                 - Mostra comandos disponíveis\n");
    printf("  pwd                  - Mostra o caminho do diretorio atual\n");
    printf("  mkdir <dir>          - Cria um novo diretorio no diretório atual\n");
    printf("  ls [name]            - Lista o conteudo do diretorio atual\n");
    printf("  cd [path]            - Altera o diretório atual\n");
    printf("  touch <file>         - Cria um novo arquivo no diretório atual\n");
    printf("  write <file> <text>  - Criar/Sobrescrever arquivos com o texto fornecido\n");
    printf("  cat <file>           - Imprime o conteúdo do arquivo\n");
    printf("  cp <src> <dst>      - Copia um arquivo\n");
    printf("  mv <old> <new>      - Renomeia/move um arquivo dentro do diretório atual\n");
    printf("  rm <file>           - Remove um arquivo\n");
    printf("  exit                 - Sai do simulador\n");
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
        printf("mkdir: Nome de diretório invalido\n");
        return;
    }

    if (strchr(name, '/')){
        printf("mkdir: Nome de diretório nao pode conter '/'\n");
        return;
    }

    if (find_child(current_dir, name)){
        printf("mkdir: Diretorio ou arquivo com esse nome ja existe\n");
        return;
    }

    FsNode* new_dir = create_node(name, NODE_DIR, current_dir); // Cria novo diretório
    add_child(current_dir, new_dir); // Adiciona ao diretório atual
}

static void cmd_ls(int argc, char** argv){
    FsNode* target = current_dir;

    // Se um nome for fornecido, tenta encontrar esse diretório
    if (argc >= 2){
        const char* name = argv[1];

       if (strcmp(name, ".") == 0){
           // Já está no diretório atual
       } else if (strcmp(name, "..") == 0) { // Sair para pasta pai
            if (current_dir->parent){
            target = current_dir->parent;
            }
         } else {
            FsNode* child = find_child(current_dir, name);
            if (!child){
                printf("ls: Diretorio ou arquivo '%s' nao encontrado\n", name);
                return;
            }
            target = child;
        }
    }

    if (target->type == NODE_FILE){
        // Mostra somente o nome se for um arquivo
        printf("%s\n", target->name);
        return;
    }

    // Lista os filhos do diretório
    FsNode* child = target->first_child;
    while(child){
        if (child->type == NODE_DIR){
            printf("%s/\n", child->name); // Ganha uma barra para identificar como diretório
        } else {
            printf("%s\n", child->name); // Arquivo normal
        }
        child = child->next_sibling;
    }
}

// Mudar diretório 
static void cmd_cd(int argc, char** argv){
    if (argc < 2){
        // Sem argumento, volta para a raíz
        current_dir = root;
        return;
    }

    // Pega o caminho fornecido
    const char* path = argv[1];

    if (strcmp(path, "/") == 0){
        current_dir = root; // Vai para raíz
        return;
    } else if (strcmp(path, ".") == 0){
        // Fica no diretório atual
    } else if (strcmp(path, "..") == 0){
        if (current_dir->parent){
            current_dir  = current_dir->parent; // Sobe para o pai
        }
        return;
    } else {
        FsNode* child = find_child(current_dir, path);
        if(!child || child->type != NODE_DIR){ 
            printf("cd: Diretorio '%s' nao encontrado\n", path);
            return;
        }
        current_dir  = child; // Muda para o diretório encontrado
    }
}

static void cmd_touch(int argc, char** argv){
    if (argc < 2){
        printf("Uso: touch <nome_arquivo>\n");
        return;
    }

    for (int i = 1; i < argc; i++){
        const char* name = argv[i];

        if (strlen(name) == 0 || strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            printf("touch: Nome de arquivo inválido '%s'\n", name);
            continue;
        }

        if (strchr(name, '/')){
            printf("touch: Nome de arquivo nao pode conter '/': '%s'\n", name);
            continue;
        }
        FsNode* existing = find_child(current_dir, name);
        if (existing){
            if (existing->type == NODE_DIR){
                printf("touch: Já existe um diretório com esse nome: '%s'\n", name);
                continue;
            } 
            // arquivo já existe -> atualiza timestamps
            if (existing->fcb) {
                time_t now = time(NULL);
                existing->fcb->accessed_at = now;
                existing->fcb->modified_at = now;
            }
        } else {
            // Cria novo arquivo
            FsNode* new_file = create_node(name, NODE_FILE, current_dir);
            new_file->fcb = create_fcb(name, FILETYPE_TEXT); // Por enquanto, todos são arquivos de texto
            add_child(current_dir, new_file);
        }
    }

}

static void cmd_write(int argc, char** argv){
    if (argc < 3){
       printf("Uso: write <nome_arquivo> <texto>\n");
       return;
    }

    const char* file_name = argv[1];

    if(strchr(file_name, '/')){
        printf("write: Nome de arquivo nao pode conter '/': '%s'\n", file_name);
        return;
    }

    // Monta o texto a partir do argv
    size_t total_len = 0;
    for (int i = 2; i < argc; i++){
        total_len += strlen(argv[i]);
        if (i < argc -1){
            total_len += 1; // espaço
        }
    }

    char* buffer = (char*)malloc(total_len +1);
    if(!buffer){
        fprintf(stderr, "Erro ao alocar memoria para conteudo do arquivo\n");
        return;
    }

    buffer[0] = '\0';
    for (int i = 2; i < argc; i++){
        strcat(buffer, argv[i]);
        if (i < argc -1){
            strcat(buffer, " ");
        }
    }

    // Verificar se o arquivo já existe
    FsNode* node = find_child(current_dir, file_name);
    if(!node){
        // Cria novo arquivo
        node = create_node(file_name, NODE_FILE, current_dir);
        node->fcb = create_fcb(file_name, FILETYPE_TEXT);
        add_child(current_dir, node);
    } else {
        if (node->type == NODE_DIR){
            printf("write: '%s' nao e um arquivo\n", file_name);
            free(buffer);
            return;
        }  
        if (!node->fcb){
            node->fcb = create_fcb(file_name, FILETYPE_TEXT);
        }
    }

    // Sobrescrever arquivo
    if(node->fcb->content){
        free(node->fcb->content);
    }

    node->fcb->content = buffer;
    node->fcb->size = total_len;

    time_t now = time(NULL);
    node->fcb->modified_at = now;
    node->fcb->accessed_at = now;
}


// Imprime o conteúdo do arquivo
static void cmd_cat(int argc, char** argv){
    if (argc < 2){
        printf("Uso: cat <nome_arquivo>\n");
        return;
    }

    const char* file_name = argv[1];

    FsNode* node = find_child(current_dir, file_name);
    if(!node){
        printf("cat: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }

    if(node->type == NODE_DIR){
        printf("cat: '%s' nao e um arquivo\n", file_name);
        return;
    }

    if(!node->fcb){
        printf("cat: Arquivo '%s' nao possui FCB\n", file_name);
        return;
    }

    if(!node->fcb->content){
         // Arquivo vazio
        node->fcb->accessed_at = time(NULL);
        return;
    }

    printf("%s\n", node->fcb->content);
    node->fcb->accessed_at = time(NULL);
}

static void cmd_cp(int argc, char** argv){
    if(argc < 3){
        printf("Uso: cp <src> <dst>\n");
        return;
    }

    const char* src_name = argv[1];
    const char* dst_name = argv[2];

    if(strchr(src_name, '/')){
        printf("cp: Nomes de arquivo nao podem conter '/'\n");
        return;
    }

    // Procura o arquivo de origem
    FsNode* src = find_child(current_dir, src_name);
    if(!src){
        printf("cp: Arquivo de origem '%s' nao encontrado\n", src_name);
        return;
    }

    if(src->type == NODE_DIR){
        printf("cp: '%s' nao e um arquivo\n", src_name);
        return;
    }

    if(!src->fcb){
        printf("cp: Arquivo de origem '%s' nao possui FCB\n", src_name);
        return;
    }

    if(find_child(current_dir, dst_name)){
        printf("cp: Não foi possível criar arquivi. Arquivo de destino '%s' ja existe\n", dst_name);
        return;
    }

    // Cria o novo arquivo
    FsNode* dst = create_node(dst_name, NODE_FILE, current_dir);
    dst->fcb = create_fcb(dst_name, src->fcb->type);


    // Copia o conteúdo, se existir
    if (src->fcb->content && src->fcb->size > 0) {
        dst->fcb->content = (char*)malloc(src->fcb->size + 1);
        if(!dst->fcb->content){
            fprintf(stderr, "Erro ao alocar memoria para conteudo do arquivo\n");
            free(dst->fcb);
            free(dst);
            return;
        }
        memcpy(dst->fcb->content, src->fcb->content, src->fcb->size);
        dst->fcb->content[src->fcb->size] = '\0';

        dst->fcb->size = src->fcb->size;
    } else {
        dst->fcb->content = NULL;
        dst->fcb->size = 0;
    }

    // timestamp do dst
    time_t now = time(NULL);
    dst->fcb->created_at = now;
    dst->fcb->modified_at = now;
    dst->fcb->accessed_at = now;

    add_child(current_dir, dst);
}
    
// Renomeia ou move um arquivo
static void cmd_mv(int argc, char** argv){
    if(argc < 3){
        printf("Uso: mv <old> <new>\n");
        return;
    }

    const char* old_name = argv[1];
    const char* new_name = argv[2];

    if(strchr(new_name, '/')){
        printf("mv: Nomes de arquivo nao podem conter '/'\n");
        return;
    }

    FsNode* node = find_child(current_dir, old_name);
    if(!node){
        printf("mv: Arquivo '%s' nao encontrado\n", old_name);
        return;
    }

    if(find_child(current_dir, new_name)){
        printf("mv: Não foi possível renomear. Arquivo '%s' ja existe\n", new_name);
        return;
    }

    // Renomeia
    strncpy(node->name, new_name, MAX_NAME_LEN -1);
    node->name[MAX_NAME_LEN -1] = '\0';

    // Se for arquivo, renomeia no FCB também
    if(node->fcb){
        strncpy(node->fcb->name, new_name, MAX_NAME_LEN -1);
        node->fcb->name[MAX_NAME_LEN -1] = '\0';
    }

}

// Remove um arquivo
static void cmd_rm(int argc, char** argv){
    if(argc < 2){
        printf("Uso: rm <nome_arquivo>\n");
        return;
    }

    const char* file_name = argv[1];

    FsNode* node = find_child(current_dir, file_name);
    if(!node){
        printf("rm: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }

    if(node->type == NODE_DIR){
        printf("rm: '%s' nao e um arquivo\n", file_name);
        return;
    }

    // Remove o nó do diretório atual
    remove_child(current_dir, node);
}

static void handle_command(int argc, char** argv){
    const char* cmd = argv[0];

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "pwd") == 0){
        cmd_pwd();
    } else if (strcmp(cmd, "mkdir") == 0){
        cmd_mkdir(argc, argv);
    } else if (strcmp(cmd, "ls") == 0){
        cmd_ls(argc, argv);
    } else if (strcmp(cmd, "cd") == 0){  
        cmd_cd(argc, argv); 
    } else if (strcmp(cmd, "touch") == 0) {
        cmd_touch(argc, argv);
    } else if (strcmp(cmd, "write") == 0) {
        cmd_write(argc, argv);
    } else if (strcmp(cmd, "cat") == 0) {
        cmd_cat(argc, argv);
    } else if (strcmp(cmd, "cp") == 0) {
        cmd_cp(argc, argv);
    } else if (strcmp(cmd, "mv") == 0) {
        cmd_mv(argc, argv);
    } else if (strcmp(cmd, "rm") == 0) {
        cmd_rm(argc, argv);
    } else {
        printf("Comando desconhecido: %s\n", cmd);
        printf("Digite 'help' para ver a lista de comandos disponiveis.\n");
    }

}