#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fs.h"
#include "fs_helpers.h"
#include "fcb_helpers.h"
#include "commands.h"
#include "permissions.h"
#include "blocks.h"


// Diretório atual 
void cmd_pwd(){
    char path[PATH_MAX_LEN];
    fs_get_path(fs_current_dir, path, sizeof(path));
    printf("%s\n", path);
}

// Criar diretório
void cmd_mkdir(int argc, char** argv){
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

    if (fs_find_child(fs_current_dir, name)){
        printf("mkdir: Diretorio ou arquivo com esse nome ja existe\n");
        return;
    }

    FsNode* new_dir = fs_create_node(name, NODE_DIR, fs_current_dir); // Cria novo diretório
    fs_add_child(fs_current_dir, new_dir); // Adiciona ao diretório atual
}

void cmd_ls(int argc, char** argv){
    FsNode* target = fs_current_dir;

    int long_format = 0;
    int arg_index   = 1;

    if (argc >= 2 && strcmp(argv[1], "-l") == 0){
        long_format = 1;
        arg_index   = 2;
    }

    // Se um nome for fornecido, tenta encontrar esse diretório
    if (argc > arg_index){
        const char* name = argv[arg_index];

        if (strcmp(name, ".") == 0){
            // Já está no diretório atual
        } else if (strcmp(name, "..") == 0) { // Sair para pasta pai
            if (fs_current_dir->parent){
                target = fs_current_dir->parent;
            }
        } else {
            FsNode* child = fs_find_child(fs_current_dir, name);
            if (!child){
                printf("ls: Diretorio ou arquivo '%s' nao encontrado\n", name);
                return;
            }
            target = child;
        }
    }

    if (target->type == NODE_FILE){
        // Se for um arquivo e tiver formato longo, mostra permissões e tamanho
        if(long_format && target->fcb){
            char perms[10];
            perms_to_string(target->fcb->permissions, perms, sizeof(perms));
            
            const char* owner_name = "unknown";
            switch (target->fcb->owner) {
                case USER_OWNER: owner_name = "owner"; break;
                case USER_GROUP: owner_name = "group"; break;
                case USER_OTHER: owner_name = "other"; break;
            }

            printf("%s %s %zu %s\n", perms, owner_name, target->fcb->size, target->name);
        }
        else {
            // Mostra somente o nome
            printf("%s\n", target->name);
        }
        return;
    }

    // Lista os filhos do diretório
    FsNode* child = target->first_child;
    while(child){
        if(long_format && child->fcb){
            char perms[10];
            perms_to_string(child->fcb->permissions, perms, sizeof(perms));
            
            const char* owner_name = "unknown";
            switch (child->fcb->owner) {
                case USER_OWNER: owner_name = "owner"; break;
                case USER_GROUP: owner_name = "group"; break;
                case USER_OTHER: owner_name = "other"; break;
            }
            printf("%s %s %zu %s\n", perms, owner_name, child->fcb->size, child->name);
        } else {
            if (child->type == NODE_DIR){
                printf("%s/\n", child->name); // Ganha uma barra para identificar como diretório
            } else {
                printf("%s\n", child->name); // Arquivo normal
            }
        }
        child = child->next_sibling;
    }
}

// Mudar diretório 
void cmd_cd(int argc, char** argv){
    if (argc < 2){
        // Sem argumento, volta para a raíz
        fs_current_dir = fs_root;
        return;
    }

    // Pega o caminho fornecido
    const char* path = argv[1];

    if (strcmp(path, "/") == 0){
        fs_current_dir = fs_root; // Vai para raíz
        return;
    } else if (strcmp(path, ".") == 0){
        // Fica no diretório atual
    } else if (strcmp(path, "..") == 0){
        if (fs_current_dir->parent){
            fs_current_dir  = fs_current_dir->parent; // Sobe para o pai
        }
        return;
    } else {
        FsNode* child = fs_find_child(fs_current_dir, path);
        if(!child || child->type != NODE_DIR){ 
            printf("cd: Diretorio '%s' nao encontrado\n", path);
            return;
        }
        fs_current_dir  = child; // Muda para o diretório encontrado
    }
}

void cmd_touch(int argc, char** argv){
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
        FsNode* existing = fs_find_child(fs_current_dir, name);
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
            FsNode* new_file = fs_create_node(name, NODE_FILE, fs_current_dir);
            new_file->fcb = create_fcb(name, FILETYPE_TEXT); // Por enquanto, todos são arquivos de texto
            fs_add_child(fs_current_dir, new_file);
        }
    }

}

void cmd_write(int argc, char** argv){
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
    FsNode* node = fs_find_child(fs_current_dir, file_name);

    if(!node){
        // Cria novo arquivo
        node = fs_create_node(file_name, NODE_FILE, fs_current_dir);
        node->fcb = create_fcb(file_name, FILETYPE_TEXT);
        fs_add_child(fs_current_dir, node);
    } else {
        if (node->type == NODE_DIR){
            printf("write: '%s' nao e um arquivo\n", file_name);
            free(buffer);
            return;
        }  
        if (!node->fcb){
            node->fcb = create_fcb(file_name, FILETYPE_TEXT);
        } else {
            // Verifica se há permissão de escrita  
            if(!perms_can_write(node->fcb)){
                printf("write: Permissão negada para escrever no arquivo '%s'\n", file_name);
                free(buffer);
                return;
            }
        }
    }

    // Sobrescrever arquivo
    if(node->fcb->content){
        free(node->fcb->content);
    }

    node->fcb->content = buffer;
    node->fcb->size = total_len;

    if (blocks_alloc_for_file(node->fcb, node->fcb->content, node->fcb->size) != 0) {
        printf("write: Falha ao alocar blocos para '%s' (disco cheio ou arquivo muito grande)\n", file_name);
    }

    time_t now = time(NULL);
    node->fcb->modified_at = now;
    node->fcb->accessed_at = now;
}


// Imprime o conteúdo do arquivo
void cmd_cat(int argc, char** argv){
    if (argc < 2){
        printf("Uso: cat <nome_arquivo>\n");
        return;
    }

    const char* file_name = argv[1];

    FsNode* node = fs_find_child(fs_current_dir, file_name);
    if(!node){
        printf("cat: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }

    if(node->type == NODE_DIR){
        printf("cat: '%s' não é um arquivo\n", file_name);
        return;
    }

    if(!node->fcb){
        printf("cat: Arquivo '%s' não possui FCB\n", file_name);
        return;
    }

    if(!perms_can_read(node->fcb)){
        printf("cat: Permissão negada para ler o arquivo '%s'\n", file_name);
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

void cmd_cp(int argc, char** argv){
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
    FsNode* src = fs_find_child(fs_current_dir, src_name);
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

    if(!perms_can_read(src->fcb)){
        printf("cp: Permissão negada para ler o arquivo '%s'\n", src_name);
        return;
    }

    if(fs_find_child(fs_current_dir, dst_name)){
        printf("cp: Não foi possível criar arquivo. Arquivo de destino '%s' ja existe\n", dst_name);
        return;
    }

    // Cria o novo arquivo
    FsNode* dst = fs_create_node(dst_name, NODE_FILE, fs_current_dir);
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

    fs_add_child(fs_current_dir, dst);
}
    
void cmd_mv(int argc, char** argv){
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

    FsNode* node = fs_find_child(fs_current_dir, old_name);
    if(!node){
        printf("mv: Arquivo '%s' nao encontrado\n", old_name);
        return;
    }

    if(fs_find_child(fs_current_dir, new_name)){
        printf("mv: Não foi possível renomear. Arquivo '%s' ja existe\n", new_name);
        return;
    }

    if(node->fcb && !perms_can_write(node->fcb)){
        printf("mv: Permissão negada para renomear o arquivo '%s'\n", old_name);
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
void cmd_rm(int argc, char** argv){
    if(argc < 2){
        printf("Uso: rm <nome_arquivo>\n");
        return;
    }

    const char* file_name = argv[1];

    FsNode* node = fs_find_child(fs_current_dir, file_name);
    if(!node){
        printf("rm: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }

    if(node->type == NODE_DIR){
        printf("rm: '%s' nao e um arquivo\n", file_name);
        return;
    }

    if (node->fcb && !perms_can_write(node->fcb)) {
        printf("rm: Permissao negada para excluir '%s'\n", file_name);
        return;
    }

    // Remove o nó do diretório atual
    fs_remove_child(fs_current_dir, node);
}

void cmd_whoami(){
    const char* name = "unknown";

    switch (fs_current_user_class){
        case USER_OWNER:
            name = "owner";
            break;
        case USER_GROUP:
            name = "group";
            break;
        case USER_OTHER:
            name = "other";
            break;
        default:
            break;
    }

    printf("%s\n", name);    
}

void cmd_user(int argc, char** argv){
    if (argc < 2){
        printf("Uso: user <owner|group|other>\n");
        return;
    }

    const char* role = argv[1];

    if (strcmp(role, "owner") == 0){
        fs_current_user_class = USER_OWNER;
    } else if (strcmp(role, "group") == 0){
        fs_current_user_class = USER_GROUP;
    } else if (strcmp(role, "other") == 0){
        fs_current_user_class = USER_OTHER;
    } else {
        printf("user: Usuario desconhecido '%s'\n", role);
    }
}

void cmd_chmod(int argc, char** argv){
    if (argc < 3){
        printf("Uso: chmod <perms> <file>\n");
        return;
    }

    const char* perm_text = argv[1];
    const char* file_name = argv[2];

    int ok = 0;

    unsigned int perms = perms_parse_numeric(perm_text, &ok);

    if(!ok){
        printf("chmod: Permissoes invalidas: '%s'\n", perm_text);
        return;
    }

    FsNode* node = fs_find_child(fs_current_dir, file_name);
    if(!node){
        printf("chmod: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }

    if(node->type == NODE_DIR){
        printf("chmod: '%s' nao e um arquivo\n", file_name);
        return;
    }

    if(!node->fcb){
        printf("chmod: Arquivo '%s' nao possui FCB\n", file_name);
        return;
    }

    node->fcb->permissions = perms;

    char perm_str[10];
    perms_to_string(perms, perm_str, sizeof(perm_str));
    printf("Permissoes de '%s' alteradas para %s \n", file_name, perm_str);
}

void cmd_stat(int argc, char** argv){
    if(argc < 2){
        printf("Uso: stat <nome_arquivo>\n");
        return;
    }

    const char* file_name = argv[1];

    FsNode* node = fs_find_child(fs_current_dir, file_name);
    if(!node){
        printf("stat: Arquivo '%s' nao encontrado\n", file_name);
        return;
    }
    if (node->type == NODE_DIR){
        printf("stat: '%s' nao e um arquivo\n", file_name);
        return;
    }

    if(!node->fcb){
        printf("stat: Arquivo '%s' nao possui FCB\n", file_name);
        return;
    }

    FCB* fcb = node->fcb;

    char perms[10];
    perms_to_string(fcb->permissions, perms, sizeof(perms));

    const char* owner_name = "unknown";
    switch (fcb->owner){
        case USER_OWNER: owner_name = "owner"; break;
        case USER_GROUP: owner_name = "group"; break;
        case USER_OTHER: owner_name = "other"; break;
    }

    printf("  Estatisticas de '%s':\n", file_name);
    printf("  Tamanho: %zu bytes\n", fcb->size);
    printf("  Permissoes: %s\n", perms);
    printf("  Proprietario: %s\n", owner_name);
    printf("  Inode: %d\n", fcb->inode);
    printf("  Criado em: %s", ctime(&fcb->created_at));
    printf("  Modificado em: %s", ctime(&fcb->modified_at));
    printf("  Ultimo acesso em: %s", ctime(&fcb->accessed_at));
    printf("  Blocos alocados (%d): ", fcb->block_count);

    blocks_dump_file(fcb);
}
