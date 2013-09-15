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
#include <assert.h>
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "var-outros.h"
#include "var-listaobj.h"
#include "var-textoobj.h"
#include "instr.h"

//#define DEBUG_CRIAR // Mostra objetos criados e apagados

TObjeto * TObjeto::IniApagar = 0;
TObjeto * TObjeto::FimApagar = 0;

//----------------------------------------------------------------------------
TObjeto::TObjeto() { assert(0); }
TObjeto::~TObjeto() { assert(0); }

//----------------------------------------------------------------------------
TObjeto * TObjeto::Criar(TClasse * c, bool criavar)
{
// Aloca memória
    int tam = sizeof(TObjeto) - 4 + c->TamObj;
    char * str = new char[tam];
    TObjeto * obj = (TObjeto*)str;
// Zera todos os bytes do objeto
    memset(str, 0, tam);
// Coloca na lista ligada
    obj->Classe = c;
    obj->Antes = c->ObjetoFim;
    (c->ObjetoFim ? c->ObjetoFim->Depois : c->ObjetoIni) = obj;
    c->ObjetoFim = obj;
    c->NumObj++;
// Chama construtores das variáveis
    if (criavar)
    {
        TVariavel v;
        for (int x=(int)c->NumVar-1; x>=0; x--)
            if (c->InstrVar[x][2] > Instr::cVarFunc &&
                    (c->IndiceVar[x] & 0x400000)==0)
            {
                v.endvar = obj->Vars + (c->IndiceVar[x] & 0x3FFFFF);
                v.defvar = c->InstrVar[x];
                v.Criar(c, obj);
            }
    }
#ifdef DEBUG_CRIAR
    printf("TObjeto( %p , %p , %s )\n", obj, obj->Classe, obj->Classe->Nome);
    fflush(stdout);
#endif
    return obj;
}

//----------------------------------------------------------------------------
void TObjeto::Apagar()
{
#ifdef DEBUG_CRIAR
    printf("~TObjeto1( %p", this); fflush(stdout);
    printf(", %p", Classe); fflush(stdout);
    printf(", %s )\n", Classe->Nome);
    fflush(stdout);
#endif
    Classe->NumObj--;
// Retira da lista de objetos que serão apagados
    if (AntesApagar || IniApagar==this)
    {
        (AntesApagar ? AntesApagar->DepoisApagar : IniApagar) = DepoisApagar;
        (DepoisApagar ? DepoisApagar->AntesApagar : FimApagar) = AntesApagar;
    }
// Remove variáveis TVarRef que apontam para o objeto
    while (VarRefIni)
        VarRefIni->MudarPont(0);
// Remove variáveis TListaX que apontam para o objeto
    while (VarListaX)
        VarListaX->Apagar();
// Remove variáveis TBlocoObj que apontam para o objeto
    while (VarBlocoObj)
        VarBlocoObj->Apagar();
// Chama destrutores das variáveis
    TVariavel v;
    for (int x=(int)Classe->NumVar-1; x>=0; x--)
        if (Classe->InstrVar[x][2] > Instr::cVarFunc &&
                (Classe->IndiceVar[x] & 0x400000)==0)
        {
            v.endvar = Vars + (Classe->IndiceVar[x] & 0x3FFFFF);
            v.defvar = Classe->InstrVar[x];
            v.Apagar();
        }
// Tira da lista ligada
    (Antes ? Antes->Depois : Classe->ObjetoIni) = Depois;
    (Depois ? Depois->Antes : Classe->ObjetoFim) = Antes;
// Libera memória
    char * x = (char*)this;
    delete[] x;
}

//----------------------------------------------------------------------------
void TObjeto::Apagar(TObjeto * obj)
{
#ifdef DEBUG_CRIAR
    printf("~TObjeto2( %p -> %p", this, obj); fflush(stdout);
    printf(", %p", Classe); fflush(stdout);
    printf(", %s )\n", Classe->Nome);
    fflush(stdout);
#endif
    Classe->NumObj--;
// Coloca o outro objeto na lista dos que serão apagados
    obj->AntesApagar = AntesApagar;
    obj->DepoisApagar = DepoisApagar;
    if (IniApagar==this) IniApagar=obj;
    if (FimApagar==this) FimApagar=obj;
// Acerta variáveis TVarRef que apontam para o objeto
    for (TVarRef * pont = VarRefIni; pont; pont=pont->Depois)
        pont->Pont = obj;
    obj->VarRefIni = VarRefIni;
// Acerta variáveis TListaX que apontam para o objeto
    for (TListaX * pont = VarListaX; pont; pont=pont->ObjDepois)
        pont->Objeto = obj;
    obj->VarListaX = VarListaX;
// Acerta variáveis TBlocoObj que apontam para o objeto
    for (TBlocoObj * pont = VarBlocoObj; pont; pont=pont->ObjDepois)
        pont->Objeto = obj;
    obj->VarBlocoObj = VarBlocoObj;
// Tira da lista ligada
    (Antes ? Antes->Depois : Classe->ObjetoIni) = Depois;
    (Depois ? Depois->Antes : Classe->ObjetoFim) = Antes;
// Libera memória
    char * x = (char*)this;
    delete[] x;
}

//----------------------------------------------------------------------------
void TObjeto::MarcarApagar()
{
    if (AntesApagar || IniApagar==this)
        return;
    AntesApagar = FimApagar;
    DepoisApagar = 0;
    (AntesApagar ? AntesApagar->DepoisApagar : IniApagar) = this;
    FimApagar = this;
}
