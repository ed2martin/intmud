/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
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
void TVariavel::setInt(int valor)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        {
            int tam = Tamanho(defvar);
            if (indice)
                mprintf(&end_char[tam*indice], tam, "%d", valor);
            else
                mprintf(end_char, tam, "%d", valor);
            break;
        }
    case Instr::cInt1:
        if (numfunc)
            SetVetorInt1(this, valor);
        else if (indice)
        {
            int ind2 = indice + numbit;
            if (valor)
                end_char[ind2/8] |= (1 << (ind2 & 7));
            else
                end_char[ind2/8] &= ~(1 << (ind2 & 7));
        }
        else if (valor)
            end_char[0] |= (1 << numbit);
        else
            end_char[0] &= ~(1 << numbit);
        break;
    case Instr::cInt8:
        if (valor<-0x80)
            valor=-0x80;
        else if (valor>0x7F)
            valor=0x7F;
        end_char[indice]=valor;
        break;
    case Instr::cUInt8:
        if (valor<0)
            valor=0;
        else if (valor>0xFF)
            valor=0xFF;
        end_char[indice]=valor;
        break;
    case Instr::cInt16:
        if (valor<-0x8000)
            end_short[indice] = -0x8000;
        else if (valor>0x7FFF)
            end_short[indice] = 0x7FFF;
        else
            end_short[indice] = valor;
        break;
    case Instr::cUInt16:
        if (valor<0)
            end_ushort[indice] = 0;
        else if (valor>0xFFFF)
            end_ushort[indice] = 0xFFFF;
        else
            end_ushort[indice] = valor;
        break;
    case Instr::cUInt32:
        if (valor<0)
            valor=0;
    case Instr::cInt32:
        end_int[indice] = valor;
        break;
    case Instr::cIntInc:
        end_incdec[indice].setInc(numfunc, valor);
        break;
    case Instr::cIntDec:
        end_incdec[indice].setDec(numfunc, valor);
        break;
    case Instr::cReal:
        end_float[indice] = valor;
        break;
    case Instr::cReal2:
        end_double[indice] = valor;
        break;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cConstVar:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;
    case Instr::cVarIniFunc:
    case Instr::cVarInt:
        valor_int = valor;
        break;
    case Instr::cVarDouble:
        valor_double = valor;
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setValor(numfunc, valor);
        break;
    case Instr::cTextoTxt:
    case Instr::cTextoVar:
    case Instr::cTextoObj:
    case Instr::cNomeObj:
    case Instr::cArqDir:
    case Instr::cArqProg:
    case Instr::cArqExec:
    case Instr::cArqLog:
    case Instr::cArqSav:
    case Instr::cArqTxt:
    case Instr::cArqMem:
        break;
    case Instr::cIntTempo:
        end_inttempo[indice].setValor(numfunc, valor);
        break;
    case Instr::cIntExec:
        end_intexec[indice].setValor(valor);
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setValor(numfunc, valor);
        break;
    case Instr::cSocket:
        end_socket[indice].setValor(numfunc, valor);
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cDebug:
        TVarDebug::setValor(numfunc, valor);
        break;
    case Instr::cIndiceObj:
        {
            char mens[20];
            mprintf(mens, sizeof(mens), "%d", valor);
            end_indiceobj[indice].setNome(mens);
            break;
        }
    case Instr::cDataHora:
        end_datahora[indice].setInt(numfunc, valor);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    case Instr::cTextoVarSub:
        end_textovarsub[indice].setInt(valor);
        break;
    case Instr::cTextoObjSub:
        end_textoobjsub[indice].setObj(0);
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::setDouble(double valor)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cIntTempo:
    case Instr::cIntExec:
    case Instr::cIndiceObj:
        setInt(DoubleToInt(valor));
        break;
    case Instr::cUInt32:
        if (valor > 0xFFFFFFFFLL)
            end_uint[indice] = 0xFFFFFFFFLL;
        else if (valor < 0)
            end_uint[indice] = 0;
        else
            end_uint[indice] = (unsigned int)valor;
        break;
    case Instr::cReal:
        end_float[indice] = valor;
        break;
    case Instr::cReal2:
        end_double[indice] = valor;
        break;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cConstVar:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;
    case Instr::cVarIniFunc:
    case Instr::cVarInt:
        valor_int = DoubleToInt(valor);
        break;
    case Instr::cVarDouble:
        valor_double = valor;
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setValor(numfunc, DoubleToInt(valor));
        break;
    case Instr::cTextoTxt:
    case Instr::cTextoVar:
    case Instr::cTextoObj:
    case Instr::cNomeObj:
    case Instr::cArqDir:
    case Instr::cArqLog:
    case Instr::cArqProg:
    case Instr::cArqExec:
    case Instr::cArqSav:
    case Instr::cArqTxt:
    case Instr::cArqMem:
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setValor(numfunc, DoubleToInt(valor));
        break;
    case Instr::cSocket:
        end_socket[indice].setValor(numfunc, DoubleToInt(valor));
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cDebug:
        TVarDebug::setValor(numfunc, DoubleToInt(valor));
        break;
    case Instr::cDataHora:
        end_datahora[indice].setDouble(numfunc, valor);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    case Instr::cTextoObjSub:
        end_textoobjsub[indice].setObj(0);
        break;
    case Instr::cTextoVarSub:
        end_textovarsub[indice].setDouble(valor);
        break;
    case Instr::cTxt1:
    case Instr::cTxt2:
        {
            char mens[100];
            if (valor >= DOUBLE_MAX || valor <= -DOUBLE_MAX)
                sprintf(mens, "%E", valor);
            else
            {
                sprintf(mens, "%.9f", valor);
                char * p = mens;
                while (*p)
                    p++;
                while (p>mens && p[-1]=='0')
                    p--;
                if (p>mens && p[-1]=='.')
                    p--;
                *p=0;
            }
            setTxt(mens);
            break;
        }
    }
}

//------------------------------------------------------------------------------
void TVariavel::setTxt(const char * txt)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        {
            int tam = Tamanho(defvar);
            if (indice)
                copiastr(&end_char[tam*indice], txt, tam);
            else
                copiastr(end_char, txt, tam);
            break;
        }
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cIntTempo:
    case Instr::cIntExec:
    case Instr::cVarIniFunc:
    case Instr::cVarInt:
    case Instr::cSocket:
    case Instr::cDebug:
    case Instr::cDataHora:
        setInt(TxtToInt(txt));
        break;
    case Instr::cUInt32:
        {
            unsigned long num;
            errno=0, num=strtoul(txt, 0, 10);
            if (errno)
                num=0;
            end_uint[indice] = num;
            break;
        }
    case Instr::cReal:
        end_float[indice] = TxtToDouble(txt);
        break;
    case Instr::cReal2:
        end_double[indice] = TxtToDouble(txt);
        break;
    case Instr::cVarDouble:
        valor_double = TxtToDouble(txt);
        break;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cConstVar:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setTxt(numfunc, txt);
        break;
    case Instr::cTextoTxt:
    case Instr::cTextoVar:
    case Instr::cTextoObj:
    case Instr::cNomeObj:
    case Instr::cArqDir:
    case Instr::cArqLog:
    case Instr::cArqProg:
    case Instr::cArqExec:
    case Instr::cArqSav:
    case Instr::cArqTxt:
    case Instr::cArqMem:
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setTxt(numfunc, txt);
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cIndiceObj:
        end_indiceobj[indice].setNome(txt);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    case Instr::cTextoVarSub:
        end_textovarsub[indice].setTxt(txt);
        break;
    case Instr::cTextoObjSub:
        end_textoobjsub[indice].setObj(0);
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::addTxt(const char * txt)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        {
            int tam = Tamanho(defvar);
            char * dest = end_char + indice * tam;
            const char * fim = dest + tam - 1;
            while (*dest)
                dest++;
            while (*txt && dest<fim)
                *dest++ = *txt++;
            *dest=0;
            break;
        }
    case Instr::cTelaTxt:
        end_telatxt[indice].addTxt(numfunc, txt);
        break;
    case Instr::cIndiceObj:
        end_indiceobj[indice].addTxt(this, txt);
        break;
    case Instr::cTextoVarSub:
        end_textovarsub[indice].addTxt(txt);
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::setObj(TObjeto * obj)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cRef:
        end_varref[indice].MudarPont(obj);
        break;
    case Instr::cVarObjeto:
        endvar = obj;
        break;
    case Instr::cTextoObjSub:
        end_textoobjsub[indice].setObj(obj);
        break;
    case Instr::cTextoVarSub:
        end_textovarsub[indice].setObj(obj);
        break;
    }
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
void TVariavel::Igual(TVariavel * v)
{
    if (indice==0xFF || v->indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(v->end_listaitem[v->indice].ListaX);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].Igual(v->end_textopos + v->indice);
        break;
    case Instr::cTextoVar:
        end_textovar[indice].Igual(v->end_textovar + v->indice);
        break;
    case Instr::cSocket:
        end_socket[indice].Igual(v->end_socket + v->indice);
        break;
    case Instr::cIndiceItem:
        end_indiceitem[indice].Igual(v->end_indiceitem + v->indice);
        break;
    case Instr::cDataHora:
        end_datahora[indice].Igual(v->end_datahora + v->indice);
    }
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
