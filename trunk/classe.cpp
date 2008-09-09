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
#include "classe.h"
#include "misc.h"

//----------------------------------------------------------------------------
TClasse::TClasse(const char * nome)
{
    Comandos=0;
    copiastr(Nome, nome, sizeof(Nome));
    RBinsert();
}

//----------------------------------------------------------------------------
TClasse::~TClasse()
{
    RBremove();
    if (Comandos)
        delete[] Comandos;
}

//----------------------------------------------------------------------------
bool TClasse::NomeClasse(char * nome)
{
    char *o,*d;
// Verifica se tem algum caracter inválido
    for (o=nome; *o; o++)
        if (tabNOMES[*(unsigned char*)o]==0)
            return false;
// Avança para início do nome
    for (o=nome; *o==' '; o++);
    if (*o>='0' && *o<='9')
        return false;
// Retira espaços desnecessários
    d=nome;
    for (; *o; o++)
        if (*o!=' ' || (o[1]!=' ' && o[1]))
            *d++=*o;
    *d=0;
// Checa tamanho do nome
    TClasse * cl=0; // Apenas para sizeof(TClasse::Nome)
    if (d-nome >= (int)sizeof(cl->Nome))
        return false;
    return true;
}

//----------------------------------------------------------------------------
TClasse * TClasse::Procura(const char * nome)
{
    int i;
    TClasse * y = RBroot;
    while (y)
    {
        i=comparaZ(nome, y->Nome);
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
int TClasse::RBcomp(TClasse * x, TClasse * y)
{
    return comparaZ(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TClasse * TClasse::RBroot=0;
#define CLASS TClasse          // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
