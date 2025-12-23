# Sistema de Backup Remoto Cliente-Servidor via Sockets

## 📌 Descrição
Este projeto consiste na implementação de um **Sistema de Backup Remoto Cliente-Servidor**, desenvolvido como trabalho final da disciplina de Redes. O sistema permite que clientes realizem operações de **criação/atualização de backup** e **restauração de arquivos** armazenados em um servidor remoto, utilizando exclusivamente **comunicação via sockets**.

A comunicação entre cliente e servidor ocorre por meio de **sockets TCP**, com **criptografia na comunicação** e **autenticação de usuários**, garantindo a confidencialidade e a segurança dos dados transmitidos.

---

## 🎯 Objetivos
- Implementar comunicação cliente-servidor utilizando sockets
- Permitir operações de backup e restauração de arquivos
- Aplicar autenticação de usuários
- Garantir criptografia na comunicação
- Consolidar conceitos fundamentais de programação em redes

---

## 🧱 Arquitetura
O sistema é dividido em três principais módulos:

- **Cliente**: responsável por autenticar o usuário e solicitar operações de backup ou restauração.
- **Servidor**: gerencia usuários, arquivos armazenados e controle de cotas.
- **Common**: contém definições do protocolo e funções compartilhadas, como criptografia.

---

## 🔐 Segurança
- Autenticação obrigatória antes de qualquer operação
- Comunicação criptografada (TLS)
- Senhas armazenadas de forma segura (hash)

---

## 📡 Protocolo de Comunicação
O protocolo de aplicação define mensagens de autenticação, backup e restauração, com envio de arquivos em blocos de dados. A documentação detalhada do protocolo encontra-se em `docs/protocolo.md`.

---

## 🧪 Funcionalidades
- Criar/atualizar backup
- Restaurar backup
- Autenticação de usuários
- Controle de cota por usuário
- (Opcional) Descoberta automática do servidor via broadcast UDP

---

## 🛠️ Tecnologias Utilizadas
- Linguagem C
- Sockets TCP e UDP
- OpenSSL (TLS)
- Sistema Linux

---

## 👨‍🎓 Autor
- Nome do Aluno

