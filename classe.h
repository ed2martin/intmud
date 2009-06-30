#ifndef CLASSE_H
#define CLASSE_H

class TObjeto;

//----------------------------------------------------------------------------
/** Definição das classes dos arquivos .map

Na inicialização do programa:
-# Criar classes
-# Colocar os comandos em TClasse::Comandos das classes
-# AcertaDeriv() para acertar herança de todas as classes
-# AcertaVar() para cada classe
-# Executar a função ini de cada classe
.

Para alterar uma classe:
-# Criar um novo TClasse::Comandos, mas ainda não apagar o antigo
-# Chamar AcertaDeriv() e AcertaVar(), nessa ordem
-# AcertaVar() para cada classe em ListaDeriv
-# Apagar o antigo TClasse::Comandos
.

Para criar uma classe:
-# Checar se nome de classe existe e é válido
-# Criar objeto TClasse
-# Acertar TClasse::Comandos
-# AcertaDeriv() passando um Comandos[] vazio como sendo o antigo
-# AcertaComandos() e AcertaVar()
.

Para apagar uma classe:
-# Se NumDeriv>0 a classe não pode ser apagada devido a herança
-# Se a classe tiver algum objeto:
   -# Marcar o primeiro objeto como pendente para apagar
   -# Chamar a função fim do primeiro objeto (será apagado aqui)
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
        /**< @note Antes de apagar, não deve haver nenhuma classe derivada */

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
    static TClasse * ProcuraIni(const char * nome);
        ///< Procura() que procura a primeira classe que começa com o nome
    static TClasse * ProcuraFim(const char * nome);
        ///< Procura() que procura a última classe que começa com o nome

    char * Comandos;    ///< Lista de comandos da classe
    char Nome[32];      ///< Nome da classe; não deve ser mudado

// Herança
    int Heranca(TClasse ** buffer, int tambuf);
        ///< Obtém classes herdadas de uma determinada classe
        /**< @param buffer Aonde colocar as classes obtidas
             @param tambuf Número de elementos de buffer
             @return Número de classes herdadas */

    static void AcertaDeriv();
        ///< Acerta ListaDeriv e NumDeriv de todas as classes

    void AcertaDeriv(char * comandos_antes);
        ///< Acerta ListaDeriv e NumDeriv quando Comandos[] de uma classe mudou
        /**< @param comandos_antes Conteúdo anterior de Comandos[] */

    TClasse ** ListaDeriv;
        ///< Lista de classes derivadas dessa, ou NULL se NumDeriv=0
    int NumDeriv;
        ///< Número de elementos de TClasse::ListaDeriv

// Variáveis
    void AcertaVar();
        ///< Acerta as variáveis da classe e dos objetos da classe
        /**< @note Acerta InstrVar, IndiceVar, NumVar, TamObj e TamVars.
             @note Acerta variáveis e objetos da classe, se necessário.
             @note Não acerta as classes derivadas. */

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

// Outros
    void AcertaComandos();
        ///< Usado internamente: acerta dados de Comandos
        /**< Acerta as instruções que contém desvio implícito, como
             "se", "enquanto", "efim", etc. */
};

//----------------------------------------------------------------------------
class TClasseVar /// Usado em TClasse::AcertaVar, para acertar as variáveis
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
