#ifndef VAR_TEXTO_H
#define VAR_TEXTO_H

#define TOTAL_TXTGR 62 // Número de objetos TTextoBloco em cada TTextoGrupo
#define TOTAL_TXTOBJ 0x108 // Número de bytes em cada TTextoBloco
// 62 * 0x108 = 0x3FF0

//----------------------------------------------------------------------------
class TTextoBloco;
class TVariavel;

class TTextoTxt  /// Variáveis TextoTxt
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TTextoTxt * destino); ///< Move objeto para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável

    void CriarFim();        ///< Cria bloco no final de TTextoTxt
    TTextoBloco * Inicio;   ///< Aonde começa o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    unsigned int Linhas;    ///< Número de linhas
    unsigned int Bytes;     ///< Número de bytes
};

//----------------------------------------------------------------------------
class TTextoPos  /// Variáveis TextoPos
{
public:
    void Apagar();          ///< Apaga objeto
    void Mover(TTextoPos * destino); ///< Move objeto para outro lugar
    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável

    TTextoTxt * TextoTxt;
};

//----------------------------------------------------------------------------
class TTextoBloco /// Bloco de texto de um objeto TTextoTxt
{
public:
    TTextoBloco * Apagar();
            ///< Retira objeto da lista e apaga objeto
            /**< @return Endereço do objeto que passou a ocupar
             *           o endereço objeto apagado */
    void Mover(TTextoBloco * destino);
            ///< Move bloco para outro lugar
            /**< @param destino Endereço destino
             *   @note Usado por TTextoGrupo */
    void AddTxt(int posic, const char * texto, bool novalinha);
            ///< Adiciona texto a partir da posição especificada
            /**< @param texto Texto a adicionar
             *   @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param novalinha Se deve adicionar "\n" após o texto */
    void ApagarTxt(int posic, int bytes);
            ///< Apaga texto a partir da posição especificada
            /**< @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param bytes Quantidade de bytes */
    int CopiarTxt(int posic, char * buffer, int tambuf);
            ///< Copia texto para uma região na memória
            /**< @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param buffer Aonde copiar
             *   @param tambuf Tamanho de buffer
             *   @return Número de bytes preenchidos em buffer, com o 0 final */
    int LinhasBytes(int posic, int numlinhas);
            ///< Obtém quantos bytes correspondem a numlinhas
            /**< Obtém quantos bytes correspondem a avançar um determinado
             *   número de linhas
             *   @param posic Posição em TTextoBloco:Texto, começa em 0
             *   @param linhas Número de linhas
             *   @return Número de bytes, com um Instr::ex_barra_n no final */

    union {
        char ObjChar[1]; ///< Para acessar o objeto TTextoBloco como char*
        TTextoTxt * TextoTxt; ///< TTextoTxt que contém objeto
    };
    TTextoBloco * Antes;  ///< TTextoBloco anterior; se (IniFim&1)!=0
    TTextoBloco * Depois; ///< Próximo TTextoBloco; se (IniFim&2)!=0
    unsigned char Linhas; ///< Número de caracteres '\n' do bloco
    unsigned char Bytes;  ///< Número de bytes do bloco
    char Texto[1];        ///< A partir daqui: texto do bloco

private:
    void AjustarTxt();
            ///< Ajusta objetos TTextoBloco reduzindo o tamanho se possível
};

//----------------------------------------------------------------------------
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
    char Lista[TOTAL_TXTGR][TOTAL_TXTOBJ]; ///< Objetos TTextoBloco
};

//----------------------------------------------------------------------------

#endif
