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
#include "classe.h"
#include "objeto.h"
#include "var-listaobj.h"
#include "var-texto.h"
#include "var-log.h"
#include "var-sav.h"
#include "var-txt.h"
#include "var-nomeobj.h"
#include "var-telatxt.h"
#include "var-socket.h"
#include "var-serv.h"
#include "var-prog.h"
#include "var-debug.h"
#include "var-indiceobj.h"
#include "var-datahora.h"
#include "var-outros.h"
#include "misc.h"

//#define DEBUG_CRIAR // Mostra quando uma variável é criada ou apagada
//#define DEBUG_MOVER // Mostra quando uma variável é movida

//----------------------------------------------------------------------------
// Usado em TVariavel::Mover()

#define MOVER_SIMPLES(classenome) \
        { \
            classenome * o = (classenome *)endvar;  \
            classenome * d = (classenome *)destino; \
            int inc=1;                              \
            if (destino > endvar)                   \
                o+=vetor-1, d+=vetor-1, inc=-1;     \
            for (; vetor; vetor--,o+=inc,d+=inc)    \
                o->Mover(d);                        \
            endvar = destino;                       \
            return;                                 \
        }

#define MOVER_COMPLETO(classenome) \
        { \
            classenome * o = (classenome *)endvar;    \
            classenome * d = (classenome *)destino;   \
            int inc=1;                                \
            if (destino > endvar)                     \
                o+=vetor-1, d+=vetor-1, inc=-1;       \
            for (; vetor; vetor--,o+=inc,d+=inc)      \
            {                                         \
                o->defvar = defvar;                   \
                o->indice = o - (classenome *)endvar; \
                o->EndObjeto(classe, objeto);         \
                o->Mover(d);                          \
            }                                         \
            endvar = destino;                         \
            return;                                   \
        }

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
    switch (instr[2])
    {
// Variáveis
    case Instr::cTxt1:      return 2 + (unsigned char)instr[Instr::endIndice];
    case Instr::cTxt2:      return 258 + (unsigned char)instr[Instr::endIndice];
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:     return 1;
    case Instr::cInt16:
    case Instr::cUInt16:    return sizeof(short);
    case Instr::cInt32:
    case Instr::cUInt32:    return sizeof(int);
    case Instr::cIntInc:
    case Instr::cIntDec:    return sizeof(TVarIncDec);
    case Instr::cReal:      return sizeof(double);
    case Instr::cRef:       return sizeof(TVarRef);
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:   return 0;

// Variáveis extras
    case Instr::cListaObj:  return sizeof(TListaObj);
    case Instr::cListaItem: return sizeof(TListaItem);
    case Instr::cTextoTxt:  return sizeof(TTextoTxt);
    case Instr::cTextoPos:  return sizeof(TTextoPos);
    case Instr::cNomeObj:   return sizeof(TVarNomeObj);
    case Instr::cArqLog:    return sizeof(TVarLog);
    case Instr::cArqSav:    return 0;
    case Instr::cArqTxt:    return sizeof(TVarTxt);
    case Instr::cIntTempo:  return sizeof(TVarIntTempo);;
    case Instr::cTelaTxt:   return sizeof(TVarTelaTxt);
    case Instr::cSocket:    return sizeof(TVarSocket);
    case Instr::cServ:      return sizeof(TVarServ);
    case Instr::cProg:      return sizeof(TVarProg);
    case Instr::cDebug:     return 0;
    case Instr::cIndiceObj: return sizeof(TIndiceObj);
    case Instr::cIndiceItem: return sizeof(TIndiceItem);
    case Instr::cDataHora:  return sizeof(TVarDataHora);

    case Instr::cVarNome:   return VAR_NOME_TAM;
    case Instr::cVarInicio:
    case Instr::cVarClasse:
    case Instr::cVarObjeto:
    case Instr::cVarInt:    return 0;
    }
    return 0;
}

//------------------------------------------------------------------------------
int TVariavel::Tamanho()
{
    int total = (indice!=0xFF ? 0 : (unsigned char)defvar[Instr::endVetor]);
    if (total<=1)       // Uma variável
        return Tamanho(defvar);
    if (defvar[2]==Instr::cInt1) // Vetor de bits
        return (total+7)/8;
    return Tamanho(defvar) * total; // Outro tipo de vetor
}

//------------------------------------------------------------------------------
TVarTipo TVariavel::Tipo()
{
    if (indice==0xFF) // Vetor
        return varOutros;
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
    case Instr::cConstNulo: return varObj;
    case Instr::cConstTxt:  return varTxt;
    case Instr::cConstNum:  return varDouble;
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:   return varOutros;

// Variáveis extras
    case Instr::cListaObj:  return varOutros;
    case Instr::cListaItem: return varOutros;
    case Instr::cTextoTxt:  return varOutros;
    case Instr::cTextoPos:
        if (defvar[Instr::endNome]=='=')
            return varInt;
        return varOutros;
    case Instr::cNomeObj:   return varOutros;
    case Instr::cArqLog:    return varOutros;
    case Instr::cArqSav:    return varOutros;
    case Instr::cArqTxt:    return varOutros;
    case Instr::cIntTempo:  return varInt;
    case Instr::cTelaTxt:
        if (defvar[Instr::endNome]=='=')
            return (defvar[Instr::endNome+1]=='0' ? varTxt : varInt);
        return varOutros;
    case Instr::cSocket:
        if (defvar[Instr::endNome]=='=')
            return varInt;
        return varOutros;
    case Instr::cServ:      return varOutros;
    case Instr::cProg:      return varOutros;
    case Instr::cDebug:
        if (defvar[Instr::endNome]=='=')
            return varInt;
        return varOutros;
    case Instr::cIndiceObj: return varTxt;
    case Instr::cIndiceItem: return varOutros;
    case Instr::cDataHora:
        if (defvar[Instr::endNome]=='=')
            return varInt;
        return varOutros;

    case Instr::cTxtFixo:
    case Instr::cVarNome:
    case Instr::cVarClasse: return varTxt;
    case Instr::cVarObjeto: return varObj;
    case Instr::cVarInt:    return varInt;
    }
    return varOutros;
}

//------------------------------------------------------------------------------
void TVariavel::Criar(TClasse * c, TObjeto * o)
{
    Redim(c, o, 0, defvar[Instr::endVetor] ?
                   (unsigned char)defvar[Instr::endVetor] : 1);
}

//------------------------------------------------------------------------------
void TVariavel::Apagar()
{
    Redim(0, 0, defvar[Instr::endVetor] ?
                (unsigned char)defvar[Instr::endVetor] : 1, 0);
}

//------------------------------------------------------------------------------
void TVariavel::Redim(TClasse * c, TObjeto * o, unsigned int antes, unsigned int depois)
{
    if (antes==depois)
        return;
// Mostra o que vai fazer
#ifdef DEBUG_CRIAR
    if (depois>antes)
        printf("Variável criada (%d a %d) %p   ", antes, depois-1, endvar);
    else
        printf("Variável apagada (%d a %d) %p   ", depois, antes-1, endvar);
    char mens[500];
    if (Instr::Decod(mens, defvar, sizeof(mens)))
        puts(mens);
    else
        printf("ERRO: %s\n", mens);
    fflush(stdout);
#endif
// Vetor de bits
    if (defvar[2]==Instr::cInt1)
    {
    // Se deve apagar variáveis: retorna
        if (depois<antes)
            return;
    // Obtém byte e bit inicial
        char * end = end_char + antes/8;
        unsigned char b = (bit + bit * 0x100) << (antes&7);
    // Se não começa no bit 0: avança bit a bit até o fim do primeiro byte
        if (b>1)
        {
            for (; antes<depois && b; antes++)
                *end &= ~b, b<<=1;
            end++;
        }
    // Preenche bytes = 0
        for (; antes+8 <= depois; antes+=8)
            *end++ = 0;
    // Avança bit a bit
        for (b=1; antes<depois; antes++)
            *end &= ~b, b<<=1;
        return;
    }
// Obtém o tamanho de uma variável
    int tam = Tamanho(defvar);
    if (tam==0)
    {
        endvar=0;
        return;
    }
// Acerta variável
    if (depois>antes)
        memset(end_char+antes*tam, 0, (depois-antes)*tam);
    switch (defvar[2])
    {
    case Instr::cRef:
        for (; depois<antes; depois++)
            end_varref[depois].MudarPont(0);
        break;
    case Instr::cIntInc:
        for (; antes<depois; antes++)
            end_incdec[antes].setInc(0);
        break;
    case Instr::cIntDec:
        for (; antes<depois; antes++)
            end_incdec[antes].setDec(0);
        break;
    case Instr::cArqLog:
        for (; antes<depois; antes++)
            end_log[antes].Criar();
        for (; depois<antes; depois++)
            end_log[depois].Apagar();
        break;
    case Instr::cArqTxt:
        for (; antes<depois; antes++)
            end_txt[antes].Criar();
        for (; depois<antes; depois++)
            end_txt[depois].Apagar();
        break;
    case Instr::cIntTempo:
        for (; antes<depois; antes++)
        {
            end_inttempo[antes].defvar = defvar;
            end_inttempo[antes].indice = antes;
            end_inttempo[antes].EndObjeto(c, o);
        }
        for (; depois<antes; depois++)
            end_inttempo[depois].setValor(0);
        break;
    case Instr::cListaObj:
        for (; antes<depois; antes++)
            end_listaobj[antes].EndObjeto(o);
        for (; depois<antes; depois++)
            end_listaobj[depois].Apagar();
        break;
    case Instr::cListaItem:
        for (; antes<depois; antes++)
        {
            end_listaitem[antes].Objeto = o;
            end_listaitem[antes].defvar = defvar;
            end_listaitem[antes].indice = antes;
        }
        for (; depois<antes; depois++)
            end_listaitem[depois].Apagar();
        break;
    case Instr::cTextoTxt:
        for (; depois<antes; depois++)
            end_textotxt[depois].Apagar();
        break;
    case Instr::cTextoPos:
        for (; antes<depois; antes++)
        {
            end_textopos[antes].Objeto = o;
            end_textopos[antes].defvar = defvar;
            end_textopos[antes].indice = antes;
        }
        for (; depois<antes; depois++)
            end_textopos[depois].Apagar();
        break;
    case Instr::cTelaTxt:
        for (; antes<depois; antes++)
        {
            end_telatxt[antes].Criar();
            end_telatxt[antes].defvar = defvar;
            end_telatxt[antes].indice = antes;
            end_telatxt[antes].EndObjeto(c, o);
        }
        for (; depois<antes; depois++)
            end_telatxt[depois].Apagar();
        break;
    case Instr::cSocket:
        for (; antes<depois; antes++)
        {
            end_socket[antes].defvar = defvar;
            end_socket[antes].indice = antes;
            end_socket[antes].EndObjeto(c, o);
        }
        for (; depois<antes; depois++)
            end_socket[depois].MudarSock(0);
        break;
    case Instr::cServ:
        for (; antes<depois; antes++)
        {
            end_serv[antes].defvar = defvar;
            end_serv[antes].indice = antes;
            end_serv[antes].EndObjeto(c, o);
            end_serv[antes].Criar();
        }
        for (; depois<antes; depois++)
            end_serv[depois].Apagar();
        break;
    case Instr::cProg:
        for (; antes<depois; antes++)
            end_prog[antes].Criar();
        for (; depois<antes; depois++)
            end_prog[depois].Apagar();
        break;
    case Instr::cIndiceObj:
        for (; antes<depois; antes++)
            end_indiceobj[antes].Objeto = o;
        for (; depois<antes; depois++)
            end_indiceobj[depois].Apagar();
        break;
    case Instr::cIndiceItem:
        for (; depois<antes; depois++)
            end_indiceitem[depois].Apagar();
        break;
    case Instr::cDataHora:
        for (; antes<depois; antes++)
            end_datahora[antes].Criar();
    }
}

//------------------------------------------------------------------------------
void TVariavel::Mover(void * destino, TClasse * classe, TObjeto * objeto)
{
    if (destino==endvar)
        return;
    int vetor = (unsigned char)defvar[Instr::endVetor];
    if (vetor==0)
        vetor++;
#ifdef DEBUG_MOVER
    printf("Variável movida (0 a %d) de %p para %p  ",
           vetor-1, endvar, destino);
    char mens1[500];
    if (Instr::Decod(mens1, defvar, sizeof(mens1)))
        puts(mens1);
    else
        printf("ERRO: %s\n", mens1);
    fflush(stdout);
#endif
    switch (defvar[2])
    {
    case Instr::cInt1:
        if (vetor <= 8)
            *(char*)destino = *(char*)endvar;
        else
            move_mem(destino, endvar, Tamanho());
        endvar = destino;
        return;
    case Instr::cInt8:
    case Instr::cUInt8:
        if (vetor <= 1)
            *(char*)destino = *(char*)endvar;
        else
            move_mem(destino, endvar, vetor);
        endvar = destino;
        return;
    case Instr::cInt16:
    case Instr::cUInt16:
        if (vetor <= 1)
        {
            short x = *(short*)endvar;
            *(short*)destino = x;
        }
        else
            move_mem(destino, endvar, vetor*sizeof(short));
        endvar = destino;
        return;
    case Instr::cInt32:
    case Instr::cUInt32:
        if (vetor <= 1)
        {
            int x = *(short*)endvar;
            *(int*)destino = x;
        }
        else
            move_mem(destino, endvar, vetor*sizeof(int));
        endvar = destino;
        return;
    case Instr::cIntInc:
    case Instr::cIntDec:
        if (vetor <= 1)
            move_mem(destino, endvar, sizeof(TVarIncDec));
        else
            move_mem(destino, endvar, vetor*sizeof(TVarIncDec));
        endvar = destino;
        return;
    case Instr::cReal:
        if (vetor <= 1)
            move_mem(destino, endvar, sizeof(double));
        else
            move_mem(destino, endvar, vetor*sizeof(double));
        endvar = destino;
        return;
    case Instr::cRef:
        MOVER_SIMPLES( TVarRef )
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        endvar = destino;
        return;

// Variáveis extras
    case Instr::cListaObj:
        {
            TListaObj * o = (TListaObj *)endvar;
            TListaObj * d = (TListaObj *)destino;
            int inc=1;
            if (destino > endvar)
                o+=vetor-1, d+=vetor-1, inc=-1;
            for (; vetor; vetor--,o+=inc,d+=inc)
            {
                o->EndObjeto(objeto);
                o->Mover(d);
            }
            endvar = destino;
            return;
        }
    case Instr::cListaItem:
        {
            TListaItem * o = (TListaItem *)endvar;
            TListaItem * d = (TListaItem *)destino;
            int inc=1;
            if (destino > endvar)
                o+=vetor-1, d+=vetor-1, inc=-1;
            for (; vetor; vetor--,o+=inc,d+=inc)
            {
                o->defvar = defvar;
                o->indice = o - (TListaItem *)endvar;
                o->Objeto = objeto;
                o->Mover(d);
            }
            endvar = destino;
            return;
        }
    case Instr::cTextoTxt:
        MOVER_SIMPLES( TTextoTxt )
    case Instr::cTextoPos:
        {
            TTextoPos * o = (TTextoPos *)endvar;
            TTextoPos * d = (TTextoPos *)destino;
            int inc=1;
            if (destino > endvar)
                o+=vetor-1, d+=vetor-1, inc=-1;
            for (; vetor; vetor--,o+=inc,d+=inc)
            {
                o->defvar = defvar;
                o->indice = o - (TTextoPos *)endvar;
                o->Objeto = objeto;
                o->Mover(d);
            }
            endvar = destino;
            return;
        }
    case Instr::cNomeObj:
        if (vetor <= 1)
            move_mem(destino, endvar, sizeof(TVarNomeObj));
        else
            move_mem(destino, endvar, vetor*sizeof(TVarNomeObj));
        endvar = destino;
        return;
    case Instr::cArqLog:
        MOVER_SIMPLES( TVarLog )
    case Instr::cArqSav:
        endvar = destino;
        return;
    case Instr::cArqTxt:
        if (vetor <= 1)
            move_mem(destino, endvar, sizeof(TVarTxt));
        else
            move_mem(destino, endvar, vetor*sizeof(TVarTxt));
        endvar = destino;
        return;
    case Instr::cIntTempo:
        MOVER_COMPLETO( TVarIntTempo )
    case Instr::cTelaTxt:
        MOVER_COMPLETO( TVarTelaTxt )
    case Instr::cSocket:
        MOVER_COMPLETO( TVarSocket )
    case Instr::cServ:
        MOVER_COMPLETO( TVarServ )
    case Instr::cProg:
        MOVER_SIMPLES( TVarProg )
    case Instr::cDebug:
        endvar = destino;
        return;
    case Instr::cIndiceObj:
        {
            TIndiceObj * o = (TIndiceObj *)endvar;
            TIndiceObj * d = (TIndiceObj *)destino;
            int inc=1;
            if (destino > endvar)
                o+=vetor-1, d+=vetor-1, inc=-1;
            for (; vetor; vetor--,o+=inc,d+=inc)
            {
                o->Objeto = objeto;
                o->Mover(d);
            }
            endvar = destino;
            return;
        }
    case Instr::cIndiceItem:
        MOVER_SIMPLES( TIndiceItem )
    case Instr::cDataHora:
        MOVER_SIMPLES( TVarDataHora )

// Outras variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        if (vetor>1)
        {
            move_mem(destino, endvar, Tamanho(defvar)*vetor);
            endvar = destino;
            return;
        }
    case Instr::cTxtFixo:
        if (destino < endvar)
            strcpy((char*)destino, (char*)endvar);
        else
            move_mem(destino, endvar, strlen((char*)endvar) + 1);
        endvar = destino;
        return;
    case Instr::cVarNome:
        move_mem(destino, endvar, VAR_NOME_TAM);
        endvar = destino;
        return;
    case Instr::cVarInicio:
        endvar = destino;
    case Instr::cVarClasse:
    case Instr::cVarObjeto:
    case Instr::cVarInt:
        return;
    }
    assert(0);
}

//------------------------------------------------------------------------------
bool TVariavel::getBool()
{
    if (indice==0xFF) // Vetor
        return false;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        if (indice==0)
            return (end_char[0] != 0);
        return (end_char[indice * Tamanho(defvar)] != 0);
    case Instr::cTxtFixo:
        return (*end_char != 0);
    case Instr::cInt8:
    case Instr::cUInt8:
        return (end_char[indice] != 0);
    case Instr::cInt1:
        if (indice)
        {
            int valor = (bit + bit * 0x100) << (indice&7);
            return end_char[indice/8] & (valor>>8);
        }
        return (end_char[0] & bit);
    case Instr::cInt16:
    case Instr::cUInt16:
        return end_short[indice];
    case Instr::cInt32:
    case Instr::cUInt32:
        return end_int[indice];
    case Instr::cIntInc:
        return end_incdec[indice].getInc();
    case Instr::cIntDec:
        return end_incdec[indice].getDec();
    case Instr::cReal:
        return (end_double[indice] != 0);
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        return defvar[(unsigned char)defvar[Instr::endIndice] + 1] != 0;
    case Instr::cConstNum:
        {
            const char * origem = defvar + defvar[Instr::endIndice];
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
    case Instr::cListaObj:
        return end_listaobj[indice].getValor();
    case Instr::cListaItem:
        return end_listaitem[indice].getValor();
    case Instr::cTextoTxt:
        return end_textotxt[indice].getValor();
    case Instr::cTextoPos:
        return end_textopos[indice].getValor(defvar);
    case Instr::cNomeObj:
        return end_nomeobj[indice].getValor();
    case Instr::cArqLog:
        return end_log[indice].getValor();
    case Instr::cArqSav:
        return 0;
    case Instr::cArqTxt:
        return end_txt[indice].getValor();
    case Instr::cIntTempo:
        return end_inttempo[indice].getValor();
    case Instr::cTelaTxt:
        return end_telatxt[indice].getValor(defvar);
    case Instr::cSocket:
        return end_socket[indice].getValor(defvar);
    case Instr::cServ:
        return end_serv[indice].getValor();
    case Instr::cProg:
        return end_prog[indice].getValor();
    case Instr::cDebug:
        return TVarDebug::getValor(defvar);
    case Instr::cIndiceObj:
        return end_indiceobj[indice].getNome()[0] != 0;
    case Instr::cIndiceItem:
        return end_indiceitem[indice].getValor();
    case Instr::cDataHora:
        return end_datahora[indice].getValor(defvar);
    case Instr::cRef:
        return end_varref[indice].Pont != 0;
    case Instr::cVarObjeto:
        return (endvar!=0);
    case Instr::cVarInt:
        return (valor_int!=0);
    }
    return 0;
}

//------------------------------------------------------------------------------
int TVariavel::getInt()
{
    if (indice==0xFF) // Vetor
        return 0;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        if (indice)
        {
            long num;
            errno=0, num=strtol(&end_char[indice * Tamanho(defvar)], 0, 10);
            return (errno ? 0 : num);
        }
    case Instr::cTxtFixo:
        {
            long num;
            errno=0, num=strtol(end_char, 0, 10);
            return (errno ? 0 : num);
        }
    case Instr::cInt1:
        if (indice==0)
            return (end_char[0] & bit) != 0;
        return getBool();
    case Instr::cInt8:
        return (signed char)end_char[indice];
    case Instr::cUInt8:
        return (unsigned char)end_char[indice];
    case Instr::cInt16:
        return end_short[indice];
    case Instr::cUInt16:
        return end_ushort[indice];
    case Instr::cInt32:
        return end_int[indice];
    case Instr::cUInt32:
        return (end_uint[indice] > 0x7FFFFFFF ?
                0x7FFFFFFF : end_uint[indice]);
    case Instr::cIntInc:
        return end_incdec[indice].getInc();
    case Instr::cIntDec:
        return end_incdec[indice].getDec();
    case Instr::cReal:
        return (int)end_double[indice];
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        {
            long num;
            errno=0, num=strtol(defvar + defvar[Instr::endIndice] + 1, 0, 10);
            return (errno ? 0 : num);
        }
    case Instr::cConstNum:
        {
            unsigned int valor = 0;
            bool negativo = false;
            const char * origem = defvar + defvar[Instr::endIndice];
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
    case Instr::cVarInt:
        return valor_int;

// Variáveis extras
    case Instr::cListaObj:
        return end_listaobj[indice].getValor();
    case Instr::cListaItem:
        return end_listaitem[indice].getValor();
    case Instr::cTextoTxt:
        return end_textotxt[indice].getValor();
    case Instr::cTextoPos:
        return end_textopos[indice].getValor(defvar);
    case Instr::cNomeObj:
        return end_nomeobj[indice].getValor();
    case Instr::cArqLog:
        return end_log[indice].getValor();
    case Instr::cArqSav:
        return 0;
    case Instr::cArqTxt:
        return end_txt[indice].getValor();
    case Instr::cIntTempo:
        return end_inttempo[indice].getValor();
    case Instr::cTelaTxt:
        return end_telatxt[indice].getValor(defvar);
    case Instr::cSocket:
        return end_socket[indice].getValor(defvar);
    case Instr::cServ:
        return end_serv[indice].getValor();
    case Instr::cProg:
        return end_prog[indice].getValor();
    case Instr::cDebug:
        return TVarDebug::getValor(defvar);
    case Instr::cIndiceObj:
        {
            long num;
            errno=0, num=strtol(end_indiceobj[indice].getNome(), 0, 10);
            return (errno ? 0 : num);
        }
    case Instr::cIndiceItem:
        return end_indiceitem[indice].getValor();
    case Instr::cDataHora:
        return end_datahora[indice].getValor(defvar);

    case Instr::cRef:
        return end_varref[indice].Pont != 0;
    case Instr::cVarObjeto:
        return (endvar!=0);
    }
    return 0;
}

//------------------------------------------------------------------------------
double TVariavel::getDouble()
{
    if (indice==0xFF) // Vetor
        return 0;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        if (indice)
        {
            double num;
            errno=0, num=strtod(&end_char[indice * Tamanho(defvar)], 0);
            return (errno ? 0 : num);
        }
    case Instr::cTxtFixo:
        {
            double num;
            errno=0, num=strtod(end_char, 0);
            return (errno ? 0 : num);
        }
    case Instr::cInt1:
        if (indice==0)
            return (end_char[0] & bit) != 0;
        return getBool();
    case Instr::cInt8:
        return (signed char)end_char[indice];
    case Instr::cUInt8:
        return (unsigned char)end_char[indice];
    case Instr::cInt16:
        return end_short[indice];
    case Instr::cUInt16:
        return end_ushort[indice];
    case Instr::cInt32:
        return end_int[indice];
    case Instr::cUInt32:
        return end_uint[indice];
    case Instr::cIntInc:
        return end_incdec[indice].getInc();
    case Instr::cIntDec:
        return end_incdec[indice].getDec();
    case Instr::cReal:
        return end_double[indice];
    case Instr::cConstNulo:
        return 0;
    case Instr::cConstTxt:
        {
            double num;
            errno=0, num=strtod(defvar + defvar[Instr::endIndice] + 1, 0);
            return (errno ? 0 : num);
        }
    case Instr::cConstNum:
        {
            double valor = 0;
            bool negativo = false;
            const char * origem = defvar + defvar[Instr::endIndice];
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
    case Instr::cVarInt:
        return valor_int;

// Variáveis extras
    case Instr::cListaObj:
        return end_listaobj[indice].getValor();
    case Instr::cListaItem:
        return end_listaitem[indice].getValor();
    case Instr::cTextoTxt:
        return end_textotxt[indice].getValor();
    case Instr::cTextoPos:
        return end_textopos[indice].getValor(defvar);
    case Instr::cNomeObj:
        return end_nomeobj[indice].getValor();
    case Instr::cArqLog:
        return end_log[indice].getValor();
    case Instr::cArqSav:
        return 0;
    case Instr::cArqTxt:
        return end_txt[indice].getValor();
    case Instr::cIntTempo:
        return end_inttempo[indice].getValor();
    case Instr::cTelaTxt:
        return end_telatxt[indice].getValor(defvar);
    case Instr::cSocket:
        return end_socket[indice].getValor(defvar);
    case Instr::cServ:
        return end_serv[indice].getValor();
    case Instr::cProg:
        return end_prog[indice].getValor();
    case Instr::cDebug:
        return TVarDebug::getValor(defvar);
    case Instr::cIndiceObj:
        {
            double num;
            errno=0, num=strtod(end_indiceobj[indice].getNome(), 0);
            return (errno ? 0 : num);
        }
    case Instr::cIndiceItem:
        return end_indiceitem[indice].getValor();
    case Instr::cDataHora:
        return end_datahora[indice].getValor(defvar);

    case Instr::cRef:
        return end_varref[indice].Pont != 0;
    case Instr::cVarObjeto:
        return (endvar!=0);
    }
    return 0;
}

//------------------------------------------------------------------------------
const char * TVariavel::getTxt()
{
    if (indice==0xFF) // Vetor
        return "";
    static char txtnum[80];
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        if (indice)
            return &end_char[indice * Tamanho(defvar)];
    case Instr::cTxtFixo:
    case Instr::cVarNome:
        return end_char;
    case Instr::cInt1:
        if (indice==0)
            return (end_char[0] & bit ? "1" : "0");
        return (getBool() ? "1" : "0");
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cIntTempo:
    case Instr::cArqLog:
    case Instr::cArqTxt:
    case Instr::cListaObj:
    case Instr::cListaItem:
    case Instr::cTextoTxt:
    case Instr::cTextoPos:
    case Instr::cNomeObj:
    case Instr::cDebug:
    case Instr::cIndiceItem:
    case Instr::cDataHora:
        sprintf(txtnum, "%d", getInt());
        return txtnum;
    case Instr::cUInt32:
        sprintf(txtnum, "%u", end_uint[indice]);
        return txtnum;
    case Instr::cReal:
        {
            double d = getDouble();
            if (d >= DOUBLE_MAX || d <= -DOUBLE_MAX)
            {
                sprintf(txtnum, "%E", getDouble());
                return txtnum;
            }
            sprintf(txtnum, "%.9f", d);
            char * p = txtnum;
            while (*p)
                p++;
            while (p>txtnum && p[-1]=='0')
                p--;
            if (p>txtnum && p[-1]=='.')
                p--;
            *p=0;
            return txtnum;
        }
    case Instr::cConstNulo:
        return "";
    case Instr::cConstTxt:
        return defvar + defvar[Instr::endIndice] + 1;
    case Instr::cConstNum:
        {
            unsigned int valor = 0; // Valor numérico sem sinal
            bool negativo = false; // Se é negativo
            int  virgula = 0;   // Casas após a vírgula
            const char * origem = defvar + defvar[Instr::endIndice];
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
            *destino=0;
            return txtnum;
        }
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        return "";

// Variáveis extras
    case Instr::cTelaTxt:
        return end_telatxt[indice].getTxt(defvar);
    case Instr::cSocket:
        if (defvar[Instr::endNome]!='=')
            return "";
        sprintf(txtnum, "%d", getInt());
        return txtnum;
    case Instr::cArqSav:
    case Instr::cServ:
    case Instr::cProg:
        return "";
    case Instr::cIndiceObj:
        return end_indiceobj[indice].getNome();

    case Instr::cRef:
        if (end_varref[indice].Pont == 0)
            break;
        return end_varref[indice].Pont->Classe->Nome;
    case Instr::cVarClasse:
        if (endvar==0)
            break;
        return ((TClasse*)endvar)->Nome;
    case Instr::cVarObjeto:
        if (endvar==0)
            break;
        return ((TObjeto*)endvar)->Classe->Nome;
    case Instr::cVarInt:
        sprintf(txtnum, "%d", valor_int);
        return txtnum;
    }
    return "";
}

//------------------------------------------------------------------------------
TObjeto * TVariavel::getObj()
{
    if (indice==0xFF && defvar[Instr::endVetor]) // Vetor
        return 0;
    switch (defvar[2])
    {
    case Instr::cRef:
        return end_varref[indice].Pont;
    case Instr::cVarObjeto:
        return (TObjeto*)endvar;
    case Instr::cListaItem:
        {
            TListaX * l = end_listaitem[indice].ListaX;
            return (l==0 ? 0 : l->Objeto);
        }
    }
    return 0;
}

//------------------------------------------------------------------------------
void TVariavel::setInt(int valor)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
        {
            int tam = Tamanho(defvar);
            if (indice)
                mprintf(&end_char[tam*indice], tam, "%d", valor);
            else
                mprintf(end_char, tam, "%d", valor);
            break;
        }
    case Instr::cInt1:
        if (indice)
        {
            int mask = (bit + bit * 0x100) << (indice&7);
            if (valor)
                end_char[indice/8] |= (mask>>8);
            else
                end_char[indice/8] &= ~(mask>>8);
            break;
        }
        if (valor)
            end_char[0] |= bit;
        else
            end_char[0] &= ~bit;
        break;
    case Instr::cInt8:
        if (valor<-0x80)
            valor=0x80;
        else if (valor>0x7F)
            valor=0x7F;
        end_char[indice]=valor;
        break;
    case Instr::cUInt8:
        if (valor<0)
            valor=0;
        else if (valor>0xFF)
            valor=0xFF;
        end_char[indice]=valor;
        break;
    case Instr::cInt16:
        if (valor<-0x8000)
            end_short[indice] = -0x8000;
        else if (valor>0x7FFF)
            end_short[indice] = 0x7FFF;
        else
            end_short[indice] = valor;
        break;
    case Instr::cUInt16:
        if (valor<0)
            end_ushort[indice] = 0;
        else if (valor>0xFFFF)
            end_ushort[indice] = 0xFFFF;
        else
            end_ushort[indice] = valor;
        break;
    case Instr::cUInt32:
        if (valor<0)
            valor=0;
    case Instr::cInt32:
        end_int[indice] = valor;
        break;
    case Instr::cIntInc:
        end_incdec[indice].setInc(valor);
        break;
    case Instr::cIntDec:
        end_incdec[indice].setDec(valor);
        break;
    case Instr::cReal:
        end_double[indice] = valor;
        break;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;
    case Instr::cVarInt:
        valor_int = valor;
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setValor(defvar, valor);
        break;
    case Instr::cTextoTxt:
    case Instr::cNomeObj:
    case Instr::cArqLog:
    case Instr::cArqSav:
    case Instr::cArqTxt:
        break;
    case Instr::cIntTempo:
        end_inttempo[indice].setValor(valor);
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setValor(defvar, valor);
        break;
    case Instr::cSocket:
        end_socket[indice].setValor(defvar, valor);
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cDebug:
        TVarDebug::setValor(defvar, valor);
        break;
    case Instr::cIndiceObj:
        {
            char mens[20];
            mprintf(mens, sizeof(mens), "%d", valor);
            end_indiceobj[indice].setNome(mens);
            break;
        }
    case Instr::cDataHora:
        end_datahora[indice].setValor(defvar, valor);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::setDouble(double valor)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cIntTempo:
    case Instr::cIndiceObj:
        setInt((int)valor);
        break;
    case Instr::cUInt32:
        if (valor > 0xFFFFFFFFLL)
            end_uint[indice] = 0xFFFFFFFFLL;
        else if (valor < 0)
            end_uint[indice] = 0;
        else
            end_uint[indice] = (unsigned int)valor;
        break;
    case Instr::cReal:
        end_double[indice] = valor;
        break;
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;
    case Instr::cVarInt:
        valor_int = (int)valor;
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setValor(defvar, (int)valor);
        break;
    case Instr::cTextoTxt:
    case Instr::cNomeObj:
    case Instr::cArqLog:
    case Instr::cArqSav:
    case Instr::cArqTxt:
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setValor(defvar, (int)valor);
        break;
    case Instr::cSocket:
        end_socket[indice].setValor(defvar, (int)valor);
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cDebug:
        TVarDebug::setValor(defvar, valor);
        break;
    case Instr::cDataHora:
        end_datahora[indice].setValor(defvar, (int)valor);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::setTxt(const char * txt)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
// Variáveis
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        {
            int tam = Tamanho(defvar);
            if (indice)
                copiastr(&end_char[tam*indice], txt, tam);
            else
                copiastr(end_char, txt, tam);
            break;
        }
    case Instr::cInt1:
    case Instr::cInt8:
    case Instr::cUInt8:
    case Instr::cInt16:
    case Instr::cUInt16:
    case Instr::cInt32:
    case Instr::cIntInc:
    case Instr::cIntDec:
    case Instr::cIntTempo:
    case Instr::cVarInt:
    case Instr::cSocket:
    case Instr::cDebug:
    case Instr::cDataHora:
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
            end_uint[indice] = num;
            break;
        }
    case Instr::cReal:
        {
            double num;
            errno=0, num=strtod(txt, 0);
            if (errno)
                num=0;
            end_double[indice] = num;
            break;
        }
    case Instr::cConstNulo:
    case Instr::cConstTxt:
    case Instr::cConstNum:
    case Instr::cConstExpr:
    case Instr::cFunc:
    case Instr::cVarFunc:
        break;

// Variáveis extras
    case Instr::cListaObj:
        break;
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(0);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].setTxt(defvar, txt);
        break;
    case Instr::cTextoTxt:
    case Instr::cNomeObj:
    case Instr::cArqLog:
    case Instr::cArqSav:
    case Instr::cArqTxt:
        break;
    case Instr::cTelaTxt:
        end_telatxt[indice].setTxt(defvar, txt);
        break;
    case Instr::cServ:
        end_serv[indice].Fechar();
        break;
    case Instr::cProg:
    case Instr::cIndiceItem:
        break;
    case Instr::cIndiceObj:
        end_indiceobj[indice].setNome(txt);
        break;
    case Instr::cRef:
        end_varref[indice].MudarPont(0);
        break;
    case Instr::cVarObjeto:
        endvar = 0;
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::addTxt(const char * txt)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cTxt1:
    case Instr::cTxt2:
    case Instr::cVarNome:
        {
            int tam = Tamanho(defvar);
            char * dest = end_char + indice * tam;
            const char * fim = dest + tam - 1;
            while (*dest)
                dest++;
            while (*txt && dest<fim)
                *dest++ = *txt++;
            *dest=0;
            break;
        }
    case Instr::cTelaTxt:
        end_telatxt[indice].addTxt(defvar, txt);
        break;
    case Instr::cIndiceObj:
        {
            char mens[100];
            mprintf(mens, sizeof(mens), "%s%s",
                    end_indiceobj[indice].getNome(), txt);
            end_indiceobj[indice].setNome(mens);
            break;
        }
        break;
    }
}

//------------------------------------------------------------------------------
void TVariavel::setObj(TObjeto * obj)
{
    if (indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cRef:
        end_varref[indice].MudarPont(obj);
        break;
    case Instr::cVarObjeto:
        endvar = obj;
        break;
    }
}

//------------------------------------------------------------------------------
int TVariavel::Compara(TVariavel * v)
{
    void * var1 = endvar;
    void * var2 = v->endvar;
    if (indice!=0xFF && v->indice!=0xFF)
        switch (defvar[2])
        {
        case Instr::cListaItem:
            var1 = end_listaitem[indice].ListaX;
            var2 = v->end_listaitem[v->indice].ListaX;
            break;
        case Instr::cTextoPos:
            return end_textopos[indice].Compara(v->end_textopos + v->indice);
        case Instr::cSocket:
            var1 = end_socket->Socket + indice;
            var2 = v->end_socket->Socket + v->indice;
            break;
        case Instr::cIndiceItem:
            var1 = end_indiceitem[indice].getIndiceObj();
            var2 =v->end_indiceitem[v->indice].getIndiceObj();
            break;
        case Instr::cDataHora:
            return end_datahora[indice].Compara(v->end_datahora + v->indice);
        }
    return (var1 == var2 ? 0 : var1 < var2 ? -1 : 1);
}

//------------------------------------------------------------------------------
void TVariavel::Igual(TVariavel * v)
{
    if (indice==0xFF || v->indice==0xFF) // Vetor
        return;
    switch (defvar[2])
    {
    case Instr::cListaItem:
        end_listaitem[indice].MudarRef(v->end_listaitem[v->indice].ListaX);
        break;
    case Instr::cTextoPos:
        end_textopos[indice].Igual(v->end_textopos + v->indice);
        break;
    case Instr::cSocket:
        end_socket[indice].Igual(v->end_socket + v->indice);
        break;
    case Instr::cIndiceItem:
        end_indiceitem[indice].Igual(v->end_indiceitem + v->indice);
        break;
    case Instr::cDataHora:
        end_datahora[indice].Igual(v->end_datahora + v->indice);
    }
}

//------------------------------------------------------------------------------
bool TVariavel::Func(const char * nome)
{
    if (indice==0xFF) // Vetor
    {
        int valor = 0;
        for (; *nome>='0' && *nome<='9'; nome++)
        {
            valor = valor * 10 + *nome - '0';
            if (valor >= 0xFF)
                return false;
        }
        if (*nome==0 && valor < (unsigned char)defvar[Instr::endVetor])
        {
            indice = valor;
            Instr::ApagarVar(this+1);
            return true;
        }
        switch (defvar[2])
        {
        case Instr::cTxt1:
        case Instr::cTxt2:
            return FuncVetorTxt(this, nome);
        }
        return false;
    }
    switch (defvar[2])
    {
    case Instr::cListaObj:
        return end_listaobj[indice].Func(this, nome);
    case Instr::cListaItem:
        return end_listaitem[indice].Func(this, nome);
    case Instr::cTextoTxt:
        return end_textotxt[indice].Func(this, nome);
    case Instr::cTextoPos:
        return end_textopos[indice].Func(this, nome);
    case Instr::cNomeObj:
        return end_nomeobj[indice].Func(this, nome);
    case Instr::cArqLog:
        return end_log[indice].Func(this, nome);
    case Instr::cArqSav:
        return TVarSav::Func(this, nome);
    case Instr::cArqTxt:
        return end_txt[indice].Func(this, nome);
    case Instr::cTelaTxt:
        return end_telatxt[indice].Func(this, nome);
    case Instr::cSocket:
        return end_socket[indice].Func(this, nome);
    case Instr::cServ:
        return end_serv[indice].Func(this, nome);
    case Instr::cProg:
        return end_prog[indice].Func(this, nome);
    case Instr::cDebug:
        return TVarDebug::Func(this, nome);
    case Instr::cIndiceItem:
        return end_indiceitem[indice].Func(this, nome);
    case Instr::cDataHora:
        return end_datahora[indice].Func(this, nome);
    }
    return false;
}
