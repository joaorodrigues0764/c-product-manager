#include "produtos.h"
#include "faturas.h"
#include "cesto.h"

/*
Liberta toda a memória antes de terminar.
 */
static void terminar(void) {
	cesto_libertar();
	faturas_libertar();
}

/**
 * Inicializa o sistema, lê comandos em ciclo e despacha-os.
 * As estruturas de dados (produtos e IVA) vivem aqui como variáveis locais
 * e são passadas por ponteiro a cada função que precisa delas.
 * @param argc número de argumentos da linha de comando
 * @param argv argumentos: argv[1] é o ficheiro de IVA opcional
 * @return 0 em caso de sucesso
 */
int main(int argc, char *argv[]) {
	Produto produtos[MAX_PRODUTOS]; /* Array de produtos registados. */
	int iva_tabela[MAX_IVA];        /* Tabela de taxas de IVA por letra. */
	int num_produtos = 0;           /* Número actual de produtos. */
	char linha[MAX_LINHA];          /* Buffer para leitura de cada linha. */
	char comando;

	if (argc >= 2)
		iva_carregar(argv[1], iva_tabela);
	else
		iva_init_default(iva_tabela);

	while (fgets(linha, MAX_LINHA, stdin) != NULL) {
		if (sscanf(linha, " %c", &comando) != 1)
			continue;

		/* Aponta para o início dos argumentos do comando */
		char *resto = linha + 1;
		while (*resto && isspace((unsigned char)*resto) && *resto != '\n')
			resto++;

		switch (comando) {
		case 'q':
			terminar();
			return 0;
		case 'p':
			comando_p(resto, produtos, &num_produtos,
				iva_tabela, cesto_tem_produto);
			break;
		case 'l':
			comando_l(resto, produtos, num_produtos);
			break;
		case 'a':
			comando_a(resto, produtos, num_produtos, iva_tabela);
			break;
		case 'r':
			comando_r(resto, produtos, num_produtos, iva_tabela);
			break;
		case 'f':
			comando_f(resto, produtos, iva_tabela);
			break;
		case 'c':
			comando_c(resto);
			break;
		case 'v':
			comando_v(resto);
			break;
		case 'd':
			{
                char aux1[MAX_LINHA];
                char aux2[MAX_LINHA];
                char ean_ou_nif[MAX_LINHA];
                int n_lidos;
                int tamanho_token;

                n_lidos = sscanf(resto, "%s %s", aux1, aux2);

                if (n_lidos < 1) {
                    break;
                }

                tamanho_token = strlen(aux1);
				/* EAN tem 8 ou 13 dígitos; qualquer outra coisa é número de factura */
                if (tamanho_token == 8 || tamanho_token == 13) {
                    int quantidade_para_remover;
                    
                    if (n_lidos == 2) {
                        quantidade_para_remover = atoi(aux2);
                    } else {
                        quantidade_para_remover = 0;
                    }

                    strcpy(ean_ou_nif, aux1);

                    comando_d_produto(ean_ou_nif, quantidade_para_remover, produtos,
                        &num_produtos, cesto_tem_produto);
                } 
                else {
                    int numero_fatura;
                    numero_fatura = atoi(aux1);

                    if (procura_fatura_existe(numero_fatura) == 1) {
                        comando_d_fatura(numero_fatura);
                    } 
                    else {
                        printf("%d: no such invoice\n", numero_fatura);
                    }
                }
            }
			break;
		default:
			break;
		}
	}
	terminar();
	return 0;
}