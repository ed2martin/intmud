/* Este programa � software livre; voc� pode redistribuir e/ou
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
#include "instr.h"
#include "classe.h"
#include "misc.h"

using namespace Instr;

//------------------------------------------------------------------------------
// Mostra como uma instru��o est� codificada
// Retorna: true = conseguiu decodificar com sucesso
//          false = erro, destino cont�m a mensagem de erro
bool Instr::Mostra(char * destino, const char * origem, int tamanho)
{
    int  expr=0;    // �ndice da express�o num�rica, 0=n�o h�
    char nome[40]; // Nome da instru��o
    *nome=0;

    switch (origem[2])
    {
    case cHerda:         // 1 byte = n�mero de classes
        {
            const char * p = origem+4;
            for (int total=(unsigned char)origem[3]; total; total--)
            {
                while (*p)
                    p++;
                p++;
            }
            if (p-origem+10 > tamanho)
            {
                copiastr(destino, "Espa�o insuficiente", tamanho);
                return false;
            }
            destino = copiastr(destino, "herda ");
            for (origem+=4; origem<p; origem++,destino++)
                *destino = (*origem==0 ? ',' : *origem);
            destino[-1]=0;
        }
        return true;
    case cExpr:  // Express�o num�rica pura
        strcpy(nome, "=");
        expr=3;
        break;
    case cComent: // Coment�rio
        if ((int)strlen(origem+3) + 3 > tamanho)
        {
            copiastr(destino, "Espa�o insuficiente", tamanho);
            return false;
        }
        sprintf(destino, "# %s", origem+3);
        return true;

// Constrole de fluxo
    case cSe:        strcpy(nome, "se"); expr=5; break;
    case cSenao1:    strcpy(nome, "sen�o"); break;
    case cSenao2:    strcpy(nome, "sen�o"); expr=5; break;
    case cFimSe:     strcpy(nome, "fimse"); break;
    case cEnquanto:  strcpy(nome, "enquanto"); expr=5; break;
    case cEFim:      strcpy(nome, "efim"); break;
    case cRet1:      strcpy(nome, "ret"); break;
    case cRet2:      strcpy(nome, "ret"); expr=3; break;
    case cSair:      strcpy(nome, "sair"); break;
    case cContinuar: strcpy(nome, "continuar"); break;
    case cTerminar:  strcpy(nome, "terminar"); break;

// Vari�veis
    case cVariaveis: break;
    case cTxt1:
        sprintf(nome, "txt%d", (unsigned char)origem[4]+1);
        break;
    case cTxt2:
        sprintf(nome, "txt%d", (unsigned char)origem[4]+257);
        break;
    case cIntb0:     strcpy(nome, "int1 (bit 0)"); break;
    case cIntb1:     strcpy(nome, "int1 (bit 1)"); break;
    case cIntb2:     strcpy(nome, "int1 (bit 2)"); break;
    case cIntb3:     strcpy(nome, "int1 (bit 3)"); break;
    case cIntb4:     strcpy(nome, "int1 (bit 4)"); break;
    case cIntb5:     strcpy(nome, "int1 (bit 5)"); break;
    case cIntb6:     strcpy(nome, "int1 (bit 6)"); break;
    case cIntb7:     strcpy(nome, "int1 (bit 7)"); break;
    case cInt8:      strcpy(nome, "int8"); break;
    case cUInt8:     strcpy(nome, "uint8"); break;
    case cInt16:     strcpy(nome, "int16"); break;
    case cUInt16:    strcpy(nome, "uint16"); break;
    case cInt32:     strcpy(nome, "int32"); break;
    case cUInt32:    strcpy(nome, "uint32"); break;
    case cIntInc:    strcpy(nome, "intinc"); break;
    case cIntDec:    strcpy(nome, "intdec"); break;
    case cReal:      strcpy(nome, "real"); break;
    case cRef:       strcpy(nome, "ref"); break;
    case cConstNulo: strcpy(nome, "const (nulo)"); break;
    case cConstTxt:  strcpy(nome, "const (txt)"); break;
    case cConstNum:  strcpy(nome, "const (num)"); break;
    case cConstExpr: strcpy(nome, "const"); break;
    case cFunc:      strcpy(nome, "func"); break;

// Vari�veis extras
    case cListaObj:  strcpy(nome, "listaobj"); break;
    case cListaTxt:  strcpy(nome, "listatxt"); break;
    case cListaMsg:  strcpy(nome, "listamsg"); break;
    case cLog:       strcpy(nome, "log"); break;
    case cVarTempo:  strcpy(nome, "vartempo"); break;
    case cSocket:    strcpy(nome, "socket"); break;
    case cServ:      strcpy(nome, "serv"); break;
    case cSalvar:    strcpy(nome, "salvar"); break;
    case cProg:      strcpy(nome, "prog"); break;
    }

// Nenhum nome - instru��o desconhecida
    if (*nome==0)
    {
        copiastr(destino, "Instru��o n�o existe", tamanho);
        return false;
    }

// Copia nome da instru��o ou vari�vel
    if (origem[2]>cVariaveis)
    {
        if ((int)strlen(nome) + (int)strlen(origem+5) + 2 > tamanho)
        {
            copiastr(destino, "Espa�o insuficiente", tamanho);
            return false;
        }
        sprintf(destino, "%s %s", nome, origem+5);
    }
    else
    {
        if ((int)strlen(nome) + 1 > tamanho)
        {
            copiastr(destino, "Espa�o insuficiente", tamanho);
            return false;
        }
        strcpy(destino, nome);
    }
    while (*destino)
       destino++, tamanho--;

// Constantes - obt�m o �ndice
    if (origem[2]==cConstNulo || origem[2]==cConstTxt ||
        origem[2]==cConstNum  || origem[2]==cConstExpr)
    {
        expr=5;
        while (origem[expr])
            expr++;
        expr++;
        if (tamanho<3)
        {
            copiastr(destino, "Espa�o insuficiente", tamanho);
            return false;
        }
        strcpy(destino, " =");
        destino += 2;
        tamanho -= 2;
    }

// Verifica se tem express�o num�rica
    if (expr==0)
        return true;

    while (true)
    {
        *nome=0;
        switch (*origem)
        {
        case ex_fim:
            *destino=0;
            return true;
        case ex_coment: // Coment�rio
            if (tamanho<3)
                return false;
            destino[0] = '#';
            destino++, tamanho--;
            while (*origem)
            {
                if (tamanho<2 || *(unsigned char*)origem < ' ')
                    return false;
                *destino++ = *origem, tamanho--;
            }
            return true;

        // Usado em textos
        case ex_barra_n: break;    // \n
        case ex_barra_b: break;    // \b
        case ex_barra_c: break;    // \c
        case ex_barra_d: break;    // \d

        // Nome de vari�vel ou fun��o (um texto)
        case ex_varini:      // In�cio do texto
        case ex_varabre:     // In�cio do texto + abre colchetes
        case ex_ponto:       // Fim do nome da vari�vel
            if (tamanho<20)
                return false;
            if (*origem==ex_varini)
                strcpy(destino, " (varini) ");
            else if (*origem==ex_ponto)
                strcpy(destino, ".");
            else
                strcpy(destino, " (varini)[");
            while (*destino)
                destino++, tamanho--;
            for (origem++; *(unsigned char*)origem>=' '; origem++)
            {
                if (tamanho<2)
                    return false;
                *destino++ = *origem++;
            }
            destino--;
            nome[0]=destino[0];
            nome[1]=0;
            break;
        case ex_varfim:      // Fim do texto
            strcpy(nome, " (varfim)"); break;
        case ex_arg:         // In�cio da lista de argumentos
            strcpy(nome, "(argumentos)"); break;
        case ex_abre:        strcpy(nome, "["); break;
        case ex_fecha:       strcpy(nome, " ]"); break;

        // Valores fixos
        case ex_nulo:        strcpy(nome, " nulo"); break;
        case ex_txt: // Texto em ASCIIZ
            *destino++ = ' ';
            strcpy(nome, "\""); break;
            while (true)
            {
                if (tamanho<3)
                {
                    *destino=0;
                    return false;
                }
                if (*origem==0)
                    break;
                switch (*origem)
                {
                case ex_txt:
                    *destino++ = '\"', tamanho--;
                    break;
                case ex_barra_n:
                    destino[0] = '\\';
                    destino[1] = 'n';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_b:
                    destino[0] = '\\';
                    destino[1] = 'b';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_c:
                    destino[0] = '\\';
                    destino[1] = 'c';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_d:
                    destino[0] = '\\';
                    destino[1] = 'd';
                    destino+=2, tamanho-=2;
                    break;
                case '\"':
                case '\\':
                    *destino++= '\"', tamanho--;
                default:
                    if (*(unsigned char*)origem >= ' ')
                        *destino++ = *origem, tamanho--;
                    else
                    {
                        *destino=0;
                        return false;
                    }
                }
                origem++;
            }
            break;
        case ex_num0:        strcpy(nome, " 0"); break;
        case ex_num1:        strcpy(nome, " 1"); break;
        case ex_num8p:
            sprintf(nome, "%u", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16p:
            sprintf(nome, "%u", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32p:
            sprintf(nome, "%u", Num32(origem+1));
            origem+=4;
            break;
        case ex_num8n:
            sprintf(nome, "-%u", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16n:
            sprintf(nome, "-%u", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32n:
            sprintf(nome, "-%u", Num32(origem+1));
            origem+=4;
            break;
        case ex_div1:        strcpy(nome, " /10"); break;
        case ex_div2:        strcpy(nome, " /100"); break;
        case ex_div3:        strcpy(nome, " /1K"); break;
        case ex_div4:        strcpy(nome, " /10K"); break;
        case ex_div5:        strcpy(nome, " /100K"); break;
        case ex_div6:        strcpy(nome, " /1M"); break;

        // Operadores num�ricos
        case exo_ini:        break; // Marca o in�cio dos operadores
        case exo_virgula:    strcpy(nome, " ,"); break;
        case exo_neg:        strcpy(nome, " -(unit�rio)"); break;
        case exo_exclamacao: strcpy(nome, " !"); break;
        case exo_mul:        strcpy(nome, " *"); break;
        case exo_div:        strcpy(nome, " /"); break;
        case exo_porcent:    strcpy(nome, " %"); break;
        case exo_add:        strcpy(nome, " +"); break;
        case exo_sub:        strcpy(nome, " -"); break;
        case exo_menor:      strcpy(nome, " <"); break;
        case exo_menorigual: strcpy(nome, " <="); break;
        case exo_maior:      strcpy(nome, " >"); break;
        case exo_maiorigual: strcpy(nome, " >="); break;
        case exo_igual:      strcpy(nome, " ="); break;
        case exo_diferente:  strcpy(nome, " !="); break;
        case exo_e:          strcpy(nome, " &"); break;
        case exo_ou:         strcpy(nome, " |"); break;
        case exo_fim:        break; // Marca o fim dos operadores
        case exo_ee:         strcpy(nome, " &(in�cio)"); break;
        case exo_ouou:       strcpy(nome, " |(in�cio)"); break;
        }
        origem++;
        if (*nome==0 || (int)strlen(nome)+4 > tamanho)
        {
            *destino=0;
            return false;
        }
        strcpy(destino, nome);
        while (destino)
            destino++, tamanho--;
    }
}
