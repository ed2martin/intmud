/* Este arquivo é software livre; você pode redistribuir e/ou
 * modificar nos termos das licenças GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "variavel.h"
#include "instr.h"
#include "instr-enum.h"
#include "classe.h"
#include "objeto.h"
#include "var-basico.h"
#include "procurar.h"
#include "misc.h"

//------------------------------------------------------------------------------
static bool VarBaseTxt_FuncVetor(TVariavel * v, const char * nome)
{
// Obtém número de variáveis e tamanho de uma variável
    int numvar = (unsigned char)v->defvar[Instr::endVetor];
    int tamvar = (unsigned char)v->defvar[Instr::endExtra] + 2;
    if (v->defvar[2] == Instr::cTxt2)
        tamvar += 256;
// Texto do vetor
    if (comparaZ(nome, "texto") == 0)
    {
    // Obtém índice inicial e a quantidade de variáveis
        int ini = 0;
        int total = numvar;
        if (Instr::VarAtual >= v + 1)
        {
            ini = v[1].getInt();
            if (ini < 0)
                ini = 0;
            if (Instr::VarAtual >= v + 2)
            {
                total = v[2].getInt() + 1;
                if (total > numvar)
                    total = numvar;
            }
        }
        total -= ini;
    // Cria variável
        const char * origem = reinterpret_cast<char*>(v->endvar) + ini * tamvar;
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total > 0; total--, origem += tamvar)
        {
            //printf("texto = ["); fflush(stdout);
            //for (const char * x = origem; *x; x++)
            //    if (*(signed char*)x >= ' ')
            //        putchar(*x);
            //    else
            //        printf("(%02X)", *(unsigned char*)x);
            //printf("]\n"); fflush(stdout);
            for (const char * o = origem; *o && destino < Instr::DadosFim - 1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Divide em palavras
    if (comparaZ(nome, "palavras") == 0)
    {
    // Obtém número de palavras (parâmetro de entrada)
        int total = numvar;
        if (Instr::VarAtual < v + 1)
            return false;
        if (Instr::VarAtual >= v + 2)
        {
            total = v[2].getInt();
            if (total > numvar)
                total = numvar;
        }
    // Copia o texto obtendo o número de palavras
        int palavras = 0;
        const char * origem = v[1].getTxt();
        char * destino = reinterpret_cast<char*>(v->endvar);
        for (; total>1; total--)
        {
            while (*origem == ' ' || *origem == Instr::ex_barra_n)
                origem++;
            if (*origem == 0)
                break;
            char * d = destino;
            destino += tamvar, palavras++;
            for (; *origem && *origem!=' ' &&
                    *origem != Instr::ex_barra_n; origem++)
                if (d < destino - 1)
                    *d++ = *origem;
            *d = 0;
        }
    // Copia o último texto
        while (*origem==' ' || *origem == Instr::ex_barra_n)
            origem++;
        if (*origem)
        {
            char * d = destino;
            destino += tamvar, palavras++;
            while (*origem && d < destino - 1)
                *d++ = *origem++;
            while (*origem == ' ' || *origem == Instr::ex_barra_n)
                origem++;
            if (*origem == 0)
            {
                for (d--; d >= destino - tamvar; d--)
                    if (*d != ' ' && *d != Instr::ex_barra_n)
                        break;
                d++;
            }
            *d = 0;
        }
    // Limpa próximas variáveis do vetor
        for (; palavras < numvar; destino += tamvar, numvar--)
            *destino = 0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(palavras);
    }
// Divide em linhas
    if (comparaZ(nome, "linhas") == 0)
    {
    // Obtém número mínimo de colunas (parâmetro de entrada)
        int colunas = tamvar - 1;
        if (Instr::VarAtual < v + 1)
            return false;
        if (Instr::VarAtual >= v + 2)
        {
            colunas = v[2].getInt();
            if (colunas > tamvar - 1)
                colunas = tamvar - 1;
            if (colunas < 0)
                colunas = 0;
        }
    // Copia o texto
        const char * origem = v[1].getTxt();
        char * destino = reinterpret_cast<char*>(v->endvar);
        int linha = 0;  // Número de linhas copiadas
        colunas -= tamvar - 1; // Quantas colunas recuar do final do texto
        while (linha < numvar)
        {
            char * d = destino;
            linha++, destino += tamvar;
        // Copia linha
            while (*origem && *origem != Instr::ex_barra_n && d < destino)
                *d++ = *origem++;
        // Linha cabe na variável
            if (d < destino)
            {
                *d = 0;
            // Fim da linha
                if (*origem == Instr::ex_barra_n)
                {
                    origem++;
                    continue;
                }
            // Fim do texto
                break;
            }
        // Obtém aonde deve dividir a linha
            d--, origem--; // Recua um caracter
            int x;
            for (x = -1; x >= colunas; x--)
                if (origem[x] == ' ')
                    break;
            if (x < colunas)
                for (x = -1; x >= colunas; x--)
                    if (origem[x] == '.')
                        break;
        // Divide a linha
            if (x >= colunas)
            {
                x++;
                origem += x;
                d += x;
            }
            *d = 0;
        }
    // Limpa próximas variáveis do vetor
        for (; linha < numvar; destino += tamvar, numvar--)
            *destino = 0;
    // Retorna o número de palavras
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(linha);
    }
// Limpa as variáveis do vetor
    if (comparaZ(nome, "limpar") == 0)
    {
        char * destino = reinterpret_cast<char*>(v->endvar);
        const char * texto = "";
        if (Instr::VarAtual > v)
            texto = v[1].getTxt();
        for (; numvar > 0; numvar--, destino += tamvar)
            copiastr(destino, texto, tamvar);
        return false;
    }
// Junta texto
    if (comparaZ(nome, "juntar") == 0)
    {
    // Obtém o texto delimitador
        int total = numvar;
        char mens[4096];
        *mens = 0;
        if (Instr::VarAtual >= v + 1)
        {
            copiastr(mens, v[1].getTxt(), sizeof(mens));
            if (Instr::VarAtual >= v + 2)
            {
                total = v[2].getInt();
                if (total > numvar)
                    total = numvar;
            }
        }
    // Cria variável
        const char * origem = reinterpret_cast<char*>(v->endvar);
        Instr::ApagarVar(v);  // Não tem importância que v seja apagado aqui
        if (!Instr::CriarVar(Instr::InstrTxtFixo))
            return false;
        if (Instr::DadosTopo >= Instr::DadosFim)
            return false;
    // Anota o texto
        char * destino = Instr::DadosTopo;
        for (; total > 0; total--, origem += tamvar)
        {
            for (const char * o = origem; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
            if (total<=1)
                break;
            for (const char * o = mens; *o && destino < Instr::DadosFim-1; )
                *destino++ = *o++;
        }
        *destino++ = 0;
    // Acerta a variável
        Instr::VarAtual->endvar = Instr::DadosTopo;
        Instr::VarAtual->tamanho = destino - Instr::DadosTopo;
        Instr::DadosTopo = destino;
        return true;
    }
// Separa texto
    int valor = 10;
    if (comparaZ(nome, "separar") == 0)
        valor = 0;
    else if (comparaZ(nome, "separarmai") == 0)
        valor = 1;
    else if (comparaZ(nome, "separardif") == 0)
        valor = 2;
    if (valor != 10)
    {
        int indice = 0;
        int indmax = numvar;
        char * destino = reinterpret_cast<char*>(v->endvar);
        while (true)
        {
            if (Instr::VarAtual < v + 2)
                break;
            if (Instr::VarAtual >= v + 3)
            {
                indmax = v[3].getInt();
                if (indmax<1)
                    break;
                if (indmax > numvar)
                    indmax = numvar;
            }
        // Obtém delimitador
            TProcurar proc;
            const char * origem = v[2].getTxt();
            int tampadrao = strlen(origem);
            if (!proc.Padrao(origem, valor))
                break;
        // Obtém texto
            origem = v[1].getTxt();
            int tam = strlen(origem);
        // Anota texto
            while (true)
            {
                indice++;
                int posic = (indice >= indmax ? -1 : proc.Proc(origem, tam));
                if (posic < 0) // Não encontrou
                {
                    copiastr(destino, origem, tamvar);
                    destino += tamvar;
                    break;
                }
                int total = (posic < tamvar - 1 ? posic : tamvar - 1);
                memcpy(destino, origem, total);
                destino[total] = 0;
                origem += posic + tampadrao;
                tam -= posic + tampadrao;
                destino += tamvar;
            }
            break;
        }
        // Limpa próximas variáveis do vetor
        for (; indice < numvar; destino += tamvar, numvar--)
            *destino = 0;
        // Retorna o número de variáveis
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(indice);
    }
// TxtRemove
    if (comparaZ(nome, "txtremove") == 0)
    {
        if (Instr::VarAtual < v + 1)
            return false;
        int remove = txtRemove(v[1].getTxt()); // O que deve remover
        if (remove == 0)
            return false;
        char * destino = reinterpret_cast<char*>(v->endvar);
        for (; numvar > 0; numvar--, destino += tamvar)
            txtRemove(destino, destino, tamvar, remove);
        return false;
    }
    return false;
}

//const char Int1_Valor[] = { 8, 0, Instr::cInt1, 0, 0, 1, '=', '0', 0 };

//------------------------------------------------------------------------------
bool VarBaseInt1_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") == 0)
    {
        unsigned int total = (unsigned char)v->defvar[Instr::endVetor];
        unsigned char * p = reinterpret_cast<unsigned char*>(v->endvar);
        unsigned char bitmask = 1 << v->numbit;
    // Preencher com 1
        if (Instr::VarAtual > v && v[1].getBool())
        {
            if (bitmask > 1)
            {
                for (; total > 0 && bitmask; total--, bitmask <<= 1)
                    *p |= bitmask;
                p++;
            }
            while (total >= 8)
                *p++ = 0xFF, total -= 8;
            for (bitmask = 1; total > 0; total--, bitmask <<= 1)
                *p |= bitmask;
            return false;
        }
    // Preencher com 0
        if (bitmask > 1)
        {
            for (; total > 0 && bitmask; total--, bitmask <<= 1)
                *p &= ~bitmask;
            p++;
        }
        while (total >= 8)
            *p++ = 0, total -= 8;
        for (bitmask = 1; total > 0; total--, bitmask <<= 1)
            *p &= ~bitmask;
        return false;
    }
    if (comparaZ(nome, "bits") == 0)
    {
        Instr::ApagarVar(v + 1);
        v->indice = 0;
        v->numfunc = 1;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
static int GetVetorInt1(TVariavel * v)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = reinterpret_cast<unsigned char*>(v->endvar);
    int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (total > 32)
        total = 32;
// Primeiro byte
    if (total > 0 && bitnum)
    {
        valor = *p++ >> bitnum;
        bitnum = 8 - bitnum;
        switch (total)
        {
        case 1: valor &= 1; break;
        case 2: valor &= 3; break;
        case 3: valor &= 7; break;
        case 4: valor &= 15; break;
        case 5: valor &= 31; break;
        case 6: valor &= 63; break;
        case 7: valor &= 127; break;
        }
        total -= bitnum;
    }
// Demais bytes
    for (; total > 0 && bitnum < 32; total -= 8,bitnum += 8)
    {
        switch (total)
        {
        case 1: valor |= (*p & 1) << bitnum; break;
        case 2: valor |= (*p & 3) << bitnum; break;
        case 3: valor |= (*p & 7) << bitnum; break;
        case 4: valor |= (*p & 15) << bitnum; break;
        case 5: valor |= (*p & 31) << bitnum; break;
        case 6: valor |= (*p & 63) << bitnum; break;
        case 7: valor |= (*p & 127) << bitnum; break;
        default: valor |= *p++ << bitnum;
        }
    }
    return valor;
}

//------------------------------------------------------------------------------
static inline int GetValorInt1(TVariavel * v)
{
    if (v->numfunc)
        return GetVetorInt1(v);
    char * p = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
    {
        int ind2 = v->indice + v->numbit;
        return (bool)(p[ind2 / 8] & (1 << (ind2 & 7)));
    }
    return (bool)(p[0] & (1 << v->numbit));
}

//------------------------------------------------------------------------------
void SetVetorInt1(TVariavel * v, int valor)
{
    unsigned int bitnum = v->numbit;
    unsigned char * p = reinterpret_cast<unsigned char*>(v->endvar);
    int total = (unsigned char)v->defvar[Instr::endVetor];
    if (total > 32)
        total = 32;
// Altera as variáveis
    while (total > 0)
    {
        int bitmask = 255;
        switch (total)
        {
        case 1: bitmask = 1; break;
        case 2: bitmask = 3; break;
        case 3: bitmask = 7; break;
        case 4: bitmask = 15; break;
        case 5: bitmask = 31; break;
        case 6: bitmask = 63; break;
        case 7: bitmask = 127; break;
        }
    // Bits já estão alinhados
        if (bitnum == 0)
        {
            *p &= ~bitmask;
            *p |= (valor & bitmask);
            p++, total -= 8, valor >>= 8;
            continue;
        }
    // Bits não estão alinhados
        *p &= ~(bitmask << bitnum);
        *p |= (valor & bitmask) << bitnum;
        p++, total -= 8 - bitnum, valor >>= 8 - bitnum;
        bitnum = 0;
    }
}

//------------------------------------------------------------------------------
static bool VarBaseInt8_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    char * ender = reinterpret_cast<char*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < -0x80)
            valor = -0x80;
        else if (valor > 0x7F)
            valor = 0x7F;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
bool VarBaseUInt8_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    char * ender = reinterpret_cast<char*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < 0)
            valor = 0;
        else if (valor > 0xFF)
            valor = 0xFF;
    }
    memset(ender, valor, total);
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseInt16_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    short * ender = reinterpret_cast<short*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < -0x8000)
            valor = -0x8000;
        else if (valor > 0x7FFF)
            valor = 0x7FFF;
    }
    if (valor == 0)
        memset(ender, 0, 2 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseUInt16_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    unsigned short * ender = reinterpret_cast<unsigned short*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    int valor = 0;
    if (Instr::VarAtual > v)
    {
        valor = v[1].getInt();
        if (valor < 0)
            valor = 0;
        else if (valor > 0xFFFF)
            valor = 0xFFFF;
    }
    if (valor == 0)
        memset(ender, 0, 2 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseInt32_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    int * ender = reinterpret_cast<int*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const int valor = (Instr::VarAtual <= v ? 0 : v[1].getInt());
    if (valor == 0)
        memset(ender, 0, 4 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseUInt32_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    unsigned int * ender = reinterpret_cast<unsigned int*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    unsigned int valor = 0;
    if (Instr::VarAtual > v)
    {
        const double val_double = v[1].getDouble();
        if (val_double > 0xFFFFFFFFLL)
            valor = 0xFFFFFFFFLL;
        else if (val_double > 0)
            valor = (unsigned int)val_double;
    }
    if (valor == 0)
        memset(ender, 0, 4 * total);
    else
        for (int x = 0; x < total; x++)
            ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseReal_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar")!=0)
        return false;
    float * ender = reinterpret_cast<float*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const float valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x = 0; x < total; x++)
        ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static bool VarBaseReal2_FuncVetor(TVariavel * v, const char * nome)
{
    if (comparaZ(nome, "limpar") != 0)
        return false;
    double * ender = reinterpret_cast<double*>(v->endvar);
    const int total = (unsigned char)v->defvar[Instr::endVetor];
    const double valor = (Instr::VarAtual <= v ? 0 : v[1].getDouble());
    for (int x = 0; x < total; x++)
        ender[x] = valor;
    return false;
}

//------------------------------------------------------------------------------
static int VarBaseTxt1_Tamanho(const char * instr)
{
    return 2 + (unsigned char)instr[Instr::endExtra];
}
static int VarBaseTxt2_Tamanho(const char * instr)
{
    return 258 + (unsigned char)instr[Instr::endExtra];
}
static int VarBaseInt1_Tamanho(const char * instr) { return 1; }
static int VarBaseInt8_Tamanho(const char * instr) { return 1; }
static int VarBaseInt16_Tamanho(const char * instr) { return sizeof(short); }
static int VarBaseInt32_Tamanho(const char * instr) { return sizeof(int); }
static int VarBaseReal_Tamanho(const char * instr) { return sizeof(float); }
static int VarBaseReal2_Tamanho(const char * instr) { return sizeof(double); }

//------------------------------------------------------------------------------
static int VarBaseTxt1_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * (2 + (unsigned char)instr[Instr::endExtra]);
}
static int VarBaseTxt2_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * (258 + (unsigned char)instr[Instr::endExtra]);
}
static int VarBaseInt1_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total == 0 ? 1 : (total + 7) / 8);

}
static int VarBaseInt8_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1);

}
static int VarBaseInt16_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(short);

}
static int VarBaseInt32_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(int);
}
static int VarBaseReal_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(float);
}
static int VarBaseReal2_TamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(double);
}

//------------------------------------------------------------------------------
static void VarBaseTxt1_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    int tam = 2 + (unsigned char)v->defvar[Instr::endExtra];
    char * ref = reinterpret_cast<char*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes * tam] = 0;
}

static void VarBaseTxt2_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    int tam = 258 + (unsigned char)v->defvar[Instr::endExtra];
    char * ref = reinterpret_cast<char*>(v->endvar);
    for (; antes < depois; antes++)
        ref[antes * tam] = 0;
}
static void VarBaseInt1_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
// Se deve apagar variáveis: retorna
    if (antes >= depois)
        return;
// Obtém byte e bit inicial
    antes += v->numbit;
    depois += v->numbit;
    char * end = reinterpret_cast<char*>(v->endvar) + antes/8;
// Avança bit a bit até o fim do primeiro byte
    if (antes & 7)
    {
        for (; antes < depois && (antes & 7); antes++)
            *end &= ~(1 << (antes & 7));
        end++;
    }
// Preenche bytes = 0
    for (; antes + 8 <= depois; antes += 8)
        *end++ = 0;
// Avança bit a bit
    for (int mask = 1; antes < depois; antes++)
        *end &= ~mask, mask <<= 1;
}
static void VarBaseInt8_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
}
static void VarBaseInt16_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    short * ref = reinterpret_cast<short*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
}
static void VarBaseInt32_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    int * ref = reinterpret_cast<int*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
}
static void VarBaseReal_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    float * ref = reinterpret_cast<float*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
}
static void VarBaseReal2_Redim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    double * ref = reinterpret_cast<double*>(v->endvar);
    if (antes < depois)
        memset(ref + antes, 0, (depois - antes) * sizeof(ref[0]));
}

//------------------------------------------------------------------------------
static void VarBaseTxt1_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    if (total == 0)
        memmove(destino, v->endvar, strlen((char*)v->endvar) + 1);
    else
        memmove(destino, v->endvar, VarBaseTxt1_TamanhoVetor(v->defvar));
}
static void VarBaseTxt2_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    int total = (unsigned char)v->defvar[Instr::endVetor];
    if (total == 0)
        memmove(destino, v->endvar, strlen((char*)v->endvar) + 1);
    else
        memmove(destino, v->endvar, VarBaseTxt2_TamanhoVetor(v->defvar));
}
static void VarBaseInt1_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseInt1_TamanhoVetor(v->defvar));
}
static void VarBaseInt8_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseInt8_TamanhoVetor(v->defvar));
}
static void VarBaseInt16_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseInt16_TamanhoVetor(v->defvar));
}
static void VarBaseInt32_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseInt32_TamanhoVetor(v->defvar));
}
static void VarBaseReal_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseReal_TamanhoVetor(v->defvar));
}
static void VarBaseReal2_MoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    memmove(destino, v->endvar, VarBaseReal2_TamanhoVetor(v->defvar));
}

//------------------------------------------------------------------------------
static bool VarBaseTxt1_GetBool(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (2 + (unsigned char)v->defvar[Instr::endExtra]);
    return *ref != 0;
}
static bool VarBaseTxt2_GetBool(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (258 + (unsigned char)v->defvar[Instr::endExtra]);
    return *ref != 0;
}
static bool VarBaseInt1_GetBool(TVariavel * v)
{
    return GetValorInt1(v);
}
static bool VarBaseInt8_GetBool(TVariavel * v)
{
    return reinterpret_cast<signed char*>(v->endvar)[v->indice];
}
static bool VarBaseInt16_GetBool(TVariavel * v)
{
    return reinterpret_cast<signed short*>(v->endvar)[v->indice];
}
static bool VarBaseInt32_GetBool(TVariavel * v)
{
    return reinterpret_cast<signed int*>(v->endvar)[v->indice];
}
static bool VarBaseReal_GetBool(TVariavel * v)
{
    return reinterpret_cast<float*>(v->endvar)[v->indice] != 0;
}
static bool VarBaseReal2_GetBool(TVariavel * v)
{
    return reinterpret_cast<double*>(v->endvar)[v->indice] != 0;
}

//------------------------------------------------------------------------------
static int VarBaseTxt1_GetInt(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (2 + (unsigned char)v->defvar[Instr::endExtra]);
    return TxtToInt(ref);
}
static int VarBaseTxt2_GetInt(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (258 + (unsigned char)v->defvar[Instr::endExtra]);
    return TxtToInt(ref);
}
static int VarBaseInt1_GetInt(TVariavel * v)
{
    return GetValorInt1(v);
}
static int VarBaseInt8_GetInt(TVariavel * v)
{
    return reinterpret_cast<signed char*>(v->endvar)[v->indice];
}
static int VarBaseUInt8_GetInt(TVariavel * v)
{
    return reinterpret_cast<unsigned char*>(v->endvar)[v->indice];
}
static int VarBaseInt16_GetInt(TVariavel * v)
{
    return reinterpret_cast<signed short*>(v->endvar)[v->indice];
}
static int VarBaseUInt16_GetInt(TVariavel * v)
{
    return reinterpret_cast<unsigned short*>(v->endvar)[v->indice];
}
static int VarBaseInt32_GetInt(TVariavel * v)
{
    return reinterpret_cast<signed int*>(v->endvar)[v->indice];
}
static int VarBaseUInt32_GetInt(TVariavel * v)
{
    unsigned int valor = reinterpret_cast<unsigned int*>(v->endvar)[v->indice];
    return (valor <= 0x7FFFFFFF ? valor : 0x7FFFFFFF);
}
static int VarBaseReal_GetInt(TVariavel * v)
{
    return DoubleToInt(reinterpret_cast<float*>(v->endvar)[v->indice]);
}
static int VarBaseReal2_GetInt(TVariavel * v)
{
    return DoubleToInt(reinterpret_cast<double*>(v->endvar)[v->indice]);
}

//------------------------------------------------------------------------------
static double VarBaseTxt1_GetDouble(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (2 + (unsigned char)v->defvar[Instr::endExtra]);
    return TxtToDouble(ref);
}
static double VarBaseTxt2_GetDouble(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (258 + (unsigned char)v->defvar[Instr::endExtra]);
    return TxtToDouble(ref);
}
static double VarBaseInt1_GetDouble(TVariavel * v)
{
    return GetValorInt1(v);
}
static double VarBaseInt8_GetDouble(TVariavel * v)
{
    return reinterpret_cast<signed char*>(v->endvar)[v->indice];
}
static double VarBaseUInt8_GetDouble(TVariavel * v)
{
    return reinterpret_cast<unsigned char*>(v->endvar)[v->indice];
}
static double VarBaseInt16_GetDouble(TVariavel * v)
{
    return reinterpret_cast<signed short*>(v->endvar)[v->indice];
}
static double VarBaseUInt16_GetDouble(TVariavel * v)
{
    return reinterpret_cast<unsigned short*>(v->endvar)[v->indice];
}
static double VarBaseInt32_GetDouble(TVariavel * v)
{
    return reinterpret_cast<signed int*>(v->endvar)[v->indice];
}
static double VarBaseUInt32_GetDouble(TVariavel * v)
{
    return reinterpret_cast<unsigned int*>(v->endvar)[v->indice];
}
static double VarBaseReal_GetDouble(TVariavel * v)
{
    return reinterpret_cast<float*>(v->endvar)[v->indice];
}
static double VarBaseReal2_GetDouble(TVariavel * v)
{
    return reinterpret_cast<double*>(v->endvar)[v->indice];
}

//------------------------------------------------------------------------------
static const char * VarBaseTxt1_GetTxt(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (2 + (unsigned char)v->defvar[Instr::endExtra]);
    return ref;
}
static const char * VarBaseTxt2_GetTxt(TVariavel * v)
{
    char * ref = reinterpret_cast<char*>(v->endvar);
    if (v->indice)
        ref += v->indice * (258 + (unsigned char)v->defvar[Instr::endExtra]);
    return ref;
}
static const char * VarBaseInt1_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", GetValorInt1(v));
    return buf;
}
static const char * VarBaseInt8_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", reinterpret_cast<signed char*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseUInt8_GetTxt(TVariavel * v)
{
   char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", reinterpret_cast<unsigned char*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseInt16_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", reinterpret_cast<signed short*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseUInt16_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", reinterpret_cast<unsigned short*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseInt32_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%d", reinterpret_cast<signed int*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseUInt32_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    sprintf(buf, "%u", reinterpret_cast<unsigned int*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseReal_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    DoubleToTxt(buf, reinterpret_cast<float*>(v->endvar)[v->indice]);
    return buf;
}
static const char * VarBaseReal2_GetTxt(TVariavel * v)
{
    char * buf = TVarInfo::BufferTxt();
    DoubleToTxt(buf, reinterpret_cast<double*>(v->endvar)[v->indice]);
    return buf;
}

//------------------------------------------------------------------------------
const TVarInfo * VarBaseTxt1()
{
    static const TVarInfo var(
        VarBaseTxt1_Tamanho,
        VarBaseTxt1_TamanhoVetor,
        TVarInfo::FTipoTxt,
        VarBaseTxt1_Redim,
        VarBaseTxt1_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseTxt1_GetBool,
        VarBaseTxt1_GetInt,
        VarBaseTxt1_GetDouble,
        VarBaseTxt1_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseTxt_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseTxt2()
{
    static const TVarInfo var(
        VarBaseTxt2_Tamanho,
        VarBaseTxt2_TamanhoVetor,
        TVarInfo::FTipoTxt,
        VarBaseTxt2_Redim,
        VarBaseTxt2_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseTxt2_GetBool,
        VarBaseTxt2_GetInt,
        VarBaseTxt2_GetDouble,
        VarBaseTxt2_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseTxt_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseInt1()
{
    static const TVarInfo var(
        VarBaseInt1_Tamanho,
        VarBaseInt1_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt1_Redim,
        VarBaseInt1_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt1_GetBool,
        VarBaseInt1_GetInt,
        VarBaseInt1_GetDouble,
        VarBaseInt1_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseInt1_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseInt8()
{
    static const TVarInfo var(
        VarBaseInt8_Tamanho,
        VarBaseInt8_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt8_Redim,
        VarBaseInt8_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt8_GetBool,
        VarBaseInt8_GetInt,
        VarBaseInt8_GetDouble,
        VarBaseInt8_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseInt8_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseUInt8()
{
    static const TVarInfo var(
        VarBaseInt8_Tamanho,
        VarBaseInt8_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt8_Redim,
        VarBaseInt8_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt8_GetBool,
        VarBaseUInt8_GetInt,
        VarBaseUInt8_GetDouble,
        VarBaseUInt8_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseUInt8_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseInt16()
{
    static const TVarInfo var(
        VarBaseInt16_Tamanho,
        VarBaseInt16_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt16_Redim,
        VarBaseInt16_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt16_GetBool,
        VarBaseInt16_GetInt,
        VarBaseInt16_GetDouble,
        VarBaseInt16_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseInt16_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseUInt16()
{
    static const TVarInfo var(
        VarBaseInt16_Tamanho,
        VarBaseInt16_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt16_Redim,
        VarBaseInt16_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt16_GetBool,
        VarBaseUInt16_GetInt,
        VarBaseUInt16_GetDouble,
        VarBaseUInt16_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseUInt16_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseInt32()
{
    static const TVarInfo var(
        VarBaseInt32_Tamanho,
        VarBaseInt32_TamanhoVetor,
        TVarInfo::FTipoInt,
        VarBaseInt32_Redim,
        VarBaseInt32_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt32_GetBool,
        VarBaseInt32_GetInt,
        VarBaseInt32_GetDouble,
        VarBaseInt32_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseInt32_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseUInt32()
{
    static const TVarInfo var(
        VarBaseInt32_Tamanho,
        VarBaseInt32_TamanhoVetor,
        TVarInfo::FTipoDouble,
        VarBaseInt32_Redim,
        VarBaseInt32_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseInt32_GetBool,
        VarBaseUInt32_GetInt,
        VarBaseUInt32_GetDouble,
        VarBaseUInt32_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseUInt32_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseReal()
{
    static const TVarInfo var(
        VarBaseReal_Tamanho,
        VarBaseReal_TamanhoVetor,
        TVarInfo::FTipoDouble,
        VarBaseReal_Redim,
        VarBaseReal_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseReal_GetBool,
        VarBaseReal_GetInt,
        VarBaseReal_GetDouble,
        VarBaseReal_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseReal_FuncVetor);
    return &var;
}

const TVarInfo * VarBaseReal2()
{
    static const TVarInfo var(
        VarBaseReal2_Tamanho,
        VarBaseReal2_TamanhoVetor,
        TVarInfo::FTipoDouble,
        VarBaseReal2_Redim,
        VarBaseReal2_MoverEnd,
        TVarInfo::FMoverDef0,
        VarBaseReal2_GetBool,
        VarBaseReal2_GetInt,
        VarBaseReal2_GetDouble,
        VarBaseReal2_GetTxt,
        TVarInfo::FGetObjNulo,
        VarBaseReal2_FuncVetor);
    return &var;
}
