#ifndef VAR_SAV_H
#define VAR_SAV_H

//----------------------------------------------------------------------------
class TVariavel;
class TObjeto;
class TVarSav /// Variável arqsav
{
public:
    static void ProcEventos(int tempoespera);
        ///< Processa eventos
    static bool Func(TVariavel * v, const char * nome);
        ///< Funções da variável arqsav
    static int Tempo(const char * arqnome);
        ///< Obtém a quantidade de minutos para expirar
        /**< @return quantidade de minutos ou 0=expirou, -1=nunca expira */
    static int Dia;  ///< Dia em relação ao início do ano, começa no 1
    static int Hora; ///< Hora atual
    static int Min;  ///< Minuto atual

private:
    static bool ObterVar(TVariavel * var, TObjeto * obj, const char * nomevar);
        ///< Obtém uma variável "sav" a partir do nome e objeto
        /**< @param var Aonde colocar os dados da variável
         *   @param obj Objeto que contém a variável
         *   @param nomevar Nome da variável
         *   @return verdadeiro se sucesso, falso se falha (var->defvar=0) */
    static int Ler(TVariavel * v, const char * arqnome);
        ///< Lê um arquivo salvo; chamado internamente por Func()
        /**< @return Quantidade de objetos lidos */
    static int Salvar(TVariavel * v, const char * arqnome);
        ///< Salva um arquivo; chamado internamente por Func()
        /**< @return 1 se conseguiu salvar, 0 se não conseguiu */
    static void Senha(char * senhacodif, const char * senha, char fator);
        ///< Codifica senha
    static bool InicVar;
        ///< Se já inicializou variáveis
    static int HoraReg;
        ///< Tempo que corresponde a dia, hora e minuto
    static int TempoSav;
        ///< Quanto tempo checou arquivos pendentes pela última vez
};

//----------------------------------------------------------------------------
class TVarSavDir /// Diretórios pendentes - para checar arquivos expirados
{
    TVarSavDir(const char * nomedir); ///< Construtor
public:
    ~TVarSavDir();  ///< Destrutor
    static void NovoDir(const char * nomedir);
        ///< Acrescenta um diretório na lista de busca
    static bool Checa();
        ///< Checa alguns arquivos pendentes
        /**< @return true se ainda tem arquivos pendentes
             @note Deve ser chamado de tempos em tempos, até retornar false */
    static void ChecaTudo();
        ///< Checa todos os arquivos pendentes
    static bool ChecaPend();
        ///< Retorna true se tem algum arquivo pendente

private:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TVarSavDir * RBfirst(void); ///< Primeiro objeto da RBT
    static TVarSavDir * RBlast(void);  ///< Último objeto da RBT
    static TVarSavDir * RBnext(TVarSavDir *); ///< Próximo objeto da RBT
    static TVarSavDir * RBprevious(TVarSavDir *); ///< Objeto anterior da RBT
    static int RBcomp(TVarSavDir * x, TVarSavDir * y); ///< Compara objetos
    static TVarSavDir * RBroot;  ///< Objeto raiz
    TVarSavDir *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    unsigned char RBcolour;

    static TVarSavDir * Inicio; ///< endereço do primeiro objeto
    static TVarSavDir * Fim; ///< endereço do último objeto
    TVarSavDir * Proximo; ///< Endereço do próximo objeto

    char * Nome; ///< Nome do diretório que deve procurar
};

//----------------------------------------------------------------------------

#endif
