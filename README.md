# 🔐 Sistema de Backup Remoto Seguro via Sockets (C)

Este projeto implementa um **sistema de backup remoto cliente-servidor**, desenvolvido em **linguagem C**, utilizando **sockets**, **TLS (OpenSSL)** e **controle de cotas por usuário**.  
O objetivo é demonstrar conceitos fundamentais de **redes de computadores**, **segurança**, **comunicação criptografada** e **gerenciamento de arquivos**.

---

## 📌 Funcionalidades

- 🔐 Comunicação segura com **TLS (OpenSSL)**
- 🔑 Autenticação de usuários
- 📦 Backup remoto de arquivos
- 📂 Restore (recuperação) de arquivos
- 📃 Listagem de arquivos disponíveis para restore
- 🧮 Controle de cota de armazenamento por usuário
- 🌐 Descoberta automática do servidor via **UDP**
- 🗂️ Organização de arquivos por usuário no servidor

---

## 🏗️ Arquitetura do Sistema

O sistema é dividido em dois módulos principais:

### 🔹 Cliente
- Descoberta automática do servidor via UDP
- Conexão segura via TLS
- Autenticação do usuário
- Execução das operações:
  - `backup <arquivo>`
  - `restore <arquivo>`
  - `list`

### 🔹 Servidor
- Escuta conexões TCP com TLS
- Responde a broadcasts UDP
- Autentica usuários
- Gerencia armazenamento por usuário
- Aplica regras de cota antes de permitir backup

---

## 🔐 Segurança

Toda a comunicação entre cliente e servidor é **criptografada utilizando TLS**, por meio da biblioteca **OpenSSL**.

- Usuário e senha não trafegam em texto puro
- Arquivos são enviados de forma criptografada
- Cada sessão TLS é associada a um usuário autenticado

> Mesmo que o tráfego seja interceptado, o conteúdo permanece protegido.

---

## 🧮 Controle de Cotas

O servidor utiliza um arquivo `quotas.txt` para definir o limite de armazenamento por usuário.

### 📄 Formato do arquivo `quotas.txt`
```txt
admin:5000000
Gleydson:10000000
