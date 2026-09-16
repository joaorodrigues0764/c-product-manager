#ifndef PRODUTOS_H
#define PRODUTOS_H

#include "comum.h"

/**
 * Preenche a tabela de IVA com os valores por omissão (A=0, B=6, C=13, D=23).
 * @param iva_tabela tabela indexada por letra (0='A')
 */
void iva_init_default(int iva_tabela[]);

/**
 * Carrega a tabela de IVA a partir de um ficheiro de texto.
 * Cada linha do ficheiro tem o formato: <LETRA> <percentagem>
 * @param ficheiro caminho para o ficheiro
 * @param iva_tabela tabela a preencher
 */
void iva_carregar(const char *ficheiro, int iva_tabela[]);

/**
 * Verifica se um código EAN-8 ou EAN-13 é válido.
 * Confirma comprimento, dígitos e dígito de verificação.
 * @param ean string com o código EAN
 * @return 1 se válido, 0 caso contrário
 */
int validacao_ian(const char *ean);

/**
 * Executa o comando p: adiciona ou actualiza um produto.
 * @param linha resto da linha após o 'p'
 * @param produtos array de produtos
 * @param num_produtos ponteiro para o contador de produtos
 * @param iva_tabela tabela de IVA
 * @param cesto_tem_produto função que verifica se o produto está no cesto
 */
void comando_p(const char *linha, Produto produtos[],
    int *num_produtos, int iva_tabela[],
    int cesto_tem_produto(int));

/**
 * Executa o comando l: lista produtos com stock > 0.
 * Suporta wildcards (* e ?) nos códigos EAN.
 * @param linha resto da linha após o 'l'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 */
void comando_l(const char *linha, Produto produtos[], int num_produtos);

/**
 * Executa o modo EAN do comando d: reduz ou remove um produto.
 * @param ean código EAN do produto a reduzir
 * @param quantidade unidades a retirar do stock
 * @param produtos array de produtos
 * @param num_produtos ponteiro para o contador de produtos
 * @param cesto_tem_produto função que verifica se o produto está no cesto
 */
void comando_d_produto(const char *ean, int quantidade,
    Produto produtos[], int *num_produtos,
    int cesto_tem_produto(int));

#endif