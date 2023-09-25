#ifndef VAR_DEBUG_H
#define VAR_DEBUG_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Debug */
class TVarDebug /// Variáveis Debug
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void Criar();           ///< Cria objeto
            /**< Após criado, acertar defvar, indice e chamar EndObjeto() */
    void Apagar();          ///< Apaga objeto
    void Mover(TVarDebug * destino); ///< Move TVarSock para outro lugar
    void EndObjeto(TClasse * c, TObjeto * o);
    static void FuncEvento(const char * evento, const char * texto);
        ///< Executa uma função
        /**< @param evento Nome do evento (ex. "msg")
         *   @param texto Texto do primeiro argumento, 0=nenhum texto */

    static bool Func(TVariavel * v, const char * nome);
        ///< Função da variável
    static TVarTipo FTipo(TVariavel * v);
        ///< Retorna o tipo de variável
    static int  getInt(int numfunc);
        ///< Ler valor numérico da variável como int
    static double getDouble(int numfunc);
        ///< Ler valor numérico da variável como double
    static void setValor(int numfunc, int valor);
        ///< Alterar valor numérico da variável
    static void Exec();
        ///< Para executar passo-a-passo

    const char * defvar;    ///< Como foi definida a variável
    union {
        TClasse * endclasse;///< Em que classe está definido
        TObjeto * endobjeto;///< Em que objeto está definido
    };
    bool b_objeto;          ///< O que usar: true=endobjeto, false=endclasse
    unsigned char indice;   ///< Índice no vetor

    static TVarDebug * ObjAtual; ///< Objeto atual, usado em FuncEvento()
    static TVarDebug * Inicio;   ///< Primeiro objeto
    TVarDebug * Antes;    ///< Objeto anterior
    TVarDebug * Depois;   ///< Próximo objeto

private:
    static bool FuncIni(TVariavel * v);    ///< Processa função Ini
    static bool FuncExec(TVariavel * v);   ///< Processa função Exec
    static bool FuncUtempo(TVariavel * v); ///< Processa função Utempo
    static bool FuncStempo(TVariavel * v); ///< Processa função Stempo
    static bool FuncMem(TVariavel * v);    ///< Processa função Mem
    static bool FuncMemMax(TVariavel * v); ///< Processa função MemMax
    static bool FuncFunc(TVariavel * v);   ///< Processa função Func
    static bool FuncCmd(TVariavel * v);    ///< Processa função Cmd
    static bool FuncPasso(TVariavel * v);  ///< Processa função Passo
    static bool FuncVer(TVariavel * v);    ///< Processa função Ver
    static bool FuncData(TVariavel * v);   ///< Processa função Data

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
};

//----------------------------------------------------------------------------

#endif
