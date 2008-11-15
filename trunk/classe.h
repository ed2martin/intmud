#ifndef CLASSE_H
#define CLASSE_H

//----------------------------------------------------------------------------
/** Definição das classes dos arquivos .map */
class TClasse /// Classes
{
public:

    TClasse(const char * nome); /**<
         Construtor
         @param nome nome da classe */
    ~TClasse(); /**<
         Destrutor
         @warning Antes de apagar uma classe, deve-se apagar as classes derivadas */

    static bool NomeClasse(char * nome); /**<
         Verifica se nome é um nome válido para classe
         @param nome Nome a pesquisar
         @return bool (se o nome é valido) */

    static TClasse * Procura(const char * nome); /**<
         Procura uma classe a partir do nome
         @param nome Nome a pesquisar
         @return Endereço da classe, ou 0 se não foi encontrada */

    void AcertaComandos(); /**< Acerta dados de Comandos */
    void AcertaVar(); /**< Acerta ListaVar e NumVar */

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[32];      ///< Nome da classe; não deve ser mudado


/** @defgroup herda TClasse Herança
    @par            classes derivadas dessa */
/** @{ */
    TClasse ** ListaDeriv; /**< Lista de classes; é NULL se NumDeriv=0 */
    int NumDeriv; /**< Número de elementos de ListaDeriv */
/** @} */


/** @defgroup vars  TClasse Variáveis
    @par            Variáveis da classe, inclusive as herdadas */
/** @{ */
    char ** InstrVar; /**< Instruções que definem as variáveis */
    unsigned int * IndiceVar; /**< Localização das variáveis da classe
                   - Bits 23-0 = índice na memória
                   - Bit 22 =0 -> índice em TObjeto::Vars
                   - Bit 22 =1 -> índice em TClasse::Vars
                   - Bit 23 =1 se variável definida na classe
                   - Bits 31-24 = máscara do bit, se for int1 */
    unsigned int NumVar; /**< Número de elementos de InstrVar e IndiceVar */
/** @} */


    unsigned int TamObj; ///< Tamanho de TObjeto::Vars (usado ao criar objetos)

    char * Vars;  ///< Variáveis da classe (do tipo comum)

// Árvore organizada por TClasse::Nome
public:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TClasse * RBfirst(void); ///< Primeiro objeto da RBT
    static TClasse * RBlast(void);  ///< Último objeto da RBT
    static TClasse * RBnext(TClasse *); ///< Próximo objeto da RBT
    static TClasse * RBprevious(TClasse *); ///< Objeto anterior da RBT
private:
    static TClasse * RBroot;  ///< Objeto raiz
    TClasse *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    static int RBcomp(TClasse * x, TClasse * y); ///< Compara objetos
    unsigned char RBcolour;
};

//----------------------------------------------------------------------------

#endif
