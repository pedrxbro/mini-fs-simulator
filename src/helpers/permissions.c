#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "permissions.h"


// Converte dígitos individuais em máscara de permissões
static unsigned int perms_from_digits(int owner, int group, int other){

    // Move os bits para suas posições corretas
    unsigned int mask = 0;
    mask  |= (owner & 0x7) << 6; // bits 6-8
    mask  |= (group & 0x7) << 3; // bits 3-5
    mask  |= (other & 0x7);      // bits 0-2
    return mask;
}

// Converte string numérica em máscara de permissões
unsigned int perms_parse_numeric(const char* text, int* ok){
    if(ok) *ok = 0; // assume falha inicialmente

    if(!text) return 0;

    size_t len = strlen(text);
    if(len != 3){
        return 0; // Deve ter exatamente 3 dígitos
    }

    // ASCII para inteiro
    int owner = text[0] - '0';
    int group = text[1] - '0';
    int other = text[2] - '0';

    // Valida o intervalo
    if (owner < 0 || owner > 7 ||
        group < 0 || group > 7 ||
        other < 0 || other > 7) {
        return 0;
    }

    if (ok) *ok = 1;

    return perms_from_digits(owner, group, other);  // Constroi a máscara  
}

void perms_to_string(unsigned int perms, char* buffer, size_t size){
    if (!buffer || size < 10) {
        return; // Buffer insuficiente
    }

    char temp[10];


    // Extrai os bits de permissão
    unsigned int owner = (perms >> 6) & 0x7;
    unsigned int group = (perms >> 3) & 0x7;
    unsigned int other = (perms     ) & 0x7;

    
    // Transforma cada bit em caractere r/w/x ou '-'
    temp[0] = (owner & PERM_READ)  ? 'r' : '-';
    temp[1] = (owner & PERM_WRITE) ? 'w' : '-';
    temp[2] = (owner & PERM_EXEC)  ? 'x' : '-';

    temp[3] = (group & PERM_READ)  ? 'r' : '-';
    temp[4] = (group & PERM_WRITE) ? 'w' : '-';
    temp[5] = (group & PERM_EXEC)  ? 'x' : '-';

    temp[6] = (other & PERM_READ)  ? 'r' : '-';
    temp[7] = (other & PERM_WRITE) ? 'w' : '-';
    temp[8] = (other & PERM_EXEC)  ? 'x' : '-';

    temp[9] = '\0';


    strncpy(buffer, temp, size - 1);
    buffer[size - 1] = '\0';
}


// Retorna apenas os bits de permissão relevantes para a classe do usuário
static unsigned int perms_effective_bits(const FCB* fcb){
    unsigned int owner_bits = (fcb->permissions >> 6) & 0x7;
    //unsigned int group_bits = (fcb->permissions >> 3) & 0x7;
    unsigned int other_bits = (fcb->permissions     ) & 0x7;

    // Se for dono, retorna os bits do dono
    if(fs_current_user_class == fcb->owner){
        return owner_bits;
    }

    // Não tratando GRUPO ainda

    // Retorna os bits de outros
    return other_bits;
}

// Verifica permissões de leitura para o usuário atual
int perms_can_read(const FCB* fcb){
    if(!fcb) return 0;
    unsigned int bits = perms_effective_bits(fcb);
    return (bits & PERM_READ) != 0;
}

// Verifica permissões de escrita para o usuário atual
int perms_can_write(const FCB* fcb){
    if(!fcb) return 0;
    unsigned int bits = perms_effective_bits(fcb);
    return (bits & PERM_WRITE) != 0;
}

// Verifica permissões de execução para o usuário atual
int perms_can_exec(const FCB* fcb){
    if(!fcb) return 0;
    unsigned int bits = perms_effective_bits(fcb);
    return (bits & PERM_EXEC) != 0;
}