#ifndef CESTO_H
#define CESTO_H

#include "comum.h"

/* Nó da lista ligada do cesto. */
typedef struct ItemCesto {
    int idx_produto;        /* Índice do produto em produtos[]. */
    int quantidade;         /* Quantidade deste produto no cesto. */
    struct ItemCesto *prox; /* Próximo item da lista. */
} ItemCesto;

/**
 * Verifica se um produto está no cesto.
 * @param idx índice do produto em produtos[]
 * @return 1 se está no cesto, 0 caso contrário
 */
int cesto_tem_produto(int idx);

/**
 * Executa o comando a: adiciona, remove ou lista o cesto.
 * @param linha resto da linha após o 'a'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @param iva_tabela tabela de IVA
 */
void comando_a(const char *linha, Produto produtos[], int num_produtos,
    int iva_tabela[]);

/*
Liberta toda a memória da lista do cesto.
 */
void cesto_libertar(void);

/**
 * Devolve o ponteiro para o primeiro item do cesto.
 * Usado por faturas.c para iterar os itens.
 * @return ponteiro para a cabeça da lista, ou NULL se vazio
 */
ItemCesto *cesto_obter_lista(void);

#endif