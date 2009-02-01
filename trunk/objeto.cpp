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
#include <assert.h>
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "var-outros.h"
#include "instr.h"

TObjeto * TObjeto::IniApagar = 0;
TObjeto * TObjeto::FimApagar = 0;

//----------------------------------------------------------------------------
TObjeto::TObjeto() { assert(0); }
TObjeto::~TObjeto() { assert(0); }

//----------------------------------------------------------------------------
TObjeto * TObjeto::Criar(TClasse * c)
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
    TVariavel v;
    for (int x=(int)c->NumVar-1; x>=0; x--)
        if (c->InstrVar[x][2] > Instr::cVarFunc &&
                (c->IndiceVar[x] & 0x400000)==0)
        {
            v.endvar = obj->Vars + (c->IndiceVar[x] & 0x3FFFFF);
            v.defvar = c->InstrVar[x];
            v.Criar(c, obj);
        }
    return obj;
}

//----------------------------------------------------------------------------
void TObjeto::Apagar()
{
    Classe->NumObj--;
// Remove variáveis TVarRef que apontam para o objeto
    while (VarRefIni)
        VarRefIni->MudarPont(0);
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
void TObjeto::MarcarApagar()
{
    if (PontApagar || FimApagar == this)
        return;
    (FimApagar ? FimApagar->PontApagar : IniApagar) = this;
    FimApagar = this;
}

//----------------------------------------------------------------------------
void TObjeto::DesmarcarApagar()
{
    if (IniApagar==0)
        return;
    TObjeto * obj = IniApagar;
    IniApagar = IniApagar->PontApagar;
    if (IniApagar==0)
        FimApagar=0;
    obj->PontApagar = 0;
}
