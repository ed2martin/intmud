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

    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel

    TTextoBloco * Inicio;   ///< Aonde come�a o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    TTextoPos * Posic;      ///< Primeiro objeto da lista
    unsigned int Linhas;    ///< N�mero de linhas
    unsigned int Bytes;     ///< N�mero de bytes
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
    int  getValor();
            ///< Ler valor num�rico da vari�vel

    TTextoTxt * TextoTxt;   ///< Vari�vel TextoTxt correspondente
    TTextoPos * Antes;      ///< Objeto anterior
    TTextoPos * Depois;     ///< Pr�ximo objeto
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

#endif
