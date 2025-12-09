#include <stdio.h>
#include <string.h>
#include "cmd.h"
#include "commands.h"  

void cmd_help(void) {
    printf("Comandos disponiveis:\n");
    printf("  help                     - Mostra comandos disponíveis\n");
    printf("  pwd                      - Mostra o caminho do diretorio atual\n");
    printf("  mkdir <dir>              - Cria um novo diretorio no diretório atual\n");
    printf("  ls [name]                - Lista o conteudo do diretorio atual\n");
    printf("  cd [path]                - Altera o diretório atual\n");
    printf("  touch <file>             - Cria um novo arquivo no diretório atual\n");
    printf("  write <file> <text>      - Criar/Sobrescrever arquivos com o texto fornecido\n");
    printf("  cat <file>               - Imprime o conteúdo do arquivo\n");
    printf("  cp <src> <dst>           - Copia um arquivo\n");
    printf("  mv <old> <new>           - Renomeia/move um arquivo dentro do diretório atual\n");
    printf("  rm <file>                - Remove um arquivo\n");
    printf("  chmod <perms> <file>     - Altera as permissões de um arquivo\n");
    printf("  user <owner|group|other> - Altera o usuario atual da simulacao\n");
    printf("  whoami                   - Mostra o usuário atual\n");
    printf("  stat <file>              - Mostra metadados e blocos do arquivo\n");
    printf("  df                       - Mostra estatisticas do disco simulado\n");
    printf("  exit                     - Sai do simulador\n");
}

// Essa função será chamada pelo shell
void cmd_handle(int argc, char** argv) {
    const char* cmd = argv[0];

    if (strcmp(cmd, "help") == 0) {
        cmd_help();
    } else if (strcmp(cmd, "pwd") == 0) {
        cmd_pwd();
    } else if (strcmp(cmd, "mkdir") == 0) {
        cmd_mkdir(argc, argv);
    } else if (strcmp(cmd, "ls") == 0) {
        cmd_ls(argc, argv);
    } else if (strcmp(cmd, "cd") == 0) {  
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
    } else if (strcmp(cmd, "chmod") == 0) {
        cmd_chmod(argc, argv);
    } else if (strcmp(cmd, "user") == 0) {
        cmd_user(argc, argv);
    } else if (strcmp(cmd, "whoami") == 0) {
        cmd_whoami();
    } else if (strcmp(cmd, "stat") == 0) {
        cmd_stat(argc, argv);
    } else if (strcmp(cmd, "df") == 0) {
        cmd_df();
    } else {
        printf("Comando desconhecido: %s\n", cmd);
        printf("Digite 'help' para ver a lista de comandos disponiveis.\n");
    }
}
