# Enunciado do Projecto - IAED 2025/2026

## Data do checkpoint: 18 de março de 2026, às 19h59m

## Data de entrega: 06 de abril de 2026, às 19h59m

## Teste Prático: 08 de abril de 2026, horário a definir

## LOG alterações

- 7-mar-26 - Publicação do enunciado.

## 1. Introdução

Pretende-se a construção de um sistema de facturação de produtos na linguagem de programação __C__. Para tal, o sistema deverá permitir a introdução e remoção tanto de produtos como de facturas. O cliente vai colocando produtos num cesto/carrinho de compras para posterior facturação. Uma factura é o resumo da quantidade de items comprados e do seu custo total, associados a um cliente (NIF e nome).

A interacção com o programa deverá ocorrer através de um conjunto de linhas compostas por uma letra (comando) e um número de argumentos dependente do comando a executar. Pode assumir que todo o *input* fornecido respeitará os tipos indicados, por exemplo, onde é esperado um valor inteiro decimal nunca será introduzida uma letra. Os possíveis comandos são listados na tabela seguinte e indicam as operações a executar.

| Comando | Acção |
|:---:|:---|
| __q__ | termina o programa |
| __p__ | introduz um novo produto no sistema |
| __l__ | lista os produtos disponíveis no sistema |
| __a__ | adiciona ao cesto a quantidade do produto indicado |
| __r__ | resumo de facturação e existências |
| __f__ | factura os produtos no cesto |
| __d__ | apaga o registo de produtos ou facturas |
| __c__ | lista as facturas de um cliente |

## 2. Especificação do problema

O objectivo do projecto é ter um sistema de facturação de produtos. Para tal são introduzidos no sistema um conjunto de produtos. O sistema gere o registo da facturação dos produtos aos clientes.

![barcodes](Gs1-barcodes.png)

Cada __produto__ é caracterizado por uma descrição, um código EAN-13 (ou EAN-8), um preço, uma classe de IVA (uma letra maiúscula) e a quantidade de unidades em existência. Se já houver produtos em armazém com o mesmo código EAN, as suas existências devem ser incrementadas na quantidade indicada e os restante valores actualizados. A designação do código EAN, EAN-8 ou EAN-13, é constituída por __8__ ou __13__ digitos decimais, devendo ser único no sistema. Os códigos EAN-13 são um número de 12 digitos seguido de um dígito de verificação calculado a partir dos anteriores: somar os dígitos nas posições pares (0, 2, 4, ...) com os dígitos nas posições ímpares (1, 3, 5, ...) multiplicados por 3 (apenas os ímpares); o dígito de verificação resultante é `(10 - (soma % 10)) % 10` e deve ser igual ao último dígito do código EAN-13. De forma similar para o código EAN-8 usando respectivamente os digitos 0, 2, 4 e 6 e os digitos 1, 3 e 5 para o cálculo do digito de verificação. A descrição de um produto começa com uma letra maiúscula e tem um comprimento máximo de __50__ *bytes*. Notar que um carácter acentuado em *utf-8* utiliza mais de um *byte*. Por exemplo `Açúcar` tem 6 letras mas ocupa 8 *bytes* (`char` em __C__).

Uma __factura__ é apenas o resumo dos items comprados e da informação do cliente, sendo caracterizada por uma quantidade de produtos e respetivo valor total a facturar, além do NIF e nome do cliente que adquire os produtos. Os produtos e respetivas quantidades vão sendo adicionados a um cesto de compras sendo criada uma factura quando o conteúdo do cesto é pago. O nome do cliente pode conter caracteres brancos (espaços ou tabulador horizontal `\t`). Neste caso, o nome do cliente é representado entre aspas. Caso não contenha caracteres brancos, o nome do cliente pode ser delimitado por aspas ou não. O nome do cliente nunca contém o carácter aspa na sua descrição e começa sempre por uma letra (maiúscula ou minúscula). O nome do cliente não tem comprimento máximo mas na maioria dos casos não excede __50__ *bytes*. Todos os valores monetários são impressos com duas casas decimais. O NIF é um número inteiro com 9 digitos decimais.

Podem existir no máximo __10000__ produtos registados para facturação em cada instante. Não existem limites no número de produtos por factura, número de resumos de facturas guardadas nem no comprimento do nome do cliente, logo deve procurar utilizar a memória estritamente necessária. Não podem existir variáveis globais. Para facilitar a introdução dos dados, pode assumir que cada instrução não excede 65535 caracteres (64K bytes). Se a memória se esgotar, o programa deve terminar de forma controlada, imprimindo a mensagem `No memory.` Antes de terminar, o programa deve libertar toda a memória reservada dinamicamente.

Deve ser possível invocar o programa com um argumento que designa o nome de um ficheiro de códigos e valores de IVA. O ficheiro é constituído por uma entrada por linha, sendo cada entrada composta de um letra maiúscula e um valor inteiro que representa a percentagem de IVA a aplicar na venda do produto, separados por um só espaço branco. Por exemplo, uma entrada pode ser __D 23__. Caso não seja indicado o ficheiro de IVA nos argumentos do programa, os valores a usar deve ser __A 0__, __B 6__, __C 13__, __D 23__. Podem existir, no máximo 26 códigos de IVA, podendo as letras não ser consecutivas. Depois de aplicado o IVA o valor resultante deve ter arredondamento simétrico aos cêntimos: 0.00499999 arredonda para zero e 0.0050000 arredonda para um (cêntimo).

## 3. Dados de Entrada

Durante a execução do programa as instruções devem ser lidas do terminal (*standard input*) na forma de um conjunto de linhas iniciadas por um carácter, que se passa a designar por comando, seguido de um número de informações dependente do comando a executar; o comando e cada uma das informações são separados por pelo menos um carácter branco.

Os comandos disponíveis são descritos de seguida. Os caracteres `<` e `>` são utilizados apenas na descrição dos comandos para indicar os parâmetros. Os parâmetros opcionais estão indicados entre caracteres `[` e `]`. As repetições estão indicadas entre caracteres `{` e `}`. Cada comando tem sempre todos os parâmetros necessários à sua correcta execução.

Cada comando indica uma determinada acção que se passa a caracterizar em termos de formato de entrada, formato de saída e erros a retornar.

<u>Se o comando puder gerar mais de um erro, deverá ser indicado apenas o primeiro.</u>

- __q__ - termina o programa:
  - Formato de entrada: `q`
  - Formato de saída: NADA

- __p__ - introduz no sistema ou actualiza um novo produto:
  - Formato de entrada: `p <ean> <iva> <preço> <quantidade> <descrição>`
  - Formato de saída: `<stock>`, quantidade de produto resultante.
  - Erros:
    - `invalid ean` no caso de o dígito de verificação EAN estar errado.
    - `invalid iva` no caso de não existir um código IVA com a designação.
    - `invalid price` no caso de preço não ser um número positivo.
    - `invalid quantity` no caso de a quantidade ser um número negativo.
    - `invalid description` no caso de a descrição exceder a dimensão máxima.
    - `product in use` no caso de se tentar alterar o preço de um produto que esteja incluído no cesto.
    - `invalid product` no caso de o número indicado exceder o número de produtos registáveis

- __l__ - lista produtos:
  - Formato de entrada: `l [ <wildcard> { <wildcard> } ]`
  - Formato de saída: Imprime informação de cada produto com o formato
    `<ean> <iva> <preço> <quantidade-vendida> <quantidade-disponível> <descrição>`

    Imprime todos os produtos disponíveis, ie, com o `stock > 0`, por ordem de criação, caso seja invocado sem argumentos ou com o wildcard ``*``.
    Em qualquer outro caso, imprime os produtos disponíveis cujos códigos EAN são indicados pela ordem que são indicados nos argumentos e, para cada um dos argumentos, pela ordem de criação.
    
    Na descrição dos códigos EAN podem ser utilizados os *wildcards* ``*`` e ``?`` representando qualquer sequência de caracteres ou um caráter, respetivamente. Por exemplo, ``560*``, ``560*23*1``, ``560?????7159?`` ou ``560100994331?``. Assim, ``1*23`` reconhece a sequência ``19865423`` tal como a sequência ``1232323232323``, mas não a sequência ``14233333`` ou ``88765123``.
  - Erros:
    - `<widlcard>: no such product` no caso de não existir nenhum produto com o *wildcard- indicado.

- __a__ - adiciona ao cesto a quantidade do produto indicado:
  - Formato de entrada: `a [ [ <quantidade> ] <ean> ]`
  - Formato de saída: Imprime a informação do produto com o formato
    `<iva> <preço-unitário> <quantidade-total> <preço-total-com-iva> <descrição>`

    O cesto está inicialmente vazio, sem produtos. O stock do produto adicionado deve ser actualizado em conformidade e produtos em rutura de stock não são removidos do sistema.
    - Caso a `<quantidade>` seja omitida deve ser considerada uma unidade.
    - Caso a `<quantidade>` seja negativa deve ser removido o número de unidades do produtos indicado do cesto.
    - Após uma adição/remoção o produto deverá ser sempre listado independentemente do número de unidades resultante.
    - Caso seja invocado sem argumentos é impressa a listagem dos produtos adicionados por ordem crescente de código EAN, sem produtos duplicados.
    Os produtos cuja quantidade seja nula não devem ser apresentados na listagem.

  - Erros:
    - `invalid ean` no caso de o dígito de verificação EAN estar errado.
    - `invalid quantity` no caso de a quantidade ser negativa e não existirem unidades suficientes do produto no cesto.
    - `<ean>: no such product` no caso de não existir nenhum produto com o `<ean>` indicado.
    - `no stock` no caso de não existir quantidade disponível.

- __r__ - resumo de facturação:
  - Formato de entrada: `r [ <ean> ]`
  - Formato de saída:
    - `<quantidade-disponível> <quantidade-vendida> <descrição>` se for invocado com um argumento.
    - `<número-total-de-items-comprados> <número-total-de-facturas-emitidas> <valor-total-facturado>` se for invocado sem argumentos. Neste caso deverá seguir-se a tabela de IVA por ordem alfabética do código e com `%` no valor, uma entrada por linha.
  - Erros:
    - `invalid ean` no caso de o dígito de verificação EAN estar errado.
    - `<ean>: no such product` no caso de não existir nenhum produto com o `<ean>` indicado.

- __f__ - factura os produtos no cesto:
  - Formato de entrada: `f [ [ <nif> ] <nome-cliente> ]`
  - Formato de saída: `<número-de-produtos> <valor-pago> <número-de-factura>`.
    - Caso o NIF seja omitido o valor registado deve ser `999999999`.
    - Caso o `<nome-cliente>` seja omitido deve ser registado o nome `Cliente final`.
    - Caso o nome de cliente seja ``error`` todos os produtos são devolvidos e não é emitida factura.
    - As facturas são numeradas sequencialmente a partir de `1`, não havendo reutilização de números mesmo se entretanto apagadas.
    - Após a facturação sem erros, o cesto fica novamente vazio, sem produtos.
  - Erros:
    - `<nif>: no such nif` no caso de não ser um NIF válido.
    - `invalid name` no caso de não ser um nome de cliente válido.

- __c__ - lista as facturas do sistema:
  - Formato de entrada: `c [ <nome-cliente> ]`
  - Formato de saída:
    - `<número-factura> <valor-facturado> <nome-cliente>` se for invocado sem argumentos. A lista é apresentada por ordem alfabética de nome de cliente e por ordem cronológica de facturação para um mesmo cliente.
    - se for invocado com um argumento, imprime todas as facturas do cliente indicado existentes no sistema, por ordem cronológica de facturação.

  - Erros:
    - `invalid name` no caso de não ser um nome de cliente válido.
    - `<nome-do-cliente>: no such client` no caso de não existir nenhuma factura associada ao cliente indicado.

- __d__ - apaga o registo de um produto ou factura:
  - Formato de entrada: `d <número> [ <quantidade> ]`
  - Formato de saída:
    - `<valor-pago> <nif> <nome-cliente>` se for invocado com um só argumento, a factura com o `<número>` indicado é removida mas os items comprados não são repostos.
    - `<quantidade-restante> <descrição>` se for invocado com dois argumentos, o `<número>` do código EAN e a quantidade. Neste caso a `<quantidade>` é retirada da quantidade disponível do produto, devendo esta ser um número positivo. Se não restar quantidade disponível do produto, este deve ser removido dos produtos registados.
  - Erros:
    - `<número>: no such invoice` no caso não existir uma factura com o número indicado.
    - `invalid ean` no caso de o dígito de verificação EAN estar errado.
    - `<ean>: no such product` no caso de não existir nenhum produto com o `<ean>` indicado.
    - `product in use` no caso de se tentar apagar um produto que esteja incluído no cesto.
    - `invalid quantity` no caso da quantidade não ser um número positivo ou a quantidade exceder o stock.

__Só poderá usar as funções de biblioteca definidas em `stdio.h`,
`stdlib.h`, `ctype.h` e `string.h`__

*Nota importante*: não é permitida a utilização da instrução `goto`, da declaração `extern`,
nem da função `qsort` nativa do C e nenhum destes *nomes* deve aparecer no vosso código.

## Exemplos de utilização dos comandos

### __Comando `p`__

O comando `p` permite introduzir no sistema um novo produto ou actualizar as caraterísticas de um produto existente.

```text
p 8700216266628 D 7.94 100 CHAMPO H&S CLASSIC 625ML
```

### __Comando `l`__

O comando `l` sem argumentos permite listar todas os produtos existentes no sistema.

```text
l
```

O comando `l` com argumentos permite listar apenas os produtos cujos códigos EAN são indicados.

```text
l 5601009* 560131210104? 8700216266628
```

### __Comando `a`__

O comando `a` permite adicionar produtos ao cesto de compras.

```text
a 3 5601009993352
```

### __Comando `r`__

O comando `r` permite imprimir o resumo de facturação

```text
r
```

ou o resumo de facturação de um produto específico

```text
r 8700216266628
```

### __Comando `f`__

O comando `f` permite facturar os produtos adicionados ao cesto de compras.

```text
f "Cliente final"
```

ou indicar o NIF do cliente

```text
f 234567890 João
```

### __Comando `c`__

O comando `c` sem argumentos permite listar todas as facturas existentes no sistema.

```text
c
```

O comando `c` com um argumento permite listar todas as facturas existentes no sistema do cliente indicado.

```text
c Xico
c "João Miguel"
```

### __Comando `d`__

O comando `d` com um argumento permite apagar uma factura.

```text
d 12
```

O comando `d` com dois argumentos permite reduzir as existências, ou mesmo apagar, um produto.

```text
d 8700216266628 21
```

## 4. Compilação e teste

O compilador a utilizar é o `gcc` com as seguintes opções de compilação: `-O3 -Wall -Wextra -Werror -Wno-unused-result`. Para compilar o programa deve executar o seguinte comando:

```text
  $ gcc -O3 -Wall -Wextra -Werror -Wno-unused-result -o proj *.c
```

O programa deverá escrever no *standard output* as respostas aos comandos apresentados no *standard input*. As respostas são igualmente linhas de texto formatadas conforme definido anteriormente neste enunciado. Tenha em atenção ao número de espaços entre elementos do seu output, assim como a ausência de espaços no final de cada linha. Procure respeitar escrupulosamente as indicações dadas.

Ver os exemplos de input e respectivos output na pasta `public-tests/`.

O programa deve ser executado da forma seguinte:

```text
  $ ./proj < test.in > test.myout
```

Posteriormente poderá comparar o seu output (`*.myout`) com o output previsto (`*.out`) usando o comando `diff`,

```text
  $ diff test.out test.myout
```

Para testar o seu programa poderá executar os passos indicados acima ou usar o comando `make` na pasta `public-tests/`.

Para utilizar o _valgrind_ ou um debugger (_gdb_, _ddd_, ...) deverá substituir a opção de compilação `-O3` por `-g`.

## 5. Entrega do Projeto

Será criado um repositório `git` para cada aluno desenvolver e submeter o projeto. Este repositório será criado no [GitLab da RNL](https://gitlab.rnl.tecnico.ulisboa.pt) e será activado quando da publicação deste enunciado.

Na sua submissão do projeto deve considerar os seguinte pontos:

- Considera-se que os seus ficheiros de desenvolvimento do projeto (`.c` e `.h`) estão na raiz do repositório e não numa directoria. *Qualquer ficheiro fora da raíz não será considerado como pertencendo ao seu projeto*.

- A última versão que estiver no repositório da RNL será considerada a submissão para avaliação do projeto. Qualquer versão anterior ou que não esteja no repositório não será considerada na avaliação.

- Antes de fazer qualquer submissão para o repositório da RNL, não se esqueça que deve sempre fazer `pull` para sincronizar o seu repositório local.

- Quando actualizar os ficheiros `.c` e `.h` no seu repositório na RNL, esta versão será avaliada e será informado se essa versão apresenta a resposta esperada num conjunto de casos de teste. Tal como no repositório dos laboratórios, o resultado da avaliação automática será colocado no repositório do aluno.

- Para que o sistema de avaliação seja executado, tem que esperar pelo menos 10 minutos. Sempre que fizer uma actualização no repositório, começa um novo período de espera de 10 minutos. Exemplos de casos de teste serão oportunamente fornecidos.

- Data do checkpoint: __18 de março de 2026, às 19h59m__. Até à data limite poderá efectuar o número de submissões que desejar, sendo utilizada para efeitos de avaliação a última versão. Deverá portanto verificar cuidadosamente que a última versão no repositório GitLab da RNL corresponde à versão do projeto que pretende que seja avaliada. Não existirão excepções a esta regra.

- Data limite de entrega do projeto: __6 de abril de 2026, às 19h59m__. Aplicam-se as mesmas regras que para o checkpoint.

## 6. Avaliação do Projeto

Na avaliação do projeto serão consideradas as seguintes componentes:

1. A primeira componente será feita automaticamente e avalia o desempenho da funcionalidade do programa realizado. Esta componente é avaliada entre 0 e 16 valores. Na data do checkpoint o projeto deverá passar a metade dos testes. Na entrega final poderá recuperar metade da cotação dos não passados no checkpoint. Assim, por exemplo,

- se no checkpoint passar a 10% dos testes e na entrega final a 70% dos testes, então serão contabilizados 10% (checkpoint) + 50% (entrega final) + (70% - 50% - 10%) / 2 = 65%
- se no checkpoint passar a 10% dos testes e na entrega final passar a apenas 55% dos testes, então serão contabilizados 10% (checkpoint) + 45% (entrega final) = 55%
- caso não passe a nenhum teste no checkpoint e a todos os testes na entrega final, então serão contabilizados 0% (checkpoint) + 50% (entrega final) + (100 - 50%) / 2 = 75%.
- caso passe a todos os testes no checkpoint tem a cotação total (100%).

2. A segunda componente avalia a qualidade do código entregue, nomeadamente os seguintes aspectos: comentários, indentação, alocação dinâmica de memória, estruturação, modularidade e divisão em ficheiros, abstracção de dados, entre outros.
Esta componente poderá variar entre -4 valores e +4 valores relativamente à classificação calculada no item anterior e será atribuída posteriormente. Algumas *guidelines* sobre este tópico podem ser encontradas em [guidelines.md](guidelines.md). Esta componente apenas é avaliada na entrega final.

3. Na segunda componente serão utilizadas as ferramentas _lizard_, _valgrind_, e a opção _-fsanitize_ do _gcc_ por forma a detectar a complexidade de código, fugas de memória (“memory leaks”) ou outras incorrecções no código, que serão penalizadas.
Aconselha-se que utilizem estas ferramentas para fazer debugging do código e corrigir eventuais incorrecções, antes da submissão do projecto. Algumas dicas para degugging podem ser encontradas em [debugging.md](debugging.md).

- A classificação da primeira componente da avaliação do projeto é obtida através da execução automática de um conjunto de testes num computador com o sistema operativo GNU/Linux. Torna-se portanto essencial que o código compile correctamente e que respeite o formato de entrada e saída dos dados descrito anteriormente. Projetos que não obedeçam ao formato indicado no enunciado serão penalizados na avaliação automática, podendo, no limite, ter 0 (zero) valores se falharem todos os testes. Os testes considerados para efeitos de avaliação poderão incluir (ou não) os disponibilizados na página da disciplina, além de um conjunto de testes adicionais. A execução de cada programa em cada teste é limitada na quantidade de memória que pode utilizar, e no tempo total disponível para execução, sendo o tempo limite distinto para cada teste.

- Note-se que o facto de um projeto passar com sucesso o conjunto de testes disponibilizado na página da disciplina não implica que esse projeto esteja totalmente correcto. Apenas indica que passou alguns testes com sucesso, mas este conjunto de testes não é exaustivo. É da responsabilidade dos alunos garantir que o código produzido está correcto.

- Em caso algum será disponibilizado qualquer tipo de informação sobre os casos de teste utilizados pelo sistema de avaliação automática. A totalidade dos ficheiros de teste usados na avaliação do projeto serão disponibilizados na página da disciplina após a data de entrega.
