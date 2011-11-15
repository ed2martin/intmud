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
#include "random.h"
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
void TTextoTxt::Ordena(int modo, const char *txt1, const char * txt2)
{
// Checa se tem algo para ordenar
    if (Linhas==0)
        return;
    int totalbytes = Bytes + 1;
    if (modo >= 1)
        totalbytes += Linhas*4;
// Até 8191 bytes: ordena sem alocar memória com new (mais rápido)
    if (totalbytes <= 8192 && Linhas<=512)
    {
        char txt[8192];
        char *lin[512*4];
        OrdenaSub(modo, txt, lin, txt1, txt2);
        return;
    }
// Mais de 8191 bytes: aloca memória com new
    char * txt = new char[totalbytes];
    char ** lin = new char*[Linhas*2];
    OrdenaSub(modo, txt, lin, txt1, txt2);
    delete[] lin;
    delete[] txt;
}

//----------------------------------------------------------------------------
void TTextoTxt::OrdenaSub(int modo, char * texto, char** linha,
        const char *txt1, const char * txt2)
{
// Lê as linhas de textotxt
    TTextoBloco * obj = Inicio;
    unsigned int indobj=0;
    unsigned int numlin = 0;
    unsigned int totallin = Linhas;
    char * dest = texto;
    while (numlin < totallin)
    {
    // Se modo>=1: conta quantos objetos e anota cabeçalho
        if (modo>=1)
        {
        // Obtém quantidade de objetos
            unsigned int valor=0;
            if (obj->Texto[indobj] < '0' || obj->Texto[indobj] > '9')
            {
                while (obj->Texto[indobj++] != Instr::ex_barra_n);
                totallin--;
                continue;
            }
            while (true)
            {
                char ch = obj->Texto[indobj];
                if (ch<'0' || ch>'9')
                    break;
                valor = valor*10+ch-'0';
                indobj++;
                if (indobj >= obj->Bytes)
                    indobj=0, obj=obj->Depois;
            }
        // Checa se é número seguido de espaço
            if (obj->Texto[indobj] != ' ')
            {
                while (obj->Texto[indobj++] != Instr::ex_barra_n);
                totallin--;
                continue;
            }
        // Avança espaços vazios
            while (true)
            {
                indobj++;
                if (indobj >= obj->Bytes)
                    indobj=0, obj=obj->Depois;
                if (obj->Texto[indobj] != ' ')
                    break;
            }
        // Anota cabeçalho
            dest[0] = valor;
            dest[1] = valor >> 8;
            dest[2] = valor >> 16;
            dest[3] = valor >> 24;
            dest += 4;
        }
    // Anota endereço do texto
        linha[numlin++] = dest;
    // Anota texto
        while (true)
        {
            char ch = obj->Texto[indobj++];
            if (indobj >= obj->Bytes)
                indobj=0, obj=obj->Depois;
            if (ch == Instr::ex_barra_n)
                break;
            *dest++ = ch;
        }
    // Próxima linha
        *dest++ = 0;
    }

// Verifica se sobrou alguma linha
    if (numlin==0)
    {
        Limpar();
        return;
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
    if (modo <= 0)
    {
        TextoIni();
        for (unsigned int x=0; x<numlin; x++)
            TextoAnota(var1[x], strlen(var1[x])+1);
        TextoFim();
        return;
    }

// Ordenação somando as linhas
    if (modo <= 2)
    {
        TextoIni();
        for (unsigned int x=0; x<numlin; )
        {
            unsigned int soma=0;
            unsigned int y = x;
            char * txtcomp = var1[x];
            do
            {
                soma += Num32(var1[y]-4);
                y++;
            } while (y<numlin && comparaZ(txtcomp, var1[y])==0);
            x = y;
            if (modo==1)
            {
                char mens[80];
                mprintf(mens, sizeof(mens), "%u ", soma);
                TextoAnota(mens, strlen(mens));
            }
            else if (soma==0)
                continue;
            else if (soma>1)
            {
                char mens[512];
                if (soma >= 0xFF000000)
                    soma = 0xFEFFFFFF;
                mprintf(mens, sizeof(mens), "%s%u%s ", txt1, soma, txt2);
                TextoAnota(mens, strlen(mens));
            }
            TextoAnota(txtcomp, strlen(txtcomp)+1);
        }
        TextoFim();
        return;
    }

// Obtém as quantidades
    for (unsigned int x=0; x<numlin; )
    {
        unsigned int soma=0;
        unsigned int y = x;
        do
        {
            soma += Num32(var1[y]-4);
            y++;
        } while (y<numlin && comparaZ(var1[x], var1[y])==0);
        if (soma >= 0xFF000000)
            soma = 0xFEFFFFFF;
        var1[x][-4] = soma;
        var1[x][-3] = soma >> 8;
        var1[x][-2] = soma >> 16;
        var1[x][-1] = soma >> 24;
        for (x++; x<y; x++)
            var1[x][-1] = 0xFF;
    }

// Resultado na ordem original
    TextoIni();
    char * txt = texto;
    for (unsigned int x=0; x<numlin; x++)
    {
        char * txtlin = txt;
        txt += 4;
        while (*txt)
            txt++;
        txt++;
        if ((unsigned char)txtlin[3] == 0xFF)
            continue;
        unsigned int soma = Num32(txtlin);
        if (modo==3)
        {
            char mens[80];
            mprintf(mens, sizeof(mens), "%u ", soma);
            TextoAnota(mens, strlen(mens));
        }
        else if (soma==0)
            continue;
        else if (soma>1)
        {
            char mens[512];
            mprintf(mens, sizeof(mens), "%s%u%s ", txt1, soma, txt2);
            TextoAnota(mens, strlen(mens));
        }
        TextoAnota(txtlin+4, txt-txtlin-4);
    }
    TextoFim();
}

//----------------------------------------------------------------------------
void TTextoTxt::DivideLin(unsigned int min, unsigned int max, bool cores)
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
    char charcor = 0;   // Para lidar com definições de cores

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
        switch (ch)
        {
        case Instr::ex_barra_n:
            charcor=0;
            col=0, dest_obj->Linhas++;
            break;
        case Instr::ex_barra_b:
            if (cores)
            {
                charcor=0;
                break;
            }
        case Instr::ex_barra_c:
        case Instr::ex_barra_d:
            if (cores)
            {
                charcor=ch;
                break;
            }
        default:
            if (charcor)
            {
                if (ch>='0' && ch<='7')
                {
                    charcor=0;
                    break;
                }
                if (charcor==Instr::ex_barra_d && (
                        (ch>='0' && ch<='9') ||
                        (ch>='A' && ch<='F') ||
                        (ch>='a' && ch<='f')))
                {
                    charcor=0;
                    break;
                }
                charcor=0;
            }
        // Checa se atingiu o tamanho da linha
            if (++col < min)
                break;
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

//----------------------------------------------------------------------------
void TTextoTxt::Rand()
{
// Checa se tem mais de uma linha
    if (Linhas<=1)
        return;
    int totalbytes = Bytes + 1;
// Até 8191 bytes: ordena sem alocar memória com new (mais rápido)
    if (totalbytes <= 8192 && Linhas<=512)
    {
        char txt[8192];
        char *lin[512*4];
        RandSub(txt, lin);
        return;
    }
// Mais de 8191 bytes: aloca memória com new
    char * txt = new char[totalbytes];
    char ** lin = new char*[Linhas*2];
    RandSub(txt, lin);
    delete[] lin;
    delete[] txt;
}

//----------------------------------------------------------------------------
void TTextoTxt::RandSub(char * texto, char** linha)
{
// Lê texto de textotxt
    char * p = texto;
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
    {
        memcpy(p, obj->Texto, obj->Bytes);
        p += obj->Bytes;
    }
    p[-1] = 0;
    assert((unsigned int)(p-texto) == Bytes);

// Divide o texto em linhas
    unsigned int numlin = 1;
    linha[0] = texto;
    for (p=texto; *p; p++)
        if (*p==Instr::ex_barra_n)
            linha[numlin++] = p+1, *p=0;
    assert(numlin == Linhas);

    //for (unsigned int x=0; x<numlin; x++) puts(linha[x]);
    //fflush(stdout);

// Altera textotxt
    TextoIni();
    for (unsigned int x=0; x<numlin; x++)
    {
        unsigned int novalin = circle_random() % (numlin-x) + x;
        TextoAnota(linha[novalin], strlen(linha[novalin])+1);
        linha[novalin] = linha[x];
    }
    TextoFim();
}

//----------------------------------------------------------------------------
void TTextoTxt::TxtRemove(int opcoes)
{
// Checa se tem alguma linha
    if (Linhas<1)
        return;
    int totalbytes = Bytes + 1;
// Até 8191 bytes: ordena sem alocar memória com new (mais rápido)
    if (totalbytes <= 8192)
    {
        char txt[8192];
        TxtRemoveSub(txt, opcoes);
        return;
    }
// Mais de 8191 bytes: aloca memória com new
    char * txt = new char[totalbytes];
    TxtRemoveSub(txt, opcoes);
    delete[] txt;
}

//----------------------------------------------------------------------------
void TTextoTxt::TxtRemoveSub(char * texto, int opcoes)
{
// Lê texto de textotxt
    char * p = texto;
    for (TTextoBloco * obj = Inicio; obj; obj=obj->Depois)
    {
        memcpy(p, obj->Texto, obj->Bytes);
        p += obj->Bytes;
    }
    p[-1] = 0;
    assert((unsigned int)(p-texto) == Bytes);

// Converte
    char * o = texto, * d = texto;
    while (*o)
    {
        char * p = o;
        while (*p && *p!=Instr::ex_barra_n)
            p++;
        char ch = *p;
        *p++ = 0; // Zero no fim da linha
        d = txtRemove(d, o, p-o, opcoes) + 1; // Converte e anota
        o = p; // Avança origem para próxima linha
        if (ch==0) // Checa se é fim do texto
            break;
    }

// Altera textotxt
    TextoIni();
    TextoAnota(texto, d-texto);
    TextoFim();
}
