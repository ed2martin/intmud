#ifndef VAR_TEXTO_H
#define VAR_TEXTO_H

#define TOTAL_TXTGR 62 // N�mero de objetos TTextoBloco em cada TTextoGrupo
#define TOTAL_TXTOBJ 0x108 // N�mero de bytes em cada TTextoBloco
// 62 * 0x108 = 0x3FF0

//----------------------------------------------------------------------------
class TTextoBloco;
class TVariavel;

class TTextoTxt  /// Vari�veis TextoTxt
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TTextoTxt * destino); ///< Move objeto para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel

    void CriarFim();        ///< Cria bloco no final de TTextoTxt
    TTextoBloco * Inicio;   ///< Aonde come�a o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    unsigned int Linhas;    ///< N�mero de linhas
    unsigned int Bytes;     ///< N�mero de bytes
};

//----------------------------------------------------------------------------
class TTextoPos  /// Vari�veis TextoPos
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TTextoPos * destino); ///< Move objeto para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();        ///< Ler valor num�rico da vari�vel

    TTextoTxt * TextoTxt;
};

//----------------------------------------------------------------------------
class TTextoBloco /// Bloco de texto de um objeto TTextoTxt
{
public:
    TTextoBloco * Apagar();
            ///< Retira objeto da lista e apaga objeto
            /**< @return Endere�o do objeto que passou a ocupar
             *           o endere�o objeto apagado */
    void Mover(TTextoBloco * destino);
            ///< Move bloco para outro lugar
            /**< @param destino Endere�o destino
             *   @note Usado por TTextoGrupo */
    void AddTxt(int posic, const char * texto, bool novalinha);
            ///< Adiciona texto a partir da posi��o especificada
            /**< @param texto Texto a adicionar
             *   @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param novalinha Se deve adicionar "\n" ap�s o texto */
    void ApagarTxt(int posic, int bytes);
            ///< Apaga texto a partir da posi��o especificada
            /**< @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param bytes Quantidade de bytes */
    int CopiarTxt(int posic, char * buffer, int tambuf);
            ///< Copia texto para uma regi�o na mem�ria
            /**< @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param buffer Aonde copiar
             *   @param tambuf Tamanho de buffer
             *   @return N�mero de bytes preenchidos em buffer, com o 0 final */
    int LinhasBytes(int posic, int numlinhas);
            ///< Obt�m quantos bytes correspondem a numlinhas
            /**< Obt�m quantos bytes correspondem a avan�ar um determinado
             *   n�mero de linhas
             *   @param posic Posi��o em TTextoBloco:Texto, come�a em 0
             *   @param linhas N�mero de linhas
             *   @return N�mero de bytes, com um Instr::ex_barra_n no final */

    union {
        char ObjChar[1]; ///< Para acessar o objeto TTextoBloco como char*
        TTextoTxt * TextoTxt; ///< TTextoTxt que cont�m objeto
    };
    TTextoBloco * Antes;  ///< TTextoBloco anterior; se (IniFim&1)!=0
    TTextoBloco * Depois; ///< Pr�ximo TTextoBloco; se (IniFim&2)!=0
    unsigned char Linhas; ///< N�mero de caracteres '\n' do bloco
    unsigned char Bytes;  ///< N�mero de bytes do bloco
    char Texto[1];        ///< A partir daqui: texto do bloco

private:
    void AjustarTxt();
            ///< Ajusta objetos TTextoBloco reduzindo o tamanho se poss�vel
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
