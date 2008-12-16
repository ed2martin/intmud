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
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "objeto.h"
#include "misc.h"

//#define DEBUG_CRIAR // Mostra quando uma variável é criada ou apagada

//----------------------------------------------------------------------------
TVariavel::TVariavel()
{
    memset(this, 0, sizeof(TVariavel));
}

//----------------------------------------------------------------------------
void TVariavel::Limpar()
{
    memset(this, 0, sizeof(TVariavel));
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
    case Instr::cInt1:
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

    case Instr::cVarNome:   return 48;
    case Instr::cVarInicio:
    case Instr::cVarClasse:
    case Instr::cVarObjeto: return 0;
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

    case Instr::cTxtFixo:
    case Instr::cVarNome:
    case Instr::cVarClasse: return varTxt;
    case Instr::cVarObjeto: return varObj;
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
bool TVariavel::getBool()
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return 0;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cTxtFixo:
    case Instr::cInt8:
    case Instr::cUInt8:
        return (*(const char*)endvar != 0);
    case Instr::cInt1:
        return (*(const char*)endvar & bit ? 1 : 0);
    case Instr::cInt16:
    case Instr::cUInt16:
        return *(char*)endvar || *((char*)endvar+1);
    case Instr::cInt32:
    case Instr::cUInt32:
        return *(unsigned char*)endvar ||
               *((unsigned char*)endvar+1) ||
               *((unsigned char*)endvar+2) ||
               *((unsigned char*)endvar+3);
    case Instr::cIntInc:
    case Instr::cIntDec:
        return 0;
    case Instr::cReal:
        return (*(double*)endvar != 0);
    case Instr::cRef:
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        return *((char*)endvar + *((char*)endvar+4)) != 0;
    case Instr::cConstNum:
        {
            const char * origem = (char*)endvar + *((char*)endvar+4);
            switch (*origem)
            {
            case Instr::ex_num1:
                return true;
            case Instr::ex_num0:
                return false;
            case Instr::ex_num8n:
            case Instr::ex_num8p:
                return origem[1]!=0;
            case Instr::ex_num16n:
            case Instr::ex_num16p:
                return origem[1]!=0 || origem[2]!=0;
            case Instr::ex_num32n:
            case Instr::ex_num32p:
                return origem[1]!=0 || origem[2]!=0 ||
                       origem[3]!=0 || origem[4]!=0;
            default:
                assert(0);
            }
        }
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return 0;

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

    case Instr::cVarObjeto:
        return (endvar!=0);
    }
    return 0;
}

//------------------------------------------------------------------------------
int TVariavel::getInt()
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return 0;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cTxtFixo:
        return atoi((const char*)endvar);
    case Instr::cInt1:
        return (*(const char*)endvar & bit ? 1 : 0);
    case Instr::cInt8:
        return *(signed char*)endvar;
    case Instr::cUInt8:
        return *(unsigned char*)endvar;
    case Instr::cInt16:
        return (short)(*(unsigned char*)endvar +
                       *((unsigned char*)endvar+1) * 0x100);
    case Instr::cUInt16:
        return *(unsigned char*)endvar +
               *((unsigned char*)endvar+1) * 0x100;
    case Instr::cInt32:
        return (int)(
                *(unsigned char*)endvar +
                *((unsigned char*)endvar+1) * 0x100 +
                *((unsigned char*)endvar+2) * 0x10000 +
                *((unsigned char*)endvar+3) * 0x1000000);
    case Instr::cUInt32:
        return (*(char*)endvar & 0x80 ? 0x7FFFFFFF :
                *(unsigned char*)endvar +
                *((unsigned char*)endvar+1) * 0x100 +
                *((unsigned char*)endvar+2) * 0x10000 +
                *((unsigned char*)endvar+3) * 0x1000000);
    case Instr::cIntInc:
    case Instr::cIntDec:
        return 0;
    case Instr::cReal:
        {
            double * d = (double*)endvar;
            if (*d < -0x80000000LL)
                return -0x80000000;
            else if (*d > 0x7FFFFFFFLL)
                return 0x7FFFFFFF;
            else
                return (int)*d;
        }
    case Instr::cRef:
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        return atoi((char*)endvar + *((char*)endvar+4));
    case Instr::cConstNum:
        {
            unsigned int valor = 0;
            bool negativo = false;
            const char * origem = (char*)endvar + *((char*)endvar+4);
            switch (*origem)
            {
            case Instr::ex_num1:
                valor=1;
            case Instr::ex_num0:
                origem++;
                break;
            case Instr::ex_num8n:
                negativo=true;
            case Instr::ex_num8p:
                valor=(unsigned char)origem[1];
                origem+=2;
                break;
            case Instr::ex_num16n:
                negativo=true;
            case Instr::ex_num16p:
                valor=Num16(origem+1);
                origem+=3;
                break;
            case Instr::ex_num32n:
                negativo=true;
            case Instr::ex_num32p:
                valor=Num32(origem+1);
                origem+=5;
                break;
            default:
                assert(0);
            }
            while (*origem>=Instr::ex_div1 && *origem<=Instr::ex_div6)
                switch (*origem++)
                {
                case Instr::ex_div1: valor/=10; break;
                case Instr::ex_div2: valor/=100; break;
                case Instr::ex_div3: valor/=1000; break;
                case Instr::ex_div4: valor/=10000; break;
                case Instr::ex_div5: valor/=100000; break;
                case Instr::ex_div6: valor/=1000000; break;
                }
            if (negativo)
                return (valor<0x80000000LL ? -valor : -0x80000000);
            return (valor<0x7FFFFFFFLL ? valor : 0x7FFFFFFF);
        }
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return 0;

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
double TVariavel::getDouble()
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return 0;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cTxtFixo:
        return atoi((char*)endvar);
    case Instr::cInt1:
        return (*(const char*)endvar & bit ? 1 : 0);
    case Instr::cInt8:
        return *(signed char*)endvar;
    case Instr::cUInt8:
        return *(unsigned char*)endvar;
    case Instr::cInt16:
        return (short)(*(unsigned char*)endvar +
                       *((unsigned char*)endvar+1) * 0x100);
    case Instr::cUInt16:
        return *(unsigned char*)endvar +
               *((unsigned char*)endvar+1) * 0x100;
    case Instr::cInt32:
        return (int)(
                *(unsigned char*)endvar +
                *((unsigned char*)endvar+1) * 0x100 +
                *((unsigned char*)endvar+2) * 0x10000 +
                *((unsigned char*)endvar+3) * 0x1000000);
    case Instr::cUInt32:
        return (unsigned int)(
                *(unsigned char*)endvar +
                *((unsigned char*)endvar+1) * 0x100 +
                *((unsigned char*)endvar+2) * 0x10000 +
                *((unsigned char*)endvar+3) * 0x1000000);
    case Instr::cIntInc:
    case Instr::cIntDec:
        return 0;
    case Instr::cReal:
        return *(double*)endvar;
    case Instr::cRef:
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        return atoi((char*)endvar + *((char*)endvar+4));
    case Instr::cConstNum:
        {
            double valor = 0;
            bool negativo = false;
            const char * origem = (char*)endvar + *((char*)endvar+4);
            switch (*origem)
            {
            case Instr::ex_num1:
                valor=1;
            case Instr::ex_num0:
                origem++;
                break;
            case Instr::ex_num8n:
                negativo=true;
            case Instr::ex_num8p:
                valor=(unsigned char)origem[1];
                origem+=2;
                break;
            case Instr::ex_num16n:
                negativo=true;
            case Instr::ex_num16p:
                valor=Num16(origem+1);
                origem+=3;
                break;
            case Instr::ex_num32n:
                negativo=true;
            case Instr::ex_num32p:
                valor=Num32(origem+1);
                origem+=5;
                break;
            default:
                assert(0);
            }
            while (*origem>=Instr::ex_div1 && *origem<=Instr::ex_div6)
                switch (*origem++)
                {
                case Instr::ex_div1: valor/=10; break;
                case Instr::ex_div2: valor/=100; break;
                case Instr::ex_div3: valor/=1000; break;
                case Instr::ex_div4: valor/=10000; break;
                case Instr::ex_div5: valor/=100000; break;
                case Instr::ex_div6: valor/=1000000; break;
                }
            return (negativo ? -valor : valor);
        }
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return 0;

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
const char * TVariavel::getTxt()
{
    static char txtnum[80];
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return "";
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cTxtFixo:
    case Instr::cVarNome:
        return (const char*)endvar;
    case Instr::cInt1:
        return (*(char*)endvar & bit ? "1" : "0");
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cReal:
        sprintf(txtnum, "%d", getInt());
        return txtnum;
    case Instr::cUInt32:
        sprintf(txtnum, "%u", (unsigned int)(
                *(unsigned char*)endvar +
                *((unsigned char*)endvar+1) * 0x100 +
                *((unsigned char*)endvar+2) * 0x10000 +
                *((unsigned char*)endvar+3) * 0x1000000));
        return txtnum;
    case Instr::cRef:
    case Instr::cConstNulo:
        return "";
    case Instr::cConstTxt:
        return (char*)endvar + *((char*)endvar+4);
    case Instr::cConstNum:
        {
            unsigned int valor = 0; // Valor numérico sem sinal
            bool negativo = false; // Se é negativo
            int  virgula = 0;   // Casas após a vírgula
            const char * origem = (char*)endvar + *((char*)endvar+4);
        // Acerta variáveis valor e negativo
            switch (*origem)
            {
            case Instr::ex_num1:
                valor=1;
            case Instr::ex_num0:
                origem++;
                break;
            case Instr::ex_num8n:
                negativo=true;
            case Instr::ex_num8p:
                valor=(unsigned char)origem[1];
                origem+=2;
                break;
            case Instr::ex_num16n:
                negativo=true;
            case Instr::ex_num16p:
                valor=Num16(origem+1);
                origem+=3;
                break;
            case Instr::ex_num32n:
                negativo=true;
            case Instr::ex_num32p:
                valor=Num32(origem+1);
                origem+=5;
                break;
            }
        // Acerta variável virgula
            while (*origem>=Instr::ex_div1 && *origem<=Instr::ex_div6)
                switch (*origem++)
                {
                case Instr::ex_div1: virgula++; break;
                case Instr::ex_div2: virgula+=2; break;
                case Instr::ex_div3: virgula+=3; break;
                case Instr::ex_div4: virgula+=4; break;
                case Instr::ex_div5: virgula+=5; break;
                case Instr::ex_div6: virgula+=6; break;
                }
            if (virgula>60)
                virgula=60;
        // Zero é sempre zero
            if (valor==0)
                return "0";
        // Obtém em nome a string correspondente ao número
            char mens[80];
            char * d=mens, *destino=txtnum;
            while (valor>0)
                *d++=valor%10+'0', valor/=10;
        // Obtém o número de dígitos
            int digitos = d-mens;
            if (digitos <= virgula)
                digitos = virgula+1;
        // Anota o número
            if (negativo)
                *destino++ = '-';
            while (digitos>0)
            {
                if (digitos==virgula)
                    *destino++ = '.';
                digitos--;
                *destino++ = (&mens[digitos]>=d ? '0' : mens[digitos]);
            }
            return txtnum;
        }
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return "";

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

    case Instr::cVarClasse:
        if (endvar==0)
            break;
        return ((TClasse*)endvar)->Nome;
    case Instr::cVarObjeto:
        if (endvar==0)
            break;
        return ((TObjeto*)endvar)->Classe->Nome;
    }
    return "";
}

//------------------------------------------------------------------------------
TObjeto * TVariavel::getObj()
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return 0;
    switch (defvar[2])
    {
    case Instr::cVarObjeto:
        return (TObjeto*)endvar;
    }
    return 0;
}

//------------------------------------------------------------------------------
void TVariavel::setInt(int valor)
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        mprintf((char*)endvar, Tamanho(defvar), "%d", valor);
        break;
    case Instr::cInt1:
        *(char*)endvar |= bit;
        if (valor==0)
            *(char*)endvar ^= bit;
        break;
    case Instr::cInt8:
        if (valor<-0x80)
            valor=0x80;
        else if (valor>0x7F)
            valor=0x7F;
        *(char*)endvar=valor;
        break;
    case Instr::cUInt8:
        if (valor<0)
            valor=0;
        else if (valor>0xFF)
            valor=0xFF;
        *(char*)endvar=valor;
        break;
    case Instr::cInt16:
        if (valor<-0x8000)
            valor=-0x8000;
        else if (valor>0x7FFF)
            valor=0x7FFF;
        *(char*)endvar = (unsigned char)valor;
        *((char*)endvar+1) = (unsigned char)(valor>>8);
        break;
    case Instr::cUInt16:
        if (valor<0)
            valor=0;
        else if (valor>0xFFFF)
            valor=0xFFFF;
        *((unsigned char*)endvar+0) = (unsigned char)valor;
        *((unsigned char*)endvar+1) = (unsigned char)(valor>>8);
        break;
    case Instr::cUInt32:
        if (valor<0)
            valor=0;
    case Instr::cInt32:
        *((unsigned char*)endvar+0) = (unsigned char)valor;
        *((unsigned char*)endvar+1) = (unsigned char)(valor>>8);
        *((unsigned char*)endvar+2) = (unsigned char)(valor>>16);
        *((unsigned char*)endvar+3) = (unsigned char)(valor>>24);
        break;
    case Instr::cIntInc:
    case Instr::cIntDec:
        break;
    case Instr::cReal:
        *(double*)endvar = valor;
        break;
    case Instr::cRef:
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
        break;
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;

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
}

//------------------------------------------------------------------------------
void TVariavel::setDouble(double valor)
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        mprintf((char*)endvar, Tamanho(defvar), "%d", (int)valor);
        break;
    case Instr::cInt1:
        *(char*)endvar |= bit;
        if (valor==0)
            *(char*)endvar ^= bit;
        break;
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
        if (valor<-0x80000000LL)
            setInt(-0x80000000);
        else if (valor>0x7FFFFFFFLL)
            setInt(0x7FFFFFFF);
        else
            setInt((int)valor);
        break;
    case Instr::cUInt32:
        {
            unsigned int x=0;
            if (valor>0xFFFFFFFFLL)
                x=0xFFFFFFFF;
            else if (valor>0)
                x=(int)valor;
            *((unsigned char*)endvar+0) = (unsigned char)x;
            *((unsigned char*)endvar+1) = (unsigned char)(x>>8);
            *((unsigned char*)endvar+2) = (unsigned char)(x>>16);
            *((unsigned char*)endvar+3) = (unsigned char)(x>>24);
            break;
        }
    case Instr::cReal:
        *(double*)endvar = valor;
        break;
    case Instr::cRef:
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;

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
}

//------------------------------------------------------------------------------
void TVariavel::setTxt(const char * txt)
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        copiastr((char*)endvar, txt, Tamanho(defvar));
        break;
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
        {
            long num;
            errno=0, num=strtol(txt, 0, 10);
            if (errno)
                num=0;
            setInt(num);
            break;
        }
    case Instr::cUInt32:
        {
            unsigned long num;
            errno=0, num=strtoul(txt, 0, 10);
            if (errno)
                num=0;
            *((unsigned char*)endvar+0) = (unsigned char)num;
            *((unsigned char*)endvar+1) = (unsigned char)(num>>8);
            *((unsigned char*)endvar+2) = (unsigned char)(num>>16);
            *((unsigned char*)endvar+3) = (unsigned char)(num>>24);
            break;
        }
    case Instr::cReal:
        {
            double num;
            errno=0, num=strtod(txt, 0);
            if (errno)
                num=0;
            setDouble(num);
            break;
        }
    case Instr::cRef:
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;

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
}

//------------------------------------------------------------------------------
void TVariavel::addTxt(const char * txt)
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return;
    switch (defvar[2])
    {
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        {
            char * dest = (char*)endvar;
            const char * fim = dest + Tamanho(defvar) - 1;
            while (*dest)
                dest++;
            while (*txt && dest<fim)
                *dest++ = *txt++;
            *dest=0;
            break;
        }
    }
}

//------------------------------------------------------------------------------
void TVariavel::setObj(TObjeto * obj)
{
    if (defvar==0 || defvar[0]==0 && defvar[1]==0)
        return;
    switch (defvar[2])
    {
    case Instr::cVarObjeto:
        endvar = obj;
        break;
    }
}
