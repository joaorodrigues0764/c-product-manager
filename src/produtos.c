#include "produtos.h"


/**
 * Preenche a tabela com os valores de IVA por omissão.
 * @param iva_tabela tabela a inicializar
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
 * Lê o ficheiro de IVA e preenche a tabela.
 * Formato de cada linha: <LETRA> <percentagem>
 * @param nome_ficheiro caminho para o ficheiro
 * @param tabela_iva tabela a preencher
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
 * Valida o dígito de verificação de um EAN-8 ou EAN-13.
 * EAN-13: posições pares peso 1, ímpares peso 3.
 * EAN-8:  mesmo esquema de pesos.
 * @param ean string com o código EAN
 * @return 1 se válido, 0 caso contrário
 */
int validacao_ian(const char *ean) {
    int len = strlen(ean);
    int soma = 0;
    if (len != 8 && len != 13) {
        return 0;
    }
    for (int h = 0; h < len; h++) {
        if (ean[h] < '0' || ean[h] > '9') {
            return 0;
        }
    }
    if (len == 13) {
        for (int i = 0; i < (len-1); i++) {
            if (i % 2 == 0) {
                soma += (ean[i] - '0');
            }
            else {
                soma += (ean[i] - '0')*3;
            }
        }
    }
    else {
        for (int i = 0; i < (len-1); i++) {
            if (i % 2 == 0) {
                soma += (ean[i] - '0');
            }
            else {
                soma += (ean[i] - '0')*3;
            }
        }
    }
    if ((ean[len-1] - '0') != ((10 - (soma%10)) % 10)) {
        return 0;
    }
    return 1;
}

/**
 * Procura um produto pelo EAN. Devolve o índice ou -1 se não existir.
 * @param ean código EAN a procurar
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @return índice do produto, ou -1
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
 * Executa o comando p: adiciona um produto novo ou atualiza um existente.
 * Se o EAN já existir, incrementa o stock e actualiza os restantes campos.
 * Erros verificados por ordem: ean, iva, price, quantity, description,
 * product in use, invalid product.
 * @param linha resto da linha após o 'p'
 * @param produtos array de produtos
 * @param num_produtos ponteiro para o contador de produtos
 * @param iva_tabela tabela de IVA
 * @param cesto_tem_produto função que verifica se o produto está no cesto
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
    /* Avança o ponteiro até ao início da descrição (após os 4 campos) */
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
			while (*p && *p != '\n' && i <= (int)MAX_DESC) {
				desc[i++] = *p++;
			}
			desc[i] = '\0';
		}
	}

    if (!validacao_ian(ean)) {
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

    /* Aceita maiúsculas ASCII (A-Z) e maiúsculas acentuadas UTF-8 (byte > 127) */
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
            /* Produto já existe: só bloqueia se o preço mudar e estiver no cesto */
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
        //É um produto novo
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
 * Imprime um produto no formato do comando l:
 * <ean> <iva> <preço> <vendido> <stock> <descrição>
 * @param produtos array de produtos
 * @param i índice do produto a imprimir
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
 * Verifica se a string str casa com o padrão pat.
 * '*' casa com qualquer sequência de caracteres (incluindo vazia).
 * '?' casa com exactamente um carácter.
 * @param pat padrão com wildcards
 * @param str string a testar
 * @return 1 se casa, 0 caso contrário
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
 * Lista os produtos com stock > 0 cujo EAN casa com pat.
 * @param pat padrão wildcard
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 * @return número de produtos listados
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
 * Executa o comando l: lista produtos disponíveis.
 * Sem args ou com '*': lista tudo. Com wildcards: filtra por EAN.
 * Emite erro se nenhum produto casar com um wildcard.
 * @param linha resto da linha após o 'l'
 * @param produtos array de produtos
 * @param num_produtos número de produtos registados
 */
void comando_l(const char *linha, Produto produtos[], int num_produtos) {
    char padrao[MAX_LINHA];
    const char *p = linha;
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0' || *p == '\n') {
        /* Sem args: lista tudo; emite erro se não houver produtos com stock */
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
 * Remove o produto no índice i deslocando os seguintes para cima.
 * @param produtos array de produtos
 * @param num_produtos ponteiro para o contador de produtos
 * @param i índice do produto a remover
 */
static void produto_remover(Produto produtos[], int *num_produtos, int indice) {
    for (int j = indice; j < *num_produtos - 1; j++) {
        produtos[j] = produtos[j + 1];
    }
    (*num_produtos)--;
}

/**
 * Executa o modo EAN do comando d: reduz o stock de um produto.
 * Se o stock chegar a zero, o produto é removido do sistema.
 * @param ean código EAN do produto
 * @param qtd quantidade a retirar do stock (deve ser positiva)
 * @param produtos array de produtos
 * @param num_produtos ponteiro para o contador de produtos
 * @param cesto_tem_produto função que verifica se o produto está no cesto
 */
void comando_d_produto(const char *ean, int qtd, Produto produtos[], int *num_produtos, int cesto_tem_produto(int)) {
    int i, encontrado = -1;

    if (!validacao_ian(ean)) {
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

