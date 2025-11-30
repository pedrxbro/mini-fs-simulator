#ifndef FCB_HELPERS_H
#define FCB_HELPERS_H

#include "fs.h"

// Cria um novo FCB com valores padrão
FCB* create_fcb(const char* name, FileType type);

// Libera memória de um FCB (incluindo conteúdo)
void free_fcb(FCB* fcb);

#endif
