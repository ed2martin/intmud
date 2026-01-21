#ifndef VAR_PROG_H
#define VAR_PROG_H

#include "classe.h"
class TVariavel;
class TVarInfo;
class TClasse;
class TObjeto;
class TArqMapa;

//----------------------------------------------------------------------------
enum TProgConsulta ///< O que consultar, TVarProg::consulta
{
    prNada,     ///< Não está consultando nada
    prArquivo,  ///< iniarq() para arquivos
    prArqCl,    ///< iniarq() para classes de um arquivo
    prClasse,   ///< iniclasse()
    prFunc,     ///< inifunc()
    prFuncTudo, ///< inifunctudo()
    prFuncCl,   ///< inifunccl()
    prHerda,    ///< iniherda()
    prHerdaTudo,///< iniherdatudo()
    prHerdaInv, ///< iniherdainv()
    prLinhaCl,  ///< inilinha() - linhas de uma classe
    prLinhaFunc,///< inilinha() - linhas de uma função
    prLinhaVar  ///< inilinha() - uma variável (uma única linha)
};

//----------------------------------------------------------------------------
/** Trata das variáveis do tipo Prog */
class TVarProg /// Variáveis Prog
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    static void LimparVar(); ///< Apaga referências do programa nas variáveis prog
private:
    void Criar();           ///< Cria objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarProg * destino); ///< Move objeto para outro lugar
    int  getValor();    ///< Ler valor numérico da variável

    static bool FuncExiste(TVariavel * v);       ///< Processa função Existe
    static bool FuncArquivo(TVariavel * v);      ///< Processa função Arquivo
    static bool FuncArqNome(TVariavel * v);      ///< Processa função ArqNome
    static bool FuncVarComum(TVariavel * v);     ///< Processa função VarComum
    static bool FuncVarSav(TVariavel * v);       ///< Processa função VarSav
    static bool FuncVarNum(TVariavel * v);       ///< Processa função VarNum
    static bool FuncVarTexto(TVariavel * v);     ///< Processa função VarTexto
    static bool FuncVarTipo(TVariavel * v);      ///< Processa função VarTipo
    static bool FuncVarLugar(TVariavel * v);     ///< Processa função VarLugar
    static bool FuncVarVetor(TVariavel * v);     ///< Processa função VarVetor
    static bool FuncConst(TVariavel * v);        ///< Processa função Const
    static bool FuncClasse(TVariavel * v);       ///< Processa função Classe

    static bool FuncIniArq(TVariavel * v);       ///< Processa função IniArq
    static bool FuncIniClasse(TVariavel * v);    ///< Processa função IniClasse
    static bool FuncIniFunc(TVariavel * v);      ///< Processa função IniFunc
    static bool FuncIniFuncTudo(TVariavel * v);  ///< Processa função IniFuncTudo
    static bool FuncIniFuncCl(TVariavel * v);    ///< Processa função IniFuncCl
    static bool FuncIniHerda(TVariavel * v);     ///< Processa função IniHerda
    static bool FuncIniHerdaTudo(TVariavel * v); ///< Processa função IniHerdatudo
    static bool FuncIniHerdaInv(TVariavel * v);  ///< Processa função IniHerdainv
    static bool FuncIniLinha(TVariavel * v);     ///< Processa função IniLinha

    static bool FuncLin(TVariavel * v);          ///< Processa função Lin
    static bool FuncNivel(TVariavel * v);        ///< Processa função Nivel
    static bool FuncDepois(TVariavel * v);       ///< Processa função Depois
    static bool FuncTexto(TVariavel * v);        ///< Processa função Texto

    static bool FuncApagar(TVariavel * v);       ///< Processa função Apagar
    static bool FuncCriar(TVariavel * v);        ///< Processa função Criar
    static bool FuncApagarLin(TVariavel * v);    ///< Processa função ApagarLin
    static bool FuncCriarLin(TVariavel * v);     ///< Processa função CriarLin
    static bool FuncFAntes(TVariavel * v);       ///< Processa função FAntes
    static bool FuncFDepois(TVariavel * v);      ///< Processa função FDepois
    static bool FuncMudar(TVariavel * v, int lugar); ///< Chamado por FuncF*
    static bool FuncRenomear(TVariavel * v);     ///< Processa função Renomear
    static bool FuncSalvar(TVariavel * v);       ///< Processa função Salvar
    static bool FuncSalvarTudo(TVariavel * v);   ///< Processa função SalvarTudo

    static bool FuncClIni(TVariavel * v);        ///< Processa função ClIni
    static bool FuncClFim(TVariavel * v);        ///< Processa função ClFim
    static bool FuncClAntes(TVariavel * v);      ///< Processa função ClAntes
    static bool FuncClDepois(TVariavel * v);     ///< Processa função ClDepois

    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);

    void MudaConsulta(TProgConsulta valor);
        ///< Muda o valor da variável consulta
    TProgConsulta consulta;     ///< O que consultar
    TClasse * Classe;           ///< A classe que está sendo consultada
    union {
        TArqMapa * ArqAtual;    ///< Valor atual da busca como arquivo
        TClasse ** PontAtual;   ///< Valor atual da busca
        TClasse * ClasseAtual;  ///< Valor atual da busca como classe
        const char * TextoAtual; ///< Valor atual da busca como texto
        int       ValorAtual;   ///< Valor atual da busca como int
    };
    union {
        TClasse ** PontFim;     ///< Valor final da busca
        TClasse * ClasseFim;    ///< Valor final da busca como classe
        const char * TextoFim;  ///< Valor final da busca como texto
        int       ValorFim;     ///< Valor final da busca como int
        TClasse * ClasseHerda[HERDA_TAM]; ///< Usado em IniHerda2
    };

    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Próximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
