#ifndef CLASSE_H
#define CLASSE_H

class TObjeto;

//----------------------------------------------------------------------------
/** Defini��o das classes dos arquivos .map

Na inicializa��o do programa:
-# Criar classes
-# Colocar os comandos em TClasse::Comandos das classes
-# AcertaDeriv() para acertar heran�a de todas as classes
-# AcertaVar() para cada classe
-# Executar a fun��o ini de cada classe
.

Para alterar uma classe:
-# Criar um novo TClasse::Comandos, mas ainda n�o apagar o antigo
-# Chamar AcertaDeriv() e AcertaVar(), nessa ordem
-# AcertaVar() para cada classe em ListaDeriv
-# Apagar o antigo TClasse::Comandos
.

Para criar uma classe:
-# Checar se nome de classe existe e � v�lido
-# Criar objeto TClasse
-# Acertar TClasse::Comandos
-# AcertaDeriv() passando um Comandos[] vazio como sendo o antigo
-# AcertaComandos() e AcertaVar()
.

Para apagar uma classe:
-# Se NumDeriv>0 a classe n�o pode ser apagada devido a heran�a
-# Se a classe tiver algum objeto:
   -# Marcar o primeiro objeto como pendente para apagar
   -# Chamar a fun��o fim do primeiro objeto (ser� apagado aqui)
   -# Voltar ao item 1
   .
-# Chamar ApagaVars()
-# Apagar objeto TClasse (com delete)
.
*/
class TClasse /// Classes
{
public:
    TClasse(const char * nome);
        ///< Construtor
        /**< @param nome da classe */
    ~TClasse();
        ///< Destrutor
        /**< @note Antes de apagar, n�o deve haver nenhuma classe derivada */

    static bool NomeValido(char * nome);
        ///< Verifica se nome � um nome v�lido para classe
        /**<
         *  @param nome Nome a pesquisar
         *  @return bool (se o nome � valido) */

    static TClasse * Procura(const char * nome);
        ///< Procura uma classe a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endere�o da classe, ou 0 se n�o foi encontrada */
    static TClasse * ProcuraIni(const char * nome);
        ///< Procura() que procura a primeira classe que come�a com o nome
    static TClasse * ProcuraFim(const char * nome);
        ///< Procura() que procura a �ltima classe que come�a com o nome

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[32];      ///< Nome da classe; n�o deve ser mudado

// Heran�a
    int Heranca(TClasse ** buffer, int tambuf);
        ///< Obt�m classes herdadas de uma determinada classe
        /**< @param buffer Aonde colocar as classes obtidas
             @param tambuf N�mero de elementos de buffer
             @return N�mero de classes herdadas */

    static void AcertaDeriv();
        ///< Acerta ListaDeriv e NumDeriv de todas as classes

    void AcertaDeriv(char * comandos_antes);
        ///< Acerta ListaDeriv e NumDeriv quando Comandos[] de uma classe mudou
        /**< @param comandos_antes Conte�do anterior de Comandos[] */

    TClasse ** ListaDeriv;
        ///< Lista de classes derivadas dessa, ou NULL se NumDeriv=0
    int NumDeriv;
        ///< N�mero de elementos de TClasse::ListaDeriv

// Vari�veis
    void AcertaVar();
        ///< Acerta as vari�veis da classe e dos objetos da classe
        /**< @note Acerta InstrVar, IndiceVar, NumVar, TamObj e TamVars.
             @note Acerta vari�veis e objetos da classe, se necess�rio.
             @note N�o acerta as classes derivadas. */

    int IndiceNome(const char * nome);
        ///< Obt�m o �ndice de uma vari�vel a partir do nome
        /**<
         *  @param nome Nome da vari�vel
         *  @return �ndice em InstrVar e IndiceVar ou -1 se n�o encontrou */
    int IndiceNomeIni(const char * nome);
        ///< IndiceNome() que procura a primeira vari�vel que come�a com o nome
    int IndiceNomeFim(const char * nome);
        ///< IndiceNome() que procura a �ltima vari�vel que come�a com o nome

    char ** InstrVar;
        ///< Instru��es que definem as vari�veis (inclusive herdadas)
        /**<  @sa Instr::Comando */
    unsigned int * IndiceVar;
        ///< Localiza��o das vari�veis (inclusive herdadas) da classe
        /**<
         *  - Bits 21-0 = �ndice na mem�ria
         *  - Bit 22 =0 -> �ndice em TObjeto::Vars
         *  - Bit 22 =1 -> �ndice em TClasse::Vars
         *  - Bit 23 =1 se vari�vel definida na classe (n�o foi herdada)
         *  - Bits 31-24 = m�scara do bit, se for int1 */
    unsigned int NumVar;
        ///< N�mero de elementos de TClasse::InstrVar e TClasse::IndiceVar

// Objetos
    TObjeto * ObjetoIni; ///< Primeiro objeto da classe
    TObjeto * ObjetoFim; ///< �ltimo objeto da classe
    unsigned int NumObj; ///< N�mero de objetos
    unsigned int TamObj; ///< Tamanho de TObjeto::Vars (usado ao criar objetos)

    char * Vars;  ///< Vari�veis da classe (do tipo comum)
    unsigned int TamVars; ///< N�mero de bytes usados em TClasse::Vars

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

// Outros
    void AcertaComandos();
        ///< Usado internamente: acerta dados de Comandos
        /**< Acerta as instru��es que cont�m desvio impl�cito, como
             "se", "enquanto", "efim", etc. */
};

//----------------------------------------------------------------------------
class TClasseVar /// Usado em TClasse::AcertaVar, para acertar as vari�veis
{
public:
    TClasseVar();             ///< Construtor
    ~TClasseVar();            ///< Destrutor
    char ** InstrVar;         ///< Valor anterior de TClasse::InstrVar
    unsigned int * IndiceVar; ///< Valor anterior de TClasse::IndiceVar
    unsigned int NumVar;      ///< Valor anterior de TClasse::NumVar
    char * Vars;              ///< Valor anterior de TClasse::Vars
    unsigned int TamVars;     ///< Valor anterior de TClasse::TamVars
};

//----------------------------------------------------------------------------

#endif
