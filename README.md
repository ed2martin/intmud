# IntMUD: Plataforma para criação de jogos de texto

[![Licença: LGPL v2.1](https://img.shields.io/badge/License-LGPL_v2.1-blue.svg)](https://www.gnu.org/licenses/lgpl-2.1)

O objetivo principal do **IntMUD** (*Interpretador MUD*) é a criação de jogos
de texto que consistem em um mundo virtual onde jogadores podem interagir
entre si, ou seja, **MUD** (*Multi-User Dungeons*). Entretanto, outros tipos
de jogos baseados em texto também é perfeitamente possível.

Música e efeitos sonoros são amplamente suportados na base de MUD (via
protocolo MSP), porém ainda um pouco limitados na janela do IntMUD (porque
depende de executar programas externos).

O projeto é dividido em três componentes principais:

1. Uma **linguagem de script** orientada a objetos e eventos.
2. Um **interpretador** leve e com o mínimo de dependências.
3. Uma **base de MUD** pronta para uso, escrita na própria linguagem do IntMUD.

## Princípios da Plataforma

- **Foco em Simplicidade:** O objetivo é permitir a criação de MUDs e outros
  jogos de texto de forma simples e acessível.
- **Autossuficiente:** Projetado para ter o mínimo de dependências, sem exigir
  a instalação de frameworks ou bibliotecas externas. O código que compila
  hoje provavelmente vai continuar compilando daqui algumas décadas.
- **Multiplataforma:** Compila e roda nativamente em Windows, Linux, Mac
  (devido à compatibilidade com BSD) e outros sistemas UNIX-like.
- **Pronto para uso:** É possível rodar um jogo localmente sem a necessidade
  de configurar uma conexão de rede ou usar um cliente de MUD, ou mesmo um
  navegador. O MUD vem configurado dessa forma, abre uma janela própria de
  console no Windows, e em outros sistemas, roda diretamente no terminal.

## Começando

### No Linux e sistemas UNIX-like

1.  **Clone o repositório e entre no diretório:**
    ```bash
    git clone https://github.com/ed2martin/intmud
    cd intmud
    ```

    Ou baixe o arquivo ZIP e descompacte (na máquina onde será compilado).

2.  **Compile o programa:**
    ```bash
    ./configure && make
    ```
    Este comando criará o executável `intmud`, que é o interpretador da linguagem.

3.  **(Opcional) Reduza o tamanho do executável:**
    ```bash
    strip --strip-all intmud
    ```

4.  **Para rodar o MUD de exemplo:**
    ```bash
    # Copie o executável para o diretório do MUD
    cp intmud mud/mud
    # Entre no diretório e execute
    cd mud
    ./mud
    ```

    Ou chame o IntMUD passando o caminho do arquivo principal:
    ```bash
    ./intmud mud/mud
    ```

### No Windows

Existem duas maneiras de executar o IntMUD no Windows:

-  **Opção 1 (Recomendada):**
   1. Baixar o pacote já compilado para Windows a partir da
      [página de Releases](https://github.com/ed2martin/intmud/releases).
   2. Copiar o arquivo intmud.exe para a pasta mud e renomear para mud.exe.
   3. Duplo clique no arquivo mud.exe da pasta mud.

-  **Opção 2 (Compilação manual):**
   1. Instale o ambiente de desenvolvimento **MinGW** ou **MinGW2**.
   2. Abra o terminal MSYS (parte do MinGW).
   3. Siga as mesmas instruções de compilação para Linux.

## Princípios da Linguagem IntMUD

É uma linguagem de script pensada especificamente para a criação de jogos
interativos e de longa duração.

- **Orientada a Objetos**: Todo o código é escrito em classes com suporte
  a herança múltipla.
- **Orientada a Eventos**: Operações demoradas (como conexões de rede) não
  travam o programa. Em vez disso, um evento é disparado quando a tarefa
  é concluída.
- **Eventos baseados em Variáveis**: Temporizadores são controlados por
  variáveis numéricas que decrementam até zero, disparando um evento.
  Isso permite inspecionar e cancelar eventos facilmente.
- **Gerenciamento de Memória Pré-alocada**: Usa alguns tipos de dados
  de tamanho fixo (textos, vetores) para minimizar a alocação dinâmica,
  garantindo maior desempenho e prevenindo fragmentação de memória.
- **Ausência de *Garbage Collector***: Garante uma execução com desempenho
  constante, sem pausas inesperadas, e dá ao desenvolvedor controle total
  sobre o uso de memória.
- **Forte em Metaprogramação**: Permite inspecionar e alterar o código-fonte
  e os objetos em tempo de execução, facilitando a depuração e a administração
  do jogo.
- **Liberação Automática de Recursos**: Recursos como conexões de rede são
  liberados automaticamente quando as variáveis ou objetos associados a eles
  são destruídas.
- **Sistema de *Watchdog* (cão de guarda)**: Funções que excedem um número
  configurável de instruções são interrompidas para evitar loops infinitos,
  garantindo a estabilidade do servidor.
- **Linguagem em português**: É uma decisão de projeto. Facilita para
  iniciantes e pode ser um trampolim para outras linguagens de programação. |

## A Base de MUD

A base de MUD do projeto serve como um ponto de partida robusto e um exemplo prático. Seus princípios são:

- **Criação sem código**: Permitir a construção de muita coisa complexa sem
  programação, através de um sistema de comandos de administração e menus
  de edição detalhados.
- **Criação com código**: Para triggers e outras coisas avançadas, é inevitável
  recorrer à programação, porém a linguagem e os exemplos que vêm com a base
  facilitam muito.
- **Alta flexibilidade**: Permitir jogos de diferentes estilos, desde
  medieval/fantasia até futurista/espacial. Flexibilidade nas fórmulas
  matemáticas, como cálculo de danos nas lutas e taxa de experiência ao
  matar um PNJ.
- **Área de Testes**: Inclui uma área específica para demonstrar os principais
  recursos da base de MUD em ação.
- **Tutorial interativo para jogadores**: Oferece uma área inicial (a ilha) que
  ensina os comandos básicos aos jogadores de forma sutil, através de tarefas.
  É como eu penso que deveria ser uma área inicial e é também um desafio de
  observação, pois exige que o jogador leia as descrições no ambiente com
  atenção.
- **Experiência de RPG**: Contém também uma área de aventura que simula
  a jogabilidade de um RPG, permitindo sentir um pouco do potencial do MUD.

## Outros Programas de Exemplo

O repositório também inclui outros programas escritos na linguagem do IntMUD para demonstrar sua versatilidade:

- **Telnet:** Um cliente Telnet básico, porém funcional.
- **Quest:** Um jogo simples de perguntas e respostas.
- **Minichat:** Um servidor de chat que aceita conexões via navegador.
- **Mudaprog:** Um código na linguagem do IntMUD que permite recarregar os
  arquivos `.int` modificados sem a necessidade de reiniciar o servidor.
  Nota: já vem incluído no código do MUD.

## Roteiro de Desenvolvimento

Por se tratar de um hobby, o desenvolvimento não é rápido. Além disso,
qualidade e estabilidade são mais importantes. Por isso, as melhorias são
implementadas de forma cuidadosa e não há prazos rígidos.

As melhorias mais urgentes são:
- Melhorar a documentação da linguagem, principalmente com exemplos reais.
- Exportar a ajuda do MUD para arquivos de texto. Atualmente essa ajuda
  só está disponível dentro do MUD.
- Reescrever o núcleo do interpretador. O atual núcleo é bastante ineficiente
  e bastante acoplado, o que impede a criação de módulos à parte.
- Reescrever da camada de rede (socket). Principalmente em ambiente Windows,
  a eficiência é baixa. Embora não seja realmente necessidade para um MUD,
  IOCP/epoll/kqueue é muito bem-vindo.

## Contribuições

Contribuições são bem-vindas, bem como discutir novas funcionalidades.
