#include "basket.h"
#include "products.h"

/* Head of the cart's linked list. */
static ItemCesto *cesto = NULL;

/**
 * Calculates the total price of a line with VAT, with symmetric rounding.
 * The 1e-9 epsilon prevents IEEE 754 representation errors.
 * @param preco unit price without VAT
 * @param percentagem_iva VAT rate in percentage
 * @param quantidade number of units
 * @return total price with VAT rounded to cents
 */
static double preco_com_iva(double preco, int percentagem_iva,
        int quantidade) {
    double conta = preco * quantidade * (1 + (percentagem_iva/100.0));
    int centimos = (int)(conta * 100.0 + 0.5 + 1e-9);
    return centimos / 100.0;
}

/**
 * Searches for an item in the cart by the product index.
 * @param i product index in products[]
 * @return pointer to ItemCesto, or NULL if not in the cart
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
 * Removes an item from the cart and frees the node.
 * @param i index of the product to remove
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
 * Prints a cart item in the format of command 'a':
 * <iva> <unit-price> <quantity> <total-price-with-vat> <description>
 * @param item item to print
 * @param produtos array of products
 * @param iva_tabela VAT table
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
 * Checks if a product is in the cart.
 * @param i product index in products[]
 * @return 1 if it is in the cart, 0 otherwise
 */
int cesto_tem_produto(int i) {
    return cesto_procura(i) != NULL;
}

/**
 * Lists the cart's content in ascending order of EAN code.
 * Uses selection sort with an auxiliary array to avoid reordering the list.
 * @param produtos array of products
 * @param iva_tabela VAT table
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
 * Executes command 'a': adds, removes, or lists products in the cart.
 * No args: lists the cart. With EAN: adds 1 unit.
 * With quantity and EAN: adds (positive) or removes (negative).
 * The product's sold field is updated accordingly.
 * @param linha rest of the line after 'a'
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @param iva_tabela VAT table
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

    if (validacao_ean(ean) == 0) {
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
        /* Returns to stock and reverts sold (quantity is negative) */
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
    /* Quantity in the cart counts as sold for the purposes of 'r' and 'l' */
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
 * Returns a pointer to the first item in the cart.
 * @return head of the list, or NULL if the cart is empty
 */
ItemCesto *cesto_obter_lista(void) {
    return cesto;
}

/**
 * Frees all nodes in the cart list.
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
