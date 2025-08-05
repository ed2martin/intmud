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
    static TVarInfo::FuncItem ListaFuncEnd[] = {
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
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        FFuncTexto,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
bool TTextoObj::FFuncTexto(TVariavel * v, const char * nome)
{
    if (nome[0] == 0)
        return false;
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TTextoObjSub sub1;
    sub1.Criar(ref);
    copiastr(sub1.NomeVar, nome, sizeof(sub1.NomeVar));
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarTextoObjSub))
    {
        sub1.Apagar();
        return false;
    }
    sub1.Mover(reinterpret_cast<TTextoObjSub*>(Instr::VarAtual->endvar));
    return true;
}

//----------------------------------------------------------------------------
// Vari�vel como texto
bool TTextoObj::FuncValor(TVariavel * v)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TObjeto * obj = nullptr;
    if (Instr::VarAtual >= v + 1)
    {
        TBlocoObj * bl = ref->Procura(v[1].getTxt());
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBfirst();
        else
            bl = ref->ProcIni(v[1].getTxt());
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBlast();
        else
            bl = ref->ProcFim(v[1].getTxt());
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl == nullptr)
        return Instr::CriarVarTxtFixo("");
    return Instr::CriarVarTexto(bl->NomeVar);
}

//----------------------------------------------------------------------------
// Mudar vari�vel
bool TTextoObj::FuncMudar(TVariavel * v)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    if (Instr::VarAtual >= v + 2)
    {
        char nomevar[64];
        copiastr(nomevar, v[1].getTxt());
        ref->Mudar(nomevar, v[2].getObj());
    }
    else if (Instr::VarAtual >= v + 1)
        ref->Mudar(v[1].getTxt(), 0);
    return false;
}

//----------------------------------------------------------------------------
// Vari�vel anterior
bool TTextoObj::FuncAntes(TVariavel * v)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->ProcAntes(v[1].getTxt());
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (Instr::VarAtual >= v + 1)
        bl = ref->ProcDepois(v[1].getTxt());
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBfirst();
        else
            bl = ref->ProcIni(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Fim
bool TTextoObj::FuncFim(TVariavel * v)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    TBlocoObj * bl = nullptr;
    if (ref->RBroot)
    {
        if (Instr::VarAtual < v + 1)
            bl = ref->RBroot->RBlast();
        else
            bl = ref->ProcFim(v[1].getTxt());
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Limpar
bool TTextoObj::FuncLimpar(TVariavel * v)
{
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    if (Instr::VarAtual < v + 1)
    {
        ref->Limpar();
        return false;
    }
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ref->ProcIni(p);
        if (ini == nullptr)
            continue;
        TBlocoObj * fim = ref->ProcFim(p);
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    if (Instr::VarAtual < v + 1)
    {
        if (ref->RBroot)
            ref->RBroot->FuncApagarSub();
        return false;
    }
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ref->ProcIni(p);
        if (ini == nullptr)
            continue;
        TBlocoObj * fim = ref->ProcFim(p);
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
    TTextoObj * ref = reinterpret_cast<TTextoObj*>(v->endvar) + v->indice;
    const char * txt = "";
    int total = 0;
    if (Instr::VarAtual >= v + 1)
        txt = v[1].getTxt();
    if (*txt == 0)
    {
        total = ref->Total;
        return Instr::CriarVarInt(v, total);
    }
    TBlocoObj * ini = ref->ProcIni(txt);
    if (ini)
    {
        TBlocoObj * fim = ref->ProcFim(txt);
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
        // Usar copiastr que é mais segura que strcpy
        copiastr(bl->NomeVar, nomevar); // Acerta o nome da variável
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
void TTextoObj::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return;
    TTextoObj * r1 = reinterpret_cast<TTextoObj*>(v1->endvar) + v1->indice;
    TTextoObj * r2 = reinterpret_cast<TTextoObj*>(v2->endvar) + v2->indice;
    if (r1 == r2)
        return;
    while (r1->RBroot)
        delete r1->RBroot;
    TBlocoObj * bl1 = (r2->RBroot ? r2->RBroot->RBfirst() : nullptr);
    for (; bl1; bl1 = TBlocoObj::RBnext(bl1))
        r1->Mudar(bl1->NomeVar, bl1->Objeto);
}

//------------------------------------------------------------------------------
bool TTextoObj::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TTextoObj * ref1 = reinterpret_cast<TTextoObj*>(v1->endvar) + v1->indice;
    TTextoObj * ref2 = reinterpret_cast<TTextoObj*>(v2->endvar) + v2->indice;
    TBlocoObj * bl1 = (ref1->RBroot ? ref1->RBroot->RBfirst() : nullptr);
    TBlocoObj * bl2 = (ref2->RBroot ? ref2->RBroot->RBfirst() : nullptr);
    while (bl1 && bl2)
    {
        if (strcmp(bl1->NomeVar, bl2->NomeVar) != 0 || bl1->Objeto != bl2->Objeto)
            return false;
        bl1 = TBlocoObj::RBnext(bl1);
        bl2 = TBlocoObj::RBnext(bl2);
    }
    return bl1 == bl2;
}

//------------------------------------------------------------------------------
unsigned char TTextoObj::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TTextoObj * ref1 = reinterpret_cast<TTextoObj*>(v1->endvar) + v1->indice;
    TTextoObj * ref2 = reinterpret_cast<TTextoObj*>(v2->endvar) + v2->indice;
    TBlocoObj * bl1 = (ref1->RBroot ? ref1->RBroot->RBfirst() : nullptr);
    TBlocoObj * bl2 = (ref2->RBroot ? ref2->RBroot->RBfirst() : nullptr);
    while (bl1 && bl2)
    {
        int cmp1 = strcmp(bl1->NomeVar, bl2->NomeVar);
        if (cmp1 != 0)
            return cmp1 < 0 ? 1 : 4;
        if (bl1->Objeto != bl2->Objeto)
            return bl1->Objeto < bl2->Objeto ? 1 : 4;
        bl1 = TBlocoObj::RBnext(bl1);
        bl2 = TBlocoObj::RBnext(bl2);
    }
    return (bl2 ? 1 : bl1 ? 4 : 0);
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
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        nullptr, -1);
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
inline int TTextoObjSub::getValor()
{
    if (TextoObj == nullptr)
        return 0;
    return (TextoObj->Procura(NomeVar) != nullptr);
}

//----------------------------------------------------------------------------
inline TObjeto * TTextoObjSub::getObj()
{
    if (TextoObj == nullptr)
        return nullptr;
    TBlocoObj * bl = TextoObj->Procura(NomeVar);
    return (bl ? bl->Objeto : nullptr);
}

//----------------------------------------------------------------------------
inline void TTextoObjSub::setObj(TObjeto * obj)
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

//------------------------------------------------------------------------------
void TTextoObjSub::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TTextoObjSub * ref = reinterpret_cast<TTextoObjSub*>(v1->endvar) + v1->indice;
    ref->setObj(v2->getObj());
}

//------------------------------------------------------------------------------
bool TTextoObjSub::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v2->Tipo() != varObj)
        return false;
    TObjeto * obj1 = reinterpret_cast<TTextoObjSub*>(v1->endvar)[v1->indice].getObj();
    return obj1 == v2->getObj();
}

//------------------------------------------------------------------------------
unsigned char TTextoObjSub::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v2->Tipo() != varObj)
        return 0;
    TObjeto * obj1 = reinterpret_cast<TTextoObjSub*>(v1->endvar)[v1->indice].getObj();
    TObjeto * obj2 = v2->getObj();
    return (obj1 == obj2 ? 2 : obj1 < obj2 ? 1 : 4);
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
