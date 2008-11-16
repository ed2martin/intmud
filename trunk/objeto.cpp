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
    return obj;
}

//----------------------------------------------------------------------------
void TObjeto::Apagar()
{
    Classe->NumObj--;
    (Antes ? Antes->Depois : Classe->ObjetoIni) = Depois;
    (Depois ? Depois->Antes : Classe->ObjetoFim) = Antes;
    char * x = (char*)this;
    delete[] x;
}
