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
#include "arqmapa.h"
#include "instr.h"
#include "misc.h"

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Inicio=0;
TArqMapa * TArqMapa::Fim=0;
unsigned char TArqMapa::ParamIndent=2;
unsigned char TArqMapa::ParamClasse=1;
unsigned char TArqMapa::ParamFunc=1;
unsigned char TArqMapa::ParamVar=0;
bool TArqMapa::MapaGrande=false;

//------------------------------------------------------------------------------
TArqMapa::TArqMapa(const char * classe)
{
    char * d=Arquivo;
    while (d<Arquivo+sizeof(Arquivo)-1 && *classe!=' ' && *classe!=0)
        *d++ = *classe++;
    *d=0;
    Anterior=Fim, Proximo=0;
    (Fim ? Fim->Proximo : Inicio)=this;
    Fim=this;
    Mudou=false;
    ClInicio=0, ClFim=0;
    Existe=false;
}

//------------------------------------------------------------------------------
TArqMapa::~TArqMapa()
{
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    (Proximo ? Proximo->Anterior : Fim) = Anterior;
    assert(ClInicio==0);
}

//----------------------------------------------------------------------------
bool TArqMapa::NomeValido(const char * nome)
{
    const char *o=nome;
    for (; *o; o++)
        if ((*o<'0' || *o>'9') && (*o<'a' || *o>'z') && *o!=' ')
            return false;
    if (o-nome >= MAPA_NOME_TAM)
        return false;
    return true;
}

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Procura(const char * nome)
{
    for (TArqMapa * arqmapa = Inicio; arqmapa; arqmapa = arqmapa->Proximo)
        if (strcmp(nome, arqmapa->Arquivo)==0)
            return arqmapa;
    return 0;
}

//------------------------------------------------------------------------------
void TArqMapa::SalvarArq(bool tudo)
{
    TArqMapa * arqmapa = Inicio;
    char char_lf[100];
    memset(char_lf, '\n', sizeof(char_lf));
    while (true)
    {
    // Checa se arquivo foi alterado
        if (!arqmapa->Mudou && !tudo)
        {
            arqmapa = arqmapa->Proximo;
            continue;
        }
    // Obtém o nome do arquivo
        if (*arqmapa->Arquivo)
            mprintf(arqext, 60, "-%s.map", arqmapa->Arquivo);
        else
            strcpy(arqext, ".map");
    // Verifica se arquivo vazio
        if (arqmapa->ClInicio==0)
        {
            if (arqmapa->Existe)
                remove(arqnome);
            TArqMapa * prox = arqmapa->Proximo;
            delete arqmapa;
            arqmapa = prox;
            continue;
        }
    // Cria arquivo
        FILE * arq = fopen("intmud-temp.txt", "w");
        if (arq==0)
        {
            arqmapa = arqmapa->Proximo;
            continue;
        }
        if (*arqmapa->Arquivo==0 && TArqMapa::Inicio)
            if (TArqMapa::Inicio->Proximo)
                fprintf(arq, "mapagrande\n\n");
        for (TClasse * cl = arqmapa->ClInicio; cl; cl=cl->ArqDepois)
        {
            int indent = 0;
            int linhas = 0;
        // Nome da classe
            fprintf(arq, "[%s]\n", cl->Nome);
            for (const char * p = cl->Comandos; p[0] || p[1]; )
            {
        // Anota instrução
                char mens[4096];
                int espaco = (indent>0 ? indent-1 : 0) * ParamIndent;
                int r = Instr::Decod(mens+espaco, p, sizeof(mens)-espaco);
                assert(r!=0);
                if (espaco)
                    memset(mens, ' ', espaco);
                fprintf(arq, "%s\n", mens);
        // Avança instrução e obtém indentação
                switch (p[2])
                {
                case Instr::cFunc:
                case Instr::cVarFunc:
                case Instr::cSe:
                case Instr::cEnquanto:
                    if (indent)
                        indent++;
                    break;
                }
                p+=Num16(p);
                if (p[0]==0 && p[1]==0)
                    break;
                int tipo=-1;
                switch (p[2])
                {
                case Instr::cConstNulo:
                case Instr::cConstTxt:
                case Instr::cConstNum:
                case Instr::cConstExpr:
                    if (indent>1)
                        indent=1;
                    tipo=ParamVar;
                    break;
                case Instr::cFunc:
                case Instr::cVarFunc:
                    indent=1;
                    tipo=ParamFunc;
                    break;
                case Instr::cFimSe:
                case Instr::cEFim:
                    if (indent>1)
                        indent--;
                    break;
                default:
                    if (indent==0 && p[2] >= Instr::cVariaveis)
                        tipo=ParamVar;
                }
        // Linhas entre funções e entre variáveis
                if (tipo>=0)
                {
                    if (linhas<tipo)
                        linhas=tipo;
                    if (linhas>0)
                        fwrite(char_lf, 1, linhas, arq);
                    linhas=tipo;
                }
            }
        // Linhas entre classes
            if (cl->ArqDepois && ParamClasse)
                fwrite(char_lf, 1, ParamClasse, arq);
        }
        fclose(arq);
    // Renomeia arquivo
#ifdef __WIN32__
        remove(arqnome);
#endif
        if (!rename(arqnome, "intmud-temp.txt"))
        {
            remove("intmud-temp.txt");
            arqmapa = arqmapa->Proximo;
            continue;
        }
        arqmapa->Existe = true;
        arqmapa->Mudou = false;
        arqmapa = arqmapa->Proximo;
    }
}
