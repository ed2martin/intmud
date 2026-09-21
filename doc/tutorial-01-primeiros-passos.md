# Tutorial do IntMUD: Primeiros Passos

## Sobre o tutorial
O objetivo deste tutorial é explicar os primeiros passos de como criar
programas e jogos usando o IntMUD. Além disso, os *triggers* da base de MUD
(código que responde a eventos dentro do jogo) são escritos nessa mesma
linguagem, por isso ela continua sendo extremamente útil.

Este tutorial não assume experiência prévia com programação. No entanto,
mesmo quem já programa em outras linguagens pode se beneficiar, pois
a linguagem IntMUD tem conceitos simplificados em alguns aspectos,
mas algumas características específicas que merecem atenção.

## Como editar e executar programas
O IntMUD é um interpretador de uma linguagem projetada principalmente para
a criação de jogos do tipo MUD (Multi-User Dungeon). Os programas são escritos
em um editor de texto simples e salvos com a extensão `.int` (por padrão,
`intmud.int`). Depois, basta executar o arquivo `intmud.exe` para ver
o programa funcionando.

O interpretador sempre busca um arquivo de texto com o mesmo nome
do executável. Portanto, se o arquivo `intmud.exe` for renomeado para
`meujogo.exe`, o código-fonte do seu jogo deverá estar no arquivo `meujogo.int`.

## Escolhendo o editor de Texto
Para programar em IntMUD, você deve usar um editor de texto sem formatação. 

* **Recomendados:** Notepad++, Bloco de Notas (Windows) ou Edivox
(do projeto Dosvox).
* **Evite:** Processadores de texto como Microsoft Word ou Wordpad. Além de
adicionarem formatações invisíveis que quebram o código, programas como o Word
bloqueiam o arquivo enquanto ele está aberto, impedindo que o IntMUD consiga
lê-lo na hora de executar o jogo.

### Cuidados importantes ao salvar o arquivo:
1. **Extensão do arquivo:** O Windows, por padrão, oculta as extensões dos
arquivos. Se você usar o Bloco de Notas e não tiver cuidado, ele pode salvar
seu arquivo como `intmud.int.txt`, o que fará com que o jogo não funcione.
Para evitar isso, na hora de salvar, altere o campo "Tipo" para
**"Todos os arquivos (*.*)"**. *(Nota: Usuários do Edivox não precisam
se preocupar com isso).*
2. **Codificação de caracteres:** O IntMUD ainda não tem suporte a UTF-8.
Ao salvar, certifique-se de escolher a codificação **ISO-8859-1** (ou ANSI).

## Organização dos programas
Um programa em IntMUD é formado por várias instruções. No início do arquivo
`intmud.int`, colocamos a configuração do programa (do jogo), seguindo
o padrão `opção = valor`. Essas opções definem se vai usar tela de terminal
ou não, as permissões (o que o programa pode fazer), em quais outros arquivos
contém o programa, etc.

Como essas configurações são lidas apenas na inicialização, elas não podem
ser alteradas pelo programa enquanto ele estiver rodando. Portanto, se você
modificar alguma opção no arquivo de texto, precisará fechar e abrir
o programa novamente para que a mudança tenha efeito. A lista completa de
opções está explicada no arquivo `manual.txt`.

Por enquanto, usaremos apenas a opção `telatxt`. Se quiser que uma janela
de terminal (semelhante ao Prompt de Comando) seja aberta quando o programa
for executado, escreva:
`telatxt = 1`
*(Se essa opção estiver ausente ou for definida como 0, nenhuma janela será aberta).*

Uma observação, a partir da versão 1.17 do IntMUD, se essa linha for omitida,
passa a ser 1 por padrão. Portanto, não é necessário incluí-la para abrir
uma janela de terminal.

Após a lista de opções (configuração do programa), vêm as **classes**.
A grosso modo, uma classe é um tipo de dado por si só e/ou um "molde" dos
objetos. Por exemplo, em um jogo pode existir a classe `p_elfo`, que contém
todas as características desse tipo de personagem.

Já os elfos que realmente aparecem no jogo são os **objetos** (cópias criadas
a partir desse molde). Como os objetos são criados dinamicamente durante
o jogo e podem estar em lugares diferentes, eles não são descritos no arquivo
`intmud.int`.

## Exemplo prático

Crie um arquivo chamado `intmud.int` e cole o código abaixo:

```text
telatxt = 1

classe usuario
telatxt tela

func iniclasse
  criar(arg0)

func ini
  tela.msg("Olá pessoal!\n")

func tela_tecla
  se arg0 == "ESC"
    terminar
  fimse

func tela_msg
  tela.msg("Bom dia\n")
```

Após salvar, execute o `intmud.exe` (ou apenas `intmud` em sistemas não Windows).

**Resultado esperado:** O texto "Olá pessoal!" aparecerá na tela e você
conseguirá mover o cursor com as setas do teclado. Sempre que pressionar
a tecla `ENTER`, a mensagem "Bom dia" será exibida. Pressionando a tecla
`ESC`, o programa é encerrado.

### Solução de Problemas Comuns

#### Texto estranho na tela (ex: `OlÃ¡ pessoal!`)
O arquivo foi salvo em UTF-8.
Solução: Abra o arquivo no editor e salve novamente escolhendo a codificação
**ISO-8859-1** (ou ANSI).

#### Letras sem acentuação
A fonte do terminal não suporta os acentos.
Solução: Pressione `ALT esquerdo + Espaço`, vá em "Propriedades" e mude
a fonte ou o tamanho da letra.

#### O programa não abre
O mais provável é o arquivo ter sido salvo como `.txt`.
Solução: No Bloco de Notas, "Salvar como", escolha o tipo
"Todos os arquivos (*.*)" e salve como `intmud.int`.

### Como fechar o programa
* **Pelo teclado (Windows):** Pressione `ESC` (conforme programado no código)
ou pressione `ALT esquerdo + Espaço`, desça com a seta até "Fechar" (ou aperte
a letra `F`) e pressione `ENTER`.
* **Pelo mouse (Windows):** Clique no botão "X" no canto superior direito
da janela.
* **Em outros sistemas (Linux/Mac):** Como o IntMUD roda direto no terminal,
basta pressionar `CTRL + C` para forçar o encerramento.

### Nota sobre Acessibilidade
O IntMUD é acessível, mas o programa em si não possui síntese de voz nativa.
Deficientes visuais devem usar um leitor de tela, como o **JAWS** ou
o **NVDA**. As setas do teclado funcionam perfeitamente para mudar a posição
do cursor, permitindo que os leitores de tela leiam o conteúdo exibido.

## Entendendo o exemplo prático

Vamos analisar o código do exemplo anterior. Essa é a estrutura mínima
necessária para criar um programa que interage com o usuário.

### 1. Configuração inicial e classes

> `telatxt = 1`

Indica que o programa deve abrir uma janela de terminal. Como vimos antes,
a partir da versão 1.17, essa linha é opcional.

> `classe usuario`

Aqui começamos a definir a classe chamada "usuario". Lembre-se: a classe
é como um "molde" para criar elementos dentro do programa.

> `telatxt tela`

Aqui criamos uma **variável** chamada `tela`, que é do tipo `telatxt`.
Pense em uma variável como uma caixa onde guardamos uma ferramenta;
neste caso, a ferramenta que nos permite interagir com o usuário.

### 2. Inicialização do programa e do objeto

> `func iniclasse`

A palavra `func` serve para criar uma **função** (um bloco de instruções).
A função `iniclasse` é especial: ela é executada automaticamente pelo
IntMUD sempre que a classe é lida no início do programa.

> `criar(arg0)`

Esta instrução cria um objeto real a partir do molde da classe. 
* Dentro da `iniclasse`, a palavra `arg0` guarda automaticamente o nome
  da classe atual. 
* Nós poderíamos escrever `criar("usuario")`, mas usando `arg0` o código
  fica inteligente: se você mudar o nome da classe no futuro, não precisará
  alterar essa linha.

> `func ini`

Esta função é chamada automaticamente sempre que um novo objeto dessa classe
é criado (o que acabamos de fazer na linha anterior com o comando `criar`).
É aqui que definimos o que acontece logo que o objeto é criado.

> `tela.msg("Olá pessoal!\n")`

Esta é a instrução que exibe o texto inicial na tela. Vamos dividi-la para
entender melhor:
* **`tela`**: É a nossa variável.
* **`.msg`**: É a ação (função) que estamos pedindo para a tela executar.
  A variável `tela` possui várias ações (funções) documentadas no `manual.txt`.
* **`("Olá pessoal!\n")`**: É o **argumento**, ou seja, a informação que
  passamos para a ação. 
  * Textos devem estar sempre entre aspas duplas (`" "`), senão o programa
  achará que "Olá pessoal!" é o nome de uma variável.
  * O símbolo **`\n`** no final significa "pular para a próxima linha"
  (Enter). Experimente apagar o `\n` do seu código, rodar o programa e ver
  o que acontece!

### 3. Interação com o teclado

> `func tela_tecla`

Esta função é ativada automaticamente sempre que o usuário pressiona uma
tecla (letras, números, setas).
*Nota: Teclas de controle como Ctrl e Shift sozinhas não ativam essa função.*
**Regra de ouro:** O nome desta função deve ser sempre o nome da sua variável
de tela seguido de `_tecla`. Como nossa variável se chama `tela`, a função
se chama `tela_tecla`.

> `se arg0 == "ESC"`

Aqui o programa toma uma decisão. O `arg0` agora guarda o nome da tecla que
o usuário apertou. 
* **Atenção aos sinais:** Usamos `==` (dois sinais de igual) para **comparar**
  se a tecla apertada foi o "ESC". Na programação, usar apenas um `=` serve
  para atribuir um valor a uma variável, ou seja, guardar um valor dentro dela
  para usar depois.

> `terminar`

Se a tecla for "ESC", esta linha encerra o programa. Como é uma instrução
de controle, ela deve ficar sozinha em sua própria linha.

> `fimse`

Todo bloco que começa com um `se` precisa terminar com um `fimse`. Se
o usuário apertar qualquer outra tecla que não seja "ESC", o programa
ignora o comando `terminar` e pula direto para este `fimse`.

### 4. Interação com texto (mensagens)

> `func tela_msg`

Esta função é ativada quando o usuário digita um texto e pressiona `ENTER`.
Seguindo a mesma regra anterior, o nome é a junção da variável (`tela`)
com `_msg`.

> `tela.msg("Bom dia\n")`

Como vimos anteriormente, esta instrução usa a ação `.msg` da nossa variável
`tela` para exibir um texto. Neste caso, sempre que o usuário apertar ENTER,
o programa responderá exibindo "Bom dia" na tela, pulando para a próxima
linha em seguida.

## Exemplo 2: Funções, Argumentos e Múltiplos Objetos

Neste segundo exemplo, vamos entender melhor como funcionam os objetos e como
podemos criar nossas próprias funções. Altere o arquivo `intmud.int` para que
ele fique assim:

```text
telatxt = 1

classe usuario
telatxt tela

func msg
  tela.msg(arg0 + "\n")

func iniclasse
  criar(arg0)
  criar(arg0)

func ini
  msg("Olá pessoal!")

func tela_tecla
  se arg0 == "ESC"
    terminar
  fimse

func tela_msg
  msg("Bom dia " + arg0)
```

Após salvar, execute o `intmud.exe` (ou apenas `intmud` em sistemas não Windows).

**Resultado esperado:** 
* O texto "Olá pessoal!" aparecerá **duas vezes** na tela.
* Ao pressionar a tecla `ENTER`, será exibida duas vezes a mensagem "Bom dia".
* Se você digitar alguma coisa (por exemplo, "Maria") e pressionar `ENTER`,
  aparecerá na tela a mensagem "Bom dia Maria" (também duas vezes).
* O que não mudou: a tecla `ESC` continua encerrando o programa e as setas
  do teclado movem o cursor.

## Entendendo o exemplo mais completo

Vamos analisar as novidades deste código:

### 1. Criando uma função própria

> `func msg`

Até agora, usamos funções que o IntMUD chama automaticamente (como `ini` ou
`tela_tecla`). Aqui, criamos uma **função auxiliar própria** para enviar
mensagens. O nome poderia ser qualquer um (como `enviar_texto`), mas `msg`
pareceu mais apropriado.

> `tela.msg(arg0 + "\n")`

O `arg0` é sempre o **primeiro argumento** que uma função recebe. Neste caso,
ele guarda o texto que queremos exibir. 
* Note o uso do sinal `+`. Na programação, quando usamos `+` com textos, ele
  serve para **juntar** (concatenar) as partes. Estamos pegando o texto
  recebido (`arg0`) e juntando com a quebra de linha (`"\n"`). 
* Em seguida, chamamos a função `tela.msg` para enviar essa mensagem completa
  para a tela.

### 2. Usando a nova função

> `func ini`
>   `msg("Olá pessoal!")`

Dentro da função `ini`, estamos chamando a nossa nova função `msg` e passando
como argumento o texto `"Olá pessoal!"`. Quando o programa pula lá para a
`func msg`, esse texto é guardado dentro do `arg0`.

### 3. Múltiplos Objetos

> `func iniclasse`
>   `criar(arg0)`
>   `criar(arg0)`

Nessa função, colocamos a instrução `criar(arg0)` duas vezes. Lembra que
a classe é um "molde"? Ao fazer isso, estamos criando
**dois objetos independentes** a partir do mesmo molde. 
É por isso que a mensagem "Olá pessoal!" da `func ini` é exibida duas vezes:
cada um dos dois objetos "nasceu" e executou a sua própria função `ini`.

### 4. Interagindo com o que o usuário digita

> `func tela_msg`
>   `msg("Bom dia " + arg0)`

Aqui vemos algo muito importante:
**o significado de `arg0` depende de onde ele está**. 
* Dentro da função `tela_msg`, o `arg0` guarda automaticamente
  **o texto que o usuário digitou** antes de pressionar ENTER. 
* Se o usuário digitou "amigos", o programa vai juntar (concatenar) o texto
  `"Bom dia "` com `"amigos"`. 
* Em seguida, ele chama a nossa função `msg` passando `"Bom dia amigos"`
  como argumento.

Novamente, o motivo de aparecerem duas mensagens iguais cada vez que
a tecla ENTER é pressionada é porque existem dois objetos idênticos
"escutando" o teclado e reagindo um após o outro. Nota, o IntMUD não
executa (chama) duas funções ao mesmo tempo. É sempre uma, e quando
retornar (terminar), vai para a próxima.

## Exemplo 3: O clássico "Olá Mundo"

Vamos simplificar. Altere o arquivo `intmud.int` para ficar assim:

```text
classe usuario

func iniclasse
  telatxt tela
  tela.msg("Olá mundo!\n")
```

Após salvar, execute o `intmud.exe` (ou apenas `intmud` em sistemas não Windows).

**Resultado esperado:** O texto "Olá mundo!" aparecerá apenas uma vez na tela.
Desta vez, as teclas `ENTER` e `ESC` não terão efeito, mas as setas do teclado
continuarão funcionando normalmente para mover o cursor.

Como a tecla `ESC` não foi programada para fechar o programa neste exemplo,
é preciso fechá-lo manualmente:
* **No Windows:** Clique no botão "X" no canto superior direito da janela ou,
  pelo teclado, pressione `ALT esquerdo + Espaço`, desça com a seta
  até "Fechar" (ou aperte a letra `F`) e pressione `ENTER`.
* **Em outros sistemas:** Basta pressionar `CTRL + C` no terminal.

## Entendendo o exemplo clássico

Vamos analisar o que esse código tem de diferente.

### 1. Variáveis Locais e Ausência de Objetos

> `func iniclasse`
>   `telatxt tela`
>   `tela.msg("Olá mundo!\n")`

Dessa vez, colocamos tudo dentro da `func iniclasse`. Nós criamos a variável
`tela` (do tipo `telatxt`) diretamente dentro da função e já a usamos para
enviar a mensagem. 
* Como a variável foi criada *dentro* da função, assim que a função termina
  de rodar, essa variável deixa de existir. Isso se chama escopo da variável,
  ou seja, o contexto onde ela foi definida e pode ser usada.
* Note que não usamos o comando `criar(arg0)`. Ou seja, a classe foi lida,
  a mensagem foi exibida, mas nenhum objeto foi criado.

### 2. Comportamento nativo vs. programado

O programa não reage às teclas `ENTER` e `ESC` simplesmente porque não
escrevemos as funções `tela_msg` e `tela_tecla` para lidar com elas.
No entanto, o comportamento das setas do teclado continua funcionando porque
essa é uma característica **nativa** (padrão) do IntMUD quando uma janela
de texto está aberta.

### 3. Tipos de classes no IntMUD

Na programação em geral, costuma-se existir três tipos de classes em relação
à quantidade de objetos, e o MUD não é diferente:

* **Classes com vários objetos:** Como personagens, itens e efeitos. Você cria
  um "molde" e gera várias cópias dele espalhadas pelo jogo.
* **Classes com um único objeto (singleton):** Classes que gerenciam coisas
  únicas, como os comandos dos jogadores. Só precisamos de um objeto desse tipo.
* **Classes sem objetos (estáticas):** Como vimos neste exemplo, algumas
  classes não precisam ter objetos. Um exemplo real na base de MUD (escrita
  na linguagem do IntMUD) é a classe `misc`. Como não há objetos, para usar
  as funções dela, é necessário chamar com o nome da classe seguido de dois
  pontos, como `misc:tempo` ou `misc:evento`.

## Conclusão

Você aprendeu neste tutorial:

* A estrutura mínima de um programa IntMUD
* Como criar e inicializar objetos
* Como interagir com o teclado e com o usuário
* Como criar funções simples
* Os três padrões de classes: múltiplos objetos, singleton e estáticas
