# Tutorial do IntMUD: Variáveis e controle de fluxo

## Sobre este arquivo
Partimos do princípio de que você já sabe como editar um arquivo do programa,
salvar como `.int`, executar o programa e fechar. Isso é explicado no arquivo
anterior.

O objetivo agora é lidar com variáveis numéricas e variáveis que armazenam
texto, e lidar com instruções de controle de fluxo, exceto laço de repetição.
Para quem vem de outras linguagens, a lógica de programação é muito parecida
com a da linguagem C, porém a sintaxe é um pouco diferente e o IntMUD não faz
distinção entre letras maiúsculas, minúsculas e acentuadas nos nomes de
classes, variáveis, funções e instruções e ao comparar textos.

## Fluxo

O IntMUD executando uma função é semelhante a uma pessoa lendo um livro de
romance: vai da primeira a última linha, em sequência. Quando encontra uma
palavra com uma nota de rodapé, interrompe a leitura, lê a nota e continua
de onde parou. Isso é o equivalente a chamar uma função. Estamos falando
aqui de leitores muito comportados, não de leitores que, por exemplo, vão dar
uma olhada no final do livro antecipadamente.

Com instruções de controle de fluxo, a leitura se assemelha mais a um livro
de RPG. Então, por exemplo, no final da página 3, você pode encontrar algo
assim:

> Você se depara com uma porta grande, que se destaca no ambiente. Se quiser
> abri-la, vá para a página 5. Se achar mais seguro espiar pela fechadura, vá
> para a página 10. E se quiser ignorá-la por enquanto, continue a leitura.

## Exemplo com se-senao-fimse

Altere o arquivo `intmud.int` para ficar assim:

```IntMUD
classe atendimento
telatxt tela
const msg = tela.msg(arg0 + "\n")
const iniclasse = criar(arg0)

func apresentacao
  msg("Se estiver com dor de dente, tecle 1 e Enter")
  msg("Se quebrou a perna, tecle 2 e Enter")
  msg("Se quebrou o braço, tecle 3 e Enter")
  msg("Se estiver com fome, tecle 4 e Enter")
  msg("Ou se estiver com sede, tecle 5 e Enter")
  msg("Se quiser falar com um de nossos atendentes, tecle 6 e Enter")

func ini
  msg("Olá, estou aqui para orientar você. Agora me diz, qual é o seu problema?")
  apresentacao

func tela_msg
  se arg0 == "1"
    msg("Dor de dente? Procure um dentista.")
  senao arg0 == "2" || arg0 == "3"
    msg("Quebrou a perna ou o braço? Chame uma ambulância.")
  senao arg0 == "4"
    msg("Está com fome? Coma alguma coisa.")
  senao arg0 == "5"
    msg("Está com sede? Beba água.")
  senao arg0 == "6"
    msg("No momento todos os nossos atendentes estão ocupados.")
    msg("No entanto, você pode teclar outra opção de 1 a 5.")
  senao
    msg("Essa opção eu não conheço: " + arg0)
    apresentacao
    ret
  fimse
  msg("Tecle outra opção, de 1 a 5.")
```

**Resultado esperado:**
* Ao rodar o programa, vem a lista de opções.
* Teclando um número de 1 a 6 (e dando Enter), vem uma resposta.
* Teclando qualquer outra coisa (e Enter), vem a lista de opções.

## Entendendo o exemplo com se-senao-fimse

```IntMUD
const msg = tela.msg(arg0 + "\n")
```

É uma forma compacta (uma linha) de escrever a `func msg` que contém uma linha,
`tela.msg(arg0 + "\n")`. Embora constante (const) dê a ideia de algo imutável,
pequenas funções, sem instruções de controle de fluxo, também podem ser
escritas dessa forma.

```IntMUD
const iniclasse = criar(arg0)
```

Idem, para a `func iniclasse`, que tinha uma linha `criar(arg0)`.

```IntMUD
func apresentacao
```

A única coisa que essa função faz é mostrar na tela as opções disponíveis.

```IntMUD
func ini
  msg("Olá, estou aqui para orientar você. Agora me diz, qual é o seu problema?")
  apresentacao
```

Quando o objeto é criado, envia uma mensagem e chama a função `apresentacao`,
que mostra a lista de opções.

```IntMUD
func tela_msg
```

Quando o usuário digita algo e tecla Enter, essa função é chamada, como visto
no arquivo anterior do tutorial. Depois disso vem uma série instruções
se-senao-fimse, testando o que o usuário digitou e enviando a mensagem
correspondente.

```IntMUD
  se arg0 == "1"
    msg("Procure um dentista.")
```

Verifica se teclou 1, e se teclou, envia mensagem e vai para o `fimse`.
Depois do `fimse` é enviada uma mensagem, a linha
`msg("Tecle outra opção, de 1 a 5.")`.

```IntMUD
  senao arg0 == "2" || arg0 == "3"
    msg("Quebrou a perna ou o braço? Chame uma ambulância.")
```

O usuário não teclou 1. Agora verifica se teclou 2 ou se teclou 3, e em caso
afirmativo, envia outra mensagem e vai para o `fimse`. O operador `||`
significa "ou". Existe outro operador importante a conhecer, mas que não é
usado aqui, `&&` significa "e" (quando as duas condições precisam ser
verdadeiras).

```IntMUD
  senao
    msg("Essa opção eu não conheço: " + arg0)
    apresentacao
    ret
```

Se nenhuma das condições anteriores for válida (o usuário não teclou um número
de 1 a 6), o programa vai para um `senao` sem argumentos. E quando não houver,
vai direto para o `fimse` correspondente.

Nesse caso, exibe uma mensagem, chama a função `apresentacao` e retorna da
função (`ret` significa retornar). Portanto, não é enviada a mensagem
"Tecle outra opção, de 1 a 5.".

Uma nota para quem já programou antes: em linguagens como C, Java ou
Javascript, a instrução `senao` corresponde a duas construções:
* Quando vem acompanhado de um teste (ex: senao arg0 == "2"), ela funciona
  exatamente como o else if.
* Quando aparece sozinho (no final do bloco), atua como um else tradicional.

## Usando casovar e casose

No exemplo anterior, nós testamos a mesma variável (`arg0`) várias vezes
usando `se` e `senao`. Quando é preciso verificar o valor de uma única variável
contra várias opções diferentes, existe outra estrutura para isso,
particularmente útil quando forem muitas opções (nesse caso são poucas),
que vale a pena aprender.

Mude a `func tela_msg` para ficar assim:

```IntMUD
func tela_msg
  casovar arg0
  casose "1"
    msg("Dor de dente? Procure um dentista.")
    sair
  casose "2"
  casose "3"
    msg("Quebrou a perna ou o braço? Chame uma ambulância.")
    sair
  casose "4"
    msg("Está com fome? Coma alguma coisa.")
    sair
  casose "5"
    msg("Está com sede? Beba água.")
    sair
  casose "6"
    msg("No momento todos os nossos atendentes estão ocupados.")
    msg("No entanto, você pode teclar outra opção de 1 a 5.")
    sair
  casose
    msg("Essa opção eu não conheço: " + arg0)
    apresentacao
    ret
  casofim
  msg("Tecle outra opção, de 1 a 5.")
```

**O que mudou**

* Essa estrutura começa com `casovar` e termina com `casofim`.
* `casovar arg0`: Avisa ao programa "olhe para o que tem dentro do arg0".
* `casose "1"`: É o mesmo que perguntar "o valor é 1?".
* A instrução sair: Quando o IntMUD entra em um casose, ele continua executando
  todas as linhas abaixo dele até encontrar a instrução sair (que faz ele pular
  para o casofim). Um erro muito comum é esquecer de colocar a instrução `sair`.
* Note que empilhamos o casose "2" e o casose "3". Como não há um sair entre
  eles, ao teclar 2, o programa "escorrega" para o caso 3.
* O casose sozinho (quando presente) funciona como um senao sozinho:
  se não for nenhuma das opções, executa o que está neste bloco. No entanto,
  o casose sem argumentos não precisa ser necessariamente a última opção
  do bloco casovar.

## Variáveis numéricas

Existem 9 tipos de variáveis cuja única finalidade é guardar números.
A diferença entre elas está na faixa de valores e na precisão.

São elas:
* int1: Pode ser 0 ou 1.
* int8: Número inteiro de -128 a 127.
* int16: Número inteiro de -32768 a 32767.
* int32: Número inteiro de -2147483648 a 2147483647.
* uint8: Número inteiro de 0 a 255.
* uint16: Número inteiro de 0 a 65535.
* uint32: Número inteiro de 0 a 4294967295.
* real: Ponto flutuante de precisão simples.
  Pode representar números na faixa de 10 elevado a -38 (menor que isso
  é considerado zero) até 10 elevado a 38 (maior que isso é considerado
  infinito), tanto positivos quanto negativos. A precisão é de 6 dígitos.
  Ocupa o mesmo espaço na memória que uma variável `int32` e corresponde
  ao tipo `float` em C.
* real2: Ponto flutuante de dupla precisão.
  Pode representar números na faixa de 10 elevado a -308 (menor que isso
  é considerado zero) até 10 elevado a 308 (maior que isso é considerado
  infinito), tanto positivos quanto negativos. A precisão é de 15 dígitos.
  Ocupa o mesmo espaço na memória que duas variáveis `int32` e corresponde
  ao tipo `double` em C.
  
A regra geral é usar o tipo de variável mais simples que resolve o problema.
Isso significa dar preferência a inteiro (só usar ponto flutuante
se necessário) e procurar inteiro de menor valor. O princípio aqui é
ganho de desempenho e menor uso de memória. Se depois comprovar-se
que o tipo escolhido não atende as necessidades, basta mudar para
outro tipo.

## Exemplo prático com variáveis numéricas

```IntMUD  
classe dados
telatxt tela
const msg = tela.msg(arg0 + "\n")
const iniclasse = criar(arg0)

func ini
  msg("Vou rolar três dados. Mas, quantas faces eles têm mesmo?")
  msg("Digite o número de faces e tecle ENTER")

func tela_msg
  uint8 dado1 = rand(1, arg0)
  uint8 dado2 = rand(1, arg0)
  uint8 dado3 = rand(1, arg0)
  msg("Rolei 3 dados de " + arg0 + " faces e o resultado foi:")
  msg("Dado 1 deu " + dado1)
  msg("Dado 2 deu " + dado2)
  msg("Dado 3 deu " + dado3)
  msg("Somando o dado 1 com o 2, deu " + (dado1 + dado2))
  msg("Somando os 3 dados deu " + (dado1 + dado2 + dado3))
  msg("E a média dos 3 dados foi " + (dado1 + dado2 + dado3) / 3)
  msg("Digite o número de faces e tecle ENTER")
```

**Resultado esperado:**
* Ao rodar o programa, ele pergunta quantas faces.
* Você digita, por exemplo, 6 (e tecla ENTER), e vem uma resposta desse tipo:

```text  
Rolei 3 dados de 6 faces e o resultado foi:
Dado 1 deu 4
Dado 2 deu 3
Dado 3 deu 6
Somando o dado 1 com o 2, deu 7
Somando os 3 dados deu 13
E a média dos 3 dados foi 4.333333333
Digite o número de faces e tecle ENTER
```

## Entendendo o programa

Ao digitar um número e teclar ENTER, a func tela_msg é executada, e a primeira
coisa que acontece é sortear os 3 números. Essas 3 linhas:

```IntMUD
  uint8 dado1 = rand(1, arg0)
  uint8 dado2 = rand(1, arg0)
  uint8 dado3 = rand(1, arg0)
```

Essa construção declara e inicializa variáveis, tudo na mesma linha.
`uint8` é o tipo (inteiro de 0 a 255), dado1 é o nome da variável e
`rand(1, arg0)` vira um número aleatório de 1 até o que está em arg0.

Pode-se testar a função `rand` de dentro do próprio MUD. É só entrar como
administrador e teclar `cmd rand(1,6)`, por exemplo. Nesse caso, vai
retornar um número aleatório de 1 a 6. Cada vez que for chamado, pode
resultar em um número diferente.

Declarar e atribuir um valor poupa algumas linhas. Separando declarar
e atribuir, o código ficaria assim:

```IntMUD
  uint8 dado1
  uint8 dado2
  uint8 dado3
  dado1 = rand(1, arg0)
  dado2 = rand(1, arg0)
  dado3 = rand(1, arg0)
```

Quando forem variáveis da classe (nesse caso, são variáveis da função),
a sintaxe de declarar e atribuir na mesma linha não é permitida.
Nesse caso, declara-se a variável na classe e inicializa-se na `func ini`.

E se o usuário entrar com números grandes, por exemplo 10000? Nesse
caso, a resposta vai ser algo parecido a isso:

```text
Rolei 3 dados de 1000 faces e o resultado foi:
Dado 1 deu 51
Dado 2 deu 255
Dado 3 deu 235
Somando o dado 1 com o 2, deu 306
Somando os 3 dados deu 541
E a média dos 3 dados foi 180.333333333
Digite o número de faces e tecle ENTER
```

E o motivo de predominar 255 é que rand(1,10000) vai gerar um número aleatório
que, quando for guardado na variável, vai ser truncado. Por exemplo, fazer
`uint8 dado1 = 500`, tem o mesmo efeito de `uint8 dado1 = 255` porque uint8
vai de 0 a 255.

Outro caso interessante é digitando 1. O resultado vai ser sempre 3 dados com
o valor 1. Mas no caso do usuário digitar 0, os dados vão variar entre os
valores 0 e 1. Isso acontece porque o IntMUD trata `rand(1,0)` como se fosse
`rand(0,1)`. E se digitar algo que não é um número, o efeito é o mesmo que
se tivesse digitado 0, porque o IntMUD assume que é 0 quando não consegue
converter o texto para número.

## Testando algumas possibilidades com o exemplo das variáveis

Onde é mostrado o resultado do dado 1 somado ao dado 2, a seguinte linha:

```IntMUD
msg("Somando o dado 1 com o 2, deu " + (dado1 + dado2))
```
E se tirar os parênteses em `(dado1 + dado2)`? O resultado é que vai juntar
o texto com o dado1 e depois com dado2. Exemplo, se o primeiro dado deu
4 e o segundo deu 6, resultaria na frase "Somando o dado 1 com o 2, deu 46".

E se você esquecer de inicializar uma variável? Exemplo, na linha que tem
`uint8 dado3 = rand(1, arg0)`, mudar para apenas `uint8 dado3`?
O programa vai dizer que o dado 3 deu 0, porque as variáveis numéricas
são inicialmente 0, até que se atribua um valor a elas.

Frequentemente, a média dos 3 dados dá um número quebrado (número com
dígitos após a vírgula). E se quisermos que seja um número inteiro?
Podemos resolver isso atribuindo a média a outra variável do tipo inteiro.
O resultado será arredondado para caber na variável. Essas duas linhas:

```IntMUD
uint8 media = (dado1 + dado2 + dado3) / 3
msg("E a média dos 3 dados foi " + media)
```

Mas há uma outra solução, mais simples nesse caso. A função `int` converte
para inteiro. A linha que mostra a média fica assim:

```IntMUD
msg("E a média dos 3 dados foi " + int((dado1 + dado2 + dado3) / 3))
```

O programa dá o resultado dos dados, um em cada linha. E se quiser mostrar
tudo na mesma linha? Algo como:

```text
Rolei 3 dados de 6 faces e o resultado foi:
4, 3 e 6
```

Nesse caso, troca-se as 3 linhas que mostram o resultado dos dados por:

```IntMUD
  msg("" + dado1 + ", " + dado2 + " e " + dado3)
```

Se tirar o texto vazio no começo, ficando apenas
`msg(dado1 + ", " + dado2 + " e " + dado3)`, ele vai simplesmente somar
`dado1` com `", "`, o que vai resultar no valor de `dado1`. O IntMUD olha
para `dado1` e percebe que é um número. Então ele entende que é uma soma
numérica, não concatenação (junção) de textos. Como o IntMUD trata textos
que não consegue converter como se o número fosse 0, ele vai somar os
valores dos dados com 0.

Supondo que os dados deram 4, 3 e 6, a soma vai dar 13, e o resultado vai
ser equivalente a `msg(13)`. E existe mais uma consequência, na função
`msg`, vai acontecer `tela.msg(13 + "\n")`, que resulta em `tela.msg(13)`.
Ou seja, o caractere `"\n"`, que faz passar para a próxima linha, não vai
para a tela e o usuário vai ver algo como:

```text  
Rolei 3 dados de 6 faces e o resultado foi:
13Somando o dado 1 com o 2, deu 7
Somando os 3 dados deu 13
E a média dos 3 dados foi 4.333333333
Digite o número de faces e tecle ENTER
```

Se não quiser colocar o texto vazio no começo, existe uma outra solução,
que é passar para texto, a função `txt`. A linha fica assim:

```IntMUD
  msg(txt(dado1) + ", " + dado2 + " e " + dado3)
```

## Variáveis de texto

Vamos usar aqui variáveis de texto de capacidade fixa, de 1 a 512 caracteres.
Quando é necessário mais que isso, pode-se usar `textotxt`, que possui algumas
funcionalidades básicas de um editor de texto, e por isso, é um pouco mais
complexo. Textotxt é assunto para outro arquivo do tutorial.

Para definir uma variável de texto, escreve-se txt seguido da quantidade
de caracteres. Exemplo, em um `txt10` cabe um texto de até 10 caracteres.

## Exemplo prático com variáveis de texto

```IntMUD
classe personagem
telatxt tela
txt20 nome # Nome do personagem
txt20 tipo1 # Nome da raça
txt20 tipo2 # Nome da classe
uint16 pvidamax # Pontos de vida máximos
uint16 pmanamax # Pontos de mana máximos
uint8 passo # Em qual etapa está (o que está pedindo para o usuário)
const msg = tela.msg(txt(arg0) + "\n")
const iniclasse = criar(arg0)

func ini
  avisomsg

func avisomsg # Pede o nome, a raça ou a classe do personagem
  se passo == 0
    msg("Qual o nome do personagem?")
  senao passo == 1
    msg("Qual a raça? Tecle anão, humano ou elfo")
  senao
    msg("Qual a classe? Tecle guerreiro, mago ou ladino")
  fimse

func tela_msg
  se passo == 0
    se arg0 != ""
      nome = arg0
      passo = 1
    fimse
  senao passo == 1
    se arg0 == "anão" || arg0 == "humano" || arg0 == "elfo"
      tipo1 = arg0
      passo = 2
    senao
      msg("Essa raça eu não conheço")
    fimse
  senao
    se arg0 == "guerreiro" || arg0 == "mago" || arg0 == "ladino"
      tipo2 = arg0
      passo = 0
      recalcula
      mostraficha
    senao
      msg("Essa classe eu não conheço")
    fimse
  fimse
  avisomsg

func recalcula # Recalcula alguns atributos do personagem
  pvidamax = 20
  pmanamax = 20
  se tipo1 == "anão"
    pvidamax = pvidamax + 20
  senao tipo1 == "humano"
    pvidamax = pvidamax + 10
    pmanamax = pmanamax + 10
  senao tipo1 == "elfo"
    pmanamax = pmanamax + 20
  fimse
  se tipo2 == "guerreiro"
    pvidamax = pvidamax + 40
  senao tipo2 == "mago"
    pmanamax = pmanamax + 40
  senao tipo2 == "ladino"
    pvidamax = pvidamax + 20
    pmanamax = pmanamax + 20
  fimse

func mostraficha
  msg("FICHA TÉCNICA")
  msg("Nome do personagem: " + nome)
  msg("Raça: " + tipo1)
  msg("Classe: " + tipo2)
  msg("Pontos máximos de vida: " + pvidamax)
  msg("Pontos máximos de mana: " + pmanamax)
```

**Resultado esperado:**

Ao rodar o programa, ele pergunta:
> Qual o nome do personagem?

Você digita um nome qualquer (sempre pressionando ENTER), exemplo, Aethelion,
e vem a pergunta seguinte:
> Qual a raça? Tecle anão, humano ou elfo

Se digitar qualquer outra coisa, vem as mensagens:
> Essa raça eu não conheço

> Qual a raça? Tecle anão, humano ou elfo

Então você digita, por exemplo, elfo.
> Qual a classe? Tecle guerreiro, mago ou ladino

Digamos que tenha digitado ladino. Agora vem a ficha completa.

```text
FICHA TÉCNICA
Nome do personagem: Aethelion
Raça: elfo
Classe: ladino
Pontos máximos de vida: 40
Pontos máximos de mana: 60
Qual o nome do personagem?
```

## Entendendo o programa

A primeira coisa que se nota são os comentários. Quando houver
o símbolo # no programa, o IntMUD ignora tudo que estiver daí em diante.
Comentários são escritos para outras pessoas que forem ler o código,
e eliminá-los produz impacto desprezível na eficiência do programa.

Outro ponto importante é a divisão de responsabilidades:
* As variáveis da ficha do personagem estão na classe personagem
* A `func avisomsg` envia mensagem com a próxima pergunta para o usuário
* A `func recalcula` calcula alguns atributos do personagem
* A `func mostraficha` mostra a ficha do personagem para o usuário
* A `func tela_msg` obtém informações do usuário e preenche a ficha

Houve também uma mudança na `const msg`, que agora ficou assim:
`const msg = tela.msg(txt(arg0) + "\n")`. No exemplo anterior, se fosse
passado um número, por exemplo, com `msg(13)`, a `const msg` mostrava
o número sem passar para a próxima linha. Essa versão corrige esse
problema convertendo o que foi passado para texto.

Outra característica interessante é que tanto faz digitar, por exemplo,
na classe, "elfo", "Elfo" ou mesmo "Élfo". Isso funciona porque, como
foi dito anteriormente, IntMUD não faz distinção entre letras maiúsculas,
minúsculas e acentuadas ao comparar textos. Vale lembrar que isso também
vale para os nomes de classes, variáveis, funções e instruções.

Quando é necessário fazer a distinção, usa-se `===` ao invés de `==`.
Exemplo, se mudar a linha
`se arg0 == "anão" || arg0 == "humano" || arg0 == "elfo"`
para `se arg0 === "anão" || arg0 === "humano" || arg0 === "elfo"`,
o usuário é obrigado a digitar o nome tal qual foi definido. Nesse
caso, digitar "Elfo" já não aceita a raça. Notar que a única mudança
foi no operador, três sinais de iguais ao invés de dois.

Ao verificar o nome, usamos a linha `se arg0 != ""`. Esse operador novo, `!=`
é exatamente o oposto do `==`, ou seja, estamos verificando se o arg0 não é
um texto vazio. Nesse contexto, o resultado é que se o usuário apenas
pressionar ENTER sem digitar nada, pergunta o nome de novo.

O que acontece se o usuário digitar um nome grande, que passa de 20
caracteres? Exemplo, "Aethelion, o elfo das montanhas sombrias".
Nesse caso, o nome será truncado (cortado) nos primeiros 20 caracteres
(incluindo espaços), porque `txt20 nome` só comporta 20 caracteres,
e aparecerá na ficha da seguinte forma:

```text
Nome do personagem: Aethelion, o elfo da
```

Há duas otimizações que pode ser feitas em linhas como essas:

```IntMUD
    pvidamax = pvidamax + 10
    pmanamax = pmanamax + 10
```

Na primeira, pega-se o conteúdo da variável pvidamax, soma-se 10 e guarda
novamente em pvidamax. O resultado é somar 10 na variável. Por exemplo,
se ela estava com 20, passa para 30. Isso funciona, mas existe o operador
`+=`, que pode facilitar essa operação. Essas duas linhas podem ser
escritas da seguinte forma:

```IntMUD
    pvidamax += 10
    pmanamax += 10
```

Essa sintaxe funciona com diversos operadores. Exemplo, se quisesse
subtrair 10 de pvidamax ao invés de somar, seria `pvidamax -= 10`.

E a segunda otimização, muito usada no código interno do MUD, mas geralmente
evitado em exemplos, é colocar tudo em uma linha, separando por vírgulas.
Essas duas linhas podem ser reduzidas em `pvidamax += 10, pmanamax += 10`.
Isso funciona quando as linhas não são de instruções de controle de fluxo.

## Entendendo a variável passo

Em diversas linguagens, é comum chamar uma função que espera o usuário digitar
algo e pressionar ENTER, e devolve o resultado (o que foi digitado).
Nesse caso, enquanto o usuário não pressionar ENTER, o programa fica travado
(congelado) nessa função. Por exemplo, em Python é algo assim:

```Python
texto = input("Digite algo e pressione ENTER: ")
print("Você digitou:", texto)
``` 

O IntMUD segue outro princípio: não se pode ficar esperando algo acontecer,
quando esse algo pode ser demorado. Ao invés disso, ele avisa, chamando uma
função, quando algo acontecer. Isso é **programação orientada a eventos**.
É dessa forma que consegue lidar com vários usuários no MUD, em paralelo,
bem como personagens não jogadores.

Como não temos funções bloqueantes, usa-se uma variável para marcar em que
etapa o programa está. Isso se chama "máquina de estados".

No exemplo, a variável `passo` pode ter os seguintes valores:
* passo = 0 se está esperando o nome
* passo = 1 se está esperando a raça
* passo = 2 se está esperando a classe

A `func avisomsg` envia uma mensagem diferente conforme a variável `passo`.
A `func tela_msg` verifica o que o usuário digitou, também conforme a
variável `passo`, atualiza essa variável, indo para a próxima etapa se
o usuário não digitou nome/raça/classe inválido, e chama `avisomsg` em seguida.

## Paralelo entre o exemplo e o MUD

Os nomes das variáveis não foram escolhidos por acaso. Experimente teclar
no MUD, como administrador(a):

```MUD
cmd nome
cmd tipo1
cmd tipo2
cmd pvidamax
cmd pmanamax
```

No entanto, alterar essas variáveis é outra história. Para alterar o nome,
use o comando de administração `mudanome`. Já `pvidamax` e `pmanamax` são o
resultado dos atributos do personagem, o que ele está vestindo, empunhando,
os efeitos, etc. O que pode ser alterado diretamente são `tipo1` e `tipo2`,
teclando, por exemplo, `cmd tipo1="Elfo",recalc++`. O `recalc++`, no MUD,
faz recalcular os atributos do personagem.

## Conclusão

Você aprendeu neste tutorial:

* O que é fluxo de controle e por que é importante em programação
* Como usar `se`, `senao` e `fimse` para tomar decisões no código
* Operadores de comparação (`==`, `===`, `!=`)
* Operadores lógicos (`&&` e `||`) para combinar múltiplas condições
* A estrutura `casovar` para organizar múltiplas escolhas de forma legível
* O que são variáveis e como armazenam dados
* Os tipos de variáveis numéricas
* Quando usar cada tipo de variável numérica (princípio do tipo mais simples)
* Como trabalhar com variáveis de texto e a função `txt()`
* Por que IntMUD usa orientação a eventos em vez de funções bloqueantes
* O padrão de máquina de estados usando a variável `passo`
* Como estruturar um programa pequeno com múltiplas funções trabalhando juntas
