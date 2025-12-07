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