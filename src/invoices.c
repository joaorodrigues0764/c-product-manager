#include "invoices.h"
#include "products.h"
#include "basket.h"

/* Linked list of issued invoices, in chronological order. */
static Fatura *faturas = NULL;

/* Total of purchased units in all active invoices. */
static int total_itens_global = 0;

/* Number of invoices already issued (does not decrement with deletions). */
static int total_faturas_emitidas = 0;

/* Total billed amount in active invoices. */
static double valor_total_global = 0.0;


static char *duplicar_string(const char *s) {
    size_t tamanho = strlen(s) + 1;
    char *copia = (char *)malloc(tamanho);
    if (copia != NULL) {
        memcpy(copia, s, tamanho);
    }
    return copia;
}
/**
 * Frees the item list of an invoice.
 * @param f invoice whose items are freed
 */
static void itens_libertar(Fatura *f) {
    ItemFatura *atual;
    ItemFatura *seguinte;
    if (f->itens == NULL) {
        return;
    }
    for (atual = f-> itens; atual != NULL; atual = seguinte) {
        seguinte = atual->prox;
        free(atual);
    }
    f->itens = NULL;
}

/**
 * Executes command 'r': billing summary.
 * No args: prints global totals and VAT table in alphabetical order.
 * With EAN: prints stock and sold quantity of the specified product.
 * @param linha rest of the line after 'r'
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @param iva_tabela VAT table
 */
void comando_r(const char *linha, Produto produtos[], int num_produtos,
        int iva_tabela[]) {
    char ean_procurado[EAN_MAX + 1];
    int i;
    int indice_prod = -1;
    const char *p = linha;

    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0' || *p == '\n') {
        printf("%d %d %.2f\n", total_itens_global, total_faturas_emitidas,
                valor_total_global);
        for (i = 0; i < MAX_IVA; i++) {
            if (iva_tabela[i] != IVA_INDEFINIDO) {
                printf("%c %d%%\n", 'A' + i, iva_tabela[i]);
            }
        }
        return;
    }
    if (sscanf(p, "%s", ean_procurado) == 1) {
        if (validacao_ean(ean_procurado) == 0) {
            printf("invalid ean\n");
            return;
        }
        for (i = 0; i < num_produtos; i++) {
            if (strcmp(produtos[i].ean, ean_procurado) == 0) {
                indice_prod = i;
                break;
            }
        }
        if (indice_prod == -1) {
            printf("%s: no such product\n", ean_procurado);
        }
        else {
            printf("%d %d %s\n", produtos[indice_prod].stock,
                    produtos[indice_prod].vendido,
                    produtos[indice_prod].desc);
        }
    }
}

/**
 * Checks if a string is a valid NIF (exactly 9 digits).
 * @param s string to check
 * @return 1 if valid, 0 otherwise
 */
static int nif_valido(const char *s) {
    if (strlen(s) != 9) return 0;

    for (int i = 0; i < 9; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

/**
 * Checks if a string is a valid customer name (starts with a letter).
 * @param s string to check
 * @return 1 if valid, 0 otherwise
 */
static int nome_valido(const char *s) {
    return s != NULL && *s != '\0' && isalpha((unsigned char)*s);
}

/**
 * Reads the customer name starting from p, with support for quotes.
 * Name in quotes: reads until the closing quote.
 * Name without quotes: reads until the end of the line.
 * @param p pointer to the start of the name
 * @param dest destination buffer
 * @param max maximum buffer size
 * @return number of read bytes, or -1 if the closing quote is missing
 */
static int ler_nome(const char *p, char *dest, int max) {
    int i = 0;
    if (*p == '"') {
        p++;
        while (*p && *p != '"' && i < max - 1) {
            dest[i++] = *p++;
        }
        if (*p != '"') return -1;
    } else {
        while (*p && *p != '\n' && i < max - 1) {
            dest[i++] = *p++;
        }
    }
    dest[i] = '\0';
    return i;
}


/**
 * Calculates the total cart value with VAT and symmetric rounding.
 * Rounding is done per line and then on the final total.
 * The 1e-9 epsilon prevents IEEE 754 representation errors.
 * @param lista head of the cart list
 * @param produtos array of products
 * @param iva_tabela VAT table
 * @return total value with VAT
 */
static double calcular_total_cesto(ItemCesto *lista, Produto produtos[], int iva_tabela[]) {
    double total = 0.0;
    ItemCesto *atual = lista;

    while (atual != NULL) {
        int idx = atual->idx_produto;
        int taxa = iva_tabela[produtos[idx].iva - 'A'];

        double preco_com_iva = produtos[idx].preco * (1.0 + taxa / 100.0);
        double linha = preco_com_iva * atual->quantidade;

        /* Epsilon absorbs IEEE 754 representation errors */
        linha = (int)(linha * 100.0 + 0.5 + 1e-9) / 100.0;
        total += linha;
        atual = atual->prox;
    }
    return (int)(total * 100.0 + 0.5 + 1e-9) / 100.0;
}

/**
 * Counts the total units in the cart.
 * @param lista head of the cart list
 * @return sum of the quantities of all items
 */
static int contar_itens_cesto(ItemCesto *lista) {
    int conta = 0;
    ItemCesto *atual = lista;
    while (atual != NULL) {
        conta += atual->quantidade;
        atual = atual->prox;
    }
    return conta;
}

/**
 * Returns all cart products to stock and reverts the sold amount.
 * Called when the customer name is "error".
 * @param lista head of the cart list
 * @param produtos array of products
 */
static void devolver_cesto_ao_stock(ItemCesto *lista, Produto produtos[]) {
    ItemCesto *atual = lista;
    while (atual != NULL) {
        produtos[atual->idx_produto].stock += atual->quantidade;
        /* Reverts the sold amount that was counted when adding to the cart */
        produtos[atual->idx_produto].vendido -= atual->quantidade;
        atual = atual->prox;
    }
}

/**
 * Executes command 'f': invoices the products in the cart.
 * No args: uses NIF 999999999 and name "Cliente final".
 * With name "error": returns everything to stock without issuing an invoice.
 * The invoice number is sequential and is never reused.
 * @param linha rest of the line after 'f'
 * @param produtos array of products
 * @param iva_tabela VAT table
 */
void comando_f(const char *linha, Produto produtos[], int iva_tabela[]) {
    static int contador_faturas = 1;
    int nif_final = NIF_PADRAO;
    char nome_final[MAX_LINHA];
    const char *p = linha;

    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '\0' || *p == '\n') {
        strcpy(nome_final, NOME);
    } else {
        char primeiro_token[MAX_LINHA];
        int i = 0;
        const char *aux = p;

        while (*aux && !isspace((unsigned char)*aux) && *aux != '\n') {
            primeiro_token[i++] = *aux++;
        }
        primeiro_token[i] = '\0';

        if (nif_valido(primeiro_token)) {
            nif_final = atoi(primeiro_token);
            p = aux;
            while (*p && isspace((unsigned char)*p)) p++;
            if (ler_nome(p, nome_final, MAX_LINHA) < 0) {
                printf("invalid name\n");
                return;
            }
        } else {
            int so_numeros = (primeiro_token[0] != '\0');
            for (int k = 0; primeiro_token[k]; k++) {
                if (!isdigit((unsigned char)primeiro_token[k])) so_numeros = 0;
            }

            if (so_numeros) {
                printf("%s: no such nif\n", primeiro_token);
                return;
            }

            if (ler_nome(p, nome_final, MAX_LINHA) < 0) {
                printf("invalid name\n");
                return;
            }
        }
    }

    if (!nome_valido(nome_final)) {
        printf("invalid name\n");
        return;
    }

    if (strcmp(nome_final, "error") == 0) {
        devolver_cesto_ao_stock(cesto_obter_lista(), produtos);
        cesto_libertar();
        return;
    }

    ItemCesto *lista_cesto = cesto_obter_lista();
    int total_unidades = contar_itens_cesto(lista_cesto);
    double valor_fatura = calcular_total_cesto(lista_cesto, produtos, iva_tabela);

    Fatura *nova = (Fatura *)malloc(sizeof(Fatura));
    if (nova == NULL) {
        printf("No memory.\n");
        exit(1);
    }

    nova->nome_cliente = duplicar_string(nome_final);
    if (nova->nome_cliente == NULL) {
        free(nova);
        printf("No memory.\n");
        exit(1);
    }

    nova->numero = contador_faturas++;
    nova->nif = nif_final;
    nova->num_itens = total_unidades;
    nova->valor_pago = valor_fatura;
    nova->itens = NULL;
    nova->prox = NULL;

    ItemCesto *it = lista_cesto;
    while (it != NULL) {
        ItemFatura *item_f = (ItemFatura *)malloc(sizeof(ItemFatura));
        if (item_f == NULL) {
            itens_libertar(nova);
            free(nova->nome_cliente);
            free(nova);
            printf("No memory.\n");
            exit(1);
        }

        int idx = it->idx_produto;
        item_f->idx_produto = idx;
        item_f->quantidade = it->quantidade;
        item_f->preco_unit = produtos[idx].preco;
        item_f->iva = produtos[idx].iva;

        item_f->prox = nova->itens;
        nova->itens = item_f;
        /* sold amount was already incremented when the product entered the cart */
        it = it->prox;
    }

    if (faturas == NULL) {
        faturas = nova;
    } else {
        Fatura *temp = faturas;
        while (temp->prox != NULL) temp = temp->prox;
        temp->prox = nova;
    }

    total_itens_global += total_unidades;
    total_faturas_emitidas++;
    valor_total_global += valor_fatura;

    cesto_libertar();
    printf("%d %.2f %d\n", total_unidades, valor_fatura, nova->numero);
}

/**
 * Prints an invoice in the format of command 'c':
 * <number> <value> <customer-name>
 * @param f invoice to print
 */
static void imprimir_fatura(const Fatura *f) {
    printf("%d %.2f %s %d\n", f->numero, f->valor_pago, f->nome_cliente, f->nif);
}

static void listar_todas_faturas_filtro(double valor_min) {
    int total = 0;
    Fatura *temp = faturas;

    while (temp != NULL) {
        total++;
        temp = temp->prox;
    }

    if (total == 0) return;

    int *visitados = (int *)calloc(total, sizeof(int));
    if (visitados == NULL) {
        exit(1);
    }

    int impressos = 0;
    while (impressos < total) {
        Fatura *menor = NULL;
        int i = 0;

        temp = faturas;
        while (temp != NULL) {
            if (!visitados[i]) {
                if (menor == NULL || strcmp(temp->nome_cliente, menor->nome_cliente) < 0) {
                    menor = temp;
                }
            }
            temp = temp->prox;
            i++;
        }

        if (menor == NULL) break;

        char nome_atual[MAX_LINHA];
        strcpy(nome_atual, menor->nome_cliente);

        i = 0;
        temp = faturas;
        while (temp != NULL) {
            if (strcmp(temp->nome_cliente, nome_atual) == 0) {
                if (temp->valor_pago > valor_min)
                    imprimir_fatura(temp);
                if (!visitados[i]) {
                    visitados[i] = 1;
                    impressos++;
                }
            }
            temp = temp->prox;
            i++;
        }
    }
    free(visitados);
}

void comando_c(const char *linha) {
    const char *p = linha;

    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '\0' || *p == '\n') {
        listar_todas_faturas_filtro(-1.0);
        return;
    }

    /* Tries to read a numeric value as the first token */
    {
        const char *aux = p;
        int i = 0;
        char tok[MAX_LINHA];
        int is_num = 1;
        int has_dot = 0;
        if (*aux == '-') tok[i++] = *aux++;
        while (*aux && !isspace((unsigned char)*aux) && *aux != '\n') {
            if (*aux == '.' && !has_dot) { has_dot = 1; tok[i++] = *aux++; }
            else if (isdigit((unsigned char)*aux)) { tok[i++] = *aux++; }
            else { is_num = 0; break; }
        }
        tok[i] = '\0';

        if (is_num && i > 0) {
            double valor_min = atof(tok);
            p = aux;
            while (*p && isspace((unsigned char)*p)) p++;

            if (*p == '\0' || *p == '\n') {
                listar_todas_faturas_filtro(valor_min);
                return;
            }

            /* value + name */
            char nome_busca[MAX_LINHA];
            if (ler_nome(p, nome_busca, MAX_LINHA) < 0 || !nome_valido(nome_busca)) {
                printf("invalid name\n");
                return;
            }
            int achou = 0;
            Fatura *atual = faturas;
            while (atual != NULL) {
                if (strcmp(atual->nome_cliente, nome_busca) == 0 &&
                        atual->valor_pago > valor_min) {
                    imprimir_fatura(atual);
                    achou = 1;
                }
                atual = atual->prox;
            }
            if (!achou) printf("%s: no such client\n", nome_busca);
            return;
        }
    }

    /* No value: name as argument */
    {
        char nome_busca[MAX_LINHA];
        if (ler_nome(p, nome_busca, MAX_LINHA) < 0 || !nome_valido(nome_busca)) {
            printf("invalid name\n");
            return;
        }
        int achou = 0;
        Fatura *atual = faturas;
        while (atual != NULL) {
            if (strcmp(atual->nome_cliente, nome_busca) == 0) {
                imprimir_fatura(atual);
                achou = 1;
            }
            atual = atual->prox;
        }
        if (!achou) printf("%s: no such client\n", nome_busca);
    }
}

static int nif_valido_str(const char *s) {
    if (strlen(s) != 9) return 0;
    for (int i = 0; i < 9; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

void comando_v(const char *linha) {
    const char *p = linha;
    char nome_busca[MAX_LINHA];
    char nif_str[MAX_LINHA];

    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '"') {
        p++;
        int i = 0;
        while (*p && *p != '"' && i < MAX_LINHA - 1)
            nome_busca[i++] = *p++;
        nome_busca[i] = '\0';
        if (*p == '"') p++;
    } else {
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && *p != '\n' && i < MAX_LINHA - 1)
            nome_busca[i++] = *p++;
        nome_busca[i] = '\0';
    }

    if (!nome_valido(nome_busca)) {
        printf("invalid name\n");
        return;
    }

    while (*p && isspace((unsigned char)*p)) p++;

    if (sscanf(p, "%s", nif_str) != 1) {
        printf("invalid name\n");
        return;
    }

    if (!nif_valido_str(nif_str)) {
        printf("%s: no such nif\n", nif_str);
        return;
    }

    int novo_nif = atoi(nif_str);

    int achou = 0;
    Fatura *f = faturas;
    while (f != NULL) {
        if (strcmp(f->nome_cliente, nome_busca) == 0) { achou = 1; break; }
        f = f->prox;
    }
    if (!achou) {
        printf("%s: no such client\n", nome_busca);
        return;
    }

    int count = 0;
    f = faturas;
    while (f != NULL) {
        if (strcmp(f->nome_cliente, nome_busca) == 0) {
            f->nif = novo_nif;
            count++;
        }
        f = f->prox;
    }
    printf("%d\n", count);
}

/**
 * Searches for an invoice by its number.
 * @param num invoice number
 * @return pointer to the Fatura, or NULL if it doesn't exist
 */
static Fatura *procura_fatura(int num) {
    Fatura *f = faturas;
    while (f != NULL) {
        if (f->numero == num) return f;
        f = f->prox;
    }
    return NULL;
}

/**
 * Checks if an invoice with the specified number exists.
 * @param num invoice number
 * @return 1 if it exists, 0 otherwise
 */
int procura_fatura_existe(int num) {
    return procura_fatura(num) != NULL;
}

/**
 * Executes the invoice mode of command 'd': removes the specified invoice.
 * Decrements the global totals for items and value (not for issued invoices).
 * Prints: <amount-paid> <nif> <customer-name>
 * @param num number of the invoice to remove
 */
void comando_d_fatura(int num) {
    Fatura *atual = faturas;
    Fatura *anterior = NULL;

    while (atual != NULL) {
        if (atual->numero == num) {
            printf("%.2f %d %s\n", atual->valor_pago, atual->nif, atual->nome_cliente);

            if (anterior == NULL) {
                faturas = atual->prox;
            } else {
                anterior->prox = atual->prox;
            }

            /* Updates global totals upon deletion (issued invoices do not decrement) */
            total_itens_global -= atual->num_itens;
            valor_total_global -= atual->valor_pago;

            itens_libertar(atual);
            free(atual->nome_cliente);
            free(atual);
            return;
        }
        anterior = atual;
        atual = atual->prox;
    }
}

/*
Frees all memory allocated by the invoices.
 */
void faturas_libertar(void) {
    Fatura *atual = faturas;
    while (atual != NULL) {
        Fatura *proximo = atual->prox;
        itens_libertar(atual);
        free(atual->nome_cliente);
        free(atual);
        atual = proximo;
    }
    faturas = NULL;
}
