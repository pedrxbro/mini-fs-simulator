# Mini File System Simulator (Mini-FS)

Trabalho avaliativo da disciplina de **Sistemas Operacionais**, implementando um mini sistema de arquivos em memória com shell própria, FCBs, permissões (RWX) e simulação de blocos de disco.

---

## 1. Ambiente de execução

### 1.1. Sistema operacional

O projeto foi desenvolvido e testado em:

- **Windows 10/11** utilizando o **WSL2 (Windows Subsystem for Linux)** com distribuição **Ubuntu**.

> Também funciona em qualquer Linux com GCC, `make` e ferramentas básicas de desenvolvimento instaladas.

### 1.2. Pré-requisito: WSL instalado (para usuários Windows)

No Windows 10/11, abra o **PowerShell como Administrador** e execute:

```powershell
wsl --install
```
Isso irá instalar o WSL, prossiga com as instruções de instalação.

### 1.3 - Clonar o repositório
Dentro do WSL, entre em uma pasta e clone o repositório:

```bash
cd /mnt/c #Para acessar o Disco Local C do Windows
mkdir Projetos
cd Projetos
```
Clonar o repositório:
```bash
git clone https://github.com/pedrxbro/mini-fs-simulator.git
cd mini-fs-simulator
```

### 1.4 - Preparar o WSL
Dentro da pasta do projeto, atualize os pacotes da distro e instale as ferramentas de compilação

- Atualizar lista de pacotes
```bash
sudo apt update
sudo apt upgrade -y
```
- Instalar compiladores e ferramentas de build
``` bash
sudo apt install -y build-essential make gcc
```

### 1.5 - Compilar o projeto
Na raíz, execute:
```bash
make clean
make all
```
Se tudo estiver correto, o comando *make* irá gerar um binário chamado:
```bash
mini_fs 
```

### 1.6 - Executar o projeto
Ainda na raíz, execute o binário, pelo comando:
```bash
./mini_fs
```
Terá um prompt como:
```bash
Inicializando sistema de arquivos...
/$
``` 
A partir daqui, é possível utilizar os comandos implementados no simulador.

Para sair:
```bash
/$ exit
Desligando sistema de arquivos
```

---

## 2. Design do Sistema e Estrutura de Dados

A implementação busca refletir, de forma didática, como um sistema de arquivos real poderia ser organizado internamente.

O simulador foi projetado seguindo princípios de **código limpo**, **modularização** e **separação de responsabilidades**, facilitando a leitura, manutenção e evolução do sistema.

### 2.1. Organização modular do código

O código-fonte está organizado em módulos independentes, cada um responsável por uma parte específica do sistema:

```text
src/
├── cmd/         # Implementação dos comandos da shell
├── helpers/     # Funções auxiliares (FS, FCB, permissões, blocos)
├── init/        # Inicialização e encerramento do sistema
├── shell/       # Loop da shell e parser de comandos
├── fs.c         # Estado global do sistema de arquivos
└── main.c       # Ponto de entrada da aplicação
```

Os arquivos de cabeçalho estão em 'include/'.
- Baixo acoplamento entre módulos
- Alta coesão de responsabilidades
- Facilidade de testes e refatorações
- Melhor compreensão do fluxo do sistema

---

### 2.2 - Estrutura de diretório em árvore

O sistema de arquivos é representado por uma **árvore de diretórios**, onde cada nó pode representar um diretório ou um arquivo.

Cada nó da árvore é representado pela estrutura `FsNode`:

```c
typedef struct FsNode {
    char name[MAX_NAME_LEN];
    NodeType type;

    struct FsNode* parent;
    struct FsNode* first_child;
    struct FsNode* next_sibling;

    FCB* fcb;
} FsNode;
```

- **parent**: aponta para o diretório pai
- **first_child**: aponta para o primeiro filho (em caso de diretório)
- **next_sibling**: aponta para o próximo irmão
- **fcb**: ponteiro para o File Control Block (apenas para arquivos)

### 2.3 - Conceito de arquivo e File Control Blcok (FCB)
Cada arquivo do sistema é representado por um File Control Block (FCB), responsável por armazenar seus metadados.

A estrutura `FCB` é definida da seguinte forma:

```c
typedef struct FCB {
    char name[MAX_NAME_LEN];
    size_t size;
    FileType type;

    time_t created_at;
    time_t modified_at;
    time_t accessed_at;

    int inode;
    unsigned int permissions;
    UserClass owner;

    int blocks[FCB_MAX_BLOCKS];
    int block_count;

    char* content;
} FCB;
```
Informações armazenadas no FCB:

- Nome e tamanho do arquivo
- Tipo do arquivo
- Datas de criação, modificação e último acesso
- Inode simulado (identificador único do arquivo)
- Permissões de acesso
- Proprietário do arquivo
- Lista de blocos alocados no disco
- Conteúdo do arquivo em memória

### 2.4 - Uso de ponteiros e alocação dinâmica

A implementação faz uso extensivo de ponteiros e alocação dinâmica de memória (malloc e free) para:

- Criar nós do sistema de arquivos (FsNode)
- Criar e destruir FCBs
- Gerenciar o conteúdo dos arquivos
- Simular a alocação e liberação de blocos de disco
- A liberação de memória é realizada de forma recursiva ao desligar o sistema ou remover arquivos, garantindo que não haja vazamentos de memória durante a execução do simulador.

---

## 3. Operações com Arquivos e Conceitos Teóricos

Esta seção descreve como o simulador implementa, na prática, os principais conceitos teóricos estudados na disciplina de Sistemas Operacionais relacionados à gerência de arquivos.

---

### 3.1 - Conceito de arquivo e seus atributos

No simulador, um arquivo é uma entidade lógica composta por:

- Conteúdo
- Metadados
- Permissões
- Associação a blocos de disco

Os atributos de um arquivo são armazenados no **File Control Block (FCB)**, que funciona como uma estrutura centralizadora de informações, de forma equivalente ao inode em sistemas Unix-like.

Entre os atributos simulados destacam-se:

- Nome
- Tamanho
- Tipo
- Datas de criação, modificação e acesso
- Proprietário
- Permissões de acesso
- Blocos de disco associados

Esses atributos são acessados e manipulados pelos comandos do sistema, garantindo consistência e controle sobre os arquivos.

---

### 3.2 - Operações básicas com arquivos

O simulador implementa as principais operações de arquivos:

#### Criar arquivos
- **Comando:** `touch <arquivo>`
- Cria um novo arquivo no diretório atual
- Um novo FCB é alocado e inicializado
- Caso o arquivo já exista, apenas os timestamps são atualizados

#### Escrever em arquivos
- **Comando:** `write <arquivo> <texto>`
- Cria ou sobrescreve o conteúdo de um arquivo
- Atualiza:
  - Conteúdo
  - Tamanho
  - Timestamp de modificação
- Aciona a alocação de blocos de disco

#### Ler arquivos
- **Comando:** `cat <arquivo>`
- Exibe o conteúdo do arquivo
- Atualiza o timestamp de acesso
- Valida permissões de leitura antes da operação

#### Copiar arquivos
- **Comando:** `cp <origem> <destino>`
- Cria uma cópia do arquivo, incluindo:
  - Conteúdo
  - Metadados relevantes
  - Blocos de disco próprios
- Demonstra o consumo adicional de espaço em disco

#### Renomear/Mover arquivos
- **Comando:** `mv <origem> <destino>`
- Modifica apenas o nome do arquivo
- Move somente o endereço
- Mantém inode, permissões e blocos associados

#### Remover arquivos
- **Comando:** `rm <arquivo>`
- Remove o nó do sistema de arquivos
- Libera o FCB associado
- Libera os blocos de disco alocados

---

### 3.3 - Estrutura de diretórios e navegação

O simulador utiliza uma estrutura de diretórios hierárquica em forma de árvore.

#### Diretórios
- São nós que podem conter outros arquivos ou diretórios
- Não possuem conteúdo nem FCB

#### Comandos relacionados
- `mkdir <dir>` → cria um diretório
- `cd <dir>` → navegação entre diretórios
- `pwd` → exibe o caminho absoluto
- `ls` / `ls -l` → lista conteúdo do diretório

A estrutura em árvore permite:
- Organização lógica do sistema
- Navegação eficiente
- Agrupamento de arquivos relacionados

---

### 3.4 - Representação do inode simulado

Cada arquivo possui um campo `inode` dentro do seu FCB.

- O inode é um identificador inteiro único
- É atribuído automaticamente no momento da criação do arquivo
- Permanece constante durante a vida do arquivo

Essa abordagem simula o comportamento real de sistemas de arquivos Unix, onde o inode identifica o arquivo independentemente do nome ou do diretório em que se encontra.

---

### 3.5 - Relação com os conceitos teóricos da disciplina

As funcionalidades implementadas demonstram, de forma prática:

- O conceito de arquivo como uma abstração do sistema operacional
- A separação entre nome do arquivo e seus metadados
- O uso de estruturas de dados para gerência de arquivos
- A importância do controle de acesso e proteção
- A gerência de espaço em disco

---

## 4. Mecanismo de Proteção de Acesso e Permissões

O simulador implementa um mecanismo de proteção de acesso baseado em **permissões RWX**, de forma semelhante aos sistemas operacionais Unix-like.

O objetivo é demonstrar, de forma prática, como o sistema operacional controla quem pode acessar, modificar ou executar um arquivo.

---

### 4.1. Classes de usuários

O sistema define três classes de usuários:

- **Owner (proprietário)**: usuário que criou o arquivo
- **Group (grupo)**: usuários pertencentes ao mesmo grupo lógico
- **Other (outros)**: quaisquer outros usuários

O usuário atual do sistema pode ser alterado por meio do comando:

```text
user owner | group | other
```
Para exibir o usuário atual:
```bash
whoami
```

---

### 4.2 Modelo de Permissões RWX
Cada arquivo possui permissões no formato RWX:
- **R** (Read) – Permissão para leitura
- **W** (Write) – Permissão para escrita
- **X** (Execute) – Permissão para execução

As permissões são organizadas em três grupos:
```text
Other | Group | Other
```
Exemplo:
```text
rw-r----- > 640
```

---
4.3 -  Representação interna das permissões (bitmask)

Internamente, as permissões são armazenadas como um valor numérico (bitmask), utilizando três bits para cada classe de usuário:
```text
R = 4
W = 2
X = 1
```
Exemplo:
```text
rw-r--r-- > 644
```
Esse valor é manipulado no código usando operações bitwise, permitindo verificar permissões de forma eficiente.

---

### 4.3 -  Implementação do comando chmod:
O comando `chmod` permite alterar as permissões de um arquivo:
```bash
chmod <modo> <arquivo>
```
Exemplo:
```bash
chmod 640 relatorio.txt
```
O simulador:
- Converte o modo numérico em bits
- Atualiza o campo `permissions` do FCB
- Passa a utilizar as novas permissões nas operações subsequentes

---

### 4.5 - Verificação de permissões durante operações

Antes de executar operações sensíveis, o sistema verifica se o usuário atual possui permissão suficiente:

- Leitura (`cat`) → exige permissão R
- Escrita (`write`, `cp`, `mv`, `rm`) → exige permissão W

A verificação considera:

1. Se o usuário atual é o proprietário do arquivo → utiliza permissões de *owner*
2. Caso contrário, se pertence ao grupo → utiliza permissões de *group*
3. Caso contrário → utiliza permissões de *other*
Essa lógica garante um controle de acesso consistente e alinhado com o modelo Unix.