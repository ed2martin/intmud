/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variavel.h"
#include "instr.h"
#include "instr-enum.h"
#include "var-incdec.h"
#include "misc.h"

//------------------------------------------------------------------------------
int TVarIncDec::getInc(int numfunc)
{
    if (valor < 0)
        return (numfunc ? -valor : valor);
    unsigned int v = TempoIni + INTTEMPO_MAX * INTTEMPO_MAX - valor;
    return (v >= INTTEMPO_MAX * INTTEMPO_MAX ? INTTEMPO_MAX * INTTEMPO_MAX - 1 : v);
}

//------------------------------------------------------------------------------
int TVarIncDec::getDec(int numfunc)
{
    if (valor < 0)
        return (numfunc ? -valor : valor);
    unsigned int v = valor - TempoIni;
    return (v >= INTTEMPO_MAX * INTTEMPO_MAX ? 0 : v);
}

//------------------------------------------------------------------------------
void TVarIncDec::setInc(int numfunc, int v)
{
// Acerta o sinal se for função abs
    if (numfunc && (getInc(0) < 0) != (v < 0))
        v = -v;
// Contagem parada
    if (v < 0)
    {
        if (v <= -INTTEMPO_MAX * INTTEMPO_MAX)
            valor = -INTTEMPO_MAX * INTTEMPO_MAX + 1;
        else
            valor = v;
        return;
    }
// Contagem andando
    if (v >= INTTEMPO_MAX * INTTEMPO_MAX)
        v = INTTEMPO_MAX * INTTEMPO_MAX - 1;
    valor = TempoIni + INTTEMPO_MAX * INTTEMPO_MAX - v;
}

//------------------------------------------------------------------------------
void TVarIncDec::setDec(int numfunc, int v)
{
// Acerta o sinal se for função abs
    if (numfunc && (getDec(0) < 0) != (v < 0))
        v = -v;
// Contagem parada
    if (v < 0)
    {
        if (v <= -INTTEMPO_MAX * INTTEMPO_MAX)
            valor = -INTTEMPO_MAX * INTTEMPO_MAX + 1;
        else
            valor = v;
        return;
    }
// Contagem andando
    if (v >= INTTEMPO_MAX * INTTEMPO_MAX)
        v = INTTEMPO_MAX * INTTEMPO_MAX - 1;
    valor = TempoIni + v;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncInc(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs") == 0)
    {
        Instr::ApagarVar(v + 1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos") == 0)
    {
        int valor = getInc(0);
        if (valor < 0)
            setInc(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg") == 0)
    {
        int valor = getInc(0);
        if (valor > 0)
            setInc(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncDec(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "abs") == 0)
    {
        Instr::ApagarVar(v + 1);
        Instr::VarAtual->numfunc = 1;
        return true;
    }
    if (comparaZ(nome, "pos") == 0)
    {
        int valor = getDec(0);
        if (valor < 0)
            setDec(0, -valor);
        return false;
    }
    if (comparaZ(nome, "neg") == 0)
    {
        int valor = getDec(0);
        if (valor > 0)
            setDec(0, -valor);
        return false;
    }
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorInc(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int numero = 0;
    if (Instr::VarAtual > v)
    {
        numero = v[1].getInt();
        if (numero >= INTTEMPO_MAX * INTTEMPO_MAX)
            numero = INTTEMPO_MAX * INTTEMPO_MAX - 1;
    }
    numero = TempoIni + INTTEMPO_MAX * INTTEMPO_MAX - numero;
    TVarIncDec * ender = reinterpret_cast<TVarIncDec*>(v->endvar);
    for (int x = 0; x < total; x++)
        ender[x].valor = numero;
    return false;
}

//------------------------------------------------------------------------------
bool TVarIncDec::FuncVetorDec(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int numero = 0;
    if (Instr::VarAtual > v)
    {
        numero = v[1].getInt();
        if (numero >= INTTEMPO_MAX * INTTEMPO_MAX)
            numero = INTTEMPO_MAX * INTTEMPO_MAX - 1;
    }
    numero = TempoIni + numero;
    TVarIncDec * ender = reinterpret_cast<TVarIncDec*>(v->endvar);
    for (int x = 0; x < total; x++)
        ender[x].valor = numero;
    return false;
}

//------------------------------------------------------------------------------
int TVarIncDec::FTamanho(const char * instr)
{
    return sizeof(TVarIncDec);
}

//------------------------------------------------------------------------------
int TVarIncDec::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarIncDec);
}

//------------------------------------------------------------------------------
const TVarInfo * TVarIncDec::InicializaInc()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoInt,
        FRedimInc,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBoolInc,
        FGetIntInc,
        FGetDoubleInc,
        FGetTxtInc,
        TVarInfo::FGetObjNulo,
        FOperadorAtribInc,
        FuncVetorInc);
    return &var;
}

//------------------------------------------------------------------------------
const TVarInfo * TVarIncDec::InicializaDec()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoInt,
        FRedimDec,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBoolDec,
        FGetIntDec,
        FGetDoubleDec,
        FGetTxtDec,
        TVarInfo::FGetObjNulo,
        FOperadorAtribDec,
        FuncVetorDec);
    return &var;
}

//------------------------------------------------------------------------------
void TVarIncDec::FRedimInc(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].valor = TempoIni + INTTEMPO_MAX * INTTEMPO_MAX;
}
void TVarIncDec::FRedimDec(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].valor = TempoIni;
}

//------------------------------------------------------------------------------
void TVarIncDec::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    memmove(destino, v->endvar, (total ? total : 1) * sizeof(TVarIncDec));
}

//------------------------------------------------------------------------------
bool TVarIncDec::FGetBoolInc(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getInc(v->numfunc);
}
bool TVarIncDec::FGetBoolDec(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getDec(v->numfunc);
}

//------------------------------------------------------------------------------
int TVarIncDec::FGetIntInc(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getInc(v->numfunc);
}
int TVarIncDec::FGetIntDec(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getDec(v->numfunc);
}

//------------------------------------------------------------------------------
double TVarIncDec::FGetDoubleInc(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getInc(v->numfunc);
}
double TVarIncDec::FGetDoubleDec(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    return ref->getDec(v->numfunc);
}

//------------------------------------------------------------------------------
const char * TVarIncDec::FGetTxtInc(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", ref->getInc(v->numfunc));
    return buf;
}
const char * TVarIncDec::FGetTxtDec(TVariavel * v)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v->endvar) + v->indice;
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", ref->getDec(v->numfunc));
    return buf;
}

//------------------------------------------------------------------------------
void TVarIncDec::FOperadorAtribInc(TVariavel * v1, TVariavel * v2)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v1->endvar) + v1->indice;
    ref->setInc(v1->numfunc, v2->getInt());
}
void TVarIncDec::FOperadorAtribDec(TVariavel * v1, TVariavel * v2)
{
    TVarIncDec * ref = reinterpret_cast<TVarIncDec*>(v1->endvar) + v1->indice;
    ref->setDec(v1->numfunc, v2->getInt());
}
