/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "var-listaobj.h"
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "instr.h"
#include "instr-enum.h"
#include "random.h"
#include "misc.h"

//#define DEBUG  // Checar listaobj e listaitem
//#define DEBUG_MSG
//#define DEBUG_MEM // Verifica se est� alocando corretamente

#include "alocamem.h"
#ifdef DEBUG_MEM
#define DEBUG1(x) TListaX::Debug(x);
//#define DEBUG1(x) printf("(%d)", __LINE__); fflush(stdout); TListaX::Debug(x);
#else
#define DEBUG1(x)
#endif

TListaX * TListaX::EndMover = nullptr;
//static TAlocaMem2<TListaX, 250, unsigned char> AlocaMem;
static TAlocaMemSimples<TListaX, 1024> AlocaMem;
//static TAlocaMemSistema<TListaX> AlocaMem;

//----------------------------------------------------------------------------
const TVarInfo * TListaObj::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "addfim",    &TListaObj::FuncAddFim },
        { "addfim1",   &TListaObj::FuncAddFim1 },
        { "addini",    &TListaObj::FuncAddIni },
        { "addini1",   &TListaObj::FuncAddIni1 },
        { "apagar",    &TListaObj::FuncApagar },
        { "fim",       &TListaObj::FuncFim },
        { "ini",       &TListaObj::FuncIni },
        { "inverter",  &TListaObj::FuncInverter },
        { "limpar",    &TListaObj::FuncLimpar },
        { "objfim",    &TListaObj::FuncObjFim },
        { "objini",    &TListaObj::FuncObjIni },
        { "objlista",  &TListaObj::FuncObjLista },
        { "possui",    &TListaObj::FuncPossui },
        { "rand",      &TListaObj::FuncRand },
        { "remove",    &TListaObj::FuncRemove },
        { "total",     &TListaObj::FuncTotal }  };
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
        TVarInfo::FOperadorAtribVazio,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FOperadorIgual2Var,
        TVarInfo::FOperadorComparaVar,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
inline void TListaObj::Apagar()
{
    if (Objeto)
        while (Inicio)
        {
            Inicio->Objeto->MarcarApagar();
            Inicio->Apagar();
        }
    else
        while (Inicio)
            Inicio->Apagar();
    DEBUG1(this)
}

//----------------------------------------------------------------------------
inline void TListaObj::Mover(TListaObj * destino, TObjeto * o)
{
    Objeto = o;
    // Acerta TListaX::Lista em todos os TListaX da lista
    for (TListaX * obj = Inicio; obj; obj = obj->ListaDepois)
        obj->Lista = destino;
    memmove(destino, this, sizeof(TListaObj));
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddInicio(TObjeto * obj)
{
    if (obj == nullptr)
        return nullptr;
// Cria objeto
    TListaX * l1 = TListaX::Criar();
// Acrescenta no topo da lista
    Total++;
    l1->Lista = this;
    l1->ListaAntes = nullptr;
    l1->ListaDepois = Inicio;
    if (Inicio)
        Inicio->ListaAntes = l1;
    Inicio = l1;
    if (Fim == nullptr)
        Fim = l1;
// Acrescenta objeto
    l1->Objeto = obj;
    l1->ObjAntes = nullptr;
    l1->ObjDepois = obj->VarListaX;
    if (obj->VarListaX)
        obj->VarListaX->ObjAntes = l1;
    obj->VarListaX = l1;
// Acerta ListaItem
    l1->ListaItem = nullptr;
    DEBUG1(this)
    return l1;
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddFim(TObjeto * obj)
{
    if (obj == nullptr)
        return nullptr;
// Cria objeto
    TListaX * l1 = TListaX::Criar();
    Total++;
// Acrescenta no final da lista
    l1->Lista = this;
    l1->ListaAntes = Fim;
    l1->ListaDepois = nullptr;
    if (Fim)
        Fim->ListaDepois = l1;
    Fim = l1;
    if (Inicio == nullptr)
        Inicio = l1;
// Acrescenta objeto
    l1->Objeto = obj;
    l1->ObjAntes = nullptr;
    l1->ObjDepois = obj->VarListaX;
    if (obj->VarListaX)
        obj->VarListaX->ObjAntes = l1;
    obj->VarListaX = l1;
// Acerta ListaItem
    l1->ListaItem = nullptr;
    DEBUG1(this)
    return l1;
}

//----------------------------------------------------------------------------
inline int TListaObj::getValor()
{
    return (Inicio != nullptr);
}

//----------------------------------------------------------------------------
TListaX * TListaObj::AddLista(TVariavel * v, TListaX * litem, int tipo)
{
    TListaX * retorno = nullptr;
    tipo &= 7;

// Verifica adicionar com refer�ncia a litem
    if (tipo & 2)
    {
        if (litem == nullptr)
            return nullptr;
        assert(litem->Lista == this);
    }

// Marca objetos que ainda n�o est�o na lista
    if (tipo >= 4)
    {
        for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
        {
            TObjeto * obj = v1->getObj();
            if (obj)
                obj->MarcaLista = 1;
            else if (v1->defvar[2] == Instr::cListaObj && v1->indice != 0xff)
            {
                TListaObj * lista1 =
                        reinterpret_cast<TListaObj*>(v1->endvar) + v1->indice;
                for (TListaX * l1 = lista1->Inicio; l1; l1 = l1->ListaDepois)
                    l1->Objeto->MarcaLista = 1;
            }
        }
        for (TListaX * l1 = Inicio; l1; l1 = l1->ListaDepois)
            l1->Objeto->MarcaLista = 0;
    }

// Adiciona objetos
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        TListaX * lini = nullptr;
        TObjeto * obj;
        if (v1->defvar[2] == Instr::cListaObj && v1->indice != 0xff)
        {
            TListaObj * lista1 =
                    reinterpret_cast<TListaObj*>(v1->endvar) + v1->indice;
            if (lista1 == this) // Mesma lista
                continue;
            lini = lista1->Inicio;
            if (lini == nullptr)
                continue;
            obj = lini->Objeto;
        }
        else
        {
            obj = v1->getObj();
            if (obj == nullptr)
                continue;
        }
        while (true)
        {
            if (tipo < 4 || obj->MarcaLista)
            {
                obj->MarcaLista = 0;
            // Cria objeto
                TListaX * l1 = TListaX::Criar();
                l1->Lista = this;
                l1->ListaItem = nullptr;
                Total++;
            // Acrescenta na lista
                switch (tipo)
                {
                case 0: // Adicionar no in�cio da lista
                case 4:
                    l1->ListaAntes = nullptr;
                    l1->ListaDepois = Inicio;
                    if (Inicio)
                        Inicio->ListaAntes = l1;
                    Inicio = l1;
                    if (Fim == nullptr)
                        Fim = l1;
                    tipo=(tipo & 4) + 3;
                    break;
                case 1: // Adicionar no fim da lista
                case 5:
                    l1->ListaAntes = Fim;
                    l1->ListaDepois = nullptr;
                    if (Fim)
                        Fim->ListaDepois = l1;
                    Fim = l1;
                    if (Inicio == nullptr)
                        Inicio = l1;
                    tipo=(tipo & 4) + 3;
                    break;
                case 2: // Adicionar antes de objeto
                case 6:
                    l1->ListaAntes = litem->ListaAntes;
                    l1->ListaDepois = litem;
                    (l1->ListaAntes ? l1->ListaAntes->ListaDepois :
                                      l1->Lista->Inicio) = l1;
                    litem->ListaAntes = l1;
                    tipo=(tipo & 4) + 3;
                    break;
                case 3: // Adicionar depois de objeto
                case 7:
                    l1->ListaAntes = litem;
                    l1->ListaDepois = litem->ListaDepois;
                    (l1->ListaDepois ? l1->ListaDepois->ListaAntes :
                                       l1->Lista->Fim) = l1;
                    litem->ListaDepois = l1;
                    break;
                }
            // Acrescenta no objeto
                l1->Objeto = obj;
                l1->ObjAntes = nullptr;
                l1->ObjDepois = obj->VarListaX;
                if (obj->VarListaX)
                    obj->VarListaX->ObjAntes = l1;
                obj->VarListaX = l1;
            // Acerta litem
                litem = l1;
            // Acerta retorno
                if (retorno == nullptr)
                    retorno = l1;
                DEBUG1(this)
            }

        // Passa para o pr�ximo objeto da lista
            if (lini == nullptr)
                break;
            lini = lini->ListaDepois;
            if (lini == nullptr)
                break;
            obj = lini->Objeto;
        }
    }
    return retorno;
}

//----------------------------------------------------------------------------
// Primeiro item da lista
bool TListaObj::FuncIni(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    bool checacl = false;
    TClasse * cl = nullptr;
    if (Instr::VarAtual >= v + 1)
        cl = TClasse::Procura(v[1].getTxt()), checacl = true;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    TListaX * obj;
    if (!checacl)
        obj = ref->Inicio;
    else if (cl == nullptr)
        obj = nullptr;
    else
        for (obj = ref->Inicio; obj && obj->Objeto->Classe != cl; )
            obj = obj->ListaDepois;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(obj);
    DEBUG1(ref)
    return true;
}

//----------------------------------------------------------------------------
// �ltimo item da lista
bool TListaObj::FuncFim(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    bool checacl = false;
    TClasse * cl = nullptr;
    if (Instr::VarAtual >= v + 1)
        cl = TClasse::Procura(v[1].getTxt()), checacl = true;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    TListaX * obj;
    if (!checacl)
        obj = ref->Fim;
    else if (cl == nullptr)
        obj = nullptr;
    else
        for (obj = ref->Fim; obj && obj->Objeto->Classe != cl; )
            obj = obj->ListaAntes;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(obj);
    DEBUG1(ref)
    return true;
}

//----------------------------------------------------------------------------
// Objeto em que foi definido
bool TListaObj::FuncObjLista(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    if (ref->Objeto == nullptr)
        return false;
    TObjeto * obj = ref->Objeto;
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Primeiro objeto da lista
bool TListaObj::FuncObjIni(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * lista = ref->Inicio;
    if (Instr::VarAtual >= v + 1)
    {
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            lista = nullptr;
        else
            while (lista && lista->Objeto->Classe != cl)
                lista = lista->ListaDepois;
    }
    TObjeto * obj = (lista ? lista->Objeto : nullptr);
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// �ltimo objeto da lista
bool TListaObj::FuncObjFim(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * lista = ref->Fim;
    if (Instr::VarAtual >= v + 1)
    {
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl == nullptr)
            lista = nullptr;
        else
            while (lista && lista->Objeto->Classe != cl)
                lista = lista->ListaAntes;
    }
    TObjeto * obj = (lista ? lista->Objeto : nullptr);
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Adiciona objetos na lista
bool TListaObj::FuncAddIni(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * valor = ref->AddLista(v, nullptr, 0);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaObj::FuncAddFim(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * valor = ref->AddLista(v, nullptr, 1);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaObj::FuncAddIni1(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * valor = ref->AddLista(v, nullptr, 4);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaObj::FuncAddFim1(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * valor = ref->AddLista(v, nullptr, 5);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
// Remove objeto da lista
bool TListaObj::FuncRemove(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
// Nenhum objeto: remove objetos duplicados
    if (Instr::VarAtual < v + 1)
    {
        int total = 0;
        for (TListaX * l1 = ref->Inicio; l1; l1 = l1->ListaDepois)
            l1->Objeto->MarcaLista = 0;
        for (TListaX * l1 = ref->Inicio; l1; )
        {
            if (l1->Objeto->MarcaLista == 0)
            {
                l1->Objeto->MarcaLista = 1;
                l1 = l1->ListaDepois;
                continue;
            }
            TListaX::EndMover = l1->ListaDepois;
            l1->Apagar();
            l1 = TListaX::EndMover, total++;
        }
        DEBUG1(ref)
        return Instr::CriarVarInt(v, total);
    }
// Remover um objeto
    if (Instr::VarAtual == v + 1 && v[1].defvar[2] != Instr::cListaObj)
    {
        int total = 0;
        TObjeto * obj = v[1].getObj();
        if (obj)
            for (TListaX * l1 = ref->Inicio; l1; )
            {
                if (l1->Objeto != obj)
                {
                    l1 = l1->ListaDepois;
                    continue;
                }
                TListaX::EndMover = l1->ListaDepois;
                l1->Apagar();
                l1 = TListaX::EndMover, total++;
            }
        DEBUG1(ref)
        return Instr::CriarVarInt(v, total);
    }
// Apagar v�rios objetos
        // Desmarca os objetos da lista
    for (TListaX * l1 = ref->Inicio; l1; l1 = l1->ListaDepois)
        l1->Objeto->MarcaLista = 0;
        // Marca os objetos que ser�o removidos
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        TObjeto * obj = v1->getObj();
        if (obj)
            obj->MarcaLista = 1;
        else if (v1->defvar[2] == Instr::cListaObj && v1->indice != 0xff)
        {
            TListaObj * lista1 =
                    reinterpret_cast<TListaObj*>(v1->endvar) + v1->indice;
            for (TListaX * l1 = lista1->Inicio; l1; l1 = l1->ListaDepois)
                l1->Objeto->MarcaLista = 1;
        }
    }
        // Remove objetos
    int total = 0;
    for (TListaX * l1 = ref->Inicio; l1; )
    {
        if (l1->Objeto->MarcaLista == 0)
        {
            l1 = l1->ListaDepois;
            continue;
        }
        TListaX::EndMover = l1->ListaDepois;
        l1->Apagar();
        l1 = TListaX::EndMover, total++;
    }
    DEBUG1(ref)
    return Instr::CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
// Organiza objetos aleatoriamente
bool TListaObj::FuncRand(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    if (ref->Total <= 1)
        return false;
// Aloca mem�ria
    TListaX * lx[256];
    TListaX ** lista = (ref->Total <= 256 ? lx : new TListaX*[ref->Total]);
// Anota objetos
    TListaX * l1 = ref->Inicio;
    for (unsigned int x = 0; x < ref->Total; x++, l1 = l1->ListaDepois)
        lista[x] = l1;
    assert(l1 == nullptr);
// Anota aleatoriamente na lista
    l1 = 0;
    for (unsigned int x = 0; x < ref->Total; x++)
    {
        unsigned int novo = circle_random() % (ref->Total - x) + x;
        lista[novo]->ListaAntes = l1;
        if (l1 == nullptr)
            ref->Inicio = lista[novo];
        else
            l1->ListaDepois = lista[novo];
        l1 = lista[novo];
        lista[novo] = lista[x];
    }
    l1->ListaDepois = nullptr;
    ref->Fim = l1;
// Desaloca mem�ria
    if (lista != lx)
        delete[] lista;
    DEBUG1(ref)
    return false;
}

//----------------------------------------------------------------------------
bool TListaObj::FuncInverter(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    TListaX * obj = ref->Inicio;
    ref->Inicio = ref->Fim;
    ref->Fim = obj;
    while (obj)
    {
        TListaX * obj2 = obj->ListaDepois;
        obj->ListaDepois = obj->ListaAntes;
        obj->ListaAntes = obj2;
        obj = obj2;
    }
    DEBUG1(ref)
    return false;
}

//----------------------------------------------------------------------------
// Remove todos os objetos
bool TListaObj::FuncLimpar(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    while (ref->Inicio)
        ref->Inicio->Apagar();
    DEBUG1(ref)
    return false;
}

//----------------------------------------------------------------------------
// Marca todos os objetos para exclus�o
bool TListaObj::FuncApagar(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    for (TListaX * obj = ref->Inicio; obj; obj = obj->ListaDepois)
        obj->Objeto->MarcarApagar();
    DEBUG1(ref)
    return false;
}

//----------------------------------------------------------------------------
// Verifica se objeto est� na lista
bool TListaObj::FuncPossui(TVariavel * v)
{
// Nenhum objeto
    if (Instr::VarAtual < v + 1)
        return Instr::CriarVarInt(v, 0);
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
// Um objeto
    if (Instr::VarAtual == v + 1 && v[1].defvar[2] != Instr::cListaObj)
    {
        int total = 0;
        TObjeto * obj = v[1].getObj();
        if (obj)
            for (TListaX * l1 = ref->Inicio; l1; l1 = l1->ListaDepois)
                if (l1->Objeto == obj)
                    total++;
        return Instr::CriarVarInt(v, total);
    }
// V�rios objetos
        // Desmarca os objetos da lista
    for (TListaX * l1 = ref->Inicio; l1; l1 = l1->ListaDepois)
        l1->Objeto->MarcaLista = 0;
        // Marca os objetos
    for (TVariavel * v1 = v + 1; v1 <= Instr::VarAtual; v1++)
    {
        TObjeto * obj = v1->getObj();
        if (obj)
            obj->MarcaLista = 1;
        else if (v1->defvar[2] == Instr::cListaObj && v1->indice != 0xff)
        {
            TListaObj * lista1 =
                    reinterpret_cast<TListaObj*>(v1->endvar) + v1->indice;
            for (TListaX * l1 = lista1->Inicio; l1; l1 = l1->ListaDepois)
                l1->Objeto->MarcaLista = 1;
        }
    }
        // Conta os objetos
    int total = 0;
    for (TListaX * l1 = ref->Inicio; l1; l1 = l1->ListaDepois)
        total += l1->Objeto->MarcaLista;
    return Instr::CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
// Quantidade de itens da lista
bool TListaObj::FuncTotal(TVariavel * v)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar) + v->indice;
    unsigned int x;
    if (Instr::VarAtual < v + 1)
        x = ref->Total;
    else
    {
        x = 0;
        TClasse * cl = TClasse::Procura(v[1].getTxt());
        if (cl)
            for (TListaX * obj = ref->Inicio; obj; obj = obj->ListaDepois)
                if (obj->Objeto->Classe == cl)
                    x++;
    }
    return Instr::CriarVarInt(v, x);
}

//------------------------------------------------------------------------------
int TListaObj::FTamanho(const char * instr)
{
    return sizeof(TListaObj);
}

//------------------------------------------------------------------------------
int TListaObj::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TListaObj);
}

//------------------------------------------------------------------------------
void TListaObj::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TListaObj * ref = reinterpret_cast<TListaObj*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].Objeto = o;
        ref[antes].Inicio = nullptr;
        ref[antes].Fim = nullptr;
        ref[antes].Total = 0;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TListaObj::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_OBJETO(TListaObj)
}

//------------------------------------------------------------------------------
bool TListaObj::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TListaObj)
int TListaObj::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TListaObj)
double TListaObj::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TListaObj)
const char * TListaObj::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TListaObj)

//----------------------------------------------------------------------------
const TVarInfo * TListaItem::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "addantes",     &TListaItem::FuncAddAntes },
        { "addantes1",    &TListaItem::FuncAddAntes1 },
        { "adddepois",    &TListaItem::FuncAddDepois },
        { "adddepois1",   &TListaItem::FuncAddDepois1 },
        { "antes",        &TListaItem::FuncAntes },
        { "depois",       &TListaItem::FuncDepois },
        { "obj",          &TListaItem::FuncObj },
        { "objantes",     &TListaItem::FuncObjAntes },
        { "objdepois",    &TListaItem::FuncObjDepois },
        { "objlista",     &TListaItem::FuncObjLista },
        { "remove",       &TListaItem::FuncRemove },
        { "removeantes",  &TListaItem::FuncRemoveAntes },
        { "removedepois", &TListaItem::FuncRemoveDepois },
        { "total",        &TListaItem::FuncTotal }  };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoOutros,
        FRedim,
        FMoverEnd,
        FMoverDef,
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
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//----------------------------------------------------------------------------
inline void TListaItem::Apagar()
{
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        DEBUG1(ListaX->Lista)
        ListaX = nullptr;
    }
}

//----------------------------------------------------------------------------
inline void TListaItem::Mover(TListaItem * destino, TObjeto * o)
{
    Objeto = o;
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TListaItem));
}

//----------------------------------------------------------------------------
inline int TListaItem::getValor()
{
    return (ListaX != nullptr);
}

//----------------------------------------------------------------------------
void TListaItem::MudarRef(TListaX * lista)
{
    if (lista == ListaX)
        return;
    if (ListaX)
    {
        (Antes ? Antes->Depois : ListaX->ListaItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        ListaX = nullptr;
    }
    if (lista)
    {
        ListaX = lista;
        Antes = nullptr;
        Depois = lista->ListaItem;
        lista->ListaItem = this;
        if (Depois)
            Depois->Antes = this;
    }
}

//----------------------------------------------------------------------------
// Quantidade de itens da lista
bool TListaItem::FuncTotal(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    unsigned int x = (ref->ListaX ? ref->ListaX->Lista->Total : 0);
    return Instr::CriarVarInt(v, x);
}

//----------------------------------------------------------------------------
// Objeto
bool TListaItem::FuncObj(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TObjeto * obj = ref->ListaX->Objeto;
    Instr::ApagarVar(v); // Nota: pode apagar o pr�prio listaitem
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Objeto em que a lista foi definida
bool TListaItem::FuncObjLista(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TObjeto * obj = ref->ListaX->Lista->Objeto;
    Instr::ApagarVar(v); // Nota: pode apagar o pr�prio listaitem
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Vai para o objeto anterior
bool TListaItem::FuncAntes(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * obj = ref->ListaX;
    if (obj == nullptr)
        return false;
    if (Instr::VarAtual < v + 1)
        obj = obj->ListaAntes;
    else for (int total = v[1].getInt(); total > 0 && obj; total--)
        obj = obj->ListaAntes;
    ref->MudarRef(obj);
    DEBUG1(ref->ListaX->Lista)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
// Vai para o pr�ximo objeto
bool TListaItem::FuncDepois(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * obj = ref->ListaX;
    if (obj == nullptr)
        return false;
    if (Instr::VarAtual < v + 1)
        obj = obj->ListaDepois;
    else for (int total=v[1].getInt(); total>0 && obj; total--)
        obj = obj->ListaDepois;
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->MudarRef(obj);
    DEBUG1(listaobj)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
// Vai para o objeto anterior da mesma classe
bool TListaItem::FuncObjAntes(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * obj = ref->ListaX;
    if (obj == nullptr)
        return false;
    TClasse * cl = obj->Objeto->Classe;
    int total = (Instr::VarAtual < v + 1 ? 1 : v[1].getInt());
    while (total > 0)
    {
        obj = obj->ListaAntes;
        if (obj == nullptr)
            break;
        if (obj->Objeto->Classe == cl)
            total--;
    }
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->MudarRef(obj);
    DEBUG1(listaobj)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
// Vai para o pr�ximo objeto da mesma classe
bool TListaItem::FuncObjDepois(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * obj = ref->ListaX;
    if (obj == nullptr)
        return false;
    TClasse * cl = obj->Objeto->Classe;
    int total = (Instr::VarAtual < v + 1 ? 1 : v[1].getInt());
    while (total > 0)
    {
        obj = obj->ListaDepois;
        if (obj == nullptr)
            break;
        if (obj->Objeto->Classe == cl)
            total--;
    }
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->MudarRef(obj);
    DEBUG1(listaobj)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
// Remove objeto da lista
bool TListaItem::FuncRemove(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->ListaX->Apagar();
    DEBUG1(listaobj)
    return false;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncRemoveAntes(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * l = ref->ListaX;
    if (l == nullptr)
        return false;
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->MudarRef(ref->ListaX->ListaAntes);
    l->Apagar();
    DEBUG1(listaobj)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncRemoveDepois(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * l = ref->ListaX;
    if (l == nullptr)
        return false;
#ifdef DEBUG_MEM
    TListaObj * listaobj = ref->ListaX->Lista;
#endif
    ref->MudarRef(ref->ListaX->ListaDepois);
    l->Apagar();
    DEBUG1(listaobj)
    Instr::ApagarVar(v + 1);
    return true;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncAddAntes(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TListaX * valor = ref->ListaX->Lista->AddLista(v, ref->ListaX, 2);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncAddDepois(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TListaX * valor = ref->ListaX->Lista->AddLista(v, ref->ListaX, 3);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncAddAntes1(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TListaX * valor = ref->ListaX->Lista->AddLista(v, ref->ListaX, 6);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//----------------------------------------------------------------------------
bool TListaItem::FuncAddDepois1(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    if (ref->ListaX == nullptr)
        return false;
    TListaX * valor = ref->ListaX->Lista->AddLista(v, ref->ListaX, 7);
    if (valor == nullptr)
        return false;
    Instr::ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrVarListaItem))
        return false;
    reinterpret_cast<TListaItem*>(Instr::VarAtual->endvar)->MudarRef(valor);
    return true;
}

//------------------------------------------------------------------------------
int TListaItem::FTamanho(const char * instr)
{
    return sizeof(TListaItem);
}

//------------------------------------------------------------------------------
int TListaItem::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TListaItem);
}

//------------------------------------------------------------------------------
void TListaItem::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].ListaX = nullptr;
        ref[antes].Objeto = o;
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
    }
    for (; depois < antes; depois++)
        ref[depois].Apagar();
}

//------------------------------------------------------------------------------
void TListaItem::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_OBJETO(TListaItem)
}

//------------------------------------------------------------------------------
void TListaItem::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TListaItem)
}

//------------------------------------------------------------------------------
bool TListaItem::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TListaItem)
int TListaItem::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TListaItem)
double TListaItem::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TListaItem)
const char * TListaItem::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TListaItem)

//------------------------------------------------------------------------------
TObjeto * TListaItem::FGetObj(TVariavel * v)
{
    TListaItem * ref = reinterpret_cast<TListaItem*>(v->endvar) + v->indice;
    TListaX * l = ref->ListaX;
    return (l ? l->Objeto : nullptr);
}

//----------------------------------------------------------------------------
void TListaItem::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return;
    TListaItem * r1 = reinterpret_cast<TListaItem*>(v1->endvar) + v1->indice;
    TListaItem * r2 = reinterpret_cast<TListaItem*>(v2->endvar) + v2->indice;
    if (r1 != r2)
        r1->MudarRef(r2->ListaX);
}

//------------------------------------------------------------------------------
bool TListaItem::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TListaItem * ref1 = reinterpret_cast<TListaItem*>(v1->endvar) + v1->indice;
    TListaItem * ref2 = reinterpret_cast<TListaItem*>(v2->endvar) + v2->indice;
    return ref1->ListaX == ref2->ListaX;
}

//------------------------------------------------------------------------------
unsigned char TListaItem::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    if (v1->defvar[2] != v2->defvar[2])
        return 0;
    TListaItem * ref1 = reinterpret_cast<TListaItem*>(v1->endvar) + v1->indice;
    TListaItem * ref2 = reinterpret_cast<TListaItem*>(v2->endvar) + v2->indice;
    return ref1->ListaX == ref2->ListaX ? 2 : ref1->ListaX < ref2->ListaX ? 1 : 4;
}

//----------------------------------------------------------------------------
void TListaX::Apagar()
{
// Retira de TListaObj
    Lista->Total--;
    (ListaAntes ? ListaAntes->ListaDepois : Lista->Inicio) = ListaDepois;
    (ListaDepois ? ListaDepois->ListaAntes : Lista->Fim) = ListaAntes;
// Retira de TObjeto
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarListaX) = ObjDepois;
    if (ObjDepois)
        ObjDepois->ObjAntes = ObjAntes;
// Retira de TListaItem
    while (ListaItem)
        ListaItem->MudarRef(nullptr);
// Apaga objeto
#ifdef DEBUG_MSG
    printf("TListaX::Apagar(%p)\n", this); fflush(stdout);
#endif
    AlocaMem.free(this); // Apagar com otimiza��o
}

//----------------------------------------------------------------------------
TListaX * TListaX::Criar()
{
    TListaX * l = AlocaMem.malloc(); // Criar com otimiza��o
#ifdef DEBUG_MSG
    printf("TListaX::Criar(%p)\n", l); fflush(stdout);
#endif
    return l;
}

//----------------------------------------------------------------------------
void TListaX::Mover(TListaX * destino)
{
#ifdef DEBUG_MSG
    printf("TListaX::Mover(%p, %p)\n", this, destino); fflush(stdout);
#endif
// Move
    memcpy(destino, this, sizeof(TListaX));
// Acerta lista
    (ListaAntes ? ListaAntes->ListaDepois : Lista->Inicio) = destino;
    (ListaDepois ? ListaDepois->ListaAntes : Lista->Fim) = destino;
// Acerta objeto
    (ObjAntes ? ObjAntes->ObjDepois : Objeto->VarListaX) = destino;
    if (ObjDepois)
        ObjDepois->ObjAntes = destino;
// Acerta ListaItem
    for (TListaItem * obj = ListaItem; obj; obj = obj->Depois)
        obj->ListaX = destino;
// Acerta EndMover
    if (EndMover == this)
        EndMover = destino;
}

//----------------------------------------------------------------------------
void TListaX::Limpar()
{
    AlocaMem.LiberaBloco();
}

//----------------------------------------------------------------------------
void TListaX::Debug(TListaObj * lista)
{
    for (TListaX * obj = lista->Inicio; obj; obj = obj->ListaDepois)
    {
    // TListaX::Lista
        assert(obj->Lista = lista);
    // TListaX::ListaAntes
        if (obj->ListaAntes)
            assert(obj->ListaAntes->ListaDepois == obj);
        else
            assert(lista->Inicio == obj);
    // TListaX::ListaDepois
        if (obj->ListaDepois)
            assert(obj->ListaDepois->ListaAntes == obj);
        else
            assert(lista->Fim == obj);
    }
}
