#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __WIN32__
 #include <windows.h>
#else
 #include <termios.h>
#endif

//---------------------------------------------------------------------------
class TConsole
{
public:
    TConsole() { Aberto=0; };
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

    bool Inic(bool completo=true);
        ///< Inicia console
        /**< @param completo false=somente para escrita, true=leitura e escrita
         *   @return true se conseguiu inicializar */
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
    void CorTxt(unsigned int novacor);
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

    unsigned char Aberto;
        ///< Se console foi aberto
        /**< 0=fechado, 1=aberto para enviar texto, 2=aberto */
    bool ini_linha;
        ///< Se passou para nova linha porque terminou a linha
    char LerTexto[16];  ///< Usado internamente por Ler()

    int Charset;
        ///< Conjunto de caracteres usado
        /**< 0x80 = UTF-8
         *   0x100 = desconhecido
         *   0x101 = iso8859-1
         *   0x102 = cp1252
         *   0x103 = ibm850
         */
    int LerUTF8; ///< Caracter anterior, lendo UTF-8
    char StrConv[0x100]; ///< Para converter c�digos ASCII 128 a 255
    char StrLer[0x100];  ///< Para converter teclas lidas

#ifdef __WIN32__
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
    bool fcntl_block;
        ///< Para acessar stdin/stdout
        /**< - Antes de ler: fcntl(STDIN_FILENO, F_SETFL, 0);
         *   - Antes de escrever: fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); */
#endif
};

//---------------------------------------------------------------------------
extern TConsole * Console; ///< Para acessar a tela

#endif
