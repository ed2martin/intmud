/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

TVarInfo * TVariavel::VarInfo = nullptr;
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
    FFuncVetor = FFuncVetorFalse;
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
        bool (*fFuncVetor)(TVariavel * v, const char * nome))
{
    FTamanho = fTamanho;
    FTamanhoVetor = fTamanhoVetor;
    FTipo = fTipo;
    FRedim = fRedim;
    FMoverEnd = fMoverEnd;
    FMoverDef = fMoverDef;
    FFuncVetor = fFuncVetor;
    FGetBool = fGetBool;
    FGetInt = fGetInt;
    FGetDouble = fGetDouble;
    FGetTxt = fGetTxt;
    FOperadorAtrib = fOperadorAtrib;
    FOperadorAdd = fOperadorAdd;
    FGetObj = fGetObj;
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
bool TVarInfo::FFuncVetorFalse(TVariavel * v, const char * nome) { return false; }

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

    //printf("\nTVarInfo=0x%x\n\n", (int)sizeof(TVarInfo));
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
int TVariavel::Compara(TVariavel * v)
{
    void * var1 = endvar;
    void * var2 = v->endvar;
    if (indice!=0xFF && v->indice!=0xFF)
        switch (defvar[2])
        {
        case Instr::cListaItem:
            var1 = end_listaitem[indice].ListaX;
            var2 = v->end_listaitem[v->indice].ListaX;
            break;
        case Instr::cTextoPos:
            return end_textopos[indice].Compara(v->end_textopos + v->indice);
        case Instr::cTextoVar:
            return end_textovar[indice].Compara(v->end_textovar + v->indice);
        case Instr::cSocket:
            var1 = end_socket->Socket + indice;
            var2 = v->end_socket->Socket + v->indice;
            break;
        case Instr::cIndiceItem:
            var1 = end_indiceitem[indice].getIndiceObj();
            var2 =v->end_indiceitem[v->indice].getIndiceObj();
            break;
        case Instr::cDataHora:
            return end_datahora[indice].Compara(v->end_datahora + v->indice);
        }
    return (var1 == var2 ? 0 : var1 < var2 ? -1 : 1);
}

//------------------------------------------------------------------------------
bool TVariavel::Func(const char * nome)
{
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
            Instr::ApagarVar(this+1);
            return true;
        }

        unsigned char cmd = (unsigned char)defvar[2];
        if (cmd < Instr::cTotalComandos)
            return VarInfo[cmd].FFuncVetor(this, nome);
        return false;
    }
    switch (defvar[2])
    {
    case Instr::cIntInc:
        return end_incdec->FuncInc(this, nome);
    case Instr::cIntDec:
        return end_incdec->FuncDec(this, nome);
    case Instr::cIntTempo:
        return end_inttempo->Func(this, nome);
    case Instr::cListaObj:
        return end_listaobj[indice].Func(this, nome);
    case Instr::cListaItem:
        return end_listaitem[indice].Func(this, nome);
    case Instr::cTextoTxt:
        return end_textotxt[indice].Func(this, nome);
    case Instr::cTextoPos:
        return end_textopos[indice].Func(this, nome);
    case Instr::cTextoVar:
        return end_textovar[indice].Func(this, nome);
    case Instr::cTextoObj:
        return end_textoobj[indice].Func(this, nome);
    case Instr::cNomeObj:
        return end_nomeobj[indice].Func(this, nome);
    case Instr::cArqDir:
        return end_arqdir[indice].Func(this, nome);
    case Instr::cArqLog:
        return end_arqlog[indice].Func(this, nome);
    case Instr::cArqProg:
        return end_arqprog[indice].Func(this, nome);
    case Instr::cArqExec:
        return end_arqexec[indice].Func(this, nome);
    case Instr::cArqSav:
        return TVarSav::Func(this, nome);
    case Instr::cArqTxt:
        return end_arqtxt[indice].Func(this, nome);
    case Instr::cArqMem:
        return end_arqmem[indice].Func(this, nome);
    case Instr::cTelaTxt:
        return end_telatxt[indice].Func(this, nome);
    case Instr::cSocket:
        return end_socket[indice].Func(this, nome);
    case Instr::cServ:
        return end_serv[indice].Func(this, nome);
    case Instr::cProg:
        return end_prog[indice].Func(this, nome);
    case Instr::cDebug:
        return TVarDebug::Func(this, nome);
    case Instr::cIndiceItem:
        return end_indiceitem[indice].Func(this, nome);
    case Instr::cDataHora:
        return end_datahora[indice].Func(this, nome);
    }
    return false;
}
