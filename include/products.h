#ifndef PRODUCTS_H
#define PRODUCTS_H

#include "common.h"

/**
 * Fills the VAT table with default values (A=0, B=6, C=13, D=23).
 * @param iva_tabela table indexed by letter (0='A')
 */
void iva_init_default(int iva_tabela[]);

/**
 * Loads the VAT table from a text file.
 * Each line of the file has the format: <LETTER> <percentage>
 * @param ficheiro path to the file
 * @param iva_tabela table to fill
 */
void iva_carregar(const char *ficheiro, int iva_tabela[]);

/**
 * Checks if an EAN-8 or EAN-13 code is valid.
 * Confirms length, digits, and check digit.
 * @param ean string with the EAN code
 * @return 1 if valid, 0 otherwise
 */
int validacao_ean(const char *ean);

/**
 * Executes command 'p': adds or updates a product.
 * @param linha rest of the line after 'p'
 * @param produtos array of products
 * @param num_produtos pointer to the product counter
 * @param iva_tabela VAT table
 * @param cesto_tem_produto function that checks if the product is in the cart
 */
void comando_p(const char *linha, Produto produtos[],
    int *num_produtos, int iva_tabela[],
    int cesto_tem_produto(int));

/**
 * Executes command 'l': lists products with stock > 0.
 * Supports wildcards (* and ?) in EAN codes.
 * @param linha rest of the line after 'l'
 * @param produtos array of products
 * @param num_produtos number of registered products
 */
void comando_l(const char *linha, Produto produtos[], int num_produtos);

/**
 * Executes EAN mode of command 'd': reduces or removes a product.
 * @param ean EAN code of the product to reduce
 * @param quantidade units to remove from stock
 * @param produtos array of products
 * @param num_produtos pointer to the product counter
 * @param cesto_tem_produto function that checks if the product is in the cart
 */
void comando_d_produto(const char *ean, int quantidade,
    Produto produtos[], int *num_produtos,
    int cesto_tem_produto(int));

#endif
