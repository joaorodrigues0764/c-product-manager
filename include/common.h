#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* Maximum length of an EAN (13 digits). */
#define EAN_MAX 13

/* Maximum description size in bytes, including '\0'. */
#define MAX_DESC 51

/* Maximum number of products in the system. */
#define MAX_PRODUTOS 10000

/* Maximum length of an input line. */
#define MAX_LINHA 65535

/* NIF used when the client does not provide one. */
#define NIF_PADRAO 999999999

/* Name used when the client does not provide one. */
#define NOME "Cliente final"

/* Number of possible entries in the VAT table (A to Z). */
#define MAX_IVA 26

/* Value indicating that a VAT code is not defined. */
#define IVA_INDEFINIDO -1

/* Product registered in the system. */
typedef struct {
    char ean[EAN_MAX + 1]; /* EAN code (8 or 13 digits). */
    char desc[MAX_DESC];   /* Product description. */
    char iva;              /* VAT code letter. */
    double preco;          /* Unit price without VAT. */
    int stock;             /* Available units in stock. */
    int vendido;           /* Units gone (in cart or invoiced). */
} Produto;

#endif
