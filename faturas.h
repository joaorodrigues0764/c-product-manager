#ifndef FATURAS_H
#define FATURAS_H

#include "comum.h"

/* Um item dentro de uma factura (produto + quantidade comprada). */
typedef struct ItemFatura {
    int idx_produto;         /* Índice do produto em produtos[]. */
    int quantidade;          /* Quantidade comprada. */
    double preco_unit;       /* Preço unitário no momento da compra. */
    char iva;                /* Código de IVA no momento da compra. */
    struct ItemFatura *prox; /* Próximo item da lista. */
} ItemFatura;

/* Factura emitida, com dados do cliente e lista de itens. */
typedef struct Fatura {
    int numero;          /* Número sequencial da factura (começa em 1). */
    int nif;             /* NIF do cliente. */
    char *nome_cliente;  /* Nome do cliente (alocado dinamicamente). */
    int num_itens;       /* Total de unidades compradas. */
    double valor_pago;   /* Valor total pago com IVA. */
    ItemFatura *itens;   /* Lista de itens da factura. */
    struct Fatura *prox; /* Próxima factura na lista. */
} Fatura;

/**
 * Executa o comando r: resumo de facturação.
 * Sem args: totais globais e tabela de IVA.
 * Com EAN: stock e vendido do produto.
 * @param linha resto da linha após o 'r'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @param iva_tabela tabela de IVA
 */
void comando_r(const char *linha, Produto produtos[], int num_produtos,
    int iva_tabela[]);

/**
 * Executa o comando f: factura o conteúdo do cesto.
 * @param linha resto da linha após o 'f'
 * @param produtos array de produtos
 * @param iva_tabela tabela de IVA
 */
void comando_f(const char *linha, Produto produtos[], int iva_tabela[]);

/**
 * Executa o comando c: lista facturas por cliente.
 * Sem args: todas as facturas por ordem alfabética de cliente.
 * Com nome: só as facturas desse cliente.
 * @param linha resto da linha após o 'c'
 */
void comando_c(const char *linha);

/**
 * Verifica se existe uma factura com o número indicado.
 * @param numero número da factura a procurar
 * @return 1 se existe, 0 caso contrário
 */
int procura_fatura_existe(int numero);

/**
 * Executa o modo factura do comando d: remove a factura indicada.
 * @param numero número da factura a remover
 */
void comando_d_fatura(int numero);

void comando_v(const char *linha);

/*
Liberta toda a memória alocada pelas facturas.
 */
void faturas_libertar(void);

#endif