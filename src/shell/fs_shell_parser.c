#include <stdio.h>
#include <string.h>

#include "fs.h"
#include "fs_helpers.h"
#include "cmd.h"

#define PATH_MAX_LEN 1024
#define MAX_TOKENS   32


// Monta o caminho absoluto de um nó 
static void get_path(FsNode* node, char* buffer, size_t size){
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



static void print_prompt(void) {
    char path[PATH_MAX_LEN];
    get_path(fs_current_dir, path, sizeof(path));
    printf("%s$ ", path);
    fflush(stdout);
}

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


void fs_shell_loop(void) {
    char line[512];
    char* argv[MAX_TOKENS];

    int running = 1;
    while (running) {
        // prompt
        print_prompt();

        // leitura
        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        // remove \n
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }

        // tokeniza
        int argc = parse_line(line, argv, MAX_TOKENS);

        if (argc == 0) {
            continue;
        }

        // exit tratado aqui
        if (strcmp(argv[0], "exit") == 0) {
            running = 0;
            continue;
        }

        // delega para a camada de comandos
        cmd_handle(argc, argv);
    }
}
