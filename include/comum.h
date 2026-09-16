#ifndef COMUM_H
#define COMUM_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Comprimento máximo de um EAN (13 dígitos). */
#define EAN_MAX 13

/* Tamanho máximo da descrição em bytes, incluindo '\0'. */
#define MAX_DESC 51

/* Número máximo de produtos no sistema. */
#define MAX_PRODUTOS 10000

/* Comprimento máximo de uma linha de input. */
#define MAX_LINHA 65535

/* NIF usado quando o cliente não indica o seu. */
#define NIF_PADRAO 999999999

/* Nome usado quando o cliente não indica o seu. */
#define NOME "Cliente final"

/* Número de entradas possíveis na tabela de IVA (A a Z). */
#define MAX_IVA 26

/* Valor que indica que um código de IVA não está definido. */
#define IVA_INDEFINIDO -1

/* Produto registado no sistema. */
typedef struct {
    char ean[EAN_MAX + 1]; /* Código EAN (8 ou 13 dígitos). */
    char desc[MAX_DESC];   /* Descrição do produto. */
    char iva;              /* Letra do código de IVA. */
    double preco;          /* Preço unitário sem IVA. */
    int stock;             /* Unidades disponíveis em armazém. */
    int vendido;           /* Unidades saídas (no cesto ou facturadas). */
} Produto;

#endif