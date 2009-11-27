#ifndef VAR_TEXTO_H
#define VAR_TEXTO_H

#define TOTAL_TXTGR 62 // Número de objetos TTextoBloco em cada TTextoGrupo
#define TOTAL_TXTOBJ 0x108 // Número de bytes em cada TTextoBloco
// 62 * 0x108 = 0x3FF0

//----------------------------------------------------------------------------
class TVariavel;
class TBlocoPos;
class TTextoPos;
class TTextoBloco;

class TTextoTxt  /// Variáveis TextoTxt
{
public:
    void Apagar();          ///< Apaga objeto
    void Limpar();          ///< Apaga o texto do objeto
    void Mover(TTextoTxt * destino); ///< Move objeto para outro lugar
    void IniBloco();        ///< Cria primeiro bloco, se não existir
    void AddTexto(const char * texto, unsigned int tamtexto);
            ///< Adiciona texto no final
            /**< @note É um pouco mais eficiente que TBlocoPos::MudarTxt */

    bool Func(TVariavel * v, const char * nome); ///< Função da variável
    int  getValor();        ///< Ler valor numérico da variável

    TTextoBloco * Inicio;   ///< Aonde começa o texto
    TTextoBloco * Fim;      ///< Aonde termina o texto
    TTextoPos * Posic;      ///< Primeiro objeto da lista
    unsigned int Linhas;    ///< Número de linhas
    unsigned int Bytes;     ///< Número de bytes
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
    void Apagar();
            ///< Apaga objeto
    void Mover(TTextoPos * destino);
            ///< Move objeto para outro lugar
    void MudarTxt(TTextoTxt * obj);
            ///< Associa objeto a TTextoTxt sem texto, desassocia se obj==0
    int  Compara(TTextoPos * v);
            ///< Operador de comparação
    void Igual(TTextoPos * v);
            ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável
    int  getValor();
            ///< Ler valor numérico da variável

    TTextoTxt * TextoTxt;   ///< Variável TextoTxt correspondente
    TTextoPos * Antes;      ///< Objeto anterior
    TTextoPos * Depois;     ///< Próximo objeto
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
    char Texto[1];        ///< A partir daqui: texto do bloco
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
