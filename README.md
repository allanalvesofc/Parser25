# 🛠 Processador e Filtro de Credenciais

## 🇧🇷 Descrição em Português

### Visão Geral
Este software é uma ferramenta para **processamento e filtragem de arquivos contendo credenciais** no formato `url:login:senha`.  
Ele remove caracteres indesejados, ignora URLs suspeitas ou internas, e exporta os dados limpos para arquivos `.txt` formatados de forma padronizada.  
Foi desenvolvido em C utilizando APIs nativas do Windows para garantir alta performance, mesmo com múltiplos arquivos, graças ao processamento **multithread**.

---

### Principais Funcionalidades
- **Seleção de Arquivos Múltiplos**  
  Interface do Windows para seleção de um ou mais arquivos para processamento.
  
- **Remoção de Caracteres Proibidos**  
  Elimina caracteres como `'`, `"`, `%`, `{}`, `[]`, `^`, `~`, `#`, `&`, `$` e `\`, além de quebras de linha extras.

- **Filtragem de URLs**  
  Ignora entradas contendo:
  - Endereços de rede interna (`192.168.*`, `127.0.0.1`, `localhost`)
  - Domínios específicos (`.vn/`, `.cz/`, `.uk/`, `.cn/`)
  - Strings relacionadas a Android

- **Tratamento de Formato**  
  - Substitui espaços por `/`
  - Remove prefixos `http://` e `https://`
  - Garante que cada linha siga o padrão `'url', 'login', 'senha'`

- **Limites de Segurança**  
  Desconsidera entradas com:
  - URL > 150 caracteres
  - Login > 100 caracteres
  - Senha > 100 caracteres

- **Processamento Paralelo**  
  Cada arquivo selecionado é processado em uma **thread separada** para maior velocidade.

- **Saída Padronizada**  
  Cria um arquivo `.txt` para cada arquivo processado, com conteúdo em UTF-8 no formato:
  ```
  ('url', 'login', 'senha');
  ```

---

### Fluxo de Funcionamento
1. Usuário seleciona um ou mais arquivos no explorador de arquivos.
2. Cada arquivo é aberto e lido linha a linha.
3. Cada linha é processada:
   - Filtra caracteres proibidos
   - Remove protocolos e espaços
   - Valida tamanho dos campos
   - Ignora URLs na blacklist
4. Os dados válidos são gravados no arquivo `.txt` correspondente.
5. Ao final, o console informa que todos os arquivos foram processados.

---

### Requisitos
- **Sistema Operacional:** Windows (qualquer versão compatível com WinAPI)
- **Compilador:** MSVC (Microsoft Visual C++) ou compatível
- **Bibliotecas:**  
  - `windows.h`
  - `shlwapi.h`
  - `commdlg.h`

---

### Possíveis Usos
- Padronizar dados exportados de sistemas que salvam credenciais.
- Limpar e filtrar grandes volumes de informações antes de importar para bancos de dados.
- Processar dumps de credenciais para análise (em ambientes autorizados).

---

## 🇺🇸 Description in English

### Overview
This software is a **file processing and credential filtering tool** for text data in the format `url:login:password`.  
It removes unwanted characters, ignores suspicious or local URLs, and exports clean, standardized `.txt` files.  
It is built in C using native Windows APIs for maximum performance, with **multithreaded** processing for multiple files.

---

### Main Features
- **Multi-File Selection**  
  Windows dialog interface for selecting one or more files.
  
- **Blacklist Character Removal**  
  Removes characters such as `'`, `"`, `%`, `{}`, `[]`, `^`, `~`, `#`, `&`, `$` and `\`, plus extra line breaks.

- **URL Filtering**  
  Ignores entries containing:
  - Internal network addresses (`192.168.*`, `127.0.0.1`, `localhost`)
  - Certain domains (`.vn/`, `.cz/`, `.uk/`, `.cn/`)
  - Strings related to Android

- **Format Handling**  
  - Replaces spaces with `/`
  - Removes `http://` and `https://` prefixes
  - Ensures each line matches the `'url', 'login', 'password'` pattern

- **Safety Limits**  
  Skips entries with:
  - URL length > 150 characters
  - Login length > 100 characters
  - Password length > 100 characters

- **Parallel Processing**  
  Each selected file is processed in a **separate thread** for faster performance.

- **Standardized Output**  
  Generates a `.txt` file for each processed file, with UTF-8 formatted lines:
  ```
  ('url', 'login', 'password');
  ```

---

### Workflow
1. User selects one or more files in the Windows file dialog.
2. Each file is opened and read line by line.
3. Each line is processed:
   - Removes blacklisted characters
   - Strips protocols and spaces
   - Validates field lengths
   - Skips blacklisted URLs
4. Valid data is written to the corresponding `.txt` file.
5. The console reports that all files have been processed.

---

### Requirements
- **Operating System:** Windows (any version supporting WinAPI)
- **Compiler:** MSVC (Microsoft Visual C++) or compatible
- **Libraries:**  
  - `windows.h`
  - `shlwapi.h`
  - `commdlg.h`

---

### Possible Use Cases
- Standardizing exported credential data from various systems.
- Cleaning and filtering large volumes of information before database import.
- Processing credential dumps for analysis (in authorized environments).
