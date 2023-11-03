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
#include <errno.h>
#include <assert.h>
#include "var-textoobj.h"
#include "variavel.h"
#include "variavel-def.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG_MEM // Mostra variáveis criadas e apagadas

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
        TVarInfo::FFuncVetorFalse);
    return &var;
}

//----------------------------------------------------------------------------
bool TTextoObj::Func(TVariavel * v, const char * nome)
{
// Lista das funções de indiceitem
// Deve obrigatoriamente estar em letras minúsculas e ordem alfabética
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
// Procura a função correspondente e executa
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
// Outro nome de variável
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
// Variável como texto
bool TTextoObj::FuncValor(TVariavel * v)
{
    TObjeto * obj = 0;
    if (Instr::VarAtual >= v+1)
    {
        TBlocoObj * bl = Procura(v[1].getTxt());
        if (bl)
            obj = bl->Objeto;
    }
    Instr::ApagarVar(v);
    if (obj==0)
        return false;
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Primeira variável como texto
bool TTextoObj::FuncValorIni(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBfirst();
        else
            bl = ProcIni(v[1].getTxt());
    }
    if (bl==0)
        return false;
    TObjeto * obj = bl->Objeto;
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Última variável como texto
bool TTextoObj::FuncValorFim(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
            bl = RBroot->RBlast();
        else
            bl = ProcFim(v[1].getTxt());
    }
    if (bl==0)
        return false;
    TObjeto * obj = bl->Objeto;
    Instr::ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
// Nome da variável
bool TTextoObj::FuncNomeVar(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = Procura(v[1].getTxt());
    Instr::ApagarVar(v);
    if (bl==0)
        return Instr::CriarVarTexto("");
    return Instr::CriarVarTexto(bl->NomeVar);
}

//----------------------------------------------------------------------------
// Mudar variável
bool TTextoObj::FuncMudar(TVariavel * v)
{
    if (Instr::VarAtual >= v+2)
    {
        char nomevar[64];
        copiastr(nomevar, v[1].getTxt());
        Mudar(nomevar, v[2].getObj());
    }
    else if (Instr::VarAtual >= v+1)
        Mudar(v[1].getTxt(), 0);
    return false;
}

//----------------------------------------------------------------------------
// Variável anterior
bool TTextoObj::FuncAntes(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcAntes(v[1].getTxt());
    if (Instr::VarAtual >= v+2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp!=0 && cmp!=2) // 0=textos iguais, 2=texto 1 contém texto 2
            bl = 0;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Próxima variável
bool TTextoObj::FuncDepois(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (Instr::VarAtual >= v+1)
        bl = ProcDepois(v[1].getTxt());
    if (Instr::VarAtual >= v+2 && bl)
    {
        int cmp = comparaZ(bl->NomeVar, v[2].getTxt());
        if (cmp!=0 && cmp!=2) // 0=textos iguais, 2=texto 1 contém texto 2
            bl = 0;
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(bl ? bl->NomeVar : "");
}

//----------------------------------------------------------------------------
// Início
bool TTextoObj::FuncIni(TVariavel * v)
{
    TBlocoObj * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
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
    TBlocoObj * bl = 0;
    if (RBroot)
    {
        if (Instr::VarAtual < v+1)
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
    if (Instr::VarAtual < v+1)
    {
        Limpar();
        return false;
    }
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ProcIni(p);
        if (ini==0)
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
    if (Instr::VarAtual < v+1)
    {
        if (RBroot)
            RBroot->FuncApagarSub();
        return false;
    }
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * p = v1->getTxt();
        TBlocoObj * ini = ProcIni(p);
        if (ini==0)
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
    if (Instr::VarAtual >= v+1)
        txt = v[1].getTxt();
    if (*txt==0)
    {
        total = Total;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(total);
    }
    TBlocoObj * ini = ProcIni(txt);
    if (ini)
    {
        TBlocoObj * fim = ProcFim(txt);
        total=1;
        while (ini && ini != fim)
            total++, ini=TBlocoObj::RBnext(ini);
    }
    Instr::ApagarVar(v);
    return Instr::CriarVarInt(total);
}

//----------------------------------------------------------------------------
void TTextoObj::Apagar()
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
void TTextoObj::Mover(TTextoObj * destino, TObjeto * o)
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
        if (i==0)
            return y;
        if (i<0)
            y = y->RBleft;
        else
            y = y->RBright;
    }
    return 0;
}

//----------------------------------------------------------------------------
TBlocoObj * TTextoObj::ProcIni(const char * texto)
{
    TBlocoObj * x = 0;
    TBlocoObj * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
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
TBlocoObj * TTextoObj::ProcFim(const char * texto)
{
    TBlocoObj * x = 0;
    TBlocoObj * y = RBroot;
    while (y)
    {
        switch (comparaVar(texto, y->NomeVar))
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
TBlocoObj * TTextoObj::ProcAntes(const char * texto)
{
    TBlocoObj * x = 0;
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
    TBlocoObj * x = 0;
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
// Inserir texto (não está no textoobj)
    if (bl==0)
    {
        if (obj==0)
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
    else if (obj==0)
        bl->Apagar();
// Alterar objeto
    else
    {
        strcpy(bl->NomeVar, nomevar); // Acerta o nome da variável
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
void TTextoObjSub::Apagar()
{
    if (TextoObj == nullptr)
        return;
    (Antes ? Antes->Depois : TextoObj->Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    TextoObj = 0;
}

//----------------------------------------------------------------------------
void TTextoObjSub::Mover(TTextoObjSub * destino)
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
    if (TextoObj==0) return 0;
    return (TextoObj->Procura(NomeVar) != 0);
}

//----------------------------------------------------------------------------
TObjeto * TTextoObjSub::getObj()
{
    if (TextoObj==0) return 0;
    TBlocoObj * bl = TextoObj->Procura(NomeVar);
    return (bl ? bl->Objeto : 0);
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
    ObjAntes = 0;
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
#define RBmask 1 // Máscara para bit 0
#define RBroot TextoObj->RBroot
#include "rbt.cpp.h"
