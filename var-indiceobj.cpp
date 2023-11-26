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
#include "var-indiceobj.h"
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
const TVarInfo * TIndiceItem::Inicializa()
{
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
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//----------------------------------------------------------------------------
void TIndiceItem::Apagar()
{
    if (IndiceObj)
    {
        (Antes ? Antes->Depois : IndiceObj->IndiceItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
}

//----------------------------------------------------------------------------
void TIndiceItem::Mover(TIndiceItem * destino)
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
int TIndiceItem::getValor()
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
bool TIndiceItem::Func(TVariavel * v, const char * nome)
{
// Lista das funções de indiceitem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
    static const struct {
        const char * Nome;
        bool (TIndiceItem::*Func)(TVariavel * v); } ExecFunc[] = {
        { "antes",        &TIndiceItem::FuncAntes },
        { "depois",       &TIndiceItem::FuncDepois },
        { "fim",          &TIndiceItem::FuncFim },
        { "ini",          &TIndiceItem::FuncIni },
        { "obj",          &TIndiceItem::FuncObj },
        { "txt",          &TIndiceItem::FuncTxt }  };
// Procura a função correspondente e executa
    int ini = 0;
    int fim = sizeof(ExecFunc) / sizeof(ExecFunc[0]) - 1;
    char mens[80];
    copiastrmin(mens, nome, sizeof(mens));
    while (ini <= fim)
    {
        int meio = (ini + fim) / 2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado == 0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado < 0) fim = meio - 1; else ini = meio + 1;
    }
    return false;
}

//----------------------------------------------------------------------------
bool TIndiceItem::FuncObj(TVariavel * v)
{
// Nenhum argumento
    if (Instr::VarAtual == v)
    {
        if (IndiceObj == nullptr)
            return false;
        TObjeto * obj = IndiceObj->Objeto;
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
// Obtém o texto e o tamanho do texto
    char mens[100];
    *mens = 0;
    if (IndiceObj)
        copiastr(mens, IndiceObj->Nome, sizeof(mens));
// Anota o texto
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
// Objeto anterior
bool TIndiceItem::FuncAntes(TVariavel * v)
{
    if (IndiceObj == nullptr)
        return false;
    int total = 1;
    TIndiceObj * obj = IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total > 0; total--)
    {
        obj = TIndiceObj::RBprevious(obj);
        if (obj == nullptr)
            break;
        if (compara(obj->Nome, IndiceObj->Nome, TamTxt) != 0)
        {
            obj = nullptr;
            break;
        }
    }
    MudarRef(obj);
    return false;
}

//----------------------------------------------------------------------------
// Próximo objeto
bool TIndiceItem::FuncDepois(TVariavel * v)
{
    if (IndiceObj == nullptr)
        return false;
    int total = 1;
    TIndiceObj * obj = IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total > 0; total--)
    {
        obj = TIndiceObj::RBnext(obj);
        if (obj == nullptr)
            break;
        if (compara(obj->Nome, IndiceObj->Nome, TamTxt) != 0)
        {
            obj = nullptr;
            break;
        }
    }
    MudarRef(obj);
    return false;
}

//----------------------------------------------------------------------------
// Primeiro objeto
bool TIndiceItem::FuncIni(TVariavel * v)
{
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
        MudarRef(indice);
        TamTxt = strlen(txt);
        return false;
    }
    MudarRef(nullptr);
    return false;
}

//----------------------------------------------------------------------------
// Último objeto
bool TIndiceItem::FuncFim(TVariavel * v)
{
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
        MudarRef(indice);
        TamTxt = strlen(txt);
        return false;
    }
    MudarRef(nullptr);
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
        TVarInfo::FFuncVetorFalse);
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
    return ref[0].Nome[0] != 0;
}

//----------------------------------------------------------------------------
int TIndiceObj::FGetInt(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return TxtToInt(ref[0].Nome);
}

//----------------------------------------------------------------------------
double TIndiceObj::FGetDouble(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return TxtToDouble(ref[0].Nome);
}

//----------------------------------------------------------------------------
const char * TIndiceObj::FGetTxt(TVariavel * v)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    return ref[0].Nome;
}

//------------------------------------------------------------------------------
void TIndiceObj::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v1->endvar) + v1->indice;
    ref->setNome(v2->getTxt());
}

//----------------------------------------------------------------------------
void TIndiceObj::addTxt(TVariavel * v, const char * txt)
{
    TIndiceObj * ref = reinterpret_cast<TIndiceObj*>(v->endvar) + v->indice;
    char * dest = ref->Nome;
    const char * fim = dest + sizeof(Nome) - 1;
    while (*dest)
        dest++;
    while (*txt && dest < fim)
        *dest++ = *txt++;
    *dest = 0;
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
