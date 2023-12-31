/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-indiceobj.h"
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
const TVarInfo * TIndiceItem::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "antes",        &TIndiceItem::FuncAntes },
        { "depois",       &TIndiceItem::FuncDepois },
        { "fim",          &TIndiceItem::FuncFim },
        { "ini",          &TIndiceItem::FuncIni },
        { "obj",          &TIndiceItem::FuncObj },
        { "txt",          &TIndiceItem::FuncTxt }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
inline void TIndiceItem::Apagar()
{
    if (IndiceObj)
    {
        (Antes ? Antes->Depois : IndiceObj->IndiceItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
}

//----------------------------------------------------------------------------
inline void TIndiceItem::Mover(TIndiceItem * destino)
{
    if (IndiceObj)
    {
        (Antes ? Antes->Depois : IndiceObj->IndiceItem) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TIndiceItem));
}

//----------------------------------------------------------------------------
inline int TIndiceItem::getValor()
{
    if (IndiceObj == nullptr)
        return 0;
    return IndiceObj->Objeto != nullptr;
}

//----------------------------------------------------------------------------
TIndiceObj * TIndiceItem::getIndiceObj()
{
    return IndiceObj;
}

//----------------------------------------------------------------------------
void TIndiceItem::MudarRef(TIndiceObj * indice)
{
    if (indice == IndiceObj)
        return;
    if (IndiceObj)
    {
        (Antes ? Antes->Depois : IndiceObj->IndiceItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        IndiceObj = nullptr;
    }
    if (indice)
    {
        IndiceObj = indice;
        Antes = nullptr;
        Depois = indice->IndiceItem;
        indice->IndiceItem = this;
        if (Depois)
            Depois->Antes = this;
    }
}

//----------------------------------------------------------------------------
bool TIndiceItem::FuncObj(TVariavel * v)
{
// Nenhum argumento
    if (Instr::VarAtual == v)
    {
        TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
        if (ref->IndiceObj == nullptr)
            return false;
        TObjeto * obj = ref->IndiceObj->Objeto;
        Instr::ApagarVar(v);
        return Instr::CriarVarObj(obj);
    }
// Pelo menos um argumento
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt == 0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::Procura(txt);
        if (indice == nullptr)
            continue;
    // Obtém objeto
        TObjeto * obj = indice->Objeto;
    // Retorna o objeto encontrado
        Instr::ApagarVar(v);
        return Instr::CriarVarObj(obj);
    }
    return false;
}

//----------------------------------------------------------------------------
// Texto
bool TIndiceItem::FuncTxt(TVariavel * v)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
// Obtém o texto e o tamanho do texto
    char mens[100];
    *mens = 0;
    if (ref->IndiceObj)
        copiastr(mens, ref->IndiceObj->Nome, sizeof(mens));
// Anota o texto
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
// Objeto anterior
bool TIndiceItem::FuncAntes(TVariavel * v)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
    if (ref->IndiceObj == nullptr)
        return false;
    int total = 1;
    TIndiceObj * obj = ref->IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total > 0; total--)
    {
        obj = TIndiceObj::RBprevious(obj);
        if (obj == nullptr)
            break;
        if (compara(obj->Nome, ref->IndiceObj->Nome, ref->TamTxt) != 0)
        {
            obj = nullptr;
            break;
        }
    }
    ref->MudarRef(obj);
    return false;
}

//----------------------------------------------------------------------------
// Próximo objeto
bool TIndiceItem::FuncDepois(TVariavel * v)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
    if (ref->IndiceObj == nullptr)
        return false;
    int total = 1;
    TIndiceObj * obj = ref->IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total > 0; total--)
    {
        obj = TIndiceObj::RBnext(obj);
        if (obj == nullptr)
            break;
        if (compara(obj->Nome, ref->IndiceObj->Nome, ref->TamTxt) != 0)
        {
            obj = nullptr;
            break;
        }
    }
    ref->MudarRef(obj);
    return false;
}

//----------------------------------------------------------------------------
// Primeiro objeto
bool TIndiceItem::FuncIni(TVariavel * v)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt == 0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::ProcIni(txt);
        if (indice == nullptr)
            continue;
    // Encontrou
        ref->MudarRef(indice);
        ref->TamTxt = strlen(txt);
        return false;
    }
    ref->MudarRef(nullptr);
    return false;
}

//----------------------------------------------------------------------------
// Último objeto
bool TIndiceItem::FuncFim(TVariavel * v)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar) + v->indice;
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt == 0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::ProcFim(txt);
        if (indice == nullptr)
            continue;
    // Encontrou
        ref->MudarRef(indice);
        ref->TamTxt = strlen(txt);
        return false;
    }
    ref->MudarRef(nullptr);
    return false;
}

//------------------------------------------------------------------------------
int TIndiceItem::FTamanho(const char * instr)
{
    return sizeof(TIndiceItem);
}

//------------------------------------------------------------------------------
int TIndiceItem::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TIndiceItem);
}

//------------------------------------------------------------------------------
void TIndiceItem::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TIndiceItem * ref = reinterpret_cast<TIndiceItem*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TIndiceItem::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TIndiceItem)
}

//------------------------------------------------------------------------------
bool TIndiceItem::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TIndiceItem)
int TIndiceItem::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TIndiceItem)
double TIndiceItem::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TIndiceItem)
const char * TIndiceItem::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TIndiceItem)

//----------------------------------------------------------------------------
void TIndiceItem::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return;
    TIndiceItem * r1 = reinterpret_cast<TIndiceItem*>(v1->endvar) + v1->indice;
    TIndiceItem * r2 = reinterpret_cast<TIndiceItem*>(v2->endvar) + v2->indice;
    if (r1 == r2)
        return;
    r1->MudarRef(r2->IndiceObj);
    r1->TamTxt = r2->TamTxt;
}

//------------------------------------------------------------------------------
bool TIndiceItem::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TIndiceItem * ref1 = reinterpret_cast<TIndiceItem*>(v1->endvar) + v1->indice;
    TIndiceItem * ref2 = reinterpret_cast<TIndiceItem*>(v2->endvar) + v2->indice;
    return ref1->getIndiceObj() == ref2->getIndiceObj();
}

//------------------------------------------------------------------------------
unsigned char TIndiceItem::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return 0;
    TIndiceItem * ref1 = reinterpret_cast<TIndiceItem*>(v1->endvar) + v1->indice;
    TIndiceItem * ref2 = reinterpret_cast<TIndiceItem*>(v2->endvar) + v2->indice;
    TIndiceObj * obj1 = ref1->getIndiceObj();
    TIndiceObj * obj2 = ref2->getIndiceObj();
    return (obj1 == obj2 ? 2 : obj1 < obj2 ? 1 : 4);;
}

//----------------------------------------------------------------------------
const TVarInfo * TIndiceObj::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoTxt,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        FOperadorAdd,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        nullptr, -1);
    return &var;
}

//----------------------------------------------------------------------------
void TIndiceObj::Apagar()
{
    while (IndiceItem)
        IndiceItem->MudarRef(nullptr);
    if (*Nome)
        RBremove();
}

//----------------------------------------------------------------------------
void TIndiceObj::Mover(TIndiceObj * destino, TObjeto * o)
{
    Objeto = o;
    // Acerta TIndiceItem::IndiceObj em todos os TIndiceItem
    for (TIndiceItem * obj = IndiceItem; obj; obj = obj->Depois)
        obj->IndiceObj = destino;
    // Acerta a RBT
    if (*Nome)
    {
        if (RBroot == this)
            RBroot = destino;
        if (RBleft)
            RBleft->RBparent = destino;
        if (RBright)
            RBright->RBparent = destino;
        if (RBparent)
        {
            if (RBparent->RBleft == this)
                RBparent->RBleft = destino;
            if (RBparent->RBright == this)
                RBparent->RBright = destino;
        }
    }
    // Move
    memmove(destino, this, sizeof(TIndiceObj));
}

//----------------------------------------------------------------------------
inline void TIndiceObj::setNome(const char * texto)
{
    if (strcmp(texto, Nome) == 0)
        return;
    while (IndiceItem)
        IndiceItem->MudarRef(nullptr);
    if (*Nome)
        RBremove();
    copiastr(Nome, texto, sizeof(Nome));
    if (*Nome)
        RBinsert();
}

//------------------------------------------------------------------------------
int TIndiceObj::FTamanho(const char * instr)
{
    return sizeof(TIndiceObj);
}

//------------------------------------------------------------------------------
int TIndiceObj::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TIndiceObj);
}

//------------------------------------------------------------------------------
void TIndiceObj::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].Objeto = o;
        ref[antes].IndiceItem = nullptr;
        ref[antes].Nome[0] = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TIndiceObj::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_OBJETO(TIndiceObj)
}

//----------------------------------------------------------------------------
bool TIndiceObj::FGetBool(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return ref->Nome[0] != 0;
}

//----------------------------------------------------------------------------
int TIndiceObj::FGetInt(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return TxtToInt(ref->Nome);
}

//----------------------------------------------------------------------------
double TIndiceObj::FGetDouble(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return TxtToDouble(ref->Nome);
}

//----------------------------------------------------------------------------
const char * TIndiceObj::FGetTxt(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return ref->Nome;
}

//------------------------------------------------------------------------------
void TIndiceObj::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v1->endvar) + v1->indice;
    ref->setNome(v2->getTxt());
}

//------------------------------------------------------------------------------
bool TIndiceObj::FOperadorAdd(TVariavel * v1, TVariavel * v2)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v1->endvar) + v1->indice;
    char mens[1024];
    snprintf(mens, sizeof(mens), "%s%s", ref->Nome, v2->getTxt());
    ref->setNome(mens);
    return true;
}

//------------------------------------------------------------------------------
bool TIndiceObj::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v1->endvar) + v1->indice;
    return v2->Tipo() == varTxt && strcmp(ref->Nome, v2->getTxt()) == 0;
}

//------------------------------------------------------------------------------
unsigned char TIndiceObj::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v1->endvar) + v1->indice;
    int result = comparaZ(ref->Nome, v2->getTxt());
    return result == 0 ? 2 : result < 0 ? 1 : 4;
}

//----------------------------------------------------------------------------
TIndiceObj * TIndiceObj::Procura(const char * nome)
{
    TIndiceObj * y = RBroot;
    while (y)
    {
        int i = comparaZ(nome, y->Nome);
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
TIndiceObj * TIndiceObj::ProcIni(const char * nome)
{
    TIndiceObj * x = nullptr;
    TIndiceObj * y = RBroot;
    while (y)
    {
        switch (comparaZ(nome, y->Nome))
        {
        case -2: // string 2 contém string 1
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
TIndiceObj * TIndiceObj::ProcFim(const char * nome)
{
    TIndiceObj * x = nullptr;
    TIndiceObj * y = RBroot;
    while (y)
    {
        switch (comparaZ(nome, y->Nome))
        {
        case -2: // string 2 contém string 1
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
int TIndiceObj::RBcomp(TIndiceObj * x, TIndiceObj * y)
{
    return comparaZ(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TIndiceObj * TIndiceObj::RBroot = nullptr;
#define CLASS TIndiceObj    // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
