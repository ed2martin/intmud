#ifndef VAR_TELATXT_H
#define VAR_TELATXT_H

#ifdef __WIN32__
 #include "windows.h"
#else
 #include <termios.h>
#endif

//----------------------------------------------------------------------------
class TVariavel;
class TClasse;
class TObjeto;

//---------------------------------------------------------------------------
class TConsole
{
public:
    TConsole() { Aberto=false; };
        ///< Construtor
    ~TConsole() { Fim(); }
        ///< Destrutor
#ifdef __WIN32__
    HANDLE Stdin(void); ///< Entrada padrão
    HANDLE Stdout(void); ///< Saída padrão
#else
    int Stdin(void); ///< Entrada padrão
    int Stdout(void); ///< Saída padrão
#endif

    bool Inic();
        ///< Inicia console
        /**< @return true se conseguiu inicializar */
    void Fim();
        ///< Encerra utilização do console
    void MudaTitulo(const char * texto);
        ///< Muda o título da janela
    void Beep();
        ///< Gera um bipe
    const char * LerTecla();
        ///< Lê uma tecla
        /**< @return String contendo o nome da tecla
                     ou 0 se nenhuma tecla pressionado */
    void Flush();
        ///< Envia informações pendentes para o console
    void EnvTxt(const char * texto, int tamanho);
        ///< Envia texto puro
        /**< @param texto Texto a ser enviado
         *   @param tamanho Tamanho do texto, em caracteres
         *   @note Processa apenas letras e '\n' */
    void CorTxt(int novacor);
        ///< Muda a cor atual
        /**< @param novacor Nova cor, no mesmo formato de CorAtual */
    void CursorIni();
        ///< Move cursor para início da linha
    void CursorLin(int valor);
        ///< Move cursor na vertical
        /**< @param valor Quantas linhas mover
         *     - Se valor > 0 move para baixo
         *     - Se valor < 0 move para cima */
    void CursorCol(int valor);
        ///< Move cursor na horizontal
        /**< @param valor Quantas colunas mover
         *     - Se valor > 0 move para direita
         *     - Se valor < 0 move para esquerda */
    void CursorPosic(int lin, int col);
        ///< Posiciona o cursor
        /**< @param lin Número da linha, começa no 0
         *   @param col Número da coluna, começa no 0 */
    void InsereLin(int valor);
        ///< Insere uma ou mais linhas em branco a partir da posição do cursor
        /**< @param valor Número de linhas a inserir */
    void ApagaLin(int valor);
        ///< Remove uma ou mais linhas a partir da posição do cursor
        /**< @param valor Número de linhas a remover */
    void InsereCol(int valor);
        ///< Insere um ou mais espaços a partir da posição do cursor
        /**< @param valor Número de espaços a inserir */
    void ApagaCol(int valor);
        ///< Remove um ou mais caracteres a partir da posição do cursor
        /**< @param valor Número de caracteres a remover */
    void LimpaFim();
        ///< Limpa da posição do cursor até o fim da linha
    void LimpaTela();
        ///< Limpa toda a tela; não muda a posição do cursor

    unsigned int CorAtual;
        ///< Cor selecionada na tela; somente leitura
        /**< - Bits 3-0 = fundo
         *   - Bits 6-4 = frente
         *   - Bit 7 = negrito, 0=desativado */
    unsigned int LinTotal;
        ///< Número de linhas na tela; somente leitura
    unsigned int ColTotal;
        ///< Número de colunas na tela; somente leitura
    unsigned int LinAtual;
        ///< Linha atual na tela; somente leitura
    unsigned int ColAtual;
        ///< Coluna atual na tela; somente leitura


    void Escrever(const char * texto, int tamanho = -1);
        ///< Envia um texto para o console
    void ProcTecla(const char * texto);
        ///< Processa uma tecla, recebida com LerTecla()
    void ProcFim();
        ///< Deve ser chamado quando acabou de usar Escrever() e ProcTecla()
    void ProcLimpa();
        ///< Limpa a tela mantendo a linha de edição
    unsigned int CorTela;
        ///< Cor dos textos, usada em Escreve()
        /**< - Bits 3-0 = fundo
         *   - Bits 6-4 = frente
         *   - Bit 7 = negrito, 0=desativado */
    bool Editor;
        ///< true=cursor no editor, false=cursor no texto
    unsigned int ColEscreve;
        ///< Coluna do texto sendo inserido na tela
    unsigned int ColEditor;
        ///< Coluna do cursor na tela (texto sendo editado)

    const char * LerLinha() { linha[tam_linha]=0; return linha; }
        ///< Retorna o conteúdo da linha sendo editada
private:
    unsigned int col_linha;  ///< linha[col_linha] = primeiro caracter na tela
    unsigned int tam_linha;  ///< Tamanho da linha, sem o byte =0 no final
    char linha[1024]; ///< Linha sendo editada

    bool Aberto;
        ///< Se console foi aberto
    bool ini_linha;
        ///< Se passou para nova linha porque terminou a linha
    char LerTexto[16];  ///< Usado internamente por Ler()

#ifdef __WIN32__
    char StrConv[0x100]; ///< Para converter códigos ASCII 128 a 255
    char StrLer[0x100];  ///< Para converter teclas lidas
    HANDLE con_in;  ///< Entrada padrão
    HANDLE con_out; ///< Saída padrão
    WORD CorAtributos; ///< Cor selecionada em CorTxt()
    bool MoverCursor;  ///< Se deve acertar a posição do cursor
    DWORD LerCont;  ///< Contador: número de eventos lidos
    DWORD LerTotal; ///< Número total de eventos
    INPUT_RECORD LerEventos[32]; ///< Eventos
#else
    struct termios term_antes; ///< Configuração anterior do terminal
    int LerPont; ///< Para processar ESC em Ler()
#endif
};

//---------------------------------------------------------------------------
class TVarTelaTxt
{
public:
    void Criar();           ///< Cria objeto
            /**< Após criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarTelaTxt * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();
                            ///< Ler valor numérico da variável

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    static bool Inic();     ///< Inicializa consolse se Console!=0
    static void Processa(); ///< Processa teclas pressionadas
    static void ProcFim();  ///< Fim do processamento do console
    static bool FuncEvento(const char * evento, const char * texto);
                    ///< Executa uma função
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @return true se não apagou o objeto, false se apagou
                     */
    static TVarTelaTxt * ObjAtual; ///< Objeto atual, usado em FuncEvento()

    static TConsole * Console;
    static TVarTelaTxt * Inicio; ///< Primeiro objeto
    TVarTelaTxt * Antes;    ///< Objeto anterior
    TVarTelaTxt * Depois;   ///< Próximo objeto
};

//---------------------------------------------------------------------------

#endif
