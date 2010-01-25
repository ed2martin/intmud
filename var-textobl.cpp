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

//----------------------------------------------------------------------------
void TTextoTxt::DivideLin(unsigned int min, unsigned int max)
{
// Verifica se tem como truncar
    if (min>max) min=max;
    if (min<2)   return;

// Verifica se texto está vazio
    if (Inicio==0)
        return;

// Cria bloco no início
    Inicio->CriarAntes();

// Procura menor posição
    unsigned int bytepos = Bytes;
    for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
        if (bytepos > pos->PosicTxt)
            bytepos = pos->PosicTxt;

// Outras variáveis
    unsigned int col = 0; // Coluna atual

    char * espaco_p = 0; // Aonde encontrou espaço para dividir a linha
    TTextoBloco * espaco_obj = 0; // Objeto correspondente
    unsigned int espaco_byte = 0;  // Número do byte a partir do início do texto

    TTextoBloco * dest_obj = Inicio; // Objeto que está escrevendo
    char * dest_p = dest_obj->Texto; // Posição atual em dest_obj
    unsigned int dest_byte = 0; // Byte atual em textotxt

    TTextoBloco * ler_obj = Inicio->Depois;  // Objeto que está lendo
    unsigned int ler_tam = ler_obj->Bytes;
    unsigned int ler_ind = 0;

// Divide as linhas
    while (true)
    {
    // Lê próximo byte de textotxt
        if (ler_ind >= ler_tam)
        {
            ler_obj = ler_obj->Depois;
            if (ler_obj==0)
                break;
            ler_tam = ler_obj->Bytes;
            ler_ind = 0;
        }
        char ch = ler_obj->Texto[ler_ind++];
    // Nova linha
        //putchar(ch==Instr::ex_barra_n ? '\n' : ch); fflush(stdout);
        if (ch==Instr::ex_barra_n)
            col=0, dest_obj->Linhas++;
    // Checa se atingiu o tamanho da linha
        else if (++col >= min)
        {
        // Antes da coluna máxima: procura último espaço
            if (col < max)
            {
                if (ch==' ')
                {
                    espaco_p = dest_p;
                    espaco_byte = dest_byte;
                    espaco_obj = dest_obj;
                }
            }
        // Encontrou espaço: substitui por ex_barra_n
            else if (espaco_p)
            {
                col = dest_byte - espaco_byte - 1;
                *espaco_p = Instr::ex_barra_n;
                espaco_p = 0;
                for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
                    if (pos->PosicTxt > espaco_byte)
                        pos->LinhaTxt++;
                espaco_obj->Linhas++;
                Linhas++;
            }
        // Não encontrou espaço
            else
            {
        // Anota ex_barra_n
                col=0, dest_obj->Linhas++;
                *dest_p++ = Instr::ex_barra_n;
                if (dest_p >= (char*)dest_obj + TOTAL_TXTOBJ)
                {
                    dest_obj->Bytes = dest_p - dest_obj->Texto;
                    if (dest_obj->Depois == ler_obj)
                        dest_obj = dest_obj->CriarDepois();
                    else
                        dest_obj = dest_obj->Depois, dest_obj->Linhas = 0;
                    dest_p = dest_obj->Texto;
                }
        // Adiciona 1 linha e 1 byte
                dest_byte++, bytepos++;
                for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
                    if (pos->PosicTxt >= dest_byte)
                        pos->PosicTxt++, pos->LinhaTxt++;
                Linhas++, Bytes++;
            }
        }
    // Acerta posição de textopos
        if (dest_byte == bytepos)
        {
            bytepos = Bytes;
            for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
            {
                if (pos->PosicTxt == dest_byte)
                {
                    pos->Bloco = dest_obj;
                    pos->PosicBloco = dest_p - dest_obj->Texto;
                }
                if (pos->PosicTxt < bytepos && pos->PosicTxt > dest_byte)
                    bytepos = pos->PosicTxt;
            }
        }
    // Anota caracter
        dest_byte++;
        *dest_p++ = ch;
        if (dest_p >= (char*)dest_obj + TOTAL_TXTOBJ)
        {
            dest_obj->Bytes = dest_p - dest_obj->Texto;
            if (dest_obj->Depois == ler_obj)
                dest_obj = dest_obj->CriarDepois();
            else
                dest_obj = dest_obj->Depois, dest_obj->Linhas = 0;
            dest_p = dest_obj->Texto;
        }
    }

// Acerta tamanho do último bloco
    dest_obj->Bytes = dest_p - dest_obj->Texto;

// Acerta textopos no final de textotxt
    for (TTextoPos * pos = Posic; pos; pos=pos->Depois)
        if (pos->PosicTxt >= Bytes)
        {
            pos->Bloco = dest_obj;
            pos->PosicBloco = dest_obj->Bytes;
            pos->PosicTxt = Bytes;
            pos->LinhaTxt = Linhas;
        }

// Apaga blocos após o último
    while (dest_obj->Depois)
        dest_obj = dest_obj->Depois, dest_obj->Bytes = dest_obj->Linhas = 0;
    while (Fim->Bytes==0)
        Fim->Apagar();
}
