#include "products.h"


/**
 * Fills the table with default VAT values.
 * @param iva_tabela table to initialize
 */
void iva_init_default(int iva_tabela[]) {
    for (int i = 0; i < MAX_IVA; i++) {
        iva_tabela[i] = IVA_INDEFINIDO;
    }
    iva_tabela['A'-'A'] = 0;
    iva_tabela['B'-'A'] = 6;
    iva_tabela['C'-'A'] = 13;
    iva_tabela['D'-'A'] = 23;
}

/**
 * Reads the VAT file and fills the table.
 * Format of each line: <LETTER> <percentage>
 * @param nome_ficheiro path to the file
 * @param tabela_iva table to fill
 */
void iva_carregar(const char *nome_ficheiro, int tabela_iva[]) {
    FILE *arquivo = fopen(nome_ficheiro, "r");
    if (arquivo == NULL) {
        return;
    }
    for (int i = 0; i < MAX_IVA; i++) {
        tabela_iva[i] = IVA_INDEFINIDO;
    }
    char letra_lida;
    int valor_lido;
    while (fscanf(arquivo, " %c %d", &letra_lida, &valor_lido) == 2) {
        if (letra_lida >= 'A' && letra_lida <= 'Z') {
            int indice = letra_lida - 'A';
            if (indice >= 0 && indice < MAX_IVA) {
                tabela_iva[indice] = valor_lido;
            }
        }
    }
    fclose(arquivo);
}

/**
 * Validates the check digit of an EAN-8 or EAN-13.
 * GTIN-13: positions alternate weights 1 and 3, starting with 1.
 * GTIN-8: positions alternate weights 3 and 1, starting with 3.
 * @param ean string with the EAN code
 * @return 1 if valid, 0 otherwise
 */
int validacao_ean(const char *ean) {
    int len = (int)strlen(ean);
    int soma = 0;

    if (len != 8 && len != 13) {
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (ean[i] < '0' || ean[i] > '9') {
            return 0;
        }
    }

    for (int i = 0; i < len - 1; i++) {
        int peso;
        if (len == 8) {
            /* GTIN-8 starts with weight 3. */
            peso = (i % 2 == 0) ? 3 : 1;
        } else {
            /* GTIN-13 starts with weight 1. */
            peso = (i % 2 == 0) ? 1 : 3;
        }
        soma += (ean[i] - '0') * peso;
    }

    return (ean[len - 1] - '0') == ((10 - (soma % 10)) % 10);
}

/**
 * Searches for a product by EAN. Returns the index or -1 if it doesn't exist.
 * @param ean EAN code to search
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @return product index, or -1
 */
static int procura_produto(const char *ean, Produto produtos[],
        int num_produtos) {
    for (int i = 0; i < num_produtos; i++) {
        if (strcmp(produtos[i].ean, ean) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Executes command 'p': adds a new product or updates an existing one.
 * If the EAN already exists, increments the stock and updates the other fields.
 * Errors checked in order: ean, iva, price, quantity, description,
 * product in use, invalid product.
 * @param linha rest of the line after 'p'
 * @param produtos array of products
 * @param num_produtos pointer to the product counter
 * @param iva_tabela VAT table
 * @param cesto_tem_produto function that checks if the product is in the cart
 */
void comando_p(const char *linha, Produto produtos[], int *num_produtos,
        int iva_tabela[], int cesto_tem_produto(int)) {
    char ean[EAN_MAX + 1];
    char iva;
    char desc[MAX_DESC + 1];
    double preco;
    int quantidade;
    int posicao, i;

    if (sscanf(linha, "%13s %c %lf %d", ean, &iva, &preco, &quantidade)
		!= 4) {
		return;
    }
    /* Advances the pointer to the start of the description (after the 4 fields) */
	{
		const char *p = linha;
		int campos = 0;
		while (campos < 4) {
			while (*p && isspace((unsigned char)*p)) p++;
			while (*p && !isspace((unsigned char)*p)) p++;
			campos++;
		}
		while (*p && isspace((unsigned char)*p)) p++;
		{
			int i = 0;
			while (*p && *p != '\n' && i < (int)MAX_DESC) {
				desc[i++] = *p++;
			}
			desc[i] = '\0';
		}
	}

    if (!validacao_ean(ean)) {
        printf("invalid ean\n");
        return;
    }

    if (iva < 'A' || iva > 'Z' || iva_tabela[iva - 'A'] == -1) {
        printf("invalid iva\n");
        return;
    }

    if (preco <= 0) {
        printf("invalid price\n");
        return;
    }

    if (quantidade < 0) {
        printf("invalid quantity\n");
        return;
    }

    {
    const char *especiais = "?!#$%&@";
    for (int k = 0; desc[k] != '\0'; k++) {
        if (strchr(especiais, desc[k]) != NULL) {
            printf("invalid character in description\n");
            return;
        }
    }
    }

    /* Accepts ASCII uppercase (A-Z) and UTF-8 uppercase with accents (byte > 127) */
    if (!isupper((unsigned char)desc[0]) && (unsigned char)desc[0] <= 127) {
        printf("invalid description\n");
        return;
    }

    int tam_descricao = 0;
    while (desc[tam_descricao] != '\0') {
        tam_descricao++;
    }

    if (tam_descricao > (MAX_DESC-1)) {
        printf("invalid description\n");
        return;
    }

    posicao = procura_produto(ean, produtos, *num_produtos);

    if (posicao != -1) {
            /* Product already exists: only blocks if the price changes and it is in the cart */
        if (produtos[posicao].preco != preco && cesto_tem_produto(posicao) == 1) {
            printf("product in use\n");
            return;
        }
        produtos[posicao].iva = iva;
        produtos[posicao].preco = preco;
        produtos[posicao].stock += quantidade;

        for (i = 0; i < (MAX_DESC-1) && desc[i] != '\0'; i++) {
            produtos[posicao].desc[i] = desc[i];
        }
        produtos[posicao].desc[i] = '\0';

        printf("%d\n", produtos[posicao].stock);
    }
    else {
        // It is a new product
        if (*num_produtos >= MAX_PRODUTOS) {
            printf("invalid product\n");
            return;
        }
        for (i = 0; ean[i] != '\0'; i++) {
            produtos[*num_produtos].ean[i] = ean[i];
        }
        produtos[*num_produtos].ean[i] = '\0';

        produtos[*num_produtos].iva = iva;
        produtos[*num_produtos].preco = preco;
        produtos[*num_produtos].stock= quantidade;
        produtos[*num_produtos].vendido = 0;

        for (i = 0; i < (MAX_DESC-1) && desc[i] != '\0'; i++) {
            produtos[*num_produtos].desc[i] = desc[i];
        }
        produtos[*num_produtos].desc[i] = '\0';

        printf("%d\n", produtos[*num_produtos].stock);
        (*num_produtos)++;

    }
}

/**
 * Prints a product in the format of command 'l':
 * <ean> <iva> <price> <sold> <stock> <description>
 * @param produtos array of products
 * @param i index of the product to print
 */
static void imprimir_produto(Produto produtos[], int i) {
    printf("%s %c %.2f %d %d %s\n",
        produtos[i].ean,
        produtos[i].iva,
        produtos[i].preco,
        produtos[i].vendido,
        produtos[i].stock,
        produtos[i].desc);
}

/**
 * Checks if the string str matches the pattern pat.
 * '*' matches any sequence of characters (including empty).
 * '?' matches exactly one character.
 * @param pat pattern with wildcards
 * @param str string to test
 * @return 1 if it matches, 0 otherwise
 */
static int wildcard_match(const char *pat, const char *str) {
    if (*pat == '\0') {
        return *str == '\0';
    }
    if (*pat == '*') {
        if (wildcard_match(pat + 1, str)) {
            return 1;
        }
        if (*str != '\0') {
            return wildcard_match(pat, str + 1);
        }
        return 0;
    }
    if (*str != '\0' && (*pat == '?' || *pat == *str)) {
        return wildcard_match(pat + 1, str + 1);
    }
    return 0;
}

/**
 * Lists products with stock > 0 whose EAN matches pat.
 * @param pat wildcard pattern
 * @param produtos array of products
 * @param num_produtos number of registered products
 * @return number of listed products
 */
static int listar_por_wildcard(const char *pat, Produto produtos[], int num_produtos) {
    int total = 0;
    for (int i = 0; i < num_produtos; i++) {
        if (produtos[i].stock > 0 && wildcard_match(pat, produtos[i].ean)) {
            imprimir_produto(produtos, i);
            total++;
        }
    }
    return total;
}

/**
 * Executes command 'l': lists available products.
 * No args or with '*': lists everything. With wildcards: filters by EAN.
 * Emits error if no product matches a wildcard.
 * @param linha rest of the line after 'l'
 * @param produtos array of products
 * @param num_produtos number of registered products
 */
void comando_l(const char *linha, Produto produtos[], int num_produtos) {
    char padrao[MAX_LINHA];
    const char *p = linha;
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0' || *p == '\n') {
        /* No args: lists everything; emits error if there are no products in stock */
        if (listar_por_wildcard("*", produtos, num_produtos) == 0)
            printf("*: no such product\n");
        return;
    }
    while (*p && *p != '\n') {
        int i = 0;
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0' || *p == '\n') {
            break;
        }
        while (*p && !isspace((unsigned char)*p) && *p != '\n') {
            padrao[i++] = *p++;
        }
        padrao[i] = '\0';
        if (listar_por_wildcard(padrao, produtos, num_produtos) == 0) {
            printf("%s: no such product\n", padrao);
        }
    }
}

/**
 * Removes the product at index i by shifting the following ones up.
 * @param produtos array of products
 * @param num_produtos pointer to the product counter
 * @param indice index of the product to remove
 */
static void produto_remover(Produto produtos[], int *num_produtos, int indice) {
    for (int j = indice; j < *num_produtos - 1; j++) {
        produtos[j] = produtos[j + 1];
    }
    (*num_produtos)--;
}

/**
 * Executes EAN mode of command 'd': reduces a product's stock.
 * If the stock reaches zero, the product is removed from the system.
 * @param ean EAN code of the product
 * @param qtd amount to remove from stock (must be positive)
 * @param produtos array of products
 * @param num_produtos pointer to the product counter
 * @param cesto_tem_produto function that checks if the product is in the cart
 */
void comando_d_produto(const char *ean, int qtd, Produto produtos[], int *num_produtos, int cesto_tem_produto(int)) {
    int i, encontrado = -1;

    if (!validacao_ean(ean)) {
        printf("invalid ean\n");
        return;
    }
    for (i = 0; i < *num_produtos; i++) {
        if (strcmp(produtos[i].ean, ean) == 0) {
            encontrado = i;
            break;
        }
    }
    if (encontrado == -1) {
        printf("%s: no such product\n", ean);
        return;
    }
    if (cesto_tem_produto(encontrado)) {
        printf("product in use\n");
        return;
    }
    if (qtd <= 0 || qtd > produtos[encontrado].stock) {
        printf("invalid quantity\n");
        return;
    }
    produtos[encontrado].stock -= qtd;
    if (produtos[encontrado].stock == 0) {
        produto_remover(produtos, num_produtos, encontrado);
    } else {
        printf("%d %s\n", produtos[encontrado].stock, produtos[encontrado].desc);
    }
}
