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
#include "var-texto.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//----------------------------------------------------------------------------
void TTextoTxt::TextoIni()
{
    IniBloco();
    Linhas = Bytes = 0;
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
        obj->Bytes = obj->Linhas = 0;
    Fim = Inicio;
}

//----------------------------------------------------------------------------
void TTextoTxt::TextoAnota(const char * txt, int total)
{
    if (total<=0)
        return;
    int bytes_por_bloco = (char*)Fim + TOTAL_TXTOBJ - Fim->Texto;
    int lin;
    Bytes += total;
// Cria bloco se o bloco atual está cheio
    if (Fim->Bytes >= bytes_por_bloco)
    {
        if (Fim->Depois)
            Fim = Fim->Depois;
        else
            Fim = Fim->CriarDepois();
    }
// Anota texto em blocos inteiros
    while (true)
    {
        int bl = bytes_por_bloco - Fim->Bytes; // Num. de bytes disponíveis
        if (bl >= total)
            break;
        lin = TextoAnotaLin(Fim->Texto + Fim->Bytes, txt, bl);
        Linhas += lin;
        Fim->Linhas += lin;
        Fim->Bytes += bl;
        txt+=bl, total-=bl;
        if (Fim->Depois)
            Fim = Fim->Depois;
        else
            Fim = Fim->CriarDepois();
    }
// Anota texto no último bloco
    lin = TextoAnotaLin(Fim->Texto + Fim->Bytes, txt, total);
    Linhas += lin;
    Fim->Linhas += lin;
    Fim->Bytes += total;
}

//----------------------------------------------------------------------------
void TTextoTxt::TextoFim()
{
// Acerta objetos TTextoPos
    for (TTextoPos * obj = Posic; obj; obj=obj->Depois)
    {
        obj->Bloco = Inicio;
        obj->PosicBloco = 0;
        obj->PosicTxt = 0;
        obj->LinhaTxt = 0;
    }
// Avança até o último bloco
    while (Fim->Depois)
        Fim = Fim->Depois;
// Apaga blocos vazios
    while (Fim && Fim->Bytes==0)
        Fim->Apagar();
}

//----------------------------------------------------------------------------
void TTextoTxt::Ordena(const char *txt1, const char * txt2)
{
// Checa se tem algo para ordenar
    if (Linhas==0)
        return;
// Até 8191 bytes: ordena sem alocar memória com new (mais rápido)
    if (Bytes <= 8191 && Linhas<=512)
    {
        char txt[8192];
        char *lin[512*2];
        Inicio->CopiarTxt(0, txt+1, Bytes);
        *txt=0;
        OrdenaSub(txt, lin, txt1, txt2);
        return;
    }
// Mais de 8191 bytes: aloca memória com new
    char * txt = new char[Bytes+1];
    char ** lin = new char*[Linhas*2];
    Inicio->CopiarTxt(0, txt+1, Bytes);
    *txt=0;
    OrdenaSub(txt, lin, txt1, txt2);
    delete[] lin;
    delete[] txt;
}

//----------------------------------------------------------------------------
void TTextoTxt::OrdenaSub(char * texto, char** linha,
        const char *txt1, const char * txt2)
{
// Separa as linhas
    unsigned int numlin = 1;
    linha[0] = texto+1;
    for (char * p = texto+1; *p; p++)
        if (*p == Instr::ex_barra_n)
            *p=0, linha[numlin++] = p+1;
    assert(numlin == Linhas);

// Avança número no início da linha
    if (txt1)
    {
        unsigned int y = 0;
        for (unsigned int x=0; x<numlin; x++)
        {
            char * p = linha[x];
            if (*p<'0' || *p>'9')
                continue;
            for (p++; *p>='0' && *p<='9'; p++);
            if (*p!=' ')
                continue;
            for (p++; *p==' '; p++);
            linha[y++] = p;
        }
        if (y==0)
        {
            Limpar();
            return;
        }
        numlin = y;
    }

#if 0
    for (unsigned int x=0; x<numlin; x++)
         printf(">%s\n", linha[x]);
    fflush(stdout);
#endif

// Ordena; resultado em var1
    char ** var1 = linha;
    char ** var2 = linha + numlin;
    for (unsigned int a=1; a<numlin; a+=a)
    {
        char ** pont = var2;
        var2 = var1;
        var1 = pont;
        int lido=0;
        for (unsigned int b=0; b<numlin; )
        {
            unsigned int b1=b, b2=b+a;
            b = b2;
            while (b1<b && b2<b+a && b2<numlin)
            {
                if (comparaZ(var2[b1], var2[b2]) > 0)
                    var1[lido++] = var2[b2++];
                else
                    var1[lido++] = var2[b1++];
            }
            while (b1<b && b1<numlin)
                var1[lido++] = var2[b1++];
            b += a;
            while (b2<b && b2<numlin)
                var1[lido++] = var2[b2++];
        }
    }

// Apenas ordenação
    if (txt1==0)
    {
        TextoIni();
        for (unsigned int x=0; x<numlin; x++)
            TextoAnota(var1[x], strlen(var1[x])+1);
        TextoFim();
        return;
    }

// Ordenação somando as linhas
    TextoIni();
    for (unsigned int x=0; x<numlin; )
    {
        unsigned int soma=0;
        unsigned int y = x;
        do
        {
            char * p = var1[y];
            while (*p)
                p--;
            soma += atoi(p+1);
            y++;
        } while (y<numlin && comparaZ(var1[x], var1[y])==0);
        x = y;
        if (txt2==0)
        {
            char mens[80];
            mprintf(mens, sizeof(mens), "%d ", soma);
            TextoAnota(mens, strlen(mens));
        }
        else if (soma==0)
            continue;
        else if (soma>1)
        {
            char mens[512];
            mprintf(mens, sizeof(mens), "%s%d%s ", txt1, soma, txt2);
            TextoAnota(mens, strlen(mens));
        }
        TextoAnota(var1[x-1], strlen(var1[x-1])+1);
    }
    TextoFim();
}
