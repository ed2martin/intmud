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
    HANDLE Stdin(void); ///< Entrada padr�o
    HANDLE Stdout(void); ///< Sa�da padr�o
#else
    int Stdin(void); ///< Entrada padr�o
    int Stdout(void); ///< Sa�da padr�o
#endif

    bool Inic();
        ///< Inicia console
        /**< @return true se conseguiu inicializar */
    void Fim();
        ///< Encerra utiliza��o do console
    void MudaTitulo(const char * texto);
        ///< Muda o t�tulo da janela
    void Beep();
        ///< Gera um bipe
    const char * LerTecla();
        ///< L� uma tecla
        /**< @return String contendo o nome da tecla
                     ou 0 se nenhuma tecla pressionado */
    void Flush();
        ///< Envia informa��es pendentes para o console
    void EnvTxt(const char * texto, int tamanho);
        ///< Envia texto puro
        /**< @param texto Texto a ser enviado
         *   @param tamanho Tamanho do texto, em caracteres
         *   @note Processa apenas letras e '\n' */
    void CorTxt(int novacor);
        ///< Muda a cor atual
        /**< @param novacor Nova cor, no mesmo formato de CorAtual */
    void CursorIni();
        ///< Move cursor para in�cio da linha
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
        /**< @param lin N�mero da linha, come�a no 0
         *   @param col N�mero da coluna, come�a no 0 */
    void InsereLin(int valor);
        ///< Insere uma ou mais linhas em branco a partir da posi��o do cursor
        /**< @param valor N�mero de linhas a inserir */
    void ApagaLin(int valor);
        ///< Remove uma ou mais linhas a partir da posi��o do cursor
        /**< @param valor N�mero de linhas a remover */
    void InsereCol(int valor);
        ///< Insere um ou mais espa�os a partir da posi��o do cursor
        /**< @param valor N�mero de espa�os a inserir */
    void ApagaCol(int valor);
        ///< Remove um ou mais caracteres a partir da posi��o do cursor
        /**< @param valor N�mero de caracteres a remover */
    void LimpaFim();
        ///< Limpa da posi��o do cursor at� o fim da linha
    void LimpaTela();
        ///< Limpa toda a tela; n�o muda a posi��o do cursor

    unsigned int CorAtual;
        ///< Cor selecionada na tela; somente leitura
        /**< - Bits 3-0 = fundo
         *   - Bits 6-4 = frente
         *   - Bit 7 = negrito, 0=desativado */
    unsigned int LinTotal;
        ///< N�mero de linhas na tela; somente leitura
    unsigned int ColTotal;
        ///< N�mero de colunas na tela; somente leitura
    unsigned int LinAtual;
        ///< Linha atual na tela; somente leitura
    unsigned int ColAtual;
        ///< Coluna atual na tela; somente leitura


    void Escrever(const char * texto, int tamanho = -1);
        ///< Envia um texto para o console
    void ProcTecla(const char * texto);
        ///< Processa uma tecla, recebida com LerTecla()
    void ProcTeclaCursor(int coluna);
        ///< Move o cursor para uma posi��o na linha de edi��o
        /**< Usado em ProcTecla */

    void ProcFim();
        ///< Deve ser chamado quando acabou de usar Escrever() e ProcTecla()
    void ProcLimpa();
        ///< Limpa a tela mantendo a linha de edi��o
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
        ///< Retorna o conte�do da linha sendo editada
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
    char StrConv[0x100]; ///< Para converter c�digos ASCII 128 a 255
    char StrLer[0x100];  ///< Para converter teclas lidas
    HANDLE con_in;  ///< Entrada padr�o
    HANDLE con_out; ///< Sa�da padr�o
    WORD CorAtributos; ///< Cor selecionada em CorTxt()
    bool MoverCursor;  ///< Se deve acertar a posi��o do cursor
    DWORD LerCont;  ///< Contador: n�mero de eventos lidos
    DWORD LerTotal; ///< N�mero total de eventos
    INPUT_RECORD LerEventos[32]; ///< Eventos
#else
    struct termios term_antes; ///< Configura��o anterior do terminal
    int LerPont; ///< Para processar ESC em Ler()
#endif
};

//---------------------------------------------------------------------------
class TVarTelaTxt
{
public:
    void Criar();           ///< Cria objeto
            /**< Ap�s criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarTelaTxt * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();
                            ///< Ler valor num�rico da vari�vel

    const char * defvar;    ///< Como foi definida a vari�vel
    union {
        TClasse * endclasse;///< Em que classe est� definido
        TObjeto * endobjeto;///< Em que objeto est� definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< �ndice no vetor

    static bool Inic();     ///< Inicializa consolse se Console!=0
    static void Processa(); ///< Processa teclas pressionadas
    static void ProcFim();  ///< Fim do processamento do console
    static bool FuncEvento(const char * evento, const char * texto);
                    ///< Executa uma fun��o
                    /**< @param evento Nome do evento (ex. "msg")
                     *   @param texto Texto do primeiro argumento, 0=nenhum texto
                     *   @return true se n�o apagou o objeto, false se apagou
                     */
    static TVarTelaTxt * ObjAtual; ///< Objeto atual, usado em FuncEvento()

    static TConsole * Console;
    static TVarTelaTxt * Inicio; ///< Primeiro objeto
    TVarTelaTxt * Antes;    ///< Objeto anterior
    TVarTelaTxt * Depois;   ///< Pr�ximo objeto
};

//---------------------------------------------------------------------------

#endif
