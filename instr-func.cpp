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
#include <math.h>
#include <assert.h>
#include "instr.h"
#include "instr-enum.h"
#include "instr-func.h"
#include "classe.h"
#include "objeto.h"
#include "variavel.h"
#include "procurar.h"
#include "random.h"
#include "misc.h"
#include "sha1.h"
#include "md5.h"

// Funções predefinidas do programa interpretado
// Vide TListaFunc, em instr.h

using namespace Instr;

//----------------------------------------------------------------------------
// Usado em bool InstrFunc::FuncTxt2() para copiar caracteres de cores
#define FUNCTXT_CORES \
    case ex_barra_b:  \
        *destino++ = *txt++;  \
        break;                \
    case ex_barra_c:          \
        if ((txt[1] >= '0' && txt[1] <= '9') ||      \
                (txt[1] >= 'A' && txt[1] <= 'J') ||  \
                (txt[1] >= 'a' && txt[1] <= 'j'))    \
            *destino++ = *txt++; \
        *destino++ = *txt++;     \
        break;        \
    case ex_barra_d:  \
        if (txt[1] >= '0' && txt[1] <= '7') \
            *destino++ = *txt++; \
    case ex_barra_n:             \
        *destino++ = *txt++;     \
        break;

//----------------------------------------------------------------------------
/// Argumento da função (arg0 a arg9)
static inline bool StaticArg(TVariavel * v, int valor)
{
    if (valor >= Instr::FuncAtual->numarg)
        return false;
    Instr::ApagarVar(v);
    Instr::VarAtual++;
    *Instr::VarAtual = *(Instr::FuncAtual->inivar + valor);
    Instr::VarAtual->tamanho = 0;
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncArg0(TVariavel * v) { return StaticArg(v, 0); }
bool InstrFunc::FuncArg1(TVariavel * v) { return StaticArg(v, 1); }
bool InstrFunc::FuncArg2(TVariavel * v) { return StaticArg(v, 2); }
bool InstrFunc::FuncArg3(TVariavel * v) { return StaticArg(v, 3); }
bool InstrFunc::FuncArg4(TVariavel * v) { return StaticArg(v, 4); }
bool InstrFunc::FuncArg5(TVariavel * v) { return StaticArg(v, 5); }
bool InstrFunc::FuncArg6(TVariavel * v) { return StaticArg(v, 6); }
bool InstrFunc::FuncArg7(TVariavel * v) { return StaticArg(v, 7); }
bool InstrFunc::FuncArg8(TVariavel * v) { return StaticArg(v, 8); }
bool InstrFunc::FuncArg9(TVariavel * v) { return StaticArg(v, 9); }

//----------------------------------------------------------------------------
bool InstrFunc::FuncArgs(TVariavel * v)
{
    return CriarVarInt(v, FuncAtual->numarg);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncCriar(TVariavel * v)
{
    if (VarAtual <= v)
        return false;
// Se VarExec for muito pequeno, pode ter problemas ao executar a função ini
    if (VarExec < 500)
    {
        VarExec = -1;
        return false;
    }
// Procura a classe
    TClasse * c = TClasse::Procura(v[1].getTxt());
    if (c==0)
        return false;
// Cria objeto
    TObjeto * obj = TObjeto::Criar(c);
    while (true)
    {
    // Procura a função ini
        int indice = c->IndiceNome("ini");
        if (indice<0) // Variável/função inexistente
            break;
        const char * defvar = c->InstrVar[indice];
        const char * instr = defvar;
        const char * expr = nullptr;
    // Verifica se é função
        if (defvar[2] == cFunc || defvar[2] == cVarFunc)
        {
            instr += Num16(instr);
            if (instr[0] == 0 && instr[1] == 0)
                break;
        }
        else if (defvar[2] == cConstExpr || defvar[2] == cConstVar)
            expr = instr + instr[endIndice];
        else
            break;
    // Verifica se pode criar função
        if (FuncAtual >= FuncFim - 1)
            break;
    // Cria função
        FuncAtual++;
        FuncAtual->nome = defvar;
        FuncAtual->linha = instr;
        FuncAtual->este = obj;
        FuncAtual->expr = expr;
        FuncAtual->inivar = v + 2;
        FuncAtual->fimvar = VarAtual + 1;
        FuncAtual->numarg = FuncAtual->fimvar - FuncAtual->inivar;
        FuncAtual->tipo = 3;
        FuncAtual->indent = 0;
        FuncAtual->objdebug = FuncAtual[-1].objdebug;
        FuncAtual->funcdebug = FuncAtual[-1].funcdebug;
        return true;
    }
// Retorna o endereço do objeto criado
    ApagarVar(v);
    VarAtual++;
    VarAtual->Limpar();
    VarAtual->defvar = InstrVarObjeto;
    VarAtual->endvar = obj;
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncApagar(TVariavel * v)
{
    for (TVariavel * var = v + 1; var <= VarAtual; var++)
    {
        TObjeto * obj = var->getObj();
        if (obj)
            obj->MarcarApagar();
    }
    return false;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatPi(TVariavel * v)
{
    return CriarVarDouble(v, M_PI);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncEste(TVariavel * v)
{
    ApagarVar(v);
    return Instr::CriarVarObj(FuncAtual->este);
}

//----------------------------------------------------------------------------
static inline double NumeroDouble(TVariavel * v)
{
    double numero = 0;
    for (TVariavel * var = v + 1; var <= Instr::VarAtual; var++)
        numero += var->getDouble();
    return numero;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntPos(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, numero < 0 ? 0 : numero);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntAbs(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, numero < 0 ? -numero : numero);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncInt(TVariavel * v)
{
    double numero = NumeroDouble(v);
    numero = round(numero);
    if (numero >= (double)-0x7FFFFFFF && numero <= (double)0x7FFFFFFF)
        return CriarVarInt(v, (int)numero);
    return CriarVarDouble(v, numero);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntDiv(TVariavel * v)
{
    double numero = NumeroDouble(v);
    numero = trunc(numero);
    if (numero >= (double)-0x7FFFFFFF && numero <= (double)0x7FFFFFFF)
        return CriarVarInt(v, (int)numero);
    return CriarVarDouble(v, numero);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatSin(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, sin(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatCos(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, cos(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatTan(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, tan(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatAsin(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, asin(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatAcos(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, acos(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatAtan(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, atan(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatExp(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, exp(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatLog(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, log(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatRaiz(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, sqrt(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatCima(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, ceil(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatBaixo(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, floor(numero));
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatRad(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, numero / 180 * M_PI);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatDeg(TVariavel * v)
{
    double numero = NumeroDouble(v);
    return CriarVarDouble(v, numero / M_PI * 180);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncMatPow(TVariavel * v)
{
    if (VarAtual != v + 2)
        return false;
    double numero = pow(v[1].getDouble(), v[2].getDouble());
    return CriarVarDouble(v, numero);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntBit(TVariavel * v)
{
    int result = 0;
    for (TVariavel * var = v + 1; var <= VarAtual; var++)
    {
        const char * p = var->getTxt();
        while (*p)
        {
            if (*p < '0' || *p > '9')
            {
                p++;
                continue;
            }
            int valor = *p - '0';
            for (p++; *p >= '0' && *p <= '9'; p++)
            {
                valor = 10 * valor + *p - '0';
                if (valor >= 32)
                    break;
            }
            if (valor < 32)
                result |= 1 << valor;
        }
    }
    return CriarVarInt(v, result);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtBit(TVariavel * v)
{
    if (VarAtual < v + 1)
        return false;
    int num = v[1].getInt();
    const char * separador = (VarAtual >= v + 2 ? v[2].getTxt() : " ");
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
    for (int x = 0; x < 32; x++, num >>= 1)
    {
        if ((num & 1) == 0)
            continue;
        if (destino != mens)
        {
            const char * p = separador;
            while (*p && destino < mens + sizeof(mens))
                *destino++ = *p++;
        }
        if (x >= 10)
            *destino++ = x / 10 + '0';
        *destino++ = x % 10 + '0';
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntBitH(TVariavel * v)
{
    char mens[BUF_MENS];
    int  inicio = BUF_MENS - 1;
    mens[BUF_MENS-1] = 0;

    for (TVariavel * var = v + 1; var <= VarAtual; var++)
    {
        const char * p = var->getTxt();
        while (*p)
        {
            if (*p < '0' || *p > '9')
            {
                p++;
                continue;
            }
            int valor = *p - '0';
            for (p++; *p >= '0' && *p <= '9'; p++)
            {
                valor = 10 * valor + *p - '0';
                if (valor >= BUF_MENS * 4)
                    break;
            }
            if (valor >= BUF_MENS * 4)
                continue;
            int ender = BUF_MENS - 1 - valor/4;
            if (ender < inicio)
            {
                memset(mens + ender, 0, inicio - ender);
                inicio = ender;
            }
            mens[ender] |= (1 << (valor & 3));
        }
    }

    for (char * p = mens + inicio; p < mens + BUF_MENS; p++)
    {
        unsigned char ch = *p;
        *p = (ch < 10 ? ch + '0' : ch + '7');
    }

    ApagarVar(v);
    return CriarVarTexto(mens + inicio, BUF_MENS - inicio);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtBitH(TVariavel * v)
{
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
    char separador[1024];
    const char * pini = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    const char * p = pini;
    int bit = 0;
    while (*p)
        p++;
    if (VarAtual >= v + 2)
        copiastr(separador, v[2].getTxt(), sizeof(separador));
    else
        strcpy(separador, " ");
    while (p >= pini)
    {
        char ch = *p--;
        if (ch >= '0' && ch <= '9')
            ch -= '0';
        else if (ch >= 'A' && ch <= 'Z')
            ch -= 'A' - 10;
        else if (ch >= 'a' && ch <= 'z')
            ch -= 'a' - 10;
        else
            continue;
        for (int x = 1; x < 16; x <<= 1, bit++)
        {
            if ((ch & x) == 0)
                continue;
            if (destino != mens)
            {
                const char * sep = separador;
                while (*sep && destino < mens + sizeof(mens))
                    *destino++ = *sep++;
            }
            if (destino >= mens+sizeof(mens)-5)
                break;
            int valor = bit;
            if (valor >= 10000) *destino++ = valor/10000 % 10 + '0';
            if (valor >= 1000)  *destino++ = valor/1000 % 10 + '0';
            if (valor >= 100)   *destino++ = valor/100 % 10 + '0';
            if (valor >= 10)    *destino++ = valor/10 % 10 + '0';
            *destino++ = valor % 10 + '0';
        }
    }

    ApagarVar(v);
    return CriarVarTexto(mens, destino-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtHex(TVariavel * v)
{
    if (VarAtual < v + 2)
        return false;
    char mens[BUF_MENS];
    const char * p = v[1].getTxt();
    int requerido = v[2].getInt();
    int total = strlen(p);
    if (requerido <= 0)
    {
        ApagarVar(v);
        return CriarVarTexto("", 0);
    }
    if (requerido > BUF_MENS)
        requerido = BUF_MENS;
    if (total >= requerido)
        memcpy(mens, p+total-requerido, requerido);
    else
    {
        memset(mens, '0', requerido-total);
        memcpy(mens+requerido-total, p, total);
    }
    ApagarVar(v);
    return CriarVarTexto(mens, requerido);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntMax(TVariavel * v)
{
    if (VarAtual <= v + 1)
    {
        if (VarAtual == v)
            return false;
        ApagarRet(v);
        return true;
    }
    double d = v[1].getDouble();
    TVariavel * pos = v + 1;
    for (TVariavel * var = v + 2; var <= VarAtual; var++)
    {
        double num = var->getDouble();
        if (d < num)
            d = num, pos = var;
    }
    ApagarVar(pos + 1);
    ApagarRet(v);
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntMin(TVariavel * v)
{
    if (VarAtual <= v + 1)
    {
        if (VarAtual == v)
            return false;
        ApagarRet(v);
        return true;
    }
    double d = v[1].getDouble();
    TVariavel * pos = v + 1;
    for (TVariavel * var = v + 2; var <= VarAtual; var++)
    {
        double num = var->getDouble();
        if (d > num)
            d = num, pos = var;
    }
    ApagarVar(pos + 1);
    ApagarRet(v);
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncRand(TVariavel * v)
{
    int result = 0;
    if (VarAtual == v + 1)
    {
        if (v[1].Tipo() == varTxt) // Se for texto, embaralha o texto
        {
            char mens[BUF_MENS];
            const char * fim = copiastr(mens, v[1].getTxt(), sizeof(mens));
            const int total = fim - mens;
            for (char * p = mens; p<fim; p++)
            {
                int ind = circle_random() % total;
                char x = mens[ind];
                mens[ind] = *p;
                *p = x;
            }
            ApagarVar(v);
            return CriarVarTexto(mens);
        }
        int max = v[1].getInt();
        if (max > 0)
            result = circle_random() % max;
    }
    else if (VarAtual > v + 1)
    {
        int min = v[1].getInt();
        int max = v[2].getInt();
        if (min < max)
            result = circle_random() % (max - min + 1) + min;
        else if (min > max)
            result = circle_random() % (min - max + 1) + max;
        else
            result = min;
    }
    return CriarVarInt(v, result);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncRef(TVariavel * v)
{
    TObjeto * obj = nullptr;
    for (TVariavel * var = v + 1; var <= VarAtual && obj == nullptr; var++)
        obj = var->getObj();
    ApagarVar(v);
    return CriarVarObj(obj);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtTipoVar(TVariavel * v)
{
    if (VarAtual < v + 1)
        return CriarVarTxtFixo(v, "nulo");
    TVariavel * v1 = v + 1;
    if (v1->indice == 0xff)
        return CriarVarTxtFixo(v, "vetor");
    if (v1->defvar[2] == cUInt32)
        return CriarVarTxtFixo(v, "int");
    switch (v1->Tipo())
    {
    case varInt:
        return CriarVarTxtFixo(v, "int");
    case varDouble:
        return CriarVarTxtFixo(v, "real");
    case varTxt:
        return CriarVarTxtFixo(v, "txt");
    case varObj:
        return CriarVarTxtFixo(v, v1->getObj() ? "ref" : "nulo");
    case varOutros:
        break;
    default:
        return CriarVarTxtFixo(v, "");
    }
    const char * txt = NomeInstr(v1->defvar);
    ApagarVar(v);
    return CriarVarTexto(txt);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtNum(TVariavel * v)
{
    char mens[80];      // Resultado
    int  flags = 0;     // bit 0=1 -> usar notação científica
                        // bit 1=1 -> separar com pontos
                        // bit 2=1 -> separar com vírgulas
    int  digitos = -1;  // Dígitos após a vírgula, 0-9 ou -1=automático

    if (VarAtual < v + 2)
        return false;

// Obtém o tipo de conversão
    for (const char * p = v[2].getTxt(); *p; p++)
        switch (*p)
        {
        case 'E':
        case 'e': flags |= 1; break;
        case '.': flags |= 2; break;
        case ',': flags |= 4; break;
        default:
            if (*p >= '0' && *p <= '9')
                digitos = *p - '0';
        }

// Obtém o texto
    while (true)
    {
        int tipo = v[1].Tipo();
    // Inteiro
        if (tipo == varInt)
        {
            const char * zeros = "000000000";
            if (flags & 1)
            {
                sprintf(mens, "%E", v[1].getDouble());
                flags = 0;
            }
            else if (digitos < 1 || digitos > 9)
                sprintf(mens, "%d", v[1].getInt());
            else
                sprintf(mens, "%d.%s", v[1].getInt(), zeros + 9 - digitos);
            break;
        }
    // Não é numérico
        if (tipo != varDouble)
            return false;
    // Double
        double d = v[1].getDouble();
    // Double fora dos limites
        if ((flags&1) || d >= DOUBLE_MAX || d <= -DOUBLE_MAX)
        {
            sprintf(mens, "%E", d);
            flags = 0;
            break;
        }
    // Quantidade fixa de dígitos
        if (digitos >= 0)
        {
            char mens2[16];
            sprintf(mens2, "%%.%df", digitos);
            sprintf(mens, mens2, d);
            break;
        }
    // Quantidade de dígitos não foi especificada
        sprintf(mens, "%.9f", d);
        char * p = mens;
        while (*p)
            p++;
        while (p > mens && p[-1] == '0')
            p--;
        if (p > mens && p[-1] == '.')
            p--;
        *p=0;
        break;
    }

// Acrescenta pontos ou vírgulas, se necessário
    if (flags & 6)
    {
        char * p = mens;
        int total;
        while (*p && *p != '.')
            p++;
        if (*p && (flags & 2))
            *p=',';
        total = (p - mens - (*mens=='-') - 1) / 3;
        char * dest = p;
        while (*dest)
            dest++;
        while (total > 0)
        {
            p -= 3;
            assert(p > mens);
            for (; dest >= p; dest--)
                dest[total] = *dest;
            dest[total--] = (flags & 2 ? '.' : ',');
        }
    }

    ApagarVar(v);
    return CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntSub(TVariavel * v)
{
    int total = 0;
    if (VarAtual >= v + 1)
    {
        const char * txt = v[1].getTxt();
        while (*txt == ' ') txt++;
        while (*txt)
        {
            while (*txt && *txt != ' ') txt++;
            while (*txt == ' ') txt++;
            total++;
        }
    }
    return CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntSubLin(TVariavel * v)
{
    int total = 0;
    if (VarAtual >= v + 1)
    {
        const char * txt = v[1].getTxt();
        if (*txt)
        {
            total++;
            while (*txt)
                if (*txt++ == ex_barra_n)
                    total++;
        }
    }
    return CriarVarInt(v, total);
}

//----------------------------------------------------------------------------
static inline bool StaticTxt(TVariavel * v, int valor)
{
    int ini = 0;        // Início
    int tam = 0x10000;  // Tamanho
    const char * txt = "";  // Texto
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
// Obtém ini, tam e txt conforme os argumentos
    if (Instr::VarAtual >= v + 3)
        tam = v[3].getInt();
    if (tam > 0)
    {
        if (Instr::VarAtual >= v + 2)
        {
            ini = v[2].getInt();
            if (ini < 0)
                ini = 0;
        }
        if (Instr::VarAtual >= v + 1)
            txt = v[1].getTxt();
    }
    switch (valor)
    {
    case 0: // Obtém o texto para função txt
        if (tam > (int)sizeof(mens))
            tam = sizeof(mens);
            // Avança para início do texto
        for (const char * fim = txt + ini; *txt && txt < fim; )
            txt++;
            // Copia texto
        for (const char * fim = txt + tam; *txt && txt < fim; )
            *destino++ = *txt++;
        break;
    case 1: // Obtém o texto para função txtsub
        while (*txt == ' ') txt++;
            // Avança para início do texto
        while (ini > 0 && *txt)
        {
            while (*txt && *txt != ' ') txt++;
            while (*txt == ' ') txt++;
            ini--;
        }
            // Copia texto
        while (tam > 0 && *txt && destino < mens + sizeof(mens))
        {
            while (*txt == ' ' && destino < mens + sizeof(mens))
                *destino++ = *txt++;
            while (*txt && *txt != ' ' && destino < mens + sizeof(mens))
                *destino++ = *txt++;
            tam--;
        }
        break;
    case 2: // Obtém o texto para função txtsublin
            // Avança para início do texto
        while (ini > 0 && *txt)
        {
            while (*txt && *txt != Instr::ex_barra_n) txt++;
            if (*txt == Instr::ex_barra_n) txt++;
            ini--;
        }
            // Copia texto
        while (tam > 0 && destino < mens + sizeof(mens))
        {
            while (*txt && *txt != Instr::ex_barra_n && destino < mens +sizeof(mens))
                *destino++ = *txt++;
            if (*txt == 0)
                break;
            if (--tam)
                *destino++ = *txt++;
        }
        break;
    }
// Anota o texto
    Instr::ApagarVar(v);
    return Instr::CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxt(TVariavel * v) { return StaticTxt(v, 0); }
bool InstrFunc::FuncTxtSub(TVariavel * v) { return StaticTxt(v, 1); }
bool InstrFunc::FuncTxtSubLin(TVariavel * v) { return StaticTxt(v, 2); }

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtFim(TVariavel * v)
{
    int tam = 0x10000;  // Tamanho
    char mens[BUF_MENS];    // Resultado
// Obtém tam conforme os argumentos
    if (VarAtual >= v + 2)
        tam = v[2].getInt();
    if (tam > (int)sizeof(mens))
        tam = sizeof(mens);
// Obtém o texto
    if (tam <= 0 || VarAtual < v + 1)
        tam = 0;
    else
    {
        const char * txt = v[1].getTxt();
        int total = strlen(txt);
        if (tam > total)
            tam = total;
        else
            txt += total - tam;
        memcpy(mens, txt, tam);
    }
// Anota o texto
    ApagarVar(v);
    return CriarVarTexto(mens, tam);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxt1(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt==' ')
        txt++;
    while (*txt && *txt!=' ' && destino<mens+sizeof(mens))
        *destino++ = *txt++;
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxt2(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt == ' ') txt++;
    while (*txt && *txt != ' ') txt++;
    while (*txt == ' ') txt++;
    while (*txt && destino < mens + sizeof(mens))
        *destino++ = *txt++;
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtCor(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens))
    {
        switch (*txt)
        {
        case ex_barra_b:
            txt++;
            break;
        case ex_barra_c:
            if ((txt[1] >= '0' && txt[1] <= '9') ||
                    (txt[1] >= 'A' && txt[1] <= 'J') ||
                    (txt[1] >= 'a' && txt[1] <= 'j'))
                txt += 2;
            else
                txt++;
            break;
        case ex_barra_d:
            if (txt[1] >= '0' && txt[1] <= '7')
                txt += 2;
            else
                txt++;
            break;
        default:
            *destino++ = *txt++;
        }
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMai(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
        switch (*txt)
        {
        FUNCTXT_CORES
        default: *destino++ = tabMAI[*(unsigned char*)txt++];
        }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMaiIni(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        while (*txt && *(unsigned char*)txt < '0' &&
                destino < mens + sizeof(mens) - 1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = *txt++;
            }
        if (*txt && destino < mens + sizeof(mens) - 1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = tabMAI[*(unsigned char*)txt++];
            }
        while (*txt && *txt != '.' && destino < mens + sizeof(mens) - 1)
            *destino++ = *txt++;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMin(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
        switch (*txt)
        {
        FUNCTXT_CORES
        default: *destino++ = tabMIN[*(unsigned char*)txt++];
        }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMaiMin(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        while (*txt && *(unsigned char*)txt<'0' &&
                destino < mens + sizeof(mens) - 1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = *txt++;
            }
        if (*txt && destino < mens + sizeof(mens) - 1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = tabMAI[*(unsigned char*)txt++];
            }
        while (*txt && *txt != '.' && destino < mens + sizeof(mens) - 1)
            switch (*txt)
            {
            FUNCTXT_CORES
            default: *destino++ = tabMIN[*(unsigned char*)txt++];
            }
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtFiltro(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
        destino = txtFiltro(destino, txt, sizeof(mens));
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtSha1Bin(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    unsigned char digest[20];
    SHA_CTX shaInfo;
    SHAInit(&shaInfo);
    SHAUpdate(&shaInfo, (unsigned char *)txt, strlen(txt));
    SHAFinal(digest, &shaInfo);
    for (int x = 0; x < 20; x += 4)
    {
        unsigned int valor = digest[x]     * 0x1000000+
                             digest[x + 1] * 0x10000+
                             digest[x + 2] * 0x100+
                             digest[x + 3];
        for (int y = 0; y < 5; y++)
        {
            *destino++ = (valor & 0x3F) + 0x21;
            valor >>= 6;
        }
    }
    *destino++ = (digest[0] & 3) +
                  (digest[4] & 3) * 4 +
                  (digest[8] & 3) * 16 + 0x21;
    *destino++ = (digest[12] & 3) +
                  (digest[16] & 3) * 4 + 0x21;
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtSha1(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    unsigned char digest[20];
    SHA_CTX shaInfo;
    SHAInit(&shaInfo);
    SHAUpdate(&shaInfo, (unsigned char *)txt, strlen(txt));
    SHAFinal(digest, &shaInfo);
    for (int x = 0; x < 20; x++)
    {
        int valor = digest[x] >> 4;
        *destino++ = (valor < 10 ? valor + '0' : valor + 'a' - 10);
        valor = digest[x] & 0x0F;
        *destino++ = (valor < 10 ? valor + '0' : valor + 'a' - 10);
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMd5(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    unsigned char digest[16];
    cvs_MD5Context md5Info;
    cvs_MD5Init(&md5Info);
    cvs_MD5Update(&md5Info, (unsigned char *)txt, strlen(txt));
    cvs_MD5Final(digest, &md5Info);
    for (int x = 0; x < 16; x++)
    {
        int valor = digest[x] >> 4;
        *destino++ = (valor < 10 ? valor + '0' : valor + 'a' - 10);
        valor = digest[x] & 0x0F;
        *destino++ = (valor < 10 ? valor + '0' : valor + 'a' - 10);
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtNome(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    destino = txtNome(destino, txt, sizeof(mens));
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}


//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtCod(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens)-2)
    {
        unsigned char ch = tabTXTCOD[*(unsigned char*)txt];
        if (ch)
            *destino++ = '@', *destino++ = ch;
        else //if (tabNOMES1[*(unsigned char*)txt])
            *destino++ = *txt;
        txt++;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtDec(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        if (*txt != '@')
            *destino++ = *txt++;
        else if (txt[1])
        {
            unsigned char ch = tabTXTDEC[*(unsigned char*)(txt + 1)];
            if (ch)
                *destino++ = ch;
            txt += 2;
        }
        else
            break;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtVis(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    for (; *txt && destino < mens + sizeof(mens)-2; txt++)
        switch (*txt)
        {
        case Instr::ex_barra_b:
            destino[0] = '\\', destino[1] = 'b', destino += 2;
            break;
        case Instr::ex_barra_c:
            destino[0] = '\\', destino[1] = 'c', destino += 2;
            break;
        case Instr::ex_barra_d:
            destino[0] = '\\', destino[1] = 'd', destino += 2;
            break;
        case Instr::ex_barra_n:
            destino[0] = '\\', destino[1] = 'n', destino += 2;
            break;
        case '\"':
        case '\\':
            *destino++ = '\\';
        default:
            *destino++ = *txt;
        }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtInvis(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    for (; *txt && destino < mens + sizeof(mens) - 1; txt++)
    {
        if (*txt != '\\')
        {
            *destino++ = *txt;
            continue;
        }
        txt++;
        if (*txt == 0)
            break;
        switch (*txt)
        {
        case 'B':
        case 'b': *destino++ = Instr::ex_barra_b; break;
        case 'C':
        case 'c': *destino++ = Instr::ex_barra_c; break;
        case 'D':
        case 'd': *destino++ = Instr::ex_barra_d; break;
        case 'N':
        case 'n': *destino++ = Instr::ex_barra_n; break;
        default: *destino++ = *txt;
        }
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtUrlCod(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    bool ini = true;
    while (*txt && destino < mens + sizeof(mens)-3)
    {
        char ch = *txt++;
        if (ch == ex_barra_n)
        {
            *destino++ = (ini ? '?' : '&');
            ini = false;
        }
        else if (ch == ' ')
            *destino++ = '+';
        else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
                (ch >= 'a' && ch <= 'z') ||
                ch == '=' || ch == '-' || ch == '/' || ch == '.')
            *destino++ = ch;
        else
        {
            switch (ch)
            {
            case ex_barra_b: ch = 1; break;
            case ex_barra_c: ch = 2; break;
            case ex_barra_d: ch = 3; break;
            }
            int parte = (ch >> 4) & 15;
            destino[0] = '%';
            destino[1] = (parte < 10 ? parte + '0' : parte + '7');
            parte = ch & 15;
            destino[2] = (parte < 10 ? parte + '0' : parte + '7');
            destino += 3;
        }
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtUrlDec(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    bool ini = true;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        char ch = *txt++;
        if (ch == '+')
            *destino++ = ' ';
        else if (ch == '?')
        {
            *destino++ = (ini ? ex_barra_n : '?');
            ini = false;
        }
        else if (ch == '&')
            *destino++ = (ini ? '&' : ex_barra_n);
        else if (ch != '%')
            *destino++ = ch;
        else
        {
            int cod;
        // Primeiro dígito
            if (txt[0] >= '0' && txt[0] <= '9')
                cod = (txt[0] - '0') * 16;
            else if ((txt[0] | 0x20) >= 'a' && (txt[0] | 0x20) <= 'f')
                cod = ((txt[0] | 0x20) - 'W') * 16;
            else
            {
                *destino++ = ch;
                continue;
            }
        // Segundo dígito
            if (txt[1] >= '0' && txt[1] <= '9')
                cod += txt[1] - '0';
            else if ((txt[1] | 0x20) >= 'a' && (txt[1] | 0x20) <= 'f')
                cod += (txt[1] | 0x20) - 'W';
            else
            {
                *destino++ = ch;
                continue;
            }
        // Obtém e anota caracter
            if (cod >= 32)
                *destino++ = cod;
            else
                switch (cod)
                {
                case 1: *destino++ = ex_barra_b; break;
                case 2: *destino++ = ex_barra_c; break;
                case 3: *destino++ = ex_barra_d; break;
                case 10: *destino++ = ex_barra_n; break;
                }
            txt += 2;
        }
    } // for
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtE(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        *destino = (*txt != '_' ? *txt : ' ');
        destino++, txt++;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtS(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    while (*txt && destino < mens + sizeof(mens) - 1)
    {
        *destino = (*txt != ' ' ? *txt : '_');
        destino++, txt++;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtRev(TVariavel * v)
{
    const char * txt = (VarAtual >= v + 1 ? v[1].getTxt() : "");
    char mens[BUF_MENS];
    char * destino = mens;
    unsigned int total = strlen(txt);
    if (total > sizeof(mens) - 1)
        total = sizeof(mens) - 1;
    const char * t = txt + total;
    for (t--; t >= txt; t--)
    {
        char ch = *t;
        if (ch == ex_barra_c)
        {
            if ((t[1] >= '0' && t[1] <= '9') ||
                (t[1] >= 'A' && t[1] <= 'J') ||
                (t[1] >= 'a' && t[1] <= 'j'))
            {
                destino[-1] = ex_barra_c;
                *destino++ = t[1];
                continue;
            }
        }
        else if (ch == ex_barra_d)
        {
            if (t[1] >= '0' && t[1] <= '7') \
            {
                destino[-1] = ex_barra_d;
                *destino++ = t[1];
                continue;
            }
        }
        *destino++ = ch;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtMudaMai(TVariavel * v)
{
    const char * txt = "";  // Texto
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
    int porcent = 100;

// Obtém argumentos
    if (VarAtual >= v + 1)
    {
        if (VarAtual >= v + 2)
            porcent = v[2].getInt();
        if (porcent <= 0) // Nenhuma possibilidade de troca
        {
            char * p = copiastr(mens, v[1].getTxt(), sizeof(mens));
            ApagarVar(v);
            return CriarVarTexto(mens, p-mens);
        }
        txt = v[1].getTxt();
    }

// Copia texto modificando
    while (destino < mens + sizeof(mens) - 1)
    {
        unsigned char ch = *txt++;
        if (ch == 0)
            break;
        switch (tabMAIMIN[ch])
        {
        case 1: // É letra minúscula
            if (porcent < 100 && circle_random() % 100
                    >= (unsigned int)porcent)
                *destino++ = ch;
            else
                *destino++ = tabMAI[ch];
            break;
        case 2: // É letra maiúscula
            if (porcent < 100 && circle_random() % 100
                    >= (unsigned int)porcent)
                *destino++ = ch;
            else
                *destino++ = tabMIN[ch];
            break;
        default: // Não é maiúscula nem minúscula
            *destino++ = ch;
        }
    }

// Anota o texto
    ApagarVar(v);
    return CriarVarTexto(mens, destino - mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtCopiaMai(TVariavel * v)
{
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;

    *mens = 0;
    if (VarAtual >= v + 1)
        destino = copiastr(mens, v[1].getTxt(), sizeof(mens));
    if (VarAtual >= v + 2)
    {
        const char * o = v[2].getTxt();
        char * d = mens;
        for (; *o && *d; o++, d++)
            switch (tabMAIMIN[*(unsigned char*)o])
            {
            case 1: // É letra minúscula
                *d = tabMIN[*(unsigned char*)d];
                break;
            case 2: // É letra maiúscula
                *d = tabMAI[*(unsigned char*)d];
                break;
            }
    }

// Anota o texto
    ApagarVar(v);
    return CriarVarTexto(mens, destino-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncEsp(TVariavel * v)
{
    static char * texto = nullptr;
    int esp = 0;
    if (texto == nullptr)
    {
        texto = new char[101];
        memset(texto, ' ', 100);
        texto[100] = 0;
    }
    if (VarAtual >= v + 1)
        esp = v[1].getInt();
    if (esp < 0)
        esp = 100;
    else if (esp > 100)
        esp = 0;
    else
        esp = 100 - esp;
    ApagarVar(v);
    if (!Instr::CriarVar(Instr::InstrTxtFixo))
        return false;
    Instr::VarAtual->endfixo = texto + esp;
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtRepete(TVariavel * v)
{
    const char * txt = "";  // Texto
    char mens[BUF_MENS];    // Resultado
    char * destino = mens;
    int repete = 0;      // Número de repetições
    int tamtxt = 0;      // Tamanho do texto em txt
    if (VarAtual >= v + 2)
    {
        repete = v[2].getInt();
        txt = v[1].getTxt();
        if (*txt == 0)
            repete = 0;
        else
            tamtxt = strlen(txt);
    }
    for (; repete > 0; repete--)
    {
        if (destino + tamtxt > mens + sizeof(mens))
            break;
        memcpy(destino, txt, tamtxt);
        destino += tamtxt;
    }
    ApagarVar(v);
    return CriarVarTexto(mens, destino-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntNome(TVariavel * v)
{
    int valor = verifNome(VarAtual >= v + 1 ? v[1].getTxt() : "");
    return CriarVarInt(v, valor);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntSenha(TVariavel * v)
{
    int valor = verifSenha(VarAtual >= v + 1 ? v[1].getTxt() : "");
    return CriarVarInt(v, valor);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtRemove(TVariavel * v)
{
    char mens[BUF_MENS]; // Resultado
    if (VarAtual < v + 2)
        return false;
    int remove = txtRemove(v[2].getTxt()); // O que deve remover
    if (remove == 0)
    {
        ApagarVar(v + 2);
        ApagarRet(v);
        return true;
    }
    char * fim = txtRemove(mens, v[1].getTxt(), sizeof(mens), remove);
    ApagarVar(v);
    return CriarVarTexto(mens, fim-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtConv(TVariavel * v)
{
    if (VarAtual != v + 2)
        return false;
    const char * texto = v[2].getTxt();
    while (true)
    {
        char mens[BUF_MENS];    // Resultado
        char * destino = mens;

        // Checa se o formato de conversão contém dois caracteres
        if (texto[0] == 0) break;
        if (texto[1] == 0) break;
        if (texto[2] != 0) break;

        // Obtém variáveis
        char conv1 = texto[0] | 0x20;
        char conv2 = texto[1] | 0x20;
        texto = v[1].getTxt();

        if (conv1 == 't' && conv2 == 'u') // "TU" de iso8859-1 para UTF8
        {
            while (*texto && destino < mens + BUF_MENS - 2)
            {
                unsigned char ch = *texto++;
                if (ch < 0x80)
                    *destino++ = ch;
                else
                {
                    *destino++ = 0xC0 + ch / 0x40;
                    *destino++ = 0x80 + (ch & 0x3F);
                }
            }
            ApagarVar(v);
            return CriarVarTexto(mens, destino-mens);
        }

        if (conv1 == 'u' && conv2 == 't') // "UT" de UTF8 para iso8859-1
        {
            int LerUTF8 = 0;
            while (*texto && destino < mens + BUF_MENS - 1)
            {
                int ch = *(unsigned char*)texto++;
                if (ch < 0x80)
                    *destino++ = ch;
                else if (ch >= 0xC0) // Primeiro byte do caracter
                {
                    if (ch < 0xE0)
                        LerUTF8 = ch & 0x1F;
                    else
                        LerUTF8 = 0;
                }
                else if (LerUTF8 != 0) // Se não está ignorando
                {
                    ch = LerUTF8 * 0x40 + (ch & 0x3F); // Obtém caracter
                    LerUTF8 = 0;
                    if (ch < 0x100) // Somente caracteres ISO8859-1
                        *destino++ = ch;
                }
            }
            ApagarVar(v);
            return CriarVarTexto(mens, destino-mens);
        }

        if (conv1 == 'd' && conv2 == 'd') // "DD"
        {
            for (; *texto; texto++)
                if (*texto >= '1' && *texto <= '9')
                    break;
            while (*texto && destino < mens+BUF_MENS-1)
            {
                unsigned char ch = *texto++;
                if (ch >= '0' && ch <= '9')
                    *destino++ = ch;
            }
            if (destino == mens)
                *destino++ = '0';
            ApagarVar(v);
            return CriarVarTexto(mens, destino-mens);
        }

        if (conv2 != 't' && conv2 != 'h' && conv2 != 'b' &&
            conv2 != 'd' && conv2 != 'x')
            break;

        if (conv1 == 't') // Decodifica texto
        {
            while (*texto && destino < mens + BUF_MENS - 2)
            {
                unsigned char ch = *texto++;
                switch (ch)
                {
                case ex_barra_b:
                    break;
                case ex_barra_c:
                    ch = *texto;
                    if (ch && (ch < '0' || ch > '9') &&
                            (ch < 'A' || ch > 'J') && (ch < 'a' || ch > 'j'))
                        *destino++ = ch, texto++;
                    break;
                case ex_barra_d:
                    ch = *texto;
                    if (ch && (ch < '0' || ch > '7'))
                        *destino++ = ch, texto++;
                    break;
                case ex_barra_n:
                    *destino++ = 13;
                    *destino++ = 10;
                    break;
                default:
                    *destino++ = ch;
                }
            }
        }
        else if (conv1 == 'h') // Decodifica hexadecimal
        {
            int valor = 0;
            for (const char * p = texto; *p; p++)
            {
                unsigned char ch = *p;
                if ((ch >= '0' && ch <= '9') ||
                    ((ch|0x20) >= 'a' && (ch|0x20) <= 'f'))
                    valor++;
            }
            valor = (valor & 1 ? 16 : 1);
            while (*texto && destino < mens+BUF_MENS-1)
            {
                unsigned char ch = *texto++;
                if (ch >= '0' && ch <= '9')
                    valor = valor * 16 + ch - '0';
                else if (ch >= 'A' && ch <= 'F')
                    valor = valor * 16 + ch - 'A' + 10;
                else if (ch >= 'a' && ch <= 'f')
                    valor = valor * 16 + ch - 'a' + 10;
                else
                    continue;
                if (valor >= 0x100)
                    *destino++ = valor, valor=1;
            }
        }
        else if (conv1 == 'b') // Decodifica base64
        {
            int valor = 1;
            while (destino < mens+BUF_MENS-3)
            {
                unsigned char ch = *texto++;
                if (ch >= 'A' && ch <= 'Z')
                    valor = valor * 64 + ch - 'A';
                else if (ch >= 'a' && ch <= 'z')
                    valor = valor * 64 + ch - 'a' + 26;
                else if (ch >= '0' && ch <= '9')
                    valor = valor * 64 + ch - '0' + 52;
                else if (ch == '+')
                    valor = valor * 64 + 62;
                else if (ch == '/')
                    valor = valor * 64 + 63;
                else if (ch == '=' || ch == 0)
                {
                    if (valor >= 0x40000)
                    {
                        *destino++ = valor >> 10;
                        *destino++ = valor >> 2;
                    }
                    else if (valor >= 0x1000)
                        *destino++ = valor >> 4;
                    if (ch == 0)
                        break;
                    valor = 1;
                    continue;
                }
                else
                    continue;
                if (valor < 0x1000000)
                    continue;
                *destino++ = valor >> 16;
                *destino++ = valor >> 8;
                *destino++ = valor;
                valor = 1;
            }
        }
        else if (conv1 == 'd') // Decodifica decimal
        {
            while (true)
            {

            // Avança até encontrar um dígito de 1 a 9
                while (*texto && (*texto < '0' || *texto > '9'))
                    texto++;
                if (*texto == 0)
                    break;
                while (*texto && (*texto < '1' || *texto > '9'))
                    texto++;
                if (*texto == 0)
                {
                    *destino++ = 0;
                    break;
                }

            // Anota em mens: cada byte corresponde ao valor de 2 dígitos
                {
                    int total = 0;
                    for (const char * p = texto; *p; p++)
                        if (*p >= '0' && *p <= '9')
                            total++;
                    if (total > BUF_MENS*2)
                        total = BUF_MENS*2;
                    if (total & 1)
                        *destino++ = *texto++ - '0';
                    int valor = 0;
                    for (; *texto; texto++)
                    {
                        if (*texto < '0' || *texto > '9')
                            continue;
                        if (valor == 0)
                            valor = *texto;
                        else
                        {
                            *destino++ = *texto - '0' + (valor - '0') * 10;
                            valor = 0;
                        }
                    }
                }

            // Junta tudo em um número grande
                int total = destino - mens;
                assert(total != 0);
                for (int y = 1; y < total; y++)
                {
                    int v = mens[y];
                    for (int x = y; x > 0; x--)
                    {
                        v += (unsigned char)mens[x-1] * 100;
                        mens[x] = (v & 0xFF);
                        v >>= 8;
                    }
                    mens[0] = v;
                }

            // Elimina bytes 0 no início do número
                for (char * p = mens; p<destino; p++)
                {
                    if (*p == 0)
                        continue;
                    if (p != mens)
                    {
                        int total = destino - p;
                        memmove(mens, p, total);
                        destino = mens + total;
                    }
                    break;
                }
                break;
            }
        }
        else if (conv1 == 'x') // Decodifica binário
        {
            int valor = 0;
            for (const char * p = texto; *p; p++)
                if (*p == '0' || *p == '1')
                    valor--;
            valor = (1 << (valor & 7));
            while (*texto && destino < mens+BUF_MENS-1)
            {
                unsigned char ch = *texto++;
                if (ch == '0')
                    valor <<= 1;
                else if (ch == '1')
                    valor = (valor << 1) + 1;
                if (valor >= 0x100)
                    *destino++ = valor, valor=1;
            }
        }
        else
            break;

        if (conv2 == 't') // Codifica texto
        {
            char * o = mens;
            for (; o < destino; o++)
                if ((*o & 0xE0) == 0)
                    break;
            char * d = o;
            while (o < destino)
            {
                char ch = *o++;
                if (ch & 0xE0)
                    *d++ = ch;
                else if (ch == 13)
                {
                    *d++ = ex_barra_n;
                    if (o < destino)
                        if (*o == 10)
                            o++;
                }
                else if (ch == 10)
                {
                    *d++ = ex_barra_n;
                    if (o < destino)
                        if (*o == 13)
                            o++;
                }
            }
            ApagarVar(v);
            return CriarVarTexto(mens, d-mens);
        }
        else if (conv2 == 'h') // Codifica em hexadecimal
        {
            char mdest[BUF_MENS];
            char * d = mdest;
            if (destino > mens+BUF_MENS/2)
                destino = mens+BUF_MENS/2;
            for (const char * o = mens; o < destino; o++)
            {
                char ch = ((*o >> 4) & 15);
                *d++ = (ch < 10 ? ch + '0' : ch + 'a' - 10);
                ch = (*o & 15);
                *d++ = (ch < 10 ? ch + '0' : ch + 'a' - 10);
            }
            ApagarVar(v);
            return CriarVarTexto(mdest, d-mdest);
        }
        else if (conv2 == 'b') // Codifica em base64
        {
            const char cod[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz0123456789+/";
            char mdest[BUF_MENS];
            char * d = mdest;
            const char * o = mens;
            const int maximo = (BUF_MENS / 4) * 3;
            if (destino > mens + maximo)
                destino = mens + maximo;
            for (; o <= destino - 3; o += 3)
            {
                int valor = (unsigned char)o[0] * 0x10000 +
                            (unsigned char)o[1] * 0x100 +
                            (unsigned char)o[2];
                *d++ = cod[(valor >> 18) & 63];
                *d++ = cod[(valor >> 12) & 63];
                *d++ = cod[(valor >> 6) & 63];
                *d++ = cod[valor & 63];
            }
            if (o == destino - 2)
            {
                int valor = (unsigned char)o[0] * 0x400 +
                            (unsigned char)o[1] * 4;
                *d++ = cod[(valor >> 12) & 63];
                *d++ = cod[(valor >> 6) & 63];
                *d++ = cod[valor & 63];
                *d++ = '=';
            }
            else if (o == destino - 1)
            {
                int valor = (unsigned char)o[0] * 0x10;
                *d++ = cod[(valor >> 6) & 63];
                *d++ = cod[valor & 63];
                *d++ = '=';
                *d++ = '=';
            }
            ApagarVar(v);
            return CriarVarTexto(mdest, d - mdest);
        }
        else if (conv2 == 'd') // Codifica em decimal
        {
            char mdest[BUF_MENS];
            char * d = mdest + BUF_MENS;
            while (destino > mens && d > mdest + 6)
            {
                char * anota = mens;
                int valor = 0;
                for (const char * p = mens; p < destino; p++)
                {
                    valor = valor * 0x100 + *(unsigned char*)p;
                    if (valor < 1000000)
                        continue;
                    *anota++ = valor / 1000000;
                    valor %= 1000000;
                }
                destino = anota;
                d -= 6;
                d[0] = valor / 100000 % 10 + '0';
                d[1] = valor / 10000 % 10 + '0';
                d[2] = valor / 1000 % 10 + '0';
                d[3] = valor / 100 % 10 + '0';
                d[4] = valor / 10 % 10 + '0';
                d[5] = valor % 10 + '0';
            }
            for (; d < mdest+BUF_MENS; d++)
                if (*d != '0')
                    break;
            if (d == mdest + BUF_MENS)
                *(--d) = '0';
            ApagarVar(v);
            return CriarVarTexto(d, mdest+BUF_MENS-d);
        }
        else //if (conv2 == 'x') // Codifica binário
        {
            char mdest[BUF_MENS];
            char * d = mdest;
            if (destino > mens + BUF_MENS / 8)
                destino = mens + BUF_MENS / 8;
            for (const char * o = mens; o < destino; o++)
            {
                unsigned char ch = *o;
                for (int x = 0; x < 8; x++, ch <<= 1)
                    *d++ = (ch & 0x80 ? '1' : '0');
            }
            ApagarVar(v);
            return CriarVarTexto(mdest, d-mdest);
        }
    }
    return false;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtChr(TVariavel * v)
{
    char mens[2];
    int valor = 0;
    if (VarAtual >= v + 1)
        valor = v[1].getInt();
    if (valor >= 32 && valor <= 255)
        mens[0] = valor;
    else
    {
        switch (valor)
        {
        case 1: mens[0] = ex_barra_b; break;
        case 2: mens[0] = ex_barra_c; break;
        case 3: mens[0] = ex_barra_d; break;
        case 10: mens[0] = ex_barra_n; break;
        default: mens[0] = 0;
        }
    }
    mens[1] = 0;
    ApagarVar(v);
    return CriarVarTexto(mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntChr(TVariavel * v)
{
    const char * p = "";
    if (VarAtual >= v + 1)
    {
        p = v[1].getTxt();
        if (VarAtual >= v + 2)
        {
            int cont = v[2].getInt();
            while (cont > 0 && *p)
                p++,cont--;
        }
    }
    int valor;
    switch (*p)
    {
    case ex_barra_b: valor = 1; break;
    case ex_barra_c: valor = 2; break;
    case ex_barra_d: valor = 3; break;
    case ex_barra_n: valor = 10; break;
    default: valor = *(unsigned char*)p;
    }
    return CriarVarInt(v, valor);
}

//----------------------------------------------------------------------------
// Calcula a distância Levenshtein entre dois textos
static inline bool StaticIntDist(TVariavel * v, int valor)
{
// Checa quantidade de argumentos
    if (Instr::VarAtual < v + 2)
    {
        int result = 0;
        if (Instr::VarAtual >= v + 1)
            result = strlen(v[1].getTxt());
        return Instr::CriarVarInt(v, result);
    }
// Variáveis
    char texto1[1024];
    char texto2[1024];
    int  dist[1024];
    int  tam1,tam2;
// Obtém os textos
    texto1[0] = 0;
    texto2[0] = 0;
    tam1 = copiastr(texto1+1, v[1].getTxt(), sizeof(texto1) - 1) - texto1;
    tam2 = copiastr(texto2+1, v[2].getTxt(), sizeof(texto2) - 1) - texto2;
    if (valor == 0)
    {
        for (char * p = texto1 + 1; *p; p++)
            *p = tabCOMPLETO[*(unsigned char*)p];
        for (char * p = texto2 + 1; *p; p++)
            *p = tabCOMPLETO[*(unsigned char*)p];
    }
    if (valor == 1)
    {
        for (char * p = texto1 + 1; *p; p++)
            *p = tabMAI[*(unsigned char*)p];
        for (char * p = texto2 + 1; *p; p++)
            *p = tabMAI[*(unsigned char*)p];
    }
// Obtém a distância
    for (int x = 0; x < tam1; x++)
        dist[x] = x;
    for (int c2 = 1; c2 < tam2; c2++)
    {
        int antes = dist[0];
        dist[0] = c2;
        for (int c1 = 1; c1 < tam1; c1++)
        {
            int atual = antes;
            if (texto1[c1] != texto2[c2])
            {
                if (atual > dist[c1 - 1])
                    atual = dist[c1 - 1];
                if (atual > dist[c1])
                    atual = dist[c1];
                atual++;
            }
            antes = dist[c1];
            dist[c1] = atual;
        }
    }
    return Instr::CriarVarInt(v, dist[tam1 - 1]);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncIntDist(TVariavel * v) { return StaticIntDist(v, 0); }
bool InstrFunc::FuncIntDistMai(TVariavel * v) { return StaticIntDist(v, 1); }
bool InstrFunc::FuncIntDistDif(TVariavel * v) { return StaticIntDist(v, 2); }

//----------------------------------------------------------------------------
static inline bool StaticTxtProc(TVariavel * v, int valor)
{
    int posic = -1;     // Retorno - posição onde encontrou
    int ini = 0;        // Início
    while (Instr::VarAtual >= v + 2)
    {
    // Obtém ini
        if (Instr::VarAtual >= v + 3)
            ini = v[3].getInt();
        if (ini < 0)
            ini = 0;
    // Obtém o padrão
        TProcurar proc;
        if (!proc.Padrao(v[2].getTxt(), valor))
            break;
    // Obtém o texto a ser procurado
        const char * p = v[1].getTxt();
        int tam = strlen(p);
        if (ini>tam)
            break;
    // Procura
        posic = proc.Proc(p+ini, tam-ini);
        if (posic >= 0)
            posic += ini;
        break;
    }
// Retorna o resultado
    return Instr::CriarVarInt(v, posic);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtProc(TVariavel * v) { return StaticTxtProc(v, 0); }
bool InstrFunc::FuncTxtProcMai(TVariavel * v) { return StaticTxtProc(v, 1); }
bool InstrFunc::FuncTxtProcDif(TVariavel * v) { return StaticTxtProc(v, 2); }

//----------------------------------------------------------------------------
static inline bool StaticProcLin(TVariavel * v, int valor)
{
    int posic = -1;     // Retorno - posição onde encontrou
    int ini = 0;        // Início
    while (Instr::VarAtual >= v + 2)
    {
    // Obtém ini
        if (Instr::VarAtual >= v + 3)
            ini = v[3].getInt();
        if (ini < 0)
            ini = 0;
    // Obtém o padrão
        const char * ptemp1 = v[2].getTxt();
        char padrao[4096];
        unsigned int tampadrao = 0;
        switch (valor)
        {
        case 0: // Não diferencia
            for (; tampadrao < sizeof(padrao) && *ptemp1; ptemp1++)
                padrao[tampadrao++] = tabCOMPLETO[*(unsigned char*)ptemp1];
            break;
        case 1: // maiúsculas = minúsculas
            for (; tampadrao < sizeof(padrao) && *ptemp1; ptemp1++)
                padrao[tampadrao++] = tabMAI[*(unsigned char*)ptemp1];
            break;
        default: // Exato
            while (tampadrao < sizeof(padrao) && *ptemp1)
                padrao[tampadrao++] = *ptemp1++;
            break;
        }
    // Obtém o texto a ser procurado
        const char * txt = v[1].getTxt();
        int linha = ini;
        while (ini > 0 && *txt)
        {
            while (*txt && *txt != Instr::ex_barra_n)
                txt++;
            if (*txt == Instr::ex_barra_n)
                txt++;
            ini--;
        }
        if (ini > 0)
            break;
    // Procura
        if (valor == 0)
        {
            while (true)
            {
                unsigned int r;
                for (r = 0; r < tampadrao; r++)
                    if (padrao[r] != tabCOMPLETO[*(unsigned char*)(txt+r)])
                        break;
                if (r == tampadrao && (txt[r] == 0 || txt[r] == Instr::ex_barra_n))
                {
                    posic = linha;
                    break;
                }
                while (*txt && *txt != Instr::ex_barra_n)
                    txt++;
                if (*txt == 0)
                    break;
                txt++, linha++;
            }
        }
        else if (valor == 1)
        {
            while (true)
            {
                unsigned int r;
                for (r = 0; r < tampadrao; r++)
                    if (padrao[r] != tabMAI[*(unsigned char*)(txt+r)])
                        break;
                if (r == tampadrao && (txt[r] == 0 || txt[r] == Instr::ex_barra_n))
                {
                    posic = linha;
                    break;
                }
                while (*txt && *txt != Instr::ex_barra_n)
                    txt++;
                if (*txt == 0)
                    break;
                txt++, linha++;
            }
        }
        else
        {
            while (true)
            {
                if (memcmp(txt, padrao, tampadrao) == 0 &&
                        (txt[tampadrao] == 0 || txt[tampadrao] == Instr::ex_barra_n))
                {
                    posic = linha;
                    break;
                }
                while (*txt && *txt != Instr::ex_barra_n)
                    txt++;
                if (*txt == 0)
                    break;
                txt++, linha++;
            }
        }
        break;
    }
// Retorna o resultado
    return Instr::CriarVarInt(v, posic);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtProcLin(TVariavel * v) { return StaticProcLin(v, 0); }
bool InstrFunc::FuncTxtProcLinMai(TVariavel * v) { return StaticProcLin(v, 1); }
bool InstrFunc::FuncTxtProcLinDif(TVariavel * v) { return StaticProcLin(v, 2); }

//----------------------------------------------------------------------------
/// Troca texto (txttroca)
static inline bool StaticTxtTroca(TVariavel * v, int valor)
{
    if (Instr::VarAtual < v + 3)
        return false;
// Obtém o padrão
    TProcurar proc;
    if (Instr::DadosTopo + 10 >= Instr::DadosFim ||
        !proc.Padrao(v[2].getTxt(), valor))
    {
        Instr::ApagarVar(v + 2);
        Instr::ApagarRet(v);
        return true;
    }
// Obtém o novo texto
    copiastr(proc.troca, v[3].getTxt(), sizeof(proc.troca));
// Prepara variáveis
    Instr::ApagarVar(v + 2);
    if (!Instr::CriarVar(Instr::InstrTxtFixo))
        return false;
    Instr::VarAtual->endvar = Instr::DadosTopo;
    proc.dest = Instr::DadosTopo;
    proc.tamdest = Instr::DadosFim - Instr::DadosTopo;
// Troca
    const char * p = v[1].getTxt();
    proc.Proc(p, strlen(p));
    Instr::VarAtual->tamanho = strlen(Instr::DadosTopo) + 1;
    Instr::DadosTopo += Instr::VarAtual->tamanho;
    Instr::ApagarRet(v);
    return true;
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtTroca(TVariavel * v) { return StaticTxtTroca(v, 0); }
bool InstrFunc::FuncTxtTrocaMai(TVariavel * v) { return StaticTxtTroca(v, 1); }
bool InstrFunc::FuncTxtTrocaDif(TVariavel * v) { return StaticTxtTroca(v, 2); }

//----------------------------------------------------------------------------
bool InstrFunc::FuncTxtSepara(TVariavel * v)
{
    const char * txt = "";  // Texto
    char mens[BUF_MENS];    // Resultado
    char separador[1024];   // Texto usado como separador
    char * destino = mens;
    unsigned int mask[64];       // bits 15-8=antes, bits 7-0=depois
    unsigned int nummask = 0;
    unsigned int tipoch = 1;  // Tipo de caracter conforme máscara
// Obtém separador e máscara de bits (opções)
    separador[0] = ' ', separador[1] = 0;
    if (VarAtual >= v + 2)
    {
        if (VarAtual >= v + 3)
            copiastr(separador, v[3].getTxt(), sizeof(separador));
        const char * opc = v[2].getTxt();
        mask[nummask] = 0;
        int valormask = 0;
        while (*opc)
            switch (*opc++)
            {
            case 'V':
            case 'v': valormask |= 0x01; break;
            case 'N':
            case 'n': valormask |= 0x02; break;
            case 'D':
            case 'd': valormask |= 0x04; break;
            case 'E':
            case 'e': valormask |= 0x08; break;
            case 'L':
            case 'l': valormask |= 0x10; break;
            case 'O':
            case 'o': valormask |= 0x20; break;
            case '+': valormask <<= 8; break;
            case ',':
                if ((valormask & 0xFF) && (valormask & 0xFF00) &&
                        nummask < sizeof(mask) / sizeof(mask[0]))
                    mask[nummask++] = valormask & 0xFFFF;
                valormask = 0;
                break;
            }
        if ((valormask & 0xFF) && (valormask & 0xFF00) &&
                nummask < sizeof(mask) / sizeof(mask[0]))
            mask[nummask++] = valormask;
    }
// Texto original
    if (VarAtual >= v + 1)
        txt = v[1].getTxt();
// Obtém o texto
    const char * pnum = txt-1; // Para detectar número
    while (true)
    {
    // Atualiza tipo de caracter
        tipoch = ((tipoch & ~0x202) << 8) |
                tabTXTSEPARA[*(unsigned char*)txt];
    // Checa início/fim de número
        if (pnum <= txt)
        {
            if (pnum == txt)
                tipoch |= 0x200;
        // Se for número, avança até o fim do número
            if ((txt[0] == '-' && txt[1] >= '0' && txt[1] <= '9') || (tipoch & 4))
            {
                tipoch |= 2;
                pnum = txt+1;
                while (*pnum >= '0' && *pnum <= '9')
                    pnum++;
                if (pnum[0] == '.' && pnum[1] >= '0' && pnum[1] <= '9')
                {
                    pnum+=2;
                    while (*pnum >= '0' && *pnum <= '9')
                        pnum++;
                }
            }
        }
    // Checa se deve acrescentar separador
        for (unsigned int x = 0; x < nummask; x++)
        {
            int valormask = mask[x] & tipoch;
            if ((valormask & 0xFF) == 0 || (valormask & 0xFF00) == 0)
                continue;
            const char * p = separador;
            while (*p && destino < mens + sizeof(mens))
                *destino++ = *p++;
            break;
        }
    // Anota texto
        if (*txt == 0 || destino >= mens+sizeof(mens))
            break;
        *destino++ = *txt++;
    }
// Retorna o texto
    ApagarVar(v);
    return CriarVarTexto(mens, destino-mens);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncObjAntes(TVariavel * v)
{
    if (VarAtual < v + 1)
        return false;
    TObjeto * obj = v[1].getObj();
    if (obj == nullptr)
        return false;
    obj = obj->Antes;
    ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncObjDepois(TVariavel * v)
{
    if (VarAtual < v + 1)
        return false;
    TObjeto * obj = v[1].getObj();
    if (obj == nullptr)
        return false;
    obj = obj->Depois;
    ApagarVar(v);
    return Instr::CriarVarObj(obj);
}

//----------------------------------------------------------------------------
bool InstrFunc::FuncTotal(TVariavel * v)
{
    int tamanho = 0;
    TObjeto * obj = nullptr;
    for (TVariavel * var = v + 1; var <= VarAtual && obj == nullptr; var++)
        switch (var->Tipo())
        {
        case varOutros:
            if (var->defvar[endVetor])
                tamanho += (unsigned char)var->defvar[endVetor];
        case varInt:
        case varDouble:
            break;
        case varTxt:
            tamanho += strlen(var->getTxt());
            break;
        case varObj:
            obj = var->getObj();
            if (obj)
                tamanho += obj->Classe->NumObj;
            break;
        }
    return CriarVarInt(v, tamanho);
}
