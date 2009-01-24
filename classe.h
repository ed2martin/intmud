#ifndef CLASSE_H
#define CLASSE_H

class TObjeto;

//----------------------------------------------------------------------------
/** Definição das classes dos arquivos .map */
class TClasse /// Classes
{
public:
    TClasse(const char * nome);
    ~TClasse();

    static bool NomeValido(char * nome);
        ///< Verifica se nome é um nome válido para classe
        /**<
         *  @param nome Nome a pesquisar
         *  @return bool (se o nome é valido) */

    static TClasse * Procura(const char * nome);
        ///< Procura uma classe a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endereço da classe, ou 0 se não foi encontrada */

    void AcertaComandos(); ///< Acerta dados de Comandos
    void AcertaVar();   ///< Acerta InstrVar, IndiceVar, NumVar e Vars
    void CriaVars();    ///< Acerta TClasse::Vars
    void ApagaVars();   ///< Limpa TClasse::Vars

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[32];      ///< Nome da classe; não deve ser mudado

// Herança
    TClasse ** ListaDeriv;
        ///< Lista de classes derivadas dessa, ou NULL se NumDeriv=0
    int NumDeriv;
        ///< Número de elementos de TClasse::ListaDeriv

// Variáveis
    int IndiceNome(const char * nome);
        ///< Obtém o índice de uma variável a partir do nome
        /**<
         *  @param nome Nome da variável
         *  @return Índice em InstrVar e IndiceVar ou -1 se não encontrou */
    int IndiceNomeIni(const char * nome);
        ///< IndiceNome() que procura a primeira variável que começa com o nome
    int IndiceNomeFim(const char * nome);
        ///< IndiceNome() que procura a última variável que começa com o nome

    char ** InstrVar;
        ///< Instruções que definem as variáveis (inclusive herdadas)
        /**<  @sa Instr::Comando */
    unsigned int * IndiceVar;
        ///< Localização das variáveis (inclusive herdadas) da classe
        /**<
         *  - Bits 21-0 = índice na memória
         *  - Bit 22 =0 -> índice em TObjeto::Vars
         *  - Bit 22 =1 -> índice em TClasse::Vars
         *  - Bit 23 =1 se variável definida na classe (não foi herdada)
         *  - Bits 31-24 = máscara do bit, se for int1 */
    unsigned int NumVar;
        ///< Número de elementos de TClasse::InstrVar e TClasse::IndiceVar

// Objetos
    TObjeto * ObjetoIni; ///< Primeiro objeto da classe
    TObjeto * ObjetoFim; ///< Último objeto da classe
    unsigned int NumObj; ///< Número de objetos
    unsigned int TamObj; ///< Tamanho de TObjeto::Vars (usado ao criar objetos)

    char * Vars;  ///< Variáveis da classe (do tipo comum)
    unsigned int TamVars; ///< Número de bytes usados em TClasse::Vars

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
