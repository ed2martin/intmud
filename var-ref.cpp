/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "var-ref.h"

//------------------------------------------------------------------------------
/*
void DebugRef()
{
 for (TClasse * cl = TClasse::RBfirst(); cl; cl=TClasse::RBnext(cl))
 {
  for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
  {
   TVarRef * vantes = nullptr;
   for (TVarRef * v1 = obj->VarRefIni; v1; v1=v1->Depois)
   {
    assert(v1->Pont == obj);
    assert(v1->Antes == vantes);
    vantes = v1;
   }
  }
 }
}

//------------------------------------------------------------------------------
void MostraRef(TVarRef * r)
{
    printf("  this = %p\n", r);
    if (r == nullptr)
        return;
    printf("  Antes = %p\n", r->Antes);
    printf("  Depois = %p\n", r->Depois);
    printf("  Pont = %p\n", r->Pont);
    if (r->Pont)
        printf("  Pont->VarRefIni = %p\n", r->Pont->VarRefIni);
    if (r->Antes)
        printf("  Antes->Depois = %p\n", r->Antes->Depois);
    if (r->Depois)
        printf("  Depois->Antes = %p\n", r->Depois->Antes);
    fflush(stdout);
}
*/

//----------------------------------------------------------------------------
const TVarInfo * TVarRef::Inicializa()
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
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//------------------------------------------------------------------------------
void TVarRef::MudarPont(TObjeto * obj)
{
// Verifica se o endereço vai mudar
    if (obj == Pont)
        return;
    //printf("ANTES\n"); MostraRef(this); fflush(stdout);
// Retira da lista
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = Depois;
        if (Depois)
            Depois->Antes = Antes;
    }
// Coloca na lista
    if (obj)
    {
        Antes = nullptr;
        Depois = obj->VarRefIni;
        if (Depois)
            Depois->Antes = this;
        obj->VarRefIni = this;
    }
// Atualiza ponteiro
    Pont = obj;
    //printf("DEPOIS\n"); MostraRef(this); fflush(stdout);
}

//------------------------------------------------------------------------------
inline void TVarRef::Mover(TVarRef * destino)
{
    if (Pont)
    {
        (Antes ? Antes->Depois : Pont->VarRefIni) = destino;
        if (Depois)
            Depois->Antes = destino;
    }
    memmove(destino, this, sizeof(TVarRef));
}

//------------------------------------------------------------------------------
int TVarRef::FTamanho(const char * instr)
{
    return sizeof(TVarRef);
}

//------------------------------------------------------------------------------
int TVarRef::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarRef);
}

//------------------------------------------------------------------------------
void TVarRef::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarRef * ref = reinterpret_cast<TVarRef*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes].Pont = nullptr;
    for (; depois < antes; depois++)
        ref[depois].MudarPont(nullptr);
}

//------------------------------------------------------------------------------
void TVarRef::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TVarRef)
}

//------------------------------------------------------------------------------
bool TVarRef::FGetBool(TVariavel * v)
{
    return reinterpret_cast<TVarRef*>(v->endvar)[ v->indice ].Pont != nullptr;
}

//------------------------------------------------------------------------------
int TVarRef::FGetInt(TVariavel * v)
{
    return reinterpret_cast<TVarRef*>(v->endvar)[ v->indice ].Pont != nullptr;
}

//------------------------------------------------------------------------------
double TVarRef::FGetDouble(TVariavel * v)
{
    return reinterpret_cast<TVarRef*>(v->endvar)[ v->indice ].Pont != nullptr;
}

//------------------------------------------------------------------------------
const char * TVarRef::FGetTxt(TVariavel * v)
{
    TObjeto * obj = reinterpret_cast<TVarRef*>(v->endvar)[ v->indice ].Pont;
    return (obj == nullptr ? "" : obj->Classe->Nome);
}

//------------------------------------------------------------------------------
TObjeto * TVarRef::FGetObj(TVariavel * v)
{
    return reinterpret_cast<TVarRef*>(v->endvar)[ v->indice].Pont;
}

//------------------------------------------------------------------------------
void TVarRef::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TVarRef * ref = reinterpret_cast<TVarRef*>(v1->endvar) + v1->indice;
    ref->MudarPont(v2->getObj());
}
