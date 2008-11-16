#ifndef CLASSE_H
#define CLASSE_H

class TObjeto;

//----------------------------------------------------------------------------
/** Defini��o das classes dos arquivos .map */
class TClasse /// Classes
{
public:
    TClasse(const char * nome); /// Construtor
        /** @param nome nome da classe */
    ~TClasse(); /// Destrutor
        /** @warning Antes de apagar uma classe,
                     deve-se apagar as classes derivadas */

    static bool NomeClasse(char * nome);
        /// Verifica se nome � um nome v�lido para classe
        /**
         *  @param nome Nome a pesquisar
         *  @return bool (se o nome � valido) */

    static TClasse * Procura(const char * nome);
        /// Procura uma classe a partir do nome
        /**
         *  @param nome Nome a pesquisar
         *  @return Endere�o da classe, ou 0 se n�o foi encontrada */

    void AcertaComandos(); ///< Acerta dados de Comandos
    void AcertaVar();   ///< Acerta ListaVar e NumVar

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[32];      ///< Nome da classe; n�o deve ser mudado

// Heran�a
    TClasse ** ListaDeriv;
        ///< Lista de classes derivadas dessa, ou NULL se NumDeriv=0
    int NumDeriv;
        ///< N�mero de elementos de TClasse::ListaDeriv

// Vari�veis
    int IndiceNome(const char * nome);
        ///< Obt�m o �ndice de uma vari�vel a partir do nome
        /**<
         *  @param nome Nome da vari�vel
         *  @return �ndice em InstrVar e IndiceVar ou -1 se n�o encontrou */
    int IndiceNome2(const char * nome);
        ///< Obt�m �ndice de uma vari�vel a partir do nome
        /**< Semelhante a IndiceNome(), exceto que procura a primeira
         *   vari�vel que come�a com o nome especificado.
         *  @param nome Nome da vari�vel
         *  @return �ndice em InstrVar e IndiceVar ou -1 se n�o encontrou */

    char ** InstrVar;
        ///< Instru��es que definem as vari�veis (inclusive herdadas)
        /**<  @sa Instr::Comando */
    unsigned int * IndiceVar;
        ///< Localiza��o das vari�veis (inclusive herdadas) da classe
        /**<
         *  - Bits 21-0 = �ndice na mem�ria
         *  - Bit 22 =0 -> �ndice em TObjeto::Vars
         *  - Bit 22 =1 -> �ndice em TClasse::Vars
         *  - Bit 23 =1 se vari�vel definida na classe
         *  - Bits 31-24 = m�scara do bit, se for int1 */
    unsigned int NumVar;
        ///< N�mero de elementos de TClasse::InstrVar e TClasse::IndiceVar

// Objetos
    TObjeto * ObjetoIni; ///< Primeiro objeto da classe
    TObjeto * ObjetoFim; ///< �ltimo objeto da classe
    unsigned int NumObj; ///< N�mero de objetos
    unsigned int TamObj; ///< Tamanho de TObjeto::Vars (usado ao criar objetos)

    char * Vars;  ///< Vari�veis da classe (do tipo comum)

// �rvore organizada por TClasse::Nome
public:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TClasse * RBfirst(void); ///< Primeiro objeto da RBT
    static TClasse * RBlast(void);  ///< �ltimo objeto da RBT
    static TClasse * RBnext(TClasse *); ///< Pr�ximo objeto da RBT
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
