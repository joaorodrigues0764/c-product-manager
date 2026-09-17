#ifndef BASKET_H
#define BASKET_H

#include "common.h"

/* Node of the cart's linked list. */
typedef struct ItemCesto {
    int idx_produto;        /* Product index in products[]. */
    int quantidade;         /* Quantity of this product in the cart. */
    struct ItemCesto *prox; /* Next item in the list. */
} ItemCesto;

/**
 * Checks if a product is in the cart.
 * @param idx index of the product in products[]
 * @return 1 if it is in the cart, 0 otherwise
 */
int cesto_tem_produto(int idx);

/**
 * Executes command 'a': adds, removes, or lists the cart.
 * @param linha rest of the line after 'a'
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @param iva_tabela VAT table
 */
void comando_a(const char *linha, Produto produtos[], int num_produtos,
    int iva_tabela[]);

/*
Frees all memory from the cart list.
 */
void cesto_libertar(void);

/**
 * Returns a pointer to the first item in the cart.
 * Used by invoices.c to iterate through the items.
 * @return pointer to the head of the list, or NULL if empty
 */
ItemCesto *cesto_obter_lista(void);

#endif
