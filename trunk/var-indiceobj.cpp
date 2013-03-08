/* Este programa é software livre; você pode redistribuir e/ou
 * modificar nos termos da GNU General Public License V2
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details at www.gnu.org
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "var-indiceobj.h"
#include "variavel.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

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
    if (IndiceObj==0)
        return 0;
    return IndiceObj->Objeto!=0;
}

//----------------------------------------------------------------------------
TIndiceObj * TIndiceItem::getIndiceObj()
{
    return IndiceObj;
}

//----------------------------------------------------------------------------
void TIndiceItem::MudarRef(TIndiceObj * indice)
{
    if (indice==IndiceObj)
        return;
    if (IndiceObj)
    {
        (Antes ? Antes->Depois : IndiceObj->IndiceItem) = Depois;
        if (Depois)
            Depois->Antes = Antes;
        IndiceObj = 0;
    }
    if (indice)
    {
        IndiceObj = indice;
        Antes = 0;
        Depois = indice->IndiceItem;
        indice->IndiceItem = this;
        if (Depois)
            Depois->Antes = this;
    }
}

//----------------------------------------------------------------------------
void TIndiceItem::Igual(TIndiceItem * v)
{
    MudarRef(v->IndiceObj);
    TamTxt = v->TamTxt;
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
        int meio = (ini+fim)/2;
        int resultado = strcmp(mens, ExecFunc[meio].Nome);
        if (resultado==0) // Se encontrou...
            return (this->*ExecFunc[meio].Func)(v);
        if (resultado<0) fim=meio-1; else ini=meio+1;
    }
    return false;
}

//----------------------------------------------------------------------------
bool TIndiceItem::FuncObj(TVariavel * v)
{
// Nenhum argumento
    if (Instr::VarAtual == v)
    {
        if (IndiceObj==0)
            return false;
        TObjeto * obj = IndiceObj->Objeto;
        if (obj==0)
            return false;
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        Instr::VarAtual->setObj(obj);
        return true;
    }
// Pelo menos um argumento
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt==0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::Procura(txt);
        if (indice==0)
            continue;
    // Obtém objeto
        TObjeto * obj = indice->Objeto;
        if (obj==0)
            continue;
    // Retorna o objeto encontrado
        Instr::ApagarVar(v);
        if (!Instr::CriarVar(Instr::InstrVarObjeto))
            return false;
        Instr::VarAtual->setObj(obj);
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------
// Texto
bool TIndiceItem::FuncTxt(TVariavel * v)
{
// Obtém o texto e o tamanho do texto
    char mens[100];
    *mens=0;
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
    if (IndiceObj==0)
        return false;
    int total = 1;
    TIndiceObj * obj = IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total>0; total--)
    {
        obj = TIndiceObj::RBprevious(obj);
        if (obj==0)
            break;
        if (compara(obj->Nome, IndiceObj->Nome, TamTxt)!=0)
        {
            obj=0;
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
    if (IndiceObj==0)
        return false;
    int total = 1;
    TIndiceObj * obj = IndiceObj;
    if (Instr::VarAtual >= v+1)
        total = v[1].getInt();
    for (; total>0; total--)
    {
        obj = TIndiceObj::RBnext(obj);
        if (obj==0)
            break;
        if (compara(obj->Nome, IndiceObj->Nome, TamTxt)!=0)
        {
            obj=0;
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
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt==0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::ProcIni(txt);
        if (indice==0)
            continue;
    // Encontrou
        MudarRef(indice);
        TamTxt = strlen(txt);
        return false;
    }
    MudarRef(0);
    return false;
}

//----------------------------------------------------------------------------
// Último objeto
bool TIndiceItem::FuncFim(TVariavel * v)
{
    for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
    {
        const char * txt = v1->getTxt();
    // Ignora texto vazio
        if (*txt==0)
            continue;
    // Procura indiceobj
        TIndiceObj * indice = TIndiceObj::ProcFim(txt);
        if (indice==0)
            continue;
    // Encontrou
        MudarRef(indice);
        TamTxt = strlen(txt);
        return false;
    }
    MudarRef(0);
    return false;
}

//----------------------------------------------------------------------------
void TIndiceObj::Apagar()
{
    while (IndiceItem)
        IndiceItem->MudarRef(0);
    if (*Nome)
        RBremove();
}

//----------------------------------------------------------------------------
void TIndiceObj::Mover(TIndiceObj * destino)
{
    // Acerta TIndiceItem::IndiceObj em todos os TIndiceItem
    for (TIndiceItem * obj = IndiceItem; obj; obj=obj->Depois)
        obj->IndiceObj = destino;
    // Acerta a RBT
    if (*Nome)
    {
        if (RBroot==this)
            RBroot=destino;
        if (RBleft)
            RBleft->RBparent=destino;
        if (RBright)
            RBright->RBparent=destino;
        if (RBparent)
        {
            if (RBparent->RBleft==this)
                RBparent->RBleft=destino;
            if (RBparent->RBright==this)
                RBparent->RBright=destino;
        }
    }
    // Move
    memmove(destino, this, sizeof(TIndiceObj));
}

//----------------------------------------------------------------------------
const char * TIndiceObj::getNome()
{
    return Nome;
}

//----------------------------------------------------------------------------
void TIndiceObj::setNome(const char * texto)
{
    while (IndiceItem)
        IndiceItem->MudarRef(0);
    if (*Nome)
        RBremove();
    copiastr(Nome, texto, sizeof(Nome));
    if (*Nome)
        RBinsert();
}

//----------------------------------------------------------------------------
TIndiceObj * TIndiceObj::Procura(const char * nome)
{
    TIndiceObj * y = RBroot;
    while (y)
    {
        int i = comparaZ(nome, y->Nome);
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
TIndiceObj * TIndiceObj::ProcIni(const char * nome)
{
    TIndiceObj * x = 0;
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
    TIndiceObj * x = 0;
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
TIndiceObj * TIndiceObj::RBroot=0;
#define CLASS TIndiceObj    // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
