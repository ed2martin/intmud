/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos da licença LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "instr-enum.h"
#include "classe.h"
#include "objeto.h"
#include "var-intexec.h"
//#include "misc.h"

TVarIntExec * TVarIntExec::Inicio = nullptr;
TVarIntExec * TVarIntExec::Fim = nullptr;

//----------------------------------------------------------------------------
const TVarInfo * TVarIntExec::Inicializa()
{
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        TVarInfo::FTipoInt,
        FRedim,
        FMoverEnd,
        FMoverDef,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//------------------------------------------------------------------------------
void TVarIntExec::ProcEventos()
{
    while (true)
    {
    // Verifica se tem objeto na lista
        TVarIntExec * obj = Inicio;
        if (obj == nullptr)
            return;

    // Retira objeto da lista
        Inicio = obj->Depois;
        if (Inicio)
            Inicio->Antes = nullptr;
        else
            Fim = nullptr;
        obj->Depois = nullptr;

    // Gera evento
        if (obj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
            if (!Instr::ExecIni(obj->endobjeto, mens))
                continue;
        }
        else if (obj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_exec", obj->defvar + Instr::endNome);
            if (!Instr::ExecIni(obj->endclasse, mens))
                continue;
        }
            // Cria argumento: índice
        Instr::ExecArg(obj->indice);
            // Executa
        Instr::ExecX();
        Instr::ExecFim();
    }
}

//------------------------------------------------------------------------------
inline int TVarIntExec::getValor()
{
    return (Antes || Inicio == this);
}

//------------------------------------------------------------------------------
void TVarIntExec::setValor(int valor)
{
    if (Antes || Inicio == this) // Se a variável for 1
    {
        if (valor)
            return;
        (Antes ? Antes->Depois : Inicio) = Depois;
        (Depois ? Depois->Antes : Fim) = Antes;
        Antes = Depois = nullptr;
    }
    else if (valor) // Se a variável for 0 e valor não for 0
    {
        Antes = Fim;
        Depois = nullptr;
        (Antes ? Antes->Depois : Inicio) = this;
        Fim = this;
    }
}

//------------------------------------------------------------------------------
inline void TVarIntExec::Mover(TVarIntExec * destino)
{
    if (Antes || Inicio == this)
    {
        (Antes ? Antes->Depois : Inicio) = destino;
        (Depois ? Depois->Antes : Fim) = destino;
    }
    memmove(destino, this, sizeof(TVarIntExec));
}

//------------------------------------------------------------------------------
inline void TVarIntExec::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto = o, b_objeto = true;
    else
        endclasse = c, b_objeto = false;
}

//------------------------------------------------------------------------------
int TVarIntExec::FTamanho(const char * instr)
{
    return sizeof(TVarIntExec);
}

//------------------------------------------------------------------------------
int TVarIntExec::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarIntExec);
}

//------------------------------------------------------------------------------
void TVarIntExec::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarIntExec * ref = reinterpret_cast<TVarIntExec*>(v->endvar);
    for (; antes < depois; antes++)
    {
        ref[antes].defvar = v->defvar;
        ref[antes].indice = antes;
        ref[antes].EndObjeto(c, o);
        ref[antes].Antes = nullptr;
        ref[antes].Depois = nullptr;
    }
    for (; depois < antes; depois++)
        ref[depois].setValor(0);
}

//------------------------------------------------------------------------------
void TVarIntExec::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_COMPLETO(TVarIntExec)
}

//------------------------------------------------------------------------------
void TVarIntExec::FMoverDef(TVariavel * v)
{
    VARIAVEL_MOVERDEF(TVarIntExec)
}

//------------------------------------------------------------------------------
bool TVarIntExec::FGetBool(TVariavel * v) VARIAVEL_FGETINT0(TVarIntExec)
int TVarIntExec::FGetInt(TVariavel * v) VARIAVEL_FGETINT0(TVarIntExec)
double TVarIntExec::FGetDouble(TVariavel * v) VARIAVEL_FGETINT0(TVarIntExec)
const char * TVarIntExec::FGetTxt(TVariavel * v) VARIAVEL_FGETTXT0(TVarIntExec)

//------------------------------------------------------------------------------
void TVarIntExec::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TVarIntExec * ref = reinterpret_cast<TVarIntExec*>(v1->endvar) + v1->indice;
    ref->setValor(v2->getInt());
}
