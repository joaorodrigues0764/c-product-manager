#include "faturas.h"
#include "produtos.h"
#include "cesto.h"

/* Lista ligada de facturas emitidas, por ordem cronológica. */
static Fatura *faturas = NULL;
 
/* Total de unidades compradas em todas as facturas activas. */
static int total_itens_global = 0;
 
/* Número de facturas já emitidas (não decrementa com eliminações). */
static int total_faturas_emitidas = 0;
 
/* Valor total facturado nas facturas activas. */
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
 * Liberta a lista de itens de uma factura.
 * @param f factura cujos itens se libertam
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
 * Executa o comando r: resumo de facturação.
 * Sem args: imprime totais globais e tabela de IVA por ordem alfabética.
 * Com EAN: imprime stock e vendido do produto indicado.
 * @param linha resto da linha após o 'r'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @param iva_tabela tabela de IVA
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
 * Verifica se uma string é um NIF válido (exactamente 9 dígitos).
 * @param s string a verificar
 * @return 1 se válido, 0 caso contrário
 */
static int nif_valido(const char *s) {
    if (strlen(s) != 9) return 0;
    
    for (int i = 0; i < 9; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

/**
 * Verifica se uma string é um nome de cliente válido (começa por letra).
 * @param s string a verificar
 * @return 1 se válido, 0 caso contrário
 */
static int nome_valido(const char *s) {
    return s != NULL && *s != '\0' && isalpha((unsigned char)*s);
}

/**
 * Lê o nome do cliente a partir de p, com suporte a aspas.
 * Nome entre aspas: lê até à aspa de fecho.
 * Nome sem aspas: lê até ao fim da linha.
 * @param p ponteiro para o início do nome
 * @param dest buffer de destino
 * @param max tamanho máximo do buffer
 * @return número de bytes lidos, ou -1 se a aspa de fecho faltar
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
 * Calcula o valor total do cesto com IVA e arredondamento simétrico.
 * O arredondamento é feito por linha e depois no total final.
 * O epsilon 1e-9 evita erros de representação IEEE 754.
 * @param lista cabeça da lista do cesto
 * @param produtos array de produtos
 * @param iva_tabela tabela de IVA
 * @return valor total com IVA
 */
static double calcular_total_cesto(ItemCesto *lista, Produto produtos[], int iva_tabela[]) {
    double total = 0.0;
    ItemCesto *atual = lista;

    while (atual != NULL) {
        int idx = atual->idx_produto;
        int taxa = iva_tabela[produtos[idx].iva - 'A'];
        
        double preco_com_iva = produtos[idx].preco * (1.0 + taxa / 100.0);
        double linha = preco_com_iva * atual->quantidade;

        /* Epsilon absorve erros de representação IEEE 754 */
        linha = (int)(linha * 100.0 + 0.5 + 1e-9) / 100.0;
        total += linha;
        atual = atual->prox;
    }
    return (int)(total * 100.0 + 0.5 + 1e-9) / 100.0;
}

/**
 * Conta o total de unidades no cesto.
 * @param lista cabeça da lista do cesto
 * @return soma das quantidades de todos os itens
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
 * Devolve todos os produtos do cesto ao stock e reverte o vendido.
 * Chamado quando o nome do cliente é "error".
 * @param lista cabeça da lista do cesto
 * @param produtos array de produtos
 */
static void devolver_cesto_ao_stock(ItemCesto *lista, Produto produtos[]) {
    ItemCesto *atual = lista;
    while (atual != NULL) {
        produtos[atual->idx_produto].stock += atual->quantidade;
        /* Reverte o vendido que foi contado ao adicionar ao cesto */
        produtos[atual->idx_produto].vendido -= atual->quantidade;
        atual = atual->prox;
    }
}

/**
 * Executa o comando f: factura os produtos no cesto.
 * Sem args: usa NIF 999999999 e nome "Cliente final".
 * Com nome "error": devolve tudo ao stock sem emitir factura.
 * O número de factura é sequencial e nunca é reutilizado.
 * @param linha resto da linha após o 'f'
 * @param produtos array de produtos
 * @param iva_tabela tabela de IVA
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
        /* vendido já foi incrementado quando o produto entrou no cesto */
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
 * Imprime uma factura no formato do comando c:
 * <número> <valor> <nome-cliente>
 * @param f factura a imprimir
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

    /* Tenta ler um valor numerico como primeiro token */
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

            /* valor + nome */
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

    /* Sem valor: nome como argumento */
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
 * Procura uma factura pelo número.
 * @param num número da factura
 * @return ponteiro para a Fatura, ou NULL se não existir
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
 * Verifica se existe uma factura com o número indicado.
 * @param num número da factura
 * @return 1 se existe, 0 caso contrário
 */
int procura_fatura_existe(int num) {
    return procura_fatura(num) != NULL;
}

/**
 * Executa o modo factura do comando d: remove a factura indicada.
 * Decrementa os totais globais de itens e valor (não o de facturas emitidas).
 * Imprime: <valor-pago> <nif> <nome-cliente>
 * @param num número da factura a remover
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

            /* Actualiza totais globais ao apagar (faturas emitidas não decrementa) */
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
Liberta toda a memória alocada pelas facturas.
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