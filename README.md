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
