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
#include "variavel.h"
#include "instr.h"
#include "objeto.h"

#define DEBUG_CRIAR // Mostra quando uma variável é criada ou apagada

//----------------------------------------------------------------------------
TVariavel::TVariavel()
{
    memset(this, 0, sizeof(this));
}

//----------------------------------------------------------------------------
void TVariavel::Limpar()
{
    memset(this, 0, sizeof(this));
}

//------------------------------------------------------------------------------
int TVariavel::Tamanho(const char * instr)
{
    if (instr==0 || instr[0]==0 && instr[1]==0)
        return 0;
    switch (instr[2])
    {
// Variáveis
    case Instr::cTxt1:      return 2 + (unsigned char)instr[4];
    case Instr::cTxt2:      return 258 + (unsigned char)instr[4];
    case Instr::cInt1:      return 0;
    case Instr::cInt8:
    case Instr::cUInt8:     return 1;
    case Instr::cInt16:
    case Instr::cUInt16:    return 2;
    case Instr::cInt32:
    case Instr::cUInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:    return 4;
    case Instr::cReal:      return sizeof(double);
    case Instr::cRef:       return sizeof(void*)*3;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:   return 0;

// Variáveis extras
   /* case Instr::cListaObj:
    case Instr::cListaTxt:
    case Instr::cListaMsg:
    case Instr::cNomeObj:
    case Instr::cLog:
    case Instr::cVarTempo:
    case Instr::cSocket:
    case Instr::cServ:
    case Instr::cSalvar:
    case Instr::cProg:
    case Instr::cIndice: */
    }
    return 0;
}

//------------------------------------------------------------------------------
int TVariavel::Tamanho()
{
    return Tamanho(defvar);
}

//------------------------------------------------------------------------------
TVarTipo TVariavel::Tipo()
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return varNulo;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:      return varTxt;
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:     return varInt;
    case Instr::cUInt32:    return varDouble;
    case Instr::cIntInc:
    case Instr::cIntDec:    return varInt;
    case Instr::cReal:      return varDouble;
    case Instr::cRef:       return varObj;
    case Instr::cConstNulo: return varNulo;
    case Instr::cConstTxt:  return varTxt;
    case Instr::cConstNum:  return varDouble;
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:   return varNulo;

// Variáveis extras
   /* case Instr::cListaObj:
    case Instr::cListaTxt:
    case Instr::cListaMsg:
    case Instr::cNomeObj:
    case Instr::cLog:
    case Instr::cVarTempo:
    case Instr::cSocket:
    case Instr::cServ:
    case Instr::cSalvar:
    case Instr::cProg:
    case Instr::cIndice: */
    }
    return varNulo;
}

//------------------------------------------------------------------------------
void TVariavel::Criar(TClasse * c, TObjeto * o)
{
#ifdef DEBUG_CRIAR
    printf("Variável criada %p %s%s   ",
           endvar, (c ? c->Nome : ""), (c && o ? ":objeto" : ""));
    char mens[500];
    if (Instr::Decod(mens, defvar, sizeof(mens)))
        puts(mens);
    else
        puts("???");
    fflush(stdout);
#endif
    int tam = Tamanho(defvar);
    if (tam)
        memset(endvar, 0, tam);
}

//------------------------------------------------------------------------------
void TVariavel::Apagar()
{
#ifdef DEBUG_CRIAR
    printf("Variável apagada %p   ", endvar);
    char mens[500];
    if (Instr::Decod(mens, defvar, sizeof(mens)))
        puts(mens);
    else
        puts("???");
    fflush(stdout);
#endif
}

//------------------------------------------------------------------------------
void TVariavel::Mover(void * destino, TClasse * c, TObjeto * o)
{
    int tam = Tamanho(defvar);
    if (tam)
        memcpy(destino, endvar, tam);
}

//------------------------------------------------------------------------------
/*
int TVariavel::getInt()
{
}
double TVariavel::getDouble()
{
}
const char * TVariavel::getTxt()
{
}
TObjeto * TVariavel::getObj()
{
}
void TVariavel::setInt(int valor)
{
}
void TVariavel::setDouble(double valor)
{
}
void TVariavel::setTxt(const char * txt)
{
}
void TVariavel::addTxt(const char * txt)
{
}
void TVariavel::setObj(TObjeto * obj)
{
}
*/
