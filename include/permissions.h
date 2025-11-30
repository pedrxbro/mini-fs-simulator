#ifndef PERMISSIONS_H
#define PERMISSIONS_H

#include <stddef.h>
#include "fs.h"

#define PERM_READ  0x4
#define PERM_WRITE 0x2
#define PERM_EXEC  0x1

// Converte uma string numerica em mascara de permissoes 
unsigned int perms_parse_numeric(const char* text, int* ok);

// Converte mascara de permissoes para string estilo "rw-r--r--"
void perms_to_string(unsigned int perms, char* buffer, size_t size);

// Verifica permissoes de acesso para o usuario atual
int perms_can_read (const FCB* fcb);
int perms_can_write(const FCB* fcb);
int perms_can_exec (const FCB* fcb);

#endif
