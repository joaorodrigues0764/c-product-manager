#include "cesto.h"
#include "produtos.h"

/* Cabeça da lista ligada do cesto. */
static ItemCesto *cesto = NULL;

/**
 * Calcula o preço total de uma linha com IVA, com arredondamento simétrico.
 * O epsilon 1e-9 evita erros de representação IEEE 754.
 * @param preco preço unitário sem IVA
 * @param percentagem_iva taxa de IVA em percentagem
 * @param quantidade número de unidades
 * @return preço total com IVA arredondado aos cêntimos
 */
static double preco_com_iva(double preco, int percentagem_iva,
        int quantidade) {
    double conta = preco * quantidade * (1 + (percentagem_iva/100.0));
    int centimos = (int)(conta * 100.0 + 0.5 + 1e-9);
    return centimos / 100.0;
}

/**
 * Procura um item no cesto pelo índice do produto.
 * @param i índice do produto em produtos[]
 * @return ponteiro para o ItemCesto, ou NULL se não estiver no cesto
 */
static ItemCesto *cesto_procura(int i) {
    ItemCesto *atual = cesto;
    while (atual != NULL) {
        if (atual->idx_produto == i) {
            return atual;
        }
        else {
            atual = atual->prox;
        }
    }
    return NULL;
}


/**
 * Remove um item do cesto e liberta o nó.
 * @param i índice do produto a remover
 */
static void cesto_remover(int i) {
    ItemCesto *atual = cesto;
    ItemCesto *anterior = NULL;

    while (atual != NULL) {
        if (atual->idx_produto == i) {
            if (anterior == NULL) {
                cesto = atual->prox;
            }
            else {
                anterior->prox = atual->prox;
            }
            free(atual);
            return;
        }
        anterior = atual;
        atual = atual->prox;
    }
}

/**
 * Imprime um item do cesto no formato do comando a:
 * <iva> <preço-unitário> <quantidade> <preço-total-com-iva> <descrição>
 * @param item item a imprimir
 * @param produtos array de produtos
 * @param iva_tabela tabela de IVA
 */
static void imprimir_item(const ItemCesto *item, Produto produtos[],
        int iva_tabela[]) {
    int i = item->idx_produto;

    char tipo_iva = produtos[i].iva;
    double preco_base = produtos[i].preco;
    char *nome_produto = produtos[i].desc;

    int posicao_na_tabela = (int)(tipo_iva - 'A');
    int percentagem_iva = iva_tabela[posicao_na_tabela];

    int quant = item->quantidade;
    double total_da_linha = preco_com_iva(preco_base, percentagem_iva, quant);

    printf("%c %.2f %d %.2f %s\n", 
           tipo_iva, 
           preco_base, 
           quant, 
           total_da_linha, 
           nome_produto);
}

/**
 * Verifica se um produto está no cesto.
 * @param i índice do produto em produtos[]
 * @return 1 se está no cesto, 0 caso contrário
 */
int cesto_tem_produto(int i) {
    return cesto_procura(i) != NULL;
}

/**
 * Lista o conteúdo do cesto por ordem crescente de código EAN.
 * Usa selection sort com array auxiliar para não reordenar a lista.
 * @param produtos array de produtos
 * @param iva_tabela tabela de IVA
 */
static void cesto_listar(Produto produtos[], int iva_tabela[]) {
    ItemCesto *atual, *menor;
    int total_itens = 0;
    int impressos = 0;
    int i;
    int *foi_impresso;

    for (atual = cesto; atual != NULL; atual = atual->prox) {
        total_itens++;
    }

    if (total_itens == 0) {
        return;
    }

    foi_impresso = (int *)calloc((size_t)total_itens, sizeof(int));

    if (foi_impresso == NULL) {
        printf("No memory.\n");
        cesto_libertar();
        exit(1);
    }

    while (impressos < total_itens) {
        menor = NULL;
        int indice_do_menor = -1;
        
        i=0;
        for (atual = cesto; atual != NULL; atual = atual->prox) {
            if (foi_impresso[i] == 1) {
                i++;
                continue;
            }
            if (menor == NULL || strcmp(produtos[atual->idx_produto].ean,
                                        produtos[menor->idx_produto].ean) < 0) {
                menor = atual;
                indice_do_menor = i;
            }
            i++;
        }
        if (menor != NULL) {
            imprimir_item(menor, produtos, iva_tabela);
            foi_impresso[indice_do_menor] =  1;
            impressos++;
        }
    }
    free(foi_impresso);
}

/**
 * Executa o comando a: adiciona, remove ou lista produtos no cesto.
 * Sem args: lista o cesto. Com EAN: adiciona 1 unidade.
 * Com quantidade e EAN: adiciona (positivo) ou remove (negativo).
 * O campo vendido do produto é actualizado em conformidade.
 * @param linha resto da linha após o 'a'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @param iva_tabela tabela de IVA
 */
void comando_a(const char *linha, Produto produtos[], int num_produtos, 
              int iva_tabela[]) {
    char ean[EAN_MAX + 1];
    char aux1[MAX_LINHA], aux2[MAX_LINHA];
    int quantidade;
    int res_sscanf;
    int i, indice_produto = -1;
    ItemCesto *item_encontrado;

    res_sscanf = sscanf(linha, "%s %s", aux1, aux2);

    if (res_sscanf <= 0) {
        cesto_listar(produtos, iva_tabela);
        return;
    }

    if (res_sscanf == 1) {
        quantidade = 1;
        strcpy(ean, aux1);
    } else {
        quantidade = atoi(aux1);
        strcpy(ean, aux2);
    }

    if (validacao_ian(ean) == 0) {
        printf("invalid ean\n");
        return;
    }

    for (i = 0; i < num_produtos; i++) {
        if (strcmp(produtos[i].ean, ean) == 0) {
            indice_produto = i;
            break;
        }
    }

    if (indice_produto == -1) {
        printf("%s: no such product\n", ean);
        return;
    }

    item_encontrado = cesto_procura(indice_produto);

    if (quantidade < 0) {
        int qtd_no_cesto = 0;
        if (item_encontrado != NULL) {
            qtd_no_cesto = item_encontrado->quantidade;
        }

        if (qtd_no_cesto + quantidade < 0) {
            printf("invalid quantity\n");
            return;
        }

        produtos[indice_produto].stock = produtos[indice_produto].stock - quantidade;
        /* Devolve ao stock e reverte o vendido (quantidade é negativa) */
        produtos[indice_produto].vendido += quantidade;
        item_encontrado->quantidade = item_encontrado->quantidade + quantidade;
        
        imprimir_item(item_encontrado, produtos, iva_tabela);

        if (item_encontrado->quantidade == 0) {
            cesto_remover(indice_produto);
        }
        return;
    }

    if (produtos[indice_produto].stock < quantidade || (quantidade == 0 && produtos[indice_produto].stock == 0)) {
        printf("no stock\n");
        return;
    }

    produtos[indice_produto].stock = produtos[indice_produto].stock - quantidade;
    /* Quantidade no cesto conta como vendida para efeitos de r e l */
    produtos[indice_produto].vendido += quantidade;

    if (item_encontrado != NULL) {
        item_encontrado->quantidade = item_encontrado->quantidade + quantidade;
    } else {
        item_encontrado = (ItemCesto *)malloc(sizeof(ItemCesto));
        if (item_encontrado == NULL) {
            printf("No memory.\n");
            cesto_libertar();
            exit(1);
        }
        item_encontrado->idx_produto = indice_produto;
        item_encontrado->quantidade = quantidade;
        item_encontrado->prox = cesto;
        cesto = item_encontrado;
    }

    imprimir_item(item_encontrado, produtos, iva_tabela);
}

/**
 * Devolve o ponteiro para o primeiro item do cesto.
 * @return cabeça da lista, ou NULL se o cesto estiver vazio
 */
ItemCesto *cesto_obter_lista(void) {
    return cesto;
}

/**
 * Liberta todos os nós da lista do cesto.
 */
void cesto_libertar(void) {
    ItemCesto *cur = cesto, *prox;
	while (cur != NULL) {
		prox = cur->prox;
		free(cur);
		cur = prox;
	}
	cesto = NULL;
}