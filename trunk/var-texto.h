#ifndef VAR_TEXTO_H
#define VAR_TEXTO_H

#define TOTAL_TXTGR 62 // N�mero de objetos TTextoBloco em cada TTextoGrupo
#define TOTAL_TXTOBJ 0x108 // N�mero de bytes em cada TTextoBloco
// 62 * 0x108 = 0x3FF0

//----------------------------------------------------------------------------
class TVariavel;
class TBlocoPos;
class TTextoPos;
class TTextoBloco;
class TObjeto;

class TTextoTxt  /// Vari�veis TextoTxt
{
public:
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoTxt * destino); ///< Move objeto para outro lugar
    void IniBloco();        ///< Cria primeiro bloco, se n�o existir
    void AddTexto(const char * texto, unsigned int tamtexto);
            ///< Adiciona texto no final
            /**< @note � um pouco mais eficiente que TBlocoPos::MudarTxt */

    void TextoIni();
            ///< In�cio: limpa texto de TTextoTxt
            /**< Deve ser chamado antes de adicionar textos com TextoAnota */
    void TextoAnota(const char * txt, int tam);
            ///< Acrescenta texto em TTextoTxt
            /**< @param txt Texto a adicionar
             *   @param tam Tamanho do texto, em bytes */
    void TextoFim();
            ///< Encerra adi��o de texto com TextoIni() e TextoAnota()
            /**< Se algum texto foi adicionado (com TextoAnota), o �ltimo
                 byte adicionado deve ser um Instr::ex_barra_n ou 0 */

    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel

    TTextoBloco * Inicio;   ///< Aonde come�a o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    TTextoPos * Posic;      ///< Primeiro objeto da lista
    unsigned int Linhas;    ///< N�mero de linhas
    unsigned int Bytes;     ///< N�mero de bytes

    void DivideLin(unsigned int min, unsigned int max, bool cores);
            ///< Diminui o tamanho das linhas dividindo em duas ou mais
            /**< @param min Tamanho m�nimo de onde a linha ser� dividida
             *   @param max Tamanho m�ximo das linhas
             *   @param cores Se defini��o de cores n�o conta como caracter */
    void Ordena(int modo, const char *txt1, const char * txt2);
            ///< Ordena linhas em ordem alfab�tica
            /**< @param modo Modo de ordena��o:
             *   - 0 = textotxt.ordena
             *   - 1 = textotxt.ordenalin sem argumentos
             *   - 2 = textotxt.ordenalin com argumentos
             *   - 3 = textotxt.juntalin sem argumentos
             *   - 4 = textotxt.juntalin com argumentos
             *   @param txt1 Texto antes do primeiro n�mero de cada linha
             *   @param txt2 Texto depois do n�mero de cada linha */
    void Rand();
            ///< Ordena linhas aleatoriamente
    void TxtRemove(int opcoes);
            ///< Remove espa�os, instru��o txtremove
            /**< @param opcoes Op��es de remo��o, obtidas com ::txtRemove() */

private:
    void OrdenaSub(int modo, char * texto, char** linha,
            const char *txt1, const char * txt2);
            ///< Chamado por Ordena(), para ordenar linhas
            /**< @param modo Modo de ordena��o, vide Ordena()
             *   @param texto Texto a ser ordenado, come�a e termina com um byte 0
             *   @param linha char*[] contendo o n�mero de linhas vezes 2
             *   @param txt1 Vide Ordena()
             *   @param txt2 Vide Ordena() */
    void RandSub(char * texto, char** linha);
            ///< Chamado por Rand(), para ordenar linhas aleatoriamente
            /**< @param texto Texto a ser ordenado, come�a e termina com um byte 0
             *   @param linha char*[] contendo a linha */
    void TxtRemoveSub(char * texto, int opcoes);
            ///< Chamado por TxtRemove
            /**< @param texto Texto a ser trabalhado
             *   @param opcoes Op��es de remo��o, obtidas com ::txtRemove() */
};

//----------------------------------------------------------------------------
class TBlocoPos /// Marca uma posi��o no texto
{
public:
    TTextoBloco * Bloco;
            ///< Bloco que cont�m o in�cio da linha
    unsigned int PosicBloco;
            ///< N�mero de bytes desde o in�cio do bloco
    unsigned int PosicTxt;
            ///< N�mero de bytes desde o in�cio do texto
    unsigned int LinhaTxt;
            ///< Linha no texto; come�a em 0
            /**< Caracteres Instr::ex_barra_n desde o in�cio do texto */

    void MoverPos(int numlinhas);
            ///< Avan�a ou recua um determinado n�mero de linhas
            /**< @param numlinhas Quantas linhas avan�ar,
             *          se menor que zero recua ao inv�s de avan�ar */
    void Mudar(const char * texto, unsigned int tamtexto,
            unsigned int tamapagar);
            ///< Muda texto a partir de uma posi��o
            /**< Substitui a quantidade especificada de bytes por um texto
             *   @param texto Texto a adicionar
             *   @param tamtexto Tamanho do texto, em bytes
             *   @param tamapagar Quantidade de bytes a apagar
             *   @note Caracteres 0 s�o substitu�dos por Instr::ex_barra_n */
};

//----------------------------------------------------------------------------
class TTextoPos : public TBlocoPos /// Vari�veis TextoPos
{
public:
    void Apagar();
            ///< Apaga objeto
    void Mover(TTextoPos * destino);
            ///< Move objeto para outro lugar
    void MudarTxt(TTextoTxt * obj);
            ///< Associa objeto a TTextoTxt sem texto, desassocia se obj==0
    int  Compara(TTextoPos * v);
            ///< Operador de compara��o
    void Igual(TTextoPos * v);
            ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome);
            ///< Fun��o da vari�vel
    static int getTipo(int numfunc);
            ///< Retorna o tipo de vari�vel
    int  getValor(int numfunc);
            ///< Ler valor num�rico da vari�vel
    void setValor(int numfunc, int valor);
            ///< Mudar o valor num�rico da vari�vel
    void setTxt(int numfunc, const char * txt);
            ///< Mudar o texto

    TTextoTxt * TextoTxt;   ///< Vari�vel TextoTxt correspondente
    TTextoPos * Antes;      ///< Objeto anterior
    TTextoPos * Depois;     ///< Pr�ximo objeto

    TObjeto * Objeto;       ///< Objeto em que o textopos foi definido
    const char * defvar;    ///< Defini��o da vari�vel; usado em var-sav.cpp
    unsigned int indice;    ///< �ndice da vari�vel
};

//----------------------------------------------------------------------------
class TTextoBloco /// Bloco de texto de um objeto TTextoTxt
{
public:
    TTextoBloco * CriarAntes();
            ///< Cria bloco antes
    TTextoBloco * CriarDepois();
            ///< Cria bloco depois
    TTextoBloco * Apagar();
            ///< Retira objeto da lista e apaga objeto
            /**< @return Endere�o do objeto que passou a ocupar
             *           o endere�o objeto apagado */
    void Mover(TTextoBloco * destino);
            ///< Move bloco para outro lugar
            /**< @param destino Endere�o destino
             *   @note Usado por TTextoGrupo */
    int LinhasBytes(unsigned int posic, int numlinhas);
            ///< Obt�m quantos bytes correspondem a numlinhas
            /**< Obt�m quantos bytes correspondem a avan�ar um determinado
             *   n�mero de linhas
             *   @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param numlinhas N�mero de linhas
             *   @return N�mero de bytes, com um Instr::ex_barra_n no final */
    int CopiarTxt(unsigned int posic, char * buffer, int tambuf);
            ///< Copia texto para uma regi�o na mem�ria
            /**< @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param buffer Aonde copiar
             *   @param tambuf Tamanho de buffer
             *   @return N�mero de bytes preenchidos em buffer, com o 0 final */

    TTextoTxt * TextoTxt; ///< Vari�vel TextoTxt que cont�m o bloco
    TTextoBloco * Antes;  ///< TTextoBloco anterior
    TTextoBloco * Depois; ///< Pr�ximo TTextoBloco
    unsigned char Linhas; ///< N�mero de caracteres Instr::ex_barra_n do bloco
    unsigned char Bytes;  ///< N�mero de bytes do bloco
    char Texto[1];        ///< A partir daqui: texto do bloco
};

//----------------------------------------------------------------------------
class TTextoGrupo /// Para otimizar aloca��o de mem�ria de TTextoBloco
{
public:
    static void ProcEventos();
            ///< Apaga objetos TTextoGrupoupo se necess�rio
    static TTextoBloco * Criar();
            ///< Aloca TTextoBloco na mem�ria
    static TTextoBloco * Apagar(TTextoBloco *);
            ///< Libera TTextoBloco da mem�ria
            /**< @return Endere�o do objeto que passou a ocupar
             *           o endere�o objeto apagado */
private:
    static TTextoGrupo * Disp;  ///< Lista de objetos dispon�vels (com Total=0)
    static TTextoGrupo * Usado; ///< Lista de objetos usados (com Total!=0)
    static unsigned long Tempo; ///< Quando o �ltimo objeto mudou
    unsigned int Total;         ///< N�mero de objetos usados
    TTextoGrupo * Depois;       ///< Pr�ximo objeto
    char Lista[TOTAL_TXTGR][TOTAL_TXTOBJ]; ///< Objetos TTextoBloco
};

//----------------------------------------------------------------------------
/// Obt�m o n�mero de linhas correspondente a um texto
int TextoNumLin(const char * texto, int total);

/// Copia texto e retorna n�mero de linhas
/** @param destino Endere�o destino
 *  @param origem Endere�o origem
 *  @param total N�mero de bytes a copiar
 *  @return N�mero de caracteres Instr::ex_barra_n copiados
 *  @note O caracter 0 � anotado como Instr::ex_barra_n */
int TextoAnotaLin(char * destino, const char * origem, int total);

//----------------------------------------------------------------------------

#endif
