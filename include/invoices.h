#ifndef INVOICES_H
#define INVOICES_H

#include "common.h"

/* An item inside an invoice (product + purchased quantity). */
typedef struct ItemFatura {
    int idx_produto;         /* Product index in products[]. */
    int quantidade;          /* Purchased quantity. */
    double preco_unit;       /* Unit price at the time of purchase. */
    char iva;                /* VAT code at the time of purchase. */
    struct ItemFatura *prox; /* Next item in the list. */
} ItemFatura;

/* Issued invoice, with customer data and list of items. */
typedef struct Fatura {
    int numero;          /* Sequential invoice number (starts at 1). */
    int nif;             /* Customer NIF (Tax ID). */
    char *nome_cliente;  /* Customer name (dynamically allocated). */
    int num_itens;       /* Total purchased units. */
    double valor_pago;   /* Total amount paid with VAT. */
    ItemFatura *itens;   /* Invoice item list. */
    struct Fatura *prox; /* Next invoice in the list. */
} Fatura;

/**
 * Executes command 'r': billing summary.
 * No args: global totals and VAT table.
 * With EAN: product stock and sold quantity.
 * @param linha rest of the line after 'r'
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @param iva_tabela VAT table
 */
void comando_r(const char *linha, Produto produtos[], int num_produtos,
    int iva_tabela[]);

/**
 * Executes command 'f': invoices the cart's content.
 * @param linha rest of the line after 'f'
 * @param produtos array of products
 * @param iva_tabela VAT table
 */
void comando_f(const char *linha, Produto produtos[], int iva_tabela[]);

/**
 * Executes command 'c': lists invoices by customer.
 * No args: all invoices in alphabetical order by customer.
 * With name: only invoices for that customer.
 * @param linha rest of the line after 'c'
 */
void comando_c(const char *linha);

/**
 * Checks if an invoice with the specified number exists.
 * @param numero invoice number to search for
 * @return 1 if it exists, 0 otherwise
 */
int procura_fatura_existe(int numero);

/**
 * Executes invoice mode of command 'd': removes the specified invoice.
 * @param numero number of the invoice to remove
 */
void comando_d_fatura(int numero);

void comando_v(const char *linha);

/*
Frees all memory allocated by the invoices.
 */
void faturas_libertar(void);

#endif
