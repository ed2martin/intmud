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
    prFuncCl,   ///< inifunccl()
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
    bool FuncExiste(TVariavel * v);       ///< Processa fun��o Existe
    bool FuncArquivo(TVariavel * v);      ///< Processa fun��o Arquivo
    bool FuncArqNome(TVariavel * v);      ///< Processa fun��o ArqNome
    bool FuncVarComum(TVariavel * v);     ///< Processa fun��o VarComum
    bool FuncVarSav(TVariavel * v);       ///< Processa fun��o VarSav
    bool FuncVarNum(TVariavel * v);       ///< Processa fun��o VarNum
    bool FuncVarTexto(TVariavel * v);     ///< Processa fun��o VarTexto
    bool FuncVarTipo(TVariavel * v);      ///< Processa fun��o VarTipo
    bool FuncVarLugar(TVariavel * v);     ///< Processa fun��o VarLugar
    bool FuncVarVetor(TVariavel * v);     ///< Processa fun��o VarVetor
    bool FuncConst(TVariavel * v);        ///< Processa fun��o Const
    bool FuncClasse(TVariavel * v);       ///< Processa fun��o Classe

    bool FuncIniArq(TVariavel * v);       ///< Processa fun��o IniArq
    bool FuncIniClasse(TVariavel * v);    ///< Processa fun��o IniClasse
    bool FuncIniFunc(TVariavel * v);      ///< Processa fun��o IniFunc
    bool FuncIniFuncTudo(TVariavel * v);  ///< Processa fun��o IniFuncTudo
    bool FuncIniFuncCl(TVariavel * v);    ///< Processa fun��o IniFuncCl
    bool FuncIniHerda(TVariavel * v);     ///< Processa fun��o IniHerda
    bool FuncIniHerdaTudo(TVariavel * v); ///< Processa fun��o IniHerdatudo
    bool FuncIniHerdaInv(TVariavel * v);  ///< Processa fun��o IniHerdainv
    bool FuncIniLinha(TVariavel * v);     ///< Processa fun��o IniLinha

    bool FuncLin(TVariavel * v);          ///< Processa fun��o Lin
    bool FuncNivel(TVariavel * v);        ///< Processa fun��o Nivel
    bool FuncDepois(TVariavel * v);       ///< Processa fun��o Depois
    bool FuncTexto(TVariavel * v);        ///< Processa fun��o Texto

    bool FuncApagar(TVariavel * v);       ///< Processa fun��o Apagar
    bool FuncCriar(TVariavel * v);        ///< Processa fun��o Criar
    bool FuncApagarLin(TVariavel * v);    ///< Processa fun��o ApagarLin
    bool FuncCriarLin(TVariavel * v);     ///< Processa fun��o CriarLin
    bool FuncFAntes(TVariavel * v);       ///< Processa fun��o FAntes
    bool FuncFDepois(TVariavel * v);      ///< Processa fun��o FDepois
    bool FuncMudar(TVariavel * v, int lugar); ///< Chamado por FuncF*
    bool FuncSalvar(TVariavel * v);       ///< Processa fun��o Salvar
    bool FuncSalvarTudo(TVariavel * v);   ///< Processa fun��o SalvarTudo

    bool FuncClIni(TVariavel * v);        ///< Processa fun��o ClIni
    bool FuncClFim(TVariavel * v);        ///< Processa fun��o ClFim
    bool FuncClAntes(TVariavel * v);      ///< Processa fun��o ClAntes
    bool FuncClDepois(TVariavel * v);     ///< Processa fun��o ClDepois

    void MudaConsulta(TProgConsulta valor);
        ///< Muda o valor da vari�vel consulta
    TProgConsulta consulta;     ///< O que consultar
    TClasse * Classe;           ///< A classe que est� sendo consultada
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
    TVarProg * Depois;          ///< Pr�ximo objeto (se consulta!=0)
};

//----------------------------------------------------------------------------

#endif
