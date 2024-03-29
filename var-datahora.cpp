/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos da licen�a LGPL. Vide arquivo COPYING.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the LGPL license. See file COPYING.
 */

// Nota - para informa��es sobre o calend�rio vide:
// http://www.tondering.dk/claus/calendar.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifdef __WIN32__
 #include <windows.h>
#endif
#include "var-datahora.h"
#include "variavel.h"
#include "variavel-def.h"
#include "instr.h"
#include "misc.h"

#define BISSEXTO(x) ((x) % 4 ? 0 : (x) % 100 ? 1 : (x) % 400 ? 0 : 1)

//------------------------------------------------------------------------------
// Teste das fun��es NumData e DataNum
#if 0
class TTeste1
{
public:
    TTeste1()
    {
        TVarDataHora v;
        for (int x = 0; x < 4000111; x++)
        {
            v.Ano = v.Mes = v.Dia = 0;
            v.NumData(x);
            int y = v.DataNum();
            if (x==y)
                continue;
            printf("Erro x=%d y=%d\n", x, y);
            exit(1);
        }
        puts("OK");
        exit(0);
    }
} t;
#endif

//------------------------------------------------------------------------------
enum DataHoraTipo
{
DataHoraAno = 1,
DataHoraMes,
DataHoraDia,
DataHoraHora,
DataHoraMin,
DataHoraSeg,
DataHoraNumDia,
DataHoraNumSeg,
DataHoraNumTotal
};

//----------------------------------------------------------------------------
const TVarInfo * TVarDataHora::Inicializa()
{
    static TVarInfo::FuncItem ListaFuncEnd[] = {
        { "agora",      &TVarDataHora::FuncAgora },
        { "ano",        &TVarDataHora::FuncAno },
        { "antes",      &TVarDataHora::FuncAntes },
        { "bissexto",   &TVarDataHora::FuncBissexto },
        { "depois",     &TVarDataHora::FuncDepois },
        { "dia",        &TVarDataHora::FuncDia },
        { "diasem",     &TVarDataHora::FuncDiaSem },
        { "hora",       &TVarDataHora::FuncHora },
        { "mes",        &TVarDataHora::FuncMes },
        { "min",        &TVarDataHora::FuncMin },
        { "novadata",   &TVarDataHora::FuncNovaData },
        { "novahora",   &TVarDataHora::FuncNovaHora },
        { "numdias",    &TVarDataHora::FuncNumDias },
        { "numseg",     &TVarDataHora::FuncNumSeg },
        { "numtotal",   &TVarDataHora::FuncNumTotal },
        { "seg",        &TVarDataHora::FuncSeg } };
    static const TVarInfo var(
        FTamanho,
        FTamanhoVetor,
        FTipo,
        FRedim,
        FMoverEnd,
        TVarInfo::FMoverDef0,
        FGetBool,
        FGetInt,
        FGetDouble,
        FGetTxt,
        TVarInfo::FGetObjNulo,
        FOperadorAtrib,
        TVarInfo::FOperadorAddFalse,
        FOperadorIgual2,
        FOperadorCompara,
        TVarInfo::FFuncTextoFalse,
        TVarInfo::FFuncVetorFalse,
        ListaFuncEnd,
        sizeof(ListaFuncEnd) / sizeof(ListaFuncEnd[0]) - 1);
    return &var;
}

//------------------------------------------------------------------------------
void TVarDataHora::Mover(TVarDataHora * destino)
{
    memmove(destino, this, sizeof(TVarDataHora));
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncAgora(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
#ifdef __WIN32__
    SYSTEMTIME lt = {};
    GetLocalTime(&lt);
    ref->Ano = lt.wYear;
    ref->Mes = lt.wMonth;
    ref->Dia = lt.wDay;
    ref->Hora = lt.wHour;
    ref->Min = lt.wMinute;
    ref->Seg = lt.wSecond;
#else
    // Nota: localtime() Converte para representa��o local de tempo
    struct tm * tempolocal;
    time_t tempoatual;
    time(&tempoatual);
    tempolocal = localtime(&tempoatual);
    ref->Ano = tempolocal->tm_year + 1900; // Ano come�a no 1900
    ref->Mes = tempolocal->tm_mon + 1; // M�s come�a no 1
    ref->Dia = tempolocal->tm_mday; // Dia come�a no 0
    ref->Hora = tempolocal->tm_hour;
    ref->Min = tempolocal->tm_min;
    ref->Seg = tempolocal->tm_sec;
#endif
    return false;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncAno(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraAno;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncAntes(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    if (ref->Dia > 1)
        ref->Dia--;
    else if (ref->Mes > 1)
        ref->Mes--, ref->Dia = ref->DiasMes();
    else if (ref->Ano > 1)
        ref->Dia = 31, ref->Mes = 12, ref->Ano--;
    return false;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncBissexto(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    return Instr::CriarVarInt(v, BISSEXTO(ref->Ano));
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncDepois(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    if (ref->Dia < ref->DiasMes())
        ref->Dia++;
    else if (ref->Mes < 12)
        ref->Dia = 1, ref->Mes++;
    else if (ref->Ano < 9999)
        ref->Dia = 1, ref->Mes = 1, ref->Ano++;
    return false;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncDia(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraDia;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncDiaSem(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    return Instr::CriarVarInt(v, (ref->DataNum() + 1) % 7);
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncHora(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraHora;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncMes(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraMes;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncMin(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraMin;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncNovaData(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    if (Instr::VarAtual - v >= 1)
    {
        int x = v[1].getInt();
        ref->Ano = (x < 1 ? 1 : x > 9999 ? 9999 : x);
    }
    if (Instr::VarAtual - v >= 2)
    {
        int x = v[2].getInt();
        ref->Mes = (x < 1 ? 1 : x > 12 ? 12 : x);
    }
    if (Instr::VarAtual - v >= 3)
    {
        int x = v[3].getInt();
        ref->Dia = (x < 1 ? 1 : x > 31 ? 31 : x);
    }
    int valor = ref->DiasMes();
    if (ref->Dia > valor)
        ref->Dia = valor;
    return false;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncNovaHora(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    if (Instr::VarAtual - v >= 1)
    {
        int x = v[1].getInt();
        ref->Hora = (x < 0 ? 0 : x > 23 ? 23 : x);
    }
    if (Instr::VarAtual - v >= 2)
    {
        int x = v[2].getInt();
        ref->Min = (x < 0 ? 0 : x > 59 ? 59 : x);
    }
    if (Instr::VarAtual - v >= 3)
    {
        int x = v[3].getInt();
        ref->Seg = (x < 0 ? 0 : x > 59 ? 59 : x);
    }
    return false;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncNumDias(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraNumDia;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncNumSeg(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraNumSeg;
    return true;
}

//----------------------------------------------------------------------------
bool TVarDataHora::FuncNumTotal(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraNumTotal;
    return true;
}
//----------------------------------------------------------------------------
bool TVarDataHora::FuncSeg(TVariavel * v)
{
    Instr::ApagarVar(v + 1);
    Instr::VarAtual->numfunc = DataHoraSeg;
    return true;
}

//------------------------------------------------------------------------------
int TVarDataHora::getInt(int numfunc)
{
    switch (numfunc)
    {
    case DataHoraAno: return Ano;
    case DataHoraMes: return Mes;
    case DataHoraDia: return Dia;
    case DataHoraHora: return Hora;
    case DataHoraMin: return Min;
    case DataHoraSeg: return Seg;
    case DataHoraNumDia: return DataNum();
    case DataHoraNumSeg: return (Hora * 60 + Min) * 60 + Seg;
    case DataHoraNumTotal:
        return DoubleToInt((Hora * 60 + Min) * 60 + Seg + DataNum() * 86400.0);
    }
    return 0;
}

//------------------------------------------------------------------------------
double TVarDataHora::getDouble(int numfunc)
{
    if (numfunc == 0)
        return 0;
    if (numfunc != DataHoraNumTotal)
        return getInt(numfunc);
    return (Hora * 60 + Min) * 60 + Seg + DataNum() * 86400.0;
}

//------------------------------------------------------------------------------
void TVarDataHora::LerSav(const char * texto)
{
    int x, y;
    for (x = 0; x < 14; x++)
        if (texto[x] < '0' || texto[x] > '9')
            return;
    x = (texto[0] - '0') * 1000 + (texto[1] - '0') * 100 +
        (texto[2] - '0') * 10 + texto[3] - '0';
    Ano = (x < 1 ? 1 : x > 9999 ? 9999 : x);
    x = (texto[4] - '0') * 10 + texto[5] - '0';
    Mes = (x < 1 ? 1 : x > 12 ? 12 : x);
    x = (texto[6] - '0') * 10 + texto[7] - '0';
    y = DiasMes();
    Dia = (x < 1 ? 1 : x > y ? y : x);
    x = (texto[8] - '0') * 10 + texto[9] - '0';
    Hora = (x > 23 ? 23 : x);
    x = (texto[10] - '0') * 10 + texto[11] - '0';
    Min = (x>59 ? 59 : x);
    x = (texto[12] - '0') * 10 + texto[13] - '0';
    Seg = (x>59 ? 59 : x);
}

//------------------------------------------------------------------------------
void TVarDataHora::SalvarSav(char * texto)
{
    texto[0] = Ano / 1000 + '0';
    texto[1] = Ano / 100 % 10 + '0';
    texto[2] = Ano / 10 % 10 + '0';
    texto[3] = Ano % 10 + '0';
    texto[4] = Mes / 10 + '0';
    texto[5] = Mes % 10 + '0';
    texto[6] = Dia / 10 + '0';
    texto[7] = Dia % 10 + '0';
    texto[8] = Hora / 10 + '0';
    texto[9] = Hora % 10 + '0';
    texto[10] = Min / 10 + '0';
    texto[11] = Min % 10 + '0';
    texto[12] = Seg / 10 + '0';
    texto[13] = Seg % 10 + '0';
    texto[14] = 0;
}

//------------------------------------------------------------------------------
int TVarDataHora::DataNum()
{
    const int DiasMesTabela[]={
                 0,  31,  31+28,  31+28+31,      // J F M A
                 31+28+31+30,                    // M
                 31+28+31+30+31,                 // J
                 31+28+31+30+31+30,              // J
                 31+28+31+30+31+30+31,           // A
                 31+28+31+30+31+30+31+31,        // S
                 31+28+31+30+31+30+31+31+30,     // O
                 31+28+31+30+31+30+31+31+30+31,  // N
                 31+28+31+30+31+30+31+31+30+31+30 }; // D

    int valor = Dia - 1;

// Obt�m dias conforme meses decorridos
    valor += DiasMesTabela[Mes - 1];
    if (Mes > 2)
        valor += BISSEXTO(Ano);

// Obt�m dias conforme anos decorridos
    int a = Ano - 1;
    valor += a * 1461 / 4 - a / 100 + a / 400;
    // ou:  valor += a * 365 + a / 4 - a / 100 + a / 400;
    // ou:  valor += a * 1461 / 4 - (a / 100 * 3 + 3) / 4;
    return valor;
}

//------------------------------------------------------------------------------
void TVarDataHora::NumData(int dias)
{
// Os anos se repetem a cada 400 anos
// Anos divis�veis por 100 s�o bissextos se forem divis�veis por 400
//     400 anos = 365*400 + 100 - 3 dias = 146097 dias
// Anos 1-100, 101-200, 201-300 cont�m 36524 dias
// Anos 301-400 cont�m 36525 dias

// Teste:
// for i in 0 36523 36524 73047 73048 109571 109572 146096 146097
//   do echo $i $[(i*4+3)/146097]
// done
// for ((i=0; i<=4; i++)); do echo $i $[i*146097/4] ; done
    int a = (dias * 4 + 3) / 146097;
    dias -= a * 146097 / 4;

// Dentro de 100 anos: A cada 4 anos, 1 � bissexto
// 1,2,3, 5,6,7 n�o s�o, 4,8 s�o, etc.
// O �ltimo pode ou n�o ser bissexto

// Teste:
// for i in 0 364 365 729 730 1094 1095 1460 1461
//   do echo $i $[(i*4+3)/1461]
// done
// for ((i=0; i<=4; i++)); do echo $i $[i*1461/4] ; done
    int b = (dias * 4 + 3) / 1461;
    dias -= b * 1461 / 4;

// Anota o ano
    Ano = a * 100 + b + 1;

// Verifica se � bissexto
    int bissexto = (b % 4 != 3 ? 0 : b != 99 ? 1 : a % 4 == 3);

// Simula fevereiro com 30 dias na contagem de dias
    if (dias >= 31 + 28 + bissexto)
        dias += 2 - bissexto;

// O ciclo se repete a cada 7 meses, totalizando 214 dias:
// 31+30+31+30+31+30+31
    int x = (dias * 7 + 3) / 214;
    Mes = x + 1; // M�s come�a no 1

// A f�rmula que funcionou para obter o dia � essa
// Teste: for ((x=0; x<=12; x++)); do echo $x $[(x*214+3)/7] ; done
    Dia = 1 + dias - (x * 214 + 3) / 7;
}

//------------------------------------------------------------------------------
int TVarDataHora::DiasMes()
{
    if (Mes == 2)
        return (Ano % 4 ? 28 : Ano % 100 ? 29 : Ano % 400 ? 28 : 29);
    if (Mes < 8)
        return 30 + Mes % 2;
    return 31 - Mes % 2;
}

//------------------------------------------------------------------------------
int TVarDataHora::FTamanho(const char * instr)
{
    return sizeof(TVarDataHora);
}

//------------------------------------------------------------------------------
int TVarDataHora::FTamanhoVetor(const char * instr)
{
    int total = (unsigned char)instr[Instr::endVetor];
    return (total ? total : 1) * sizeof(TVarDataHora);
}

//------------------------------------------------------------------------------
TVarTipo TVarDataHora::FTipo(TVariavel * v)
{
    switch (v->numfunc)
    {
    case 0: return varOutros;
    case DataHoraNumTotal: return varDouble;
    default: return varInt;
    }
}

//------------------------------------------------------------------------------
void TVarDataHora::FRedim(TVariavel * v, TClasse * c, TObjeto * o,
        unsigned int antes, unsigned int depois)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar);
    for (; antes < depois; antes++)
    {
        TVarDataHora * ref2 = ref + antes;
        ref2->Ano = 1, ref2->Mes = 1, ref2->Dia = 1;
        ref2->Hora = 0, ref2->Min = 0, ref2->Seg = 0;
    }
}

//------------------------------------------------------------------------------
void TVarDataHora::FMoverEnd(TVariavel * v, void * destino, TClasse * c, TObjeto * o)
{
    VARIAVEL_MOVER_SIMPLES(TVarDataHora)
}

//------------------------------------------------------------------------------
bool TVarDataHora::FGetBool(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar) + v->indice;
    return ref->getInt(v->numfunc);
}

//------------------------------------------------------------------------------
int TVarDataHora::FGetInt(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar) + v->indice;
    return ref->getInt(v->numfunc);
}

//------------------------------------------------------------------------------
double TVarDataHora::FGetDouble(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar) + v->indice;
    return ref->getDouble(v->numfunc);
}

//------------------------------------------------------------------------------
const char * TVarDataHora::FGetTxt(TVariavel * v)
{
    TVarDataHora * ref = reinterpret_cast<TVarDataHora*>(v->endvar) + v->indice;
    char * buf = TVarInfo::BufferTxt();
    if (FTipo(v) == varDouble)
        sprintf(buf, "%.0f", ref->getDouble(v->numfunc));
    else
        sprintf(buf, "%d", ref->getInt(v->numfunc));
    return buf;
}

//------------------------------------------------------------------------------
void TVarDataHora::FOperadorAtrib(TVariavel * v1, TVariavel * v2)
{
    TVarDataHora * r1 = reinterpret_cast<TVarDataHora*>(v1->endvar) + v1->indice;
    switch (v1->numfunc)
    {
    case DataHoraAno:
        {
            int valor = v2->getInt();
            r1->Ano = (valor < 1 ? 1 : valor > 9999 ? 9999 : valor);
            if (r1->Mes == 2 && r1->Dia == 29 && !BISSEXTO(r1->Ano))
                r1->Dia--;
            break;
        }
    case DataHoraMes:
        {
            int valor = v2->getInt();
            r1->Mes = (valor < 1 ? 1 : valor > 12 ? 12 : valor);
            int x = r1->DiasMes();
            if (r1->Dia > x)
                r1->Dia = x;
            break;
        }
    case DataHoraDia:
        {
            int valor = v2->getInt();
            if (valor < 1)
                r1->Dia = 1;
            else
            {
                int x = r1->DiasMes();
                r1->Dia = (valor < x ? valor : x);
            }
            break;
        }
    case DataHoraHora:
        {
            int valor = v2->getInt();
            r1->Hora = (valor < 0 ? 0 : valor > 23 ? 23 : valor);
            break;
        }
    case DataHoraMin:
        {
            int valor = v2->getInt();
            r1->Min = (valor < 0 ? 0 : valor > 59 ? 59 : valor);
            break;
        }
    case DataHoraSeg:
        {
            int valor = v2->getInt();
            r1->Seg = (valor < 0 ? 0 : valor > 59 ? 59 : valor);
            break;
        }
    case DataHoraNumDia:
        {
            int valor = v2->getInt();
            if (valor < 0) // 1/1/1
                valor = 0;
            if (valor > 3652058) // 31/12/9999
                valor = 3652058;
            r1->NumData(valor);
            break;
        }
    case DataHoraNumTotal:
        {
            double valor = v2->getDouble();
            if (!(valor > 0)) // Nota: double pode ser NaN
            {
                r1->NumData(0);
                r1->Seg = 0, r1->Min = 0, r1->Hora = 0;
                break;
            }
            double data = trunc(valor / 86400.0);
            r1->NumData(DoubleToInt(data));
            int hns = (int)nearbyint(valor - data * 86400.0);
            if (hns < 0)
                hns = 0;
            if (hns > 24 * 60 * 60 - 1)
                hns = 24 * 60 * 60 - 1;
            r1->Seg = hns % 60;
            hns /= 60;
            r1->Min = hns % 60;
            r1->Hora = hns / 60;
            break;
        }
    case DataHoraNumSeg:
        {
            int valor = v2->getInt();
            if (valor < 0)
                valor = 0;
            if (valor > 24 * 60 * 60 - 1)
                valor = 24 * 60 * 60 - 1;
            r1->Seg = valor % 60;
            valor /= 60;
            r1->Min = valor % 60;
            r1->Hora = valor / 60;
            break;
        }
    default:
        if (v1->defvar[2] == v2->defvar[2])
        {
            TVarDataHora * r2 = reinterpret_cast<TVarDataHora*>(v2->endvar) + v2->indice;
            if (r1 != r2)
                *r1 = *r2;
        }
        break;
    }
}

//------------------------------------------------------------------------------
bool TVarDataHora::FOperadorIgual2(TVariavel * v1, TVariavel * v2)
{
    TVarDataHora * ref1 = reinterpret_cast<TVarDataHora*>(v1->endvar) + v1->indice;
    if (v1->numfunc == 0)
        return v2->TipoNumerico() && ref1->getDouble(v1->numfunc) == v2->getDouble();
    if (v1->defvar[2] != v2->defvar[2])
        return false;
    TVarDataHora * ref2 = reinterpret_cast<TVarDataHora*>(v2->endvar) + v2->indice;
    return ref1->Ano == ref2->Ano && ref1->Mes == ref2->Mes &&
        ref1->Dia == ref2->Dia && ref1->Hora == ref2->Hora &&
        ref1->Min == ref2->Min && ref1->Seg == ref2->Seg;
}

//------------------------------------------------------------------------------
unsigned char TVarDataHora::FOperadorCompara(TVariavel * v1, TVariavel * v2)
{
    TVarDataHora * ref1 = reinterpret_cast<TVarDataHora*>(v1->endvar) + v1->indice;
    if (v1->numfunc == DataHoraNumTotal)
        return TVarInfo::ComparaDouble(ref1->getDouble(v1->numfunc), v2);
    if (v1->numfunc != 0)
        return TVarInfo::ComparaInt(ref1->getDouble(v1->numfunc), v2);
    if (v1->defvar[2] != v2->defvar[2])
        return 0;
    TVarDataHora * ref2 = reinterpret_cast<TVarDataHora*>(v2->endvar) + v2->indice;
    if (ref1->Ano  != ref2->Ano)  return (ref1->Ano  < ref2->Ano  ? 1 : 4);
    if (ref1->Mes  != ref2->Mes)  return (ref1->Mes  < ref2->Mes  ? 1 : 4);
    if (ref1->Dia  != ref2->Dia)  return (ref1->Dia  < ref2->Dia  ? 1 : 4);
    if (ref1->Hora != ref2->Hora) return (ref1->Hora < ref2->Hora ? 1 : 4);
    if (ref1->Min  != ref2->Min)  return (ref1->Min  < ref2->Min  ? 1 : 4);
    if (ref1->Seg  != ref2->Seg)  return (ref1->Seg  < ref2->Seg  ? 1 : 4);
    return 2;
}
