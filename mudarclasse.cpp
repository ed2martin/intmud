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
#include "mudarclasse.h"
#include "classe.h"
#include "objeto.h"
#include "instr.h"
#include "misc.h"

TMudarClasse * TMudarClasse::Inicio=0;
TMudarClasse * TMudarClasse::Fim=0;

//----------------------------------------------------------------------------
void TMudarClasse::MudarComandos(char * com)
{
    if (com==Comandos)
        return;
    if (Comandos)
        delete[] Comandos;
    Comandos = com;
}

//----------------------------------------------------------------------------
TMudarClasse::TMudarClasse(const char * nome)
{
    RBcolour=0;
    Comandos=0;
    copiastr(Nome, nome, sizeof(Nome));
    RBinsert();
    Antes=Fim;
    Depois=0;
    (Antes ? Antes->Depois : Inicio) = this;
    Fim = this;
}

//----------------------------------------------------------------------------
TMudarClasse::~TMudarClasse()
{
    if (Comandos)
        delete[] Comandos;
    RBremove();
    (Antes ? Antes->Depois : Inicio) = Depois;
    (Depois ? Depois->Antes : Fim) = Antes;
}

//----------------------------------------------------------------------------
bool TMudarClasse::ExecPasso()
{
    while (Inicio)
    {
    // Obtém a classe
        TClasse * cl = TClasse::Procura(Inicio->Nome);
    // Apagar classe
        if (Inicio->RBcolour & 2)
        {
        // Verifica se pode apagar
            if (cl==0 || cl->NumDeriv>0)
                Inicio->RBcolour &= ~2;
        // Verifica se tem objetos da classe
            else if (cl->ObjetoIni)
            {
                for (TObjeto * obj = cl->ObjetoIni; obj; obj=obj->Depois)
                    obj->MarcarApagar();
                return true;
            }
        // Apaga classe
            else
            {
                Inicio->RBcolour &= ~2;
                delete cl;
                cl=0;
            }
        }
    // Verifica se deve criar/alterar classe
        if (Inicio->Comandos==0)
        {
            delete Inicio;
            continue;
        }
    // Verifica se instrução herda ainda contém classes válidas
        if ((Inicio->Comandos[0] || Inicio->Comandos[1]) &&
            Inicio->Comandos[2]==Instr::cHerda)
        {
            const char * p = Inicio->Comandos + 4;
            for (unsigned char x = Inicio->Comandos[3]; x; x--)
            {
                if (TClasse::Procura(p)==0)
                {
                    p=0;
                    delete Inicio;
                    break;
                }
                while (*p++);
            }
            if (p==0)
                continue;
        }
    // Alterar classe
        if (cl)
        {
            char * antigo_com = cl->Comandos;
            cl->Comandos = Inicio->Comandos;
            Inicio->Comandos = 0;
            delete Inicio;
            cl->AcertaDeriv();
            cl->AcertaVar();
            for (int x=0; x<cl->NumDeriv; x++)
                cl->ListaDeriv[x]->AcertaVar();
            delete[] antigo_com;
            continue;
        }
    // Criar classe
        if (TClasse::NomeValido(Inicio->Nome))
        {
            char mens[2] = { 0, 0 };
            cl = new TClasse(Inicio->Nome);
            cl->Comandos = Inicio->Comandos;
            Inicio->Comandos = 0;
            cl->AcertaDeriv(mens);
            cl->AcertaVar();
            if (Instr::ExecIni(cl, "iniclasse"))
            {
                delete Inicio;
                Instr::ExecArg(cl->Nome);
                Instr::ExecX();
                return true;
            }
        }
        delete Inicio;
    }
    return false;
}

//----------------------------------------------------------------------------
TMudarClasse * TMudarClasse::Procurar(const char * nome)
{
    TMudarClasse * y = RBroot;
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
int TMudarClasse::RBcomp(TMudarClasse * x, TMudarClasse * y)
{
    return comparaZ(x->Nome, y->Nome);
}

//----------------------------------------------------------------------------
TMudarClasse * TMudarClasse::RBroot=0;
#define CLASS TMudarClasse          // Nome da classe
#define RBmask 1 // Máscara para bit 0
#include "rbt.cpp.h"
