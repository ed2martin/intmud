#ifndef VAR_SAV_H
#define VAR_SAV_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarSav /// Vari�vel arqsav
{
public:
    static void ProcEventos(int tempoespera);
        ///< Processa eventos
    static bool Func(TVariavel * v, const char * nome);
        ///< Fun��es da vari�vel arqsav
    static int Tempo(const char * arqnome);
        ///< Obt�m a quantidade de minutos para expirar
        /// Retorna: quantidade de minutos ou 0=expirou, -1=nunca expira
    static int Dia;  ///< Dia em rela��o ao in�cio do ano, come�a no 1
    static int Hora; ///< Hora atual
    static int Min;  ///< Minuto atual

private:
    static void Senha(char * senhacodif, const char * senha, char fator);
        ///< Codifica senha
    static bool InicVar;
        ///< Se j� inicializou vari�veis
    static int HoraReg;
        ///< Tempo que corresponde a dia, hora e minuto
    static int TempoSav;
        ///< Quanto tempo checou arquivos pendentes pela �ltima vez
};

//----------------------------------------------------------------------------
class TVarSavDir /// Diret�rios pendentes - para checar arquivos expirados
{
    TVarSavDir(const char * nomedir); ///< Construtor
public:
    ~TVarSavDir();  ///< Destrutor
    static void NovoDir(const char * nomedir);
        ///< Acrescenta um diret�rio na lista de busca
    static bool Checa();
        ///< Checa alguns arquivos pendentes
        /**< @return true se ainda tem arquivos pendentes
             @note Deve ser chamado de tempos em tempos, at� retornar false */
    static void ChecaTudo();
        ///< Checa todos os arquivos pendentes
    static bool ChecaPend();
        ///< Retorna true se tem algum arquivo pendente

private:
    void RBinsert(void);        ///< Insere objeto na RBT
    void RBremove(void);        ///< Remove objeto da RBT
    static TVarSavDir * RBfirst(void); ///< Primeiro objeto da RBT
    static TVarSavDir * RBlast(void);  ///< �ltimo objeto da RBT
    static TVarSavDir * RBnext(TVarSavDir *); ///< Pr�ximo objeto da RBT
    static TVarSavDir * RBprevious(TVarSavDir *); ///< Objeto anterior da RBT
    static int RBcomp(TVarSavDir * x, TVarSavDir * y); ///< Compara objetos
    static TVarSavDir * RBroot;  ///< Objeto raiz
    TVarSavDir *RBleft,*RBright,*RBparent; ///< Objetos filhos e objeto pai
    void RBleft_rotate(void);
    void RBright_rotate(void);
    unsigned char RBcolour;

    static TVarSavDir * Inicio; ///< endere�o do primeiro objeto
    static TVarSavDir * Fim; ///< endere�o do �ltimo objeto
    TVarSavDir * Proximo; ///< Endere�o do pr�ximo objeto

    char * Nome; ///< Nome do diret�rio que deve procurar
};

//----------------------------------------------------------------------------

#endif
