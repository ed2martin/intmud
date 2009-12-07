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

//#define DEBUG

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Inicio=0;
TArqMapa * TArqMapa::Fim=0;
bool TArqMapa::MapaGrande=false;
unsigned char TArqMapa::ParamIndent=2;
unsigned char TArqMapa::ParamClasse=1;
unsigned char TArqMapa::ParamFunc=1;
unsigned char TArqMapa::ParamVar=0;

//------------------------------------------------------------------------------
TArqMapa::TArqMapa(const char * arqnome)
{
    copiastr(Arquivo, arqnome, sizeof(Arquivo));
    Anterior=Fim, Proximo=0;
    (Fim ? Fim->Proximo : Inicio)=this;
    Fim=this;
    Mudou=false;
    ClInicio=0, ClFim=0;
    Existe=false;
#ifdef DEBUG
    printf("TArqMapa( %s )\n", Arquivo); fflush(stdout);
#endif
}

//------------------------------------------------------------------------------
TArqMapa::~TArqMapa()
{
#ifdef DEBUG
    printf("~TArqMapa( %s )\n", Arquivo); fflush(stdout);
#endif
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
    if (o-nome >= INT_NOME_TAM)
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
    TArqMapa * arqmapa = Fim;
    char char_lf[100];
    memset(char_lf, '\n', sizeof(char_lf));
    //if (Inicio->Mudou) puts("-------------"); fflush(stdout);
    while (arqmapa)
    {
    // Checa instrução mapagrande
        if (*arqmapa->Arquivo==0 && (Inicio!=Fim) != MapaGrande)
        {
            MapaGrande = (Inicio!=Fim);
            Inicio->Mudou=true;
        }
    // Checa se arquivo foi alterado
        if (!arqmapa->Mudou && !tudo)
        {
            arqmapa = arqmapa->Anterior;
            continue;
        }
    // Obtém o nome do arquivo
        if (*arqmapa->Arquivo)
            mprintf(arqext, 60, "-%s." INT_EXT, arqmapa->Arquivo);
        else
            strcpy(arqext, "." INT_EXT);
    // Verifica se arquivo vazio
        if (arqmapa->ClInicio==0 && *arqmapa->Arquivo)
        {
            if (arqmapa->Existe)
                remove(arqnome);
            TArqMapa * antes = arqmapa->Anterior;
            delete arqmapa;
            arqmapa = antes;
            continue;
        }
    // Cria arquivo
        FILE * arq = fopen("intmud-temp.txt", "w");
        if (arq==0)
        {
            arqmapa = arqmapa->Anterior;
            continue;
        }
        if (*arqmapa->Arquivo==0 && MapaGrande)
            fprintf(arq, "mapagrande\n\n");
        for (TClasse * cl = arqmapa->ClInicio; cl; cl=cl->ArqDepois)
        {
            int indent = 0;
            int linhas = 0;
        // Nome da classe
            fprintf(arq, "[%s]\n", cl->Nome);
            for (const char * p = cl->Comandos; p[0] || p[1]; p+=Num16(p))
            {
        // Obtém indentação (antes)
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
        // Anota instrução
                char mens[4096];
                int espaco = (indent>0 && p[2]!=Instr::cComent ?
                              indent-1 : 0) * ParamIndent;
                int r = Instr::Decod(mens+espaco, p, sizeof(mens)-espaco);
                assert(r!=0);
                if (espaco)
                    memset(mens, ' ', espaco);
                fprintf(arq, "%s\n", mens);
        // Obtém indentação (depois)
                switch (p[2])
                {
                case Instr::cFunc:
                case Instr::cVarFunc:
                    indent=2;
                    break;
                case Instr::cSe:
                case Instr::cEnquanto:
                    if (indent)
                        indent++;
                    break;
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
#ifdef DEBUG
        printf("TArqMapa::Salvar( %s )\n", arqnome); fflush(stdout);
#endif
        if (rename("intmud-temp.txt", arqnome))
        {
            arqmapa->Existe = true;
            arqmapa->Mudou = false;
        }
        else
            remove("intmud-temp.txt");
        arqmapa = arqmapa->Anterior;
    }
}
