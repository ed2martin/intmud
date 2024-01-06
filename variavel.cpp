/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "instr-enum.h"
#include "classe.h"
#include "objeto.h"
#include "var-listaobj.h"
#include "var-texto.h"
#include "var-textovar.h"
#include "var-textoobj.h"
#include "var-arqdir.h"
#include "var-arqlog.h"
#include "var-arqprog.h"
#include "var-arqexec.h"
#include "var-arqsav.h"
#include "var-arqtxt.h"
#include "var-arqmem.h"
#include "var-nomeobj.h"
#include "var-telatxt.h"
#include "var-socket.h"
#include "var-serv.h"
#include "var-prog.h"
#include "var-debug.h"
#include "var-indiceobj.h"
#include "var-datahora.h"
#include "var-basico.h"
#include "var-outros.h"
#include "var-ref.h"
#include "var-incdec.h"
#include "var-inttempo.h"
#include "var-intexec.h"
#include "misc.h"

// Se deve checar se as funções estão em letras minúsculas e em ordem crescente
//#define DEBUG_FUNC

// Mostrar algumas variáveis
//#define DEBUG_ESTAT

TVarInfo * TVariavel::VarInfo = nullptr;
TVarInfo::FuncExec * TVariavel::VarExecEnd = nullptr;
int TVariavel::VarExecFim = -1;
char * TVarInfo::EndBufferTxt = new char[0x400];
unsigned short TVarInfo::NumBufferTxt = 0;

//----------------------------------------------------------------------------
TVarInfo::TVarInfo()
{
    FTamanho = FTamanho0;
    FTamanhoVetor = FTamanho0;
    FTipo = FTipoOutros;
    FRedim = FRedim0;
    FMoverEnd = FMoverEnd0;
    FMoverDef = FMoverDef0;
    FGetBool = FGetBoolFalse;
    FGetInt = FGetInt0;
    FGetDouble = FGetDouble0;
    FGetTxt = FGetTxtVazio;
    FGetObj = FGetObjNulo;
    FOperadorAtrib = FOperadorAtribVazio;
    FOperadorAdd = FOperadorAddFalse;
    FOperadorIgual2 = FOperadorIgual2Var;
    FOperadorCompara = FOperadorComparaVar;
    FFuncTexto = FFuncTextoFalse;
    FFuncVetor = FFuncVetorFalse;
    FFuncListaEnd = nullptr;
    FFuncListaFim = -1;
}

//----------------------------------------------------------------------------
TVarInfo::TVarInfo(int (*fTamanho)(const char * instr),
        int (*fTamanhoVetor)(const char * instr),
        TVarTipo (*fTipo)(TVariavel * v),
        void (*fRedim)(TVariavel * v, TClasse * c, TObjeto * o,
                unsigned int antes, unsigned int depois),
        void (*fMoverEnd)(TVariavel * v, void * destino,
                TClasse * c, TObjeto * o),
        void (*fMoverDef)(TVariavel * v),
        bool (*fGetBool)(TVariavel * v),
        int (*fGetInt)(TVariavel * v),
        double (*fGetDouble)(TVariavel * v),
        const char * (*fGetTxt)(TVariavel * v),
        TObjeto * (*fGetObj)(TVariavel * v),
        void (*fOperadorAtrib)(TVariavel * v1, TVariavel * v2),
        bool (*fOperadorAdd)(TVariavel * v1, TVariavel * v2),
        bool (*fOperadorIgual2)(TVariavel * v1, TVariavel * v2),
        unsigned char (*fOperadorCompara)(TVariavel * v1, TVariavel * v2),
        bool (*fFuncTexto)(TVariavel * v, const char * nome),
        bool (*fFuncVetor)(TVariavel * v, const char * nome),
        FuncItem * fFuncListaEnd,
        int fFuncListaFim)
{
    FTamanho = fTamanho;
    FTamanhoVetor = fTamanhoVetor;
    FTipo = fTipo;
    FRedim = fRedim;
    FMoverEnd = fMoverEnd;
    FMoverDef = fMoverDef;
    FGetBool = fGetBool;
    FGetInt = fGetInt;
    FGetDouble = fGetDouble;
    FGetTxt = fGetTxt;
    FGetObj = fGetObj;
    FOperadorAtrib = fOperadorAtrib;
    FOperadorAdd = fOperadorAdd;
    FOperadorIgual2 = fOperadorIgual2;
    FOperadorCompara = fOperadorCompara;
    FFuncTexto = fFuncTexto;
    FFuncVetor = fFuncVetor;
    FFuncListaEnd = fFuncListaEnd;
    FFuncListaFim = fFuncListaFim;
}

//----------------------------------------------------------------------------
int TVarInfo::FTamanho0(const char * instr) { return 0; }
TVarTipo TVarInfo::FTipoOutros(TVariavel * v) { return varOutros; }
TVarTipo TVarInfo::FTipoInt(TVariavel * v) { return varInt; }
TVarTipo TVarInfo::FTipoDouble(TVariavel * v) { return varDouble; }
TVarTipo TVarInfo::FTipoTxt(TVariavel * v) { return varTxt; }
TVarTipo TVarInfo::FTipoObj(TVariavel * v) { return varObj; }
void TVarInfo::FRedim0(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois)
{
    v->endvar = nullptr;
}
void TVarInfo::FMoverEnd0(TVariavel * v, void * destino,
        TClasse * c, TObjeto * o) {}
void TVarInfo::FMoverDef0(TVariavel * v) {}
bool TVarInfo::FGetBoolFalse(TVariavel * v) { return false; }
int TVarInfo::FGetInt0(TVariavel * v) { return 0; }
double TVarInfo::FGetDouble0(TVariavel * v) { return 0; }
const char * TVarInfo::FGetTxtVazio(TVariavel * v) { return ""; }
TObjeto * TVarInfo::FGetObjNulo(TVariavel * v) { return nullptr; }
void TVarInfo::FOperadorAtribVazio(TVariavel * v1, TVariavel * v2) {}
bool TVarInfo::FOperadorAddFalse(TVariavel * v1, TVariavel * v2) { return false; }
bool TVarInfo::FOperadorIgual2Var(TVariavel * v1, TVariavel * v2)
{
    return (v1->endvar == v2->endvar && v1->indice == v2->indice &&
            v1->defvar[2] == v2->defvar[2]);
}
unsigned char TVarInfo::FOperadorComparaVar(TVariavel * v1, TVariavel * v2)
{
    return (v1->endvar == v2->endvar && v1->indice == v2->indice &&
            v1->defvar[2] == v2->defvar[2] ? 2 : 0);
}
bool TVarInfo::FFuncTextoFalse(TVariavel * v, const char * nome) { return false; }
bool TVarInfo::FFuncVetorFalse(TVariavel * v, const char * nome) { return false; }
bool TVarInfo::FFuncFalse(TVariavel * v) { return false; }

//----------------------------------------------------------------------------
void TVariavel::Inicializa()
{
    if (VarInfo)
        return;
    VarInfo = new TVarInfo[Instr::cTotalComandos];

    //TBlocoVarDec::PreparaIni();
    TExec::Inicializa();

// Variáveis
    VarInfo[Instr::cTxt1] =     *VarBaseTxt1();
    VarInfo[Instr::cTxt2] =     *VarBaseTxt2();
    VarInfo[Instr::cInt1] =     *VarBaseInt1();
    VarInfo[Instr::cInt8] =     *VarBaseInt8();
    VarInfo[Instr::cUInt8] =    *VarBaseUInt8();
    VarInfo[Instr::cInt16] =    *VarBaseInt16();
    VarInfo[Instr::cUInt16] =   *VarBaseUInt16();
    VarInfo[Instr::cInt32] =    *VarBaseInt32();
    VarInfo[Instr::cUInt32] =   *VarBaseUInt32();
    VarInfo[Instr::cReal] =     *VarBaseReal();
    VarInfo[Instr::cReal2] =    *VarBaseReal2();
    VarInfo[Instr::cConstNulo] =*VarOutrosConstNulo();
    VarInfo[Instr::cConstTxt] = *VarOutrosConstTxt();
    VarInfo[Instr::cConstNum] = *VarOutrosConstNum();
    VarInfo[Instr::cConstExpr]= *VarOutrosConstExpr();
    VarInfo[Instr::cConstVar] = *VarOutrosConstVar();
    VarInfo[Instr::cFunc] =     *VarOutrosFunc();
    VarInfo[Instr::cVarFunc] =  *VarOutrosVarFunc();
    VarInfo[Instr::cIntInc] =   *TVarIncDec::InicializaInc();
    VarInfo[Instr::cIntDec] =   *TVarIncDec::InicializaDec();
    VarInfo[Instr::cRef] =      *TVarRef::Inicializa();

// Variáveis extras
    VarInfo[Instr::cListaObj] = *TListaObj::Inicializa();
    VarInfo[Instr::cListaItem] =*TListaItem::Inicializa();
    VarInfo[Instr::cTextoTxt] = *TTextoTxt::Inicializa();
    VarInfo[Instr::cTextoPos] = *TTextoPos::Inicializa();
    VarInfo[Instr::cTextoVar] = *TTextoVar::Inicializa();
    VarInfo[Instr::cTextoObj] = *TTextoObj::Inicializa();
    VarInfo[Instr::cNomeObj] =  *TVarNomeObj::Inicializa();
    VarInfo[Instr::cArqDir] =   *TVarArqDir::Inicializa();
    VarInfo[Instr::cArqLog] =   *TVarArqLog::Inicializa();
    VarInfo[Instr::cArqProg] =  *TVarArqProg::Inicializa();
    VarInfo[Instr::cArqExec] =  *TVarArqExec::Inicializa();
    VarInfo[Instr::cArqSav] =   *TVarSav::Inicializa();
    VarInfo[Instr::cArqTxt] =   *TVarArqTxt::Inicializa();
    VarInfo[Instr::cArqMem] =   *TVarArqMem::Inicializa();
    VarInfo[Instr::cIntTempo] = *TVarIntTempo::Inicializa();
    VarInfo[Instr::cIntExec] =  *TVarIntExec::Inicializa();
    VarInfo[Instr::cTelaTxt] =  *TVarTelaTxt::Inicializa();
    VarInfo[Instr::cSocket] =   *TVarSocket::Inicializa();
    VarInfo[Instr::cServ] =     *TVarServ::Inicializa();
    VarInfo[Instr::cProg] =     *TVarProg::Inicializa();
    VarInfo[Instr::cDebug] =    *TVarDebug::Inicializa();
    VarInfo[Instr::cIndiceObj] = *TIndiceObj::Inicializa();
    VarInfo[Instr::cIndiceItem] = *TIndiceItem::Inicializa();
    VarInfo[Instr::cDataHora] = *TVarDataHora::Inicializa();

    VarInfo[Instr::cTxtFixo] =   *VarOutrosTxtFixo();
    VarInfo[Instr::cVarNome] =   *VarOutrosVarNome();
    VarInfo[Instr::cVarInicio] = *VarOutrosVarInicio();
    VarInfo[Instr::cVarIniFunc] =*VarOutrosVarIniFunc();
    VarInfo[Instr::cVarClasse] = *VarOutrosVarClasse();
    VarInfo[Instr::cVarObjeto] = *VarOutrosVarObjeto();
    VarInfo[Instr::cVarInt] =    *VarOutrosVarInt();
    VarInfo[Instr::cVarDouble] = *VarOutrosVarDouble();
    VarInfo[Instr::cTextoVarSub] =*TTextoVarSub::Inicializa();
    VarInfo[Instr::cTextoObjSub] =*TTextoObjSub::Inicializa();

// Verifica se as funções estão em letras minúsculas e em ordem crescente
#ifdef DEBUG_FUNC
    for (int tipo = 0; tipo < Instr::cTotalComandos; tipo++)
    {
        char mens[80];
        int total = VarInfo[tipo].FFuncListaFim;
        TVarInfo::FuncItem * item = VarInfo[tipo].FFuncListaEnd;
        for (int indice = 0; indice <= total; indice++)
        {
            copiastrmin(mens, item[indice].Nome, sizeof(mens));
            assert(strcmp(mens, item[indice].Nome) == 0);
        }
        for (int indice = 0; indice <= total - 1; indice++)
            assert(strcmp(item[indice].Nome, item[indice + 1].Nome) <= 0);
    }
#endif

// Obtém o número de funções somando todas as variáveis
    int totalfunc = 0, totalvar = 0;
    for (int tipo = 0; tipo < Instr::cTotalComandos; tipo++)
    {
        int total = VarInfo[tipo].FFuncListaFim;
        if (total >= 0)
            totalfunc += total + 1, totalvar++;
    }

// Aloca memória
    struct LocalFunc
    {
        const char * nome; // Nome da função
        unsigned char tipo; // Alguma constante de Instr::Comando
        unsigned char indtipo; // Índice em FFuncListaEnd da variável
        unsigned char indfunc; // Índice na lista de todas as funções
    };
    struct LocalVar
    {
        int inicio; // LocalFunc[inicio] = primeiro elemento
        int total; // LocalFunc[total] = quantidade de elementos
    };
    LocalFunc * localfunc = new LocalFunc[totalfunc * 2];
    LocalVar * localvar = new LocalVar[totalvar + 1];
    int atualfunc = 0; // localfunc[atualfunc] = próximo item que será inserido
    int atualvar = 0; // localvar[atualvar] = próxima lista que será inserida

// Preenche localfunc com os dados das funções e localvar com os dados das variáveis
    for (int tipo = 0; tipo < Instr::cTotalComandos; tipo++)
    {
        int total = VarInfo[tipo].FFuncListaFim;
        if (total < 0)
            continue;
        TVarInfo::FuncItem * item = VarInfo[tipo].FFuncListaEnd;
        localvar[atualvar].inicio = atualfunc;
        localvar[atualvar].total = total + 1;
        atualvar++;
        for (int indice = 0; indice <= total; indice++)
        {
            localfunc[atualfunc].nome = item[indice].Nome;
            localfunc[atualfunc].tipo = tipo;
            localfunc[atualfunc].indtipo = indice;
            atualfunc++;
        }
    }

// Usa localvar para ordenar os dados em localfunc
    int iniciovar = 0; // localvar[iniciovar] = lista mais antiga não verificada
    while (atualvar > 1)
    {
    // Se juntou as listas (cada 2 listas vira uma), volta ao início
        if (iniciovar >= atualvar)
        {
            atualvar = iniciovar / 2;
            iniciovar = 0;
            if (atualfunc == totalfunc * 2)
                atualfunc = 0;
            continue;
        }
        //printf("Verificando localvar[%d] de %d, atualfunc=%d de %d\n",
        //        iniciovar, atualvar, atualfunc, totalfunc);
    // Obtém início e número de elementos das duas listas que serão juntadas
        int inicio1 = localvar[iniciovar].inicio;
        int total1 = localvar[iniciovar].total;
        int inicio2 = 0, total2 = 0;
        if (iniciovar + 1 < atualvar) // Se segunda lista existe
        {
            inicio2 = localvar[iniciovar + 1].inicio;
            total2 = localvar[iniciovar + 1].total;
        }
    // Obtém a lista destino
        int destino = iniciovar / 2;
        iniciovar += 2;
        localvar[destino].inicio = atualfunc;
        localvar[destino].total = total1 + total2;
    // Preenche a lista destino
        while (total1 && total2)
        {
            if (strcmp(localfunc[inicio1].nome, localfunc[inicio2].nome) <= 0)
                localfunc[atualfunc++] = localfunc[inicio1++], total1--;
            else
                localfunc[atualfunc++] = localfunc[inicio2++], total2--;
        }
        while (total1)
            localfunc[atualfunc++] = localfunc[inicio1++], total1--;
        while (total2)
            localfunc[atualfunc++] = localfunc[inicio2++], total2--;
        assert(atualfunc <= totalfunc * 2);
    }
    assert(atualfunc == 0 || atualfunc == totalfunc);
    LocalFunc * listafunc = localfunc + (atualfunc == 0 ? totalfunc : 0);

// Verifica nomes repetidos e obtém o número de funções com nomes diferentes
    int funcnum = 0;
    const char * funcnome = listafunc[0].nome;
    for (int cont = 0; cont < totalfunc; cont++)
    {
        LocalFunc * l = listafunc + cont;
        int dif = strcmp(funcnome, l->nome);
        assert(dif <= 0);
        if (dif != 0)
            funcnome = l->nome, funcnum++;
        l->indfunc = funcnum;
    }
    funcnum++;

// Nota: se houver mais de 255 funções com nomes diferentes,
// o código do IntMUD precisa ser mudado
    assert(funcnum < 0xff);

// Constrói um vetor de funções
    VarExecEnd = new TVarInfo::FuncExec[funcnum];
    VarExecFim = funcnum - 1;
    for (int cont = 0; cont < funcnum; cont++)
    {
        VarExecEnd[cont].Nome = nullptr;
        for (int cmd = 0; cmd < Instr::cTotalComandos; cmd++)
            VarExecEnd[cont].Func[cmd] = TVarInfo::FFuncFalse;
    }
    for (int cont = 0; cont < totalfunc; cont++)
    {
        LocalFunc * l = listafunc + cont;
        VarExecEnd[l->indfunc].Nome = l->nome;
        VarExecEnd[l->indfunc].Func[l->tipo] =
                VarInfo[l->tipo].FFuncListaEnd[l->indtipo].Func;
        //assert(l->tipo < Instr::cTotalComandos && l->indfunc < funcnum);
    }

// Mostra algumas variáveis
#ifdef DEBUG_ESTAT
    printf("Func");
    int cont2 = -1;
    for (int cont1 = 0; cont1 < totalfunc; cont1++)
    {
        LocalFunc * l = listafunc + cont1;
        if (cont2 != l->indfunc)
        {
            cont2++;
            printf("\n%d", cont2);
        }
        printf(" %s", l->nome);
    }
    printf("\nTotalComandos = %d\nTVarInfo = 0x%x\n",
            Instr::cTotalComandos, (int)sizeof(TVarInfo));
    printf("totalfunc = %d\ntotalvar = %d\nfuncnum = %d\n",
            totalfunc, totalvar, funcnum);
    printf("TVarInfo::FuncExec = %d, total alocado = %d\n",
            (int)sizeof(TVarInfo::FuncExec),
           (int)sizeof(TVarInfo::FuncExec) * funcnum);
    fflush(stdout);
#endif

// Libera memória alocada
    delete[] localfunc;
    delete[] localvar;
}

//----------------------------------------------------------------------------
TVariavel::TVariavel()
{
    defvar = nullptr;
    nomevar = nullptr;
    endvar = nullptr;
    tamanho = 0;
    indice = 0;
    numbit = 0;
    numfunc = 0;
}

//----------------------------------------------------------------------------
void TVariavel::Limpar()
{
    defvar = nullptr;
    nomevar = nullptr;
    endvar = nullptr;
    tamanho = 0;
    indice = 0;
    numbit = 0;
    numfunc = 0;
}

//------------------------------------------------------------------------------
int TVariavel::FuncNum(const char * nome)
{
    int ini = 0, fim = VarExecFim;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, VarExecEnd[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return meio;
        if (resultado < 0)
            fim = meio - 1;
        else
            ini = meio + 1;
    }
    return -1;
}

//------------------------------------------------------------------------------
bool TVariavel::FuncExec(const char * nome)
{
    unsigned char cmd = (unsigned char)defvar[2];
    if (cmd >= Instr::cTotalComandos)
        return false;
    if (indice == 0xFF) // Vetor
    {
        int valor = 0;
        for (; *nome >= '0' && *nome <= '9'; nome++)
        {
            valor = valor * 10 + *nome - '0';
            if (valor >= 0xFF)
                return false;
        }
        if (*nome == 0 && valor < (unsigned char)defvar[Instr::endVetor])
        {
            indice = valor;
            Instr::ApagarVar(this + 1);
            return true;
        }
        return VarInfo[cmd].FFuncVetor(this, nome);
    }
    int fim = VarInfo[cmd].FFuncListaFim;
    if (fim < 0)
        return VarInfo[cmd].FFuncTexto(this, nome);
    TVarInfo::FuncItem * lista = VarInfo[cmd].FFuncListaEnd;
    int ini = 0;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, lista[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return lista[meio].Func(this);
        if (resultado < 0)
            fim = meio - 1;
        else
            ini = meio + 1;
    }
    return VarInfo[cmd].FFuncTexto(this, nome);
}
