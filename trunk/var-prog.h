#ifndef VAR_PROG_H
#define VAR_PROG_H

//----------------------------------------------------------------------------
#include "classe.h"
class TVariavel;
class TClasse;
class TObjeto;
class TArqMapa;

//----------------------------------------------------------------------------
enum TProgConsulta ///< O que consultar, TVarProg::consulta
{
    prNada,     ///< N�o est� consultando nada
    prArquivo,  ///< iniarq() para arquivos
    prArqCl,    ///< iniarq() para classes de um arquivo
    prClasse,   ///< iniclasse()
    prFunc,     ///< inifunc()
    prFuncTudo, ///< inifunctudo()
    prHerda,    ///< iniherda()
    prHerdaTudo,///< iniherdatudo()
    prHerdaInv, ///< iniherdainv()
    prLinhaCl,  ///< inilinha() - linhas de uma classe
    prLinhaFunc,///< inilinha() - linhas de uma fun��o
    prLinhaVar  ///< inilinha() - uma vari�vel (uma �nica linha)
};

//----------------------------------------------------------------------------
/** Trata das vari�veis do tipo Prog */
class TVarProg /// Vari�veis Prog
{
public:
    void Criar();           ///< Cria objeto
    void Apagar();          ///< Apaga objeto
    void Mover(TVarProg * destino); ///< Move objeto para outro lugar
    static void LimparVar(); ///< Apaga refer�ncias do programa nas vari�veis prog
    bool Func(TVariavel * v, const char * nome); ///< Fun��o da vari�vel
    int  getValor();    ///< Ler valor num�rico da vari�vel

private:
    bool FuncExiste(TVariavel * v); ///< Processa fun��o existe
    bool FuncArquivo(TVariavel * v); ///< Processa fun��o arquivo
    bool FuncArqNome(TVariavel * v); ///< Processa fun��o arqnome
    bool FuncVarComum(TVariavel * v); ///< Processa fun��o varcomum
    bool FuncVarSav(TVariavel * v); ///< Processa fun��o varsav
    bool FuncVarNum(TVariavel * v); ///< Processa fun��o varnum
    bool FuncVarTexto(TVariavel * v); ///< Processa fun��o vartexto
    bool FuncVarTipo(TVariavel * v); ///< Processa fun��o vartipo
    bool FuncVarVetor(TVariavel * v); ///< Processa fun��o varvetor
    bool FuncConst(TVariavel * v); ///< Processa fun��o const
    bool FuncClasse(TVariavel * v); ///< Processa fun��o classe

    bool FuncIniArq(TVariavel * v); ///< Processa fun��o iniarq
    bool FuncIniClasse(TVariavel * v); ///< Processa fun��o iniclasse
    bool FuncIniFunc(TVariavel * v); ///< Processa fun��o inifunc
    bool FuncIniFuncTudo(TVariavel * v); ///< Processa fun��o inifunctudo
    bool FuncIniHerda(TVariavel * v); ///< Processa fun��o iniherda
    bool FuncIniHerdaTudo(TVariavel * v); ///< Processa fun��o iniherdatudo
    bool FuncIniHerdaInv(TVariavel * v); ///< Processa fun��o iniherdainv
    bool FuncIniLinha(TVariavel * v); ///< Processa fun��o inilinha

    bool FuncLin(TVariavel * v); ///< Processa fun��o lin
    bool FuncNivel(TVariavel * v); ///< Processa fun��o nivel
    bool FuncDepois(TVariavel * v); ///< Processa fun��o depois
    bool FuncTexto(TVariavel * v); ///< Processa fun��o texto

    bool FuncApagar(TVariavel * v);
    bool FuncCriar(TVariavel * v);
    bool FuncApagarLin(TVariavel * v);
    bool FuncCriarLin(TVariavel * v);
    bool FuncSalvar(TVariavel * v);
    bool FuncSalvarTudo(TVariavel * v);

    void MudaConsulta(TProgConsulta valor);
        ///< Muda o valor da vari�vel consulta
    TProgConsulta consulta;      ///< O que consultar
    TClasse * Classe;   ///< A classe que est� sendo consultada
    union {
        TArqMapa * ArqAtual; ///< Valor atual da busca como arquivo
        TClasse ** PontAtual;  ///< Valor atual da busca
        TClasse * ClasseAtual; ///< Valor atual da busca como classe
        const char * TextoAtual;  ///< Valor atual da busca como texto
        int       ValorAtual;  ///< Valor atual da busca como int
    };
    union {
        TClasse ** PontFim;  ///< Valor final da busca
        TClasse * ClasseFim; ///< Valor final da busca como classe
        const char * TextoFim;  ///< Valor final da busca como texto
        int       ValorFim;  ///< Valor final da busca como int
        TClasse * ClasseHerda[HERDA_TAM]; ///< Usado em IniHerda2
    };

    static TVarProg * Inicio;   ///< Primeiro objeto (com consulta!=0)
    TVarProg * Antes;           ///< Objeto anterior (se consulta!=0)
    TVarProg * Depois;          ///< Pr�ximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
