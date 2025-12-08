#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"


#define PATH_MAX_LEN 1024
#define MAX_TOKENS 32

// Variáveis globais do sistema de arquivos
FsNode* fs_root = NULL;
FsNode* fs_current_dir = NULL;

UserClass fs_current_user_class = USER_OWNER;