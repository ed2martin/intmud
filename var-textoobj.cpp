/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "var-textoobj.h"
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_MEM // Mostra vari�veis criadas e apagadas

//----------------------------------------------------------------------------
const TVarInfo * TTextoObj::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        TVarInfo::FGetBoolFalse,
        TVarInfo::FGetInt0,
        TVarInfo::FGetDouble0,
        TVarInfo::FGetTxtVazio,
        TVarInfo::FGetObjNulo,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//----------------------------------------------------------------------------
bool TTextoObj::Func(TVariavel * v, const char * nome)
{
// Lista das fun��es de indiceitem
// Deve obrigatoriamente estar em letras min�sculas e ordem alfab�tica
    static const struct {
        const char * Nome;
        bool (TTextoObj::*Func)(TVariavel * v); } ExecFunc[] = {
        { "antes",        &TTextoObj::FuncAntes },
        { "apagar",       &TTextoObj::FuncApagar },
        { "depois",       &TTextoObj::FuncDepois },
        { "fim",          &TTextoObj::FuncFim },
        { "ini",          &TTextoObj::FuncIni },
        { "limpar",       &TTextoObj::FuncLimpar },
        { "mudar",        &TTextoObj::FuncMudar },
        { "nomevar",      &TTextoObj::FuncNomeVar },
        { "total",        &TTextoObj::FuncTotal },
        { "valor",        &TTextoObj::FuncValor },
        { "valorfim",     &TTextoObj::FuncValorFim },
        { "valorini",     &TTextoObj::FuncValorIni } };
// Procura a fun��o correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
// Outro nome de vari�vel
    TTextoObjSub sub1;
    sub1.Criar(this);
    copiastr(sub1.NomeVar, nome, sizeof(sub1.NomeVar));
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarTextoObjSub))
    {
        sub1.Apagar();
        return false;
    }
    sub1.Mover(Instr::VarAtual->end_textoobjsub);
    return true;
}

//----------------------------------------------------------------------------
// Vari�vel como texto
bool TTextoObj::FuncValor(TVariavel * v)
{
    TObjeto * obj = nullptr;
    if (Instr::VarAtual >= v + 1)
    {
        TBlocoObj * bl = Procura(v[1].getTxt());
        if (bl)
            obj = bl->Objeto;
    }
    Instr::ApagarVar(v);
    if (obj == nullptr)
        return false;
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Primeira vari�vel como texto
bool TTextoObj::FuncValorIni(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = RBroot->RBfirst();
        else
            bl = ProcIni(v[1].getTxt());
    }
    if (bl == nullptr)
        return false;
    TObjeto * obj = bl->Objeto;
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// �ltima vari�vel como texto
bool TTextoObj::FuncValorFim(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = RBroot->RBlast();
        else
            bl = ProcFim(v[1].getTxt());
    }
    if (bl == nullptr)
        return false;
    TObjeto * obj = bl->Objeto;
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Nome da vari�vel
bool TTextoObj::FuncNomeVar(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl == nullptr)
        return Instr::CriarVarTxtFixo("");
    return Instr::CriarVarTexto(bl->NomeVar);
}

//----------------------------------------------------------------------------
// Mudar vari�vel
bool TTextoObj::FuncMudar(TVariavel * v)
{
    if (Instr::VarAtual >= v + 2)
    {
        char nomevar[64];
        copiastr(nomevar, v[1].getTxt());
        Mudar(nomevar, v[2].getObj());
    }
    else if (Instr::VarAtual >= v + 1)
        Mudar(v[1].getTxt(), 0);
    return false;
}

//----------------------------------------------------------------------------
// Vari�vel anterior
bool TTextoObj::FuncAntes(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ProcAntes(v[1].getTxt());
    if (Instr::VarAtual >= v + 2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp != 0 && cmp != 2) // 0=textos iguais, 2=texto 1 cont�m texto 2
            bl = nullptr;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Pr�xima vari�vel
bool TTextoObj::FuncDepois(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ProcDepois(v[1].getTxt());
    if (Instr::VarAtual >= v+2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp != 0 && cmp != 2) // 0=textos iguais, 2=texto 1 cont�m texto 2
            bl = nullptr;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// In�cio
bool TTextoObj::FuncIni(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = RBroot->RBfirst();
        else
            bl = ProcIni(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Fim
bool TTextoObj::FuncFim(TVariavel * v)
{
    TBlocoObj * bl = nullptr;
    if (RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = RBroot->RBlast();
        else
            bl = ProcFim(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Limpar
bool TTextoObj::FuncLimpar(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
    {
        Limpar();
        return false;
    }
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ProcIni(p);
        if (ini == nullptr)
            continue;
        TBlocoObj * fim = ProcFim(p);
        while (ini != fim)
        {
            TBlocoObj * bl = TBlocoObj::RBnext(ini);
            ini->Apagar();
            ini = bl;
        }
        ini->Apagar();
    }
    return false;
}

//----------------------------------------------------------------------------
// Apagar
bool TTextoObj::FuncApagar(TVariavel * v)
{
    if (Instr::VarAtual < v + 1)
    {
        if (RBroot)
            RBroot->FuncApagarSub();
        return false;
    }
    for (TVariavel * v1 = v + 1; v1<=Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ProcIni(p);
        if (ini == nullptr)
            continue;
        TBlocoObj * fim = ProcFim(p);
        while (ini != fim)
        {
            ini->Objeto->MarcarApagar();
            ini = TBlocoObj::RBnext(ini);
        }
        ini->Objeto->MarcarApagar();
    }
    return false;
}

//----------------------------------------------------------------------------
void TBlocoObj::FuncApagarSub()
{
    Objeto->MarcarApagar();
    if (RBleft)
        RBleft->FuncApagarSub();
    if (RBright)
        RBright->FuncApagarSub();
}

//----------------------------------------------------------------------------
// Total
bool TTextoObj::FuncTotal(TVariavel * v)
{
    const char * txt = "";
    int total = 0;
    if (Instr::VarAtual >= v + 1)
        txt = v[1].getTxt();
    if (*txt == 0)
    {
        total = Total;
        return Instr::CriarVarInt(v, total);
    }
    TBlocoObj * ini = ProcIni(txt);
    if (ini)
    {
        TBlocoObj * fim = ProcFim(txt);
        total = 1;
        while (ini && ini != fim)
            total++, ini = TBlocoObj::RBnext(ini);
    }
    return Instr::CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
inline void TTextoObj::Apagar()
{
    if (Objeto)
        while (RBroot)
        {
            RBroot->Objeto->MarcarApagar();
            RBroot->Apagar();
        }
    else
        while (RBroot)
            RBroot->Apagar();
    while (Inicio)
        Inicio->Apagar();
}

//----------------------------------------------------------------------------
void TTextoObj::Limpar()
{
    while (RBroot)
        RBroot->Apagar();
}

//----------------------------------------------------------------------------
inline void TTextoObj::Mover(TTextoObj * destino, TObjeto * o)
{
    Objeto = o;
    if (RBroot)
        RBroot->MoveTextoObj(destino);
    for (TTextoObjSub * obj = Inicio; obj; obj=obj->Depois)
        obj->TextoObj = destino;
    memmove(destino, this, sizeof(TTextoObj));
}

//----------------------------------------------------------------------------
void TBlocoObj::MoveTextoObj(TTextoObj * textoobj)
{
    TextoObj = textoobj;
    if (RBleft)
        RBleft->MoveTextoObj(textoobj);
    if (RBright)
        RBright->MoveTextoObj(textoobj);
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::Procura(const char * texto)
{
    TBlocoObj * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i == 0)
            return y;
        if (i < 0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return nullptr;
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::ProcIni(const char * texto)
{
    TBlocoObj * x = nullptr;
    TBlocoObj * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 cont�m string 1
        case 0:  // encontrou
            x = y;
        case -1:
            y = y->RBleft;
            break;
        default:
            y = y->RBright;
        }
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::ProcFim(const char * texto)
{
    TBlocoObj * x = nullptr;
    TBlocoObj * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
        {
        case -2: // string 2 cont�m string 1
        case 0:  // encontrou
            x = y;
            y = y->RBright;
            break;
        case -1:
            y = y->RBleft;
            break;
        default:
            y = y->RBright;
        }
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::ProcAntes(const char * texto)
{
    TBlocoObj * x = nullptr;
    TBlocoObj * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i <= 0)
            y = y->RBleft;
        else
            x=y, y = y->RBright;
    }
    return x;
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::ProcDepois(const char * texto)
{
    TBlocoObj * x = nullptr;
    TBlocoObj * y = RBroot;
    while (y)
    {
        int i = comparaVar(texto, y->NomeVar);
        if (i >= 0)
            y = y->RBright;
        else
            x=y, y = y->RBleft;
    }
    return x;
}

//----------------------------------------------------------------------------
void TTextoObj::Mudar(const char * nomevar, TObjeto * obj)
{
    TBlocoObj * bl = Procura(nomevar);
// Inserir texto (n�o est� no textoobj)
    if (bl == nullptr)
    {
        if (obj == nullptr)
            return;
        int total1 = strlen(nomevar) + 1; // Tamanho do texto
        int total2 = total1 + (bl->NomeVar - (char*)bl); // Tamanho do bloco
        bl = reinterpret_cast<TBlocoObj*>(new char[total2]);
        memcpy(bl->NomeVar, nomevar, total1);
        bl->TextoObj = this;
        bl->InsereLista(obj); // Insere na lista ligada
        bl->RBinsert();     // Insere na RBT
        Total++;
#ifdef DEBUG_MEM
        printf("Criar %p: %s\n", bl, bl->NomeVar); fflush(stdout);
#endif
    }
// Apagar texto
    else if (obj == nullptr)
        bl->Apagar();
// Alterar objeto
    else
    {
        strcpy(bl->NomeVar, nomevar); // Acerta o nome da vari�vel
        bl->RemoveLista(); // Remove da lista ligada
        bl->InsereLista(obj); // Adiciona na lista ligada
    }
}

//------------------------------------------------------------------------------
int TTextoObj::FTamanho(const char * instr)
{
    return sizeof(TTextoObj);
}

//------------------------------------------------------------------------------
int TTextoObj::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoObj);
}

//------------------------------------------------------------------------------
void TTextoObj::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].RBroot = nullptr;
        ref[antes].Inicio = nullptr;
        ref[antes].Objeto = o;
        ref[antes].Total = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoObj::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_OBJETO(TTextoObj)
}

//------------------------------------------------------------------------------
const TVarInfo * TTextoObjSub::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoObj,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        FGetObj,
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//----------------------------------------------------------------------------
void TTextoObjSub::Criar(TTextoObj * var)
{
    TextoObj = var;
    Antes = nullptr;
    Depois = var->Inicio;
    if (var->Inicio)
        var->Inicio->Antes = this;
    var->Inicio = this;
}

//----------------------------------------------------------------------------
inline void TTextoObjSub::Apagar()
{
    if (TextoObj == nullptr)
        return;
    (Antes ? Antes->Depois : TextoObj->Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    TextoObj = 0;
}

//----------------------------------------------------------------------------
inline void TTextoObjSub::Mover(TTextoObjSub * destino)
{
    if (TextoObj)
    {
        (Antes ? Antes->Depois : TextoObj->Inicio) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TTextoObjSub));
}

//----------------------------------------------------------------------------
int TTextoObjSub::getValor()
{
    if (TextoObj == nullptr)
        return 0;
    return (TextoObj->Procura(NomeVar) != nullptr);
}

//----------------------------------------------------------------------------
TObjeto * TTextoObjSub::getObj()
{
    if (TextoObj == nullptr)
        return nullptr;
    TBlocoObj * bl = TextoObj->Procura(NomeVar);
    return (bl ? bl->Objeto : nullptr);
}

//----------------------------------------------------------------------------
void TTextoObjSub::setObj(TObjeto * obj)
{
    if (TextoObj)
        TextoObj->Mudar(NomeVar, obj);
}

//------------------------------------------------------------------------------
int TTextoObjSub::FTamanho(const char * instr)
{
    return sizeof(TTextoObjSub);
}

//------------------------------------------------------------------------------
int TTextoObjSub::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TTextoObjSub);
}

//------------------------------------------------------------------------------
void TTextoObjSub::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TTextoObjSub * ref = reinterpret_cast<TTextoObjSub*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].TextoObj = nullptr;
        ref[antes].NomeVar[0] = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TTextoObjSub::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TTextoObjSub)
}

//------------------------------------------------------------------------------
bool TTextoObjSub::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TTextoObjSub)
int TTextoObjSub::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TTextoObjSub)
double TTextoObjSub::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TTextoObjSub)
const char * TTextoObjSub::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TTextoObjSub)

//------------------------------------------------------------------------------
TObjeto * TTextoObjSub::FGetObj(TVariavel * v)
{
    return reinterpret_cast<TTextoObjSub*>(v->endvar)[ v->indice].getObj();
}

//----------------------------------------------------------------------------
void TBlocoObj::InsereLista(TObjeto * obj)
{
    Objeto = obj;
    ObjAntes = nullptr;
    ObjDepois = obj->VarBlocoObj;
    if (ObjDepois)
        ObjDepois->ObjAntes = this;
    obj->VarBlocoObj = this;
}

//----------------------------------------------------------------------------
void TBlocoObj::RemoveLista()
{
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarBlocoObj) = ObjDepois;
    if (ObjDepois)
        ObjDepois->ObjAntes = ObjAntes;
}

//----------------------------------------------------------------------------
void TBlocoObj::Apagar()
{
#ifdef DEBUG_MEM
    printf("Apagar %p: %s\n", this, NomeVar); fflush(stdout);
#endif
    TextoObj->Total--;
    RBremove();        // Remove da RBT
    RemoveLista();     // Remove da lista ligada
    delete[] (char*)this;
}

//----------------------------------------------------------------------------
int TBlocoObj::RBcomp(TBlocoObj * x, TBlocoObj * y)
{
    return comparaVar(x->NomeVar, y->NomeVar);
}

//----------------------------------------------------------------------------
#define CLASS TBlocoObj          // Nome da classe
#define RBmask 1 // M�scara para bit 0
#define RBroot TextoObj->RBroot
#include "rbt.cpp.h"
