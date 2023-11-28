#ifndef VAR_TEXTO_H
#define VAR_TEXTO_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TBlocoPos;
class TTextoPos;
class TTextoBloco;
class TClasse;
class TObjeto;

class TTextoTxt  /// Variáveis TextoTxt
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoTxt * destino); ///< Move objeto para outro lugar
    void IniBloco();        ///< Cria primeiro bloco, se não existir
    void AddTexto(const char * texto, unsigned int tamtexto);
            ///< Adiciona texto no final
            /**< @note É um pouco mais eficiente que TBlocoPos::MudarTxt */

    void TextoIni();
            ///< Início: limpa texto de TTextoTxt
            /**< Deve ser chamado antes de adicionar textos com TextoAnota */
    void TextoAnota(const char * txt, int tam);
            ///< Acrescenta texto em TTextoTxt
            /**< @param txt Texto a adicionar
             *   @param tam Tamanho do texto, em bytes */
    void TextoFim();
            ///< Encerra adição de texto com TextoIni() e TextoAnota()
            /**< Se algum texto foi adicionado (com TextoAnota), o último
                 byte adicionado deve ser um Instr::ex_barra_n ou 0 */

    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável

    TTextoBloco * Inicio;   ///< Aonde começa o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    TTextoPos * Posic;      ///< Primeiro objeto da lista
    unsigned int Linhas;    ///< Número de linhas
    unsigned int Bytes;     ///< Número de bytes

    void DivideLin(unsigned int min, unsigned int max, bool cores);
            ///< Diminui o tamanho das linhas dividindo em duas ou mais
            /**< @param min Tamanho mínimo de onde a linha será dividida
             *   @param max Tamanho máximo das linhas
             *   @param cores Se definição de cores não conta como caracter */
    void Ordena(int modo, const char *txt1, const char * txt2);
            ///< Ordena linhas em ordem alfabética
            /**< @param modo Modo de ordenação:
             *   - 0 = textotxt.ordena
             *   - 1 = textotxt.ordenalin sem argumentos
             *   - 2 = textotxt.ordenalin com argumentos
             *   - 3 = textotxt.juntalin sem argumentos
             *   - 4 = textotxt.juntalin com argumentos
             *   @param txt1 Texto antes do primeiro número de cada linha
             *   @param txt2 Texto depois do número de cada linha */
    void Rand();
            ///< Ordena linhas aleatoriamente
    void TxtRemove(int opcoes);
            ///< Remove espaços, instrução txtremove
            /**< @param opcoes Opções de remoção, obtidas com ::txtRemove() */

private:
    bool FuncIni(TVariavel * v);
    bool FuncFim(TVariavel * v);
    bool FuncAddIni(TVariavel * v);
    bool FuncAddFim(TVariavel * v);
    bool FuncRemove(TVariavel * v);
    bool FuncLimpar(TVariavel * v);
    bool FuncLinhas(TVariavel * v);
    bool FuncBytes(TVariavel * v);
    bool FuncOrdena(TVariavel * v);
    bool FuncOrdenaLin(TVariavel * v);
    bool FuncRand(TVariavel * v);
    bool FuncTxtRemove(TVariavel * v);
    bool FuncJuntaLin(TVariavel * v);
    bool FuncDivideLin(TVariavel * v);
    bool FuncDivideLinCor(TVariavel * v);
    bool FuncJuntar(TVariavel * v);
    bool FuncLer(TVariavel * v);
    bool FuncSalvar(TVariavel * v);
    bool FuncClipLer(TVariavel * v);
    bool FuncClipSalvar(TVariavel * v);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);

    void OrdenaSub(int modo, char * texto, char** linha,
            const char *txt1, const char * txt2);
            ///< Chamado por Ordena(), para ordenar linhas
            /**< @param modo Modo de ordenação, vide Ordena()
             *   @param texto Texto a ser ordenado, começa e termina com um byte 0
             *   @param linha char*[] contendo o número de linhas vezes 2
             *   @param txt1 Vide Ordena()
             *   @param txt2 Vide Ordena() */
    void RandSub(char * texto, char** linha);
            ///< Chamado por Rand(), para ordenar linhas aleatoriamente
            /**< @param texto Texto a ser ordenado, começa e termina com um byte 0
             *   @param linha char*[] contendo a linha */
    void TxtRemoveSub(char * texto, int opcoes);
            ///< Chamado por TxtRemove
            /**< @param texto Texto a ser trabalhado
             *   @param opcoes Opções de remoção, obtidas com ::txtRemove() */
};

//----------------------------------------------------------------------------
class TBlocoPos /// Marca uma posição no texto
{
public:
    TTextoBloco * Bloco;
            ///< Bloco que contém o início da linha
    unsigned int PosicBloco;
            ///< Número de bytes desde o início do bloco
    unsigned int PosicTxt;
            ///< Número de bytes desde o início do texto
    unsigned int LinhaTxt;
            ///< Linha no texto; começa em 0
            /**< Caracteres Instr::ex_barra_n desde o início do texto */

    void MoverPos(int numlinhas);
            ///< Avança ou recua um determinado número de linhas
            /**< @param numlinhas Quantas linhas avançar,
             *          se menor que zero recua ao invés de avançar */
    void Mudar(const char * texto, unsigned int tamtexto,
            unsigned int tamapagar);
            ///< Muda texto a partir de uma posição
            /**< Substitui a quantidade especificada de bytes por um texto
             *   @param texto Texto a adicionar
             *   @param tamtexto Tamanho do texto, em bytes
             *   @param tamapagar Quantidade de bytes a apagar
             *   @note Caracteres 0 são substituídos por Instr::ex_barra_n */
};

//----------------------------------------------------------------------------
class TTextoPos : public TBlocoPos /// Variáveis TextoPos
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Apagar();
            ///< Apaga objeto
    void Mover(TTextoPos * destino, TObjeto * o);
            ///< Move objeto para outro lugar
    void MudarTxt(TTextoTxt * obj);
            ///< Associa objeto a TTextoTxt sem texto, desassocia se obj==0
    int  Compara(TTextoPos * v);
            ///< Operador de comparação
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável
    static TVarTipo FTipo(TVariavel * v);
            ///< Retorna o tipo de variável
    int  getValor(int numfunc);
            ///< Ler valor numérico da variável
    void setValor(int numfunc, int valor);
            ///< Mudar o valor numérico da variável
    void setTxt(int numfunc, const char * txt);
            ///< Mudar o texto

    TTextoTxt * TextoTxt;   ///< Variável TextoTxt correspondente
    TTextoPos * Antes;      ///< Objeto anterior
    TTextoPos * Depois;     ///< Próximo objeto

    TObjeto * Objeto;       ///< Objeto em que o textopos foi definido
    const char * defvar;    ///< Definição da variável; usado em var-sav.cpp
    unsigned int indice;    ///< Índice da variável

private:
    bool FuncAntes(TVariavel * v, int valor);
    bool FuncDepois(TVariavel * v, int valor);
    bool FuncLin(TVariavel * v, int valor);
    bool FuncLinha(TVariavel * v, int valor);
    bool FuncByte(TVariavel * v, int valor);
    bool FuncTexto(TVariavel * v, int valor);
    bool FuncTextoLin(TVariavel * v, int valor);
    bool FuncMudar(TVariavel * v, int valor);
    bool FuncAdd(TVariavel * v, int valor);
    bool FuncRemove(TVariavel * v, int valor);
    bool FuncJuntar(TVariavel * v, int valor);
    bool FuncTxtProc(TVariavel * v, int valor);
    bool FuncMd5Sha1(TVariavel * v, int valor);

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static void FMoverDef(TVariavel * v);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
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
            /**< @return Endereço do objeto que passou a ocupar
             *           o endereço objeto apagado */
    void Mover(TTextoBloco * destino);
            ///< Move bloco para outro lugar
            /**< @param destino Endereço destino
             *   @note Usado por TTextoGrupo */
    int LinhasBytes(unsigned int posic, int numlinhas);
            ///< Obtém quantos bytes correspondem a numlinhas
            /**< Obtém quantos bytes correspondem a avançar um determinado
             *   número de linhas
             *   @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param numlinhas Número de linhas
             *   @return Número de bytes, com um Instr::ex_barra_n no final */
    int CopiarTxt(unsigned int posic, char * buffer, int tambuf);
            ///< Copia texto para uma região na memória
            /**< @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param buffer Aonde copiar
             *   @param tambuf Tamanho de buffer
             *   @return Número de bytes preenchidos em buffer, com o 0 final */

    TTextoTxt * TextoTxt; ///< Variável TextoTxt que contém o bloco
    TTextoBloco * Antes;  ///< TTextoBloco anterior
    TTextoBloco * Depois; ///< Próximo TTextoBloco
    unsigned char Linhas; ///< Número de caracteres Instr::ex_barra_n do bloco
    unsigned char Bytes;  ///< Número de bytes do bloco
    char Texto[0xF0];     ///< A partir daqui: texto do bloco
};

//----------------------------------------------------------------------------
#define TOTAL_TEXTOTXTX 128
class TTextoGrupo /// Para otimizar alocação de memória de TTextoBloco
{
public:
    static void ProcEventos();
            ///< Apaga objetos TTextoGrupoupo se necessário
    static TTextoBloco * Criar();
            ///< Aloca TTextoBloco na memória
    static TTextoBloco * Apagar(TTextoBloco *);
            ///< Libera TTextoBloco da memória
            /**< @return Endereço do objeto que passou a ocupar
             *           o endereço objeto apagado */
private:
    static TTextoGrupo * Disp;  ///< Lista de objetos disponívels (com Total=0)
    static TTextoGrupo * Usado; ///< Lista de objetos usados (com Total!=0)
    static unsigned long Tempo; ///< Quando o último objeto mudou
    unsigned int Total;         ///< Número de objetos usados
    TTextoGrupo * Depois;       ///< Próximo objeto
    TTextoBloco Lista[TOTAL_TEXTOTXTX]; ///< Objetos TTextoBloco
};

//----------------------------------------------------------------------------
/// Obtém o número de linhas correspondente a um texto
int TextoNumLin(const char * texto, int total);

/// Copia texto e retorna número de linhas
/** @param destino Endereço destino
 *  @param origem Endereço origem
 *  @param total Número de bytes a copiar
 *  @return Número de caracteres Instr::ex_barra_n copiados
 *  @note O caracter 0 é anotado como Instr::ex_barra_n */
int TextoAnotaLin(char * destino, const char * origem, int total);

//----------------------------------------------------------------------------

#endif
