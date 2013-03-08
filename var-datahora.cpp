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

// Nota - para informações sobre o calendário vide:
// http://www.tondering.dk/claus/calendar.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "var-datahora.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#define BISSEXTO(x) ((x)%4 ? 0 : (x)%100 ? 1 : (x)%400 ? 0 : 1)

//------------------------------------------------------------------------------
// Teste das funções NumData e DataNum
#if 0
class TTeste1
{
public:
    TTeste1()
    {
        TVarDataHora v;
        for (int x=0; x<4000111; x++)
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

//------------------------------------------------------------------------------
void TVarDataHora::Criar()
{
    Ano=1, Mes=1, Dia=1;
    Hora=0, Min=0, Seg=0;
}

//------------------------------------------------------------------------------
void TVarDataHora::Mover(TVarDataHora * destino)
{
    memmove(destino, this, sizeof(TVarDataHora));
}

//------------------------------------------------------------------------------
int TVarDataHora::Compara(TVarDataHora * v)
{
    if (Ano  != v->Ano)  return (Ano  < v->Ano  ? -1 : 1);
    if (Mes  != v->Mes)  return (Mes  < v->Mes  ? -1 : 1);
    if (Dia  != v->Dia)  return (Dia  < v->Dia  ? -1 : 1);
    if (Hora != v->Hora) return (Hora < v->Hora ? -1 : 1);
    if (Min  != v->Min)  return (Min  < v->Min  ? -1 : 1);
    if (Seg  != v->Seg)  return (Seg  < v->Seg  ? -1 : 1);
    return 0;
}

//------------------------------------------------------------------------------
void TVarDataHora::Igual(TVarDataHora * v)
{
    Ano  = v->Ano;
    Mes  = v->Mes;
    Dia  = v->Dia;
    Hora = v->Hora;
    Min  = v->Min;
    Seg  = v->Seg;
}

//------------------------------------------------------------------------------
bool TVarDataHora::Func(TVariavel * v, const char * nome)
{
// Lista das funções de datahora
// Deve obrigatoriamente estar em ordem alfabética
    const char * DataHoraFunc[] = {
            "agora",
            "ano",
            "antes",
            "bissexto",
            "depois",
            "dia",
            "diasem",
            "hora",
            "mes",
            "min",
            "novadata",
            "novahora",
            "numdias",
            "numseg",
            "numtotal",
            "seg" };
// Procura a função correspondente
    int ini = 0;
    int fim = sizeof(DataHoraFunc) / sizeof(DataHoraFunc[0]) - 1;
    int meio;
    while (true)
    {
        if (ini > fim)
            return false;
        meio = (ini+fim)/2;
        int resultado = comparaZ(nome, DataHoraFunc[meio]);
        if (resultado==0) // Se encontrou...
            break;
        if (resultado<0)
            fim = meio-1;
        else
            ini = meio+1;
    }
// Encontrou
    switch (meio)
    {
    case 0: // Agora
        {
            time_t tempoatual;
            struct tm * tempolocal;
            time(&tempoatual);
            // localtime() Converte para representação local de tempo
            tempolocal=localtime(&tempoatual);
            Ano = tempolocal->tm_year + 1900; // Ano começa no 1900
            Mes = tempolocal->tm_mon + 1; // Mês começa no 1
            Dia = tempolocal->tm_mday; // Dia começa no 0
            Hora = tempolocal->tm_hour;
            Min = tempolocal->tm_min;
            Seg = tempolocal->tm_sec;
            return false;
        }
    case 1: // Ano
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraAno;
        return true;
    case 2: // Antes
        if (Dia>1)
            Dia--;
        else if (Mes>1)
            Mes--, Dia = DiasMes();
        else if (Ano>1)
            Dia=31, Mes=12, Ano--;
        return false;
    case 3: // Bissexto
        ini = BISSEXTO(Ano);
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(ini);
    case 4: // Depois
        if (Dia < DiasMes())
            Dia++;
        else if (Mes < 12)
            Dia=1, Mes++;
        else if (Ano < 9999)
            Dia=1, Mes=1, Ano++;
        return false;
    case 5: // Dia
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraDia;
        return true;
    case 6: // DiaSem
        ini = (DataNum() + 1) % 7;
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(ini);
    case 7: // Hora
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraHora;
        return true;
    case 8: // Mes
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraMes;
        return true;
    case 9: // Min
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraMin;
        return true;
    case 10: // NovaData
        if (Instr::VarAtual - v >= 1)
        {
            int x = v[1].getInt();
            Ano = (x<1 ? 1 : x>9999 ? 9999 : x);
        }
        if (Instr::VarAtual - v >= 2)
        {
            int x = v[2].getInt();
            Mes = (x<1 ? 1 : x>12 ? 12 : x);
        }
        if (Instr::VarAtual - v >= 3)
        {
            int x = v[3].getInt();
            Dia = (x<1 ? 1 : x>31 ? 31 : x);
        }
        ini = DiasMes();
        if (Dia > ini)
            Dia = ini;
        return false;
    case 11: // NovaHora
        if (Instr::VarAtual - v >= 1)
        {
            int x = v[1].getInt();
            Hora = (x<0 ? 0 : x>23 ? 23 : x);
        }
        if (Instr::VarAtual - v >= 2)
        {
            int x = v[2].getInt();
            Min = (x<0 ? 0 : x>59 ? 59 : x);
        }
        if (Instr::VarAtual - v >= 3)
        {
            int x = v[3].getInt();
            Seg = (x<0 ? 0 : x>59 ? 59 : x);
        }
        return false;
    case 12: // NumDia
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraNumDia;
        return true;
    case 13: // NumSeg
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraNumSeg;
        return true;
    case 14: // NumTotal
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraNumTotal;
        return true;
    case 15: // Seg
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = DataHoraSeg;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
int TVarDataHora::getTipo(int numfunc)
{
    switch (numfunc)
    {
    case 0: return varOutros;
    case DataHoraNumTotal: return varDouble;
    default: return varInt;
    }
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
    case DataHoraNumSeg: return (Hora*60+Min)*60+Seg;
    case DataHoraNumTotal:
        return DoubleToInt((Hora*60+Min)*60+Seg + DataNum() * 86400.0);
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
    return (Hora*60+Min)*60+Seg + DataNum() * 86400.0;
}

//------------------------------------------------------------------------------
void TVarDataHora::setInt(int numfunc, int valor)
{
    int x;
    switch (numfunc)
    {
    case DataHoraAno:
        Ano = (valor<1 ? 1 : valor>9999 ? 9999 : valor);
        if (Mes==2 && Dia==29 && !BISSEXTO(Ano))
            Dia--;
        break;
    case DataHoraMes:
        Mes = (valor<1 ? 1 : valor>12 ? 12 : valor);
        x = DiasMes();
        if (Dia > x)
            Dia = x;
        break;
    case DataHoraDia:
        if (valor<1)
            Dia=1;
        else
        {
            x = DiasMes();
            Dia = (valor<x ? valor : x);
        }
        break;
    case DataHoraHora:
        Hora = (valor<0 ? 0 : valor>23 ? 23 : valor);
        break;
    case DataHoraMin:
        Min = (valor<0 ? 0 : valor>59 ? 59 : valor);
        break;
    case DataHoraSeg:
        Seg = (valor<0 ? 0 : valor>59 ? 59 : valor);
        break;
    case DataHoraNumDia:
        if (valor < 0) // 1/1/1
            valor = 0;
        if (valor > 3652058) // 31/12/9999
            valor = 3652058;
        NumData(valor);
        break;
    case DataHoraNumTotal:
        if (valor <= 0)
        {
            NumData(0);
            Seg = 0, Min = 0, Hora = 0;
            break;
        }
        NumData(valor/86400);
        valor %= 86400;
        // Muda hora/minuto/segundo em seguida
    case DataHoraNumSeg:
        if (valor < 0)
            valor = 0;
        if (valor > 24*60*60-1)
            valor = 24*60*60-1;
        Seg = valor%60;
        valor /= 60;
        Min = valor%60;
        Hora = valor/60;
        break;
    }
}

//------------------------------------------------------------------------------
void TVarDataHora::setDouble(int numfunc, double valor)
{
    if (numfunc!=DataHoraNumTotal || valor<=0)
    {
        setInt(numfunc, DoubleToInt(valor));
        return;
    }
    double data = trunc(valor / 86400.0);
    valor -= data * 86400.0;
    NumData(DoubleToInt(data));
    setInt(DataHoraNumSeg, (int)nearbyint(valor));
}

//------------------------------------------------------------------------------
void TVarDataHora::LerSav(const char * texto)
{
    int x, y;
    for (x=0; x<14; x++)
        if (texto[x]<'0' || texto[x]>'9')
            return;
    x = (texto[0] - '0') * 1000 + (texto[1] - '0') * 100 +
        (texto[2] - '0') * 10 + texto[3] - '0';
    Ano = (x<1 ? 1 : x>9999 ? 9999 : x);
    x = (texto[4] - '0') * 10 + texto[5] - '0';
    Mes = (x<1 ? 1 : x>12 ? 12 : x);
    x = (texto[6] - '0') * 10 + texto[7] - '0';
    y = DiasMes();
    Dia = (x<1 ? 1 : x>y ? y : x);
    x = (texto[8] - '0') * 10 + texto[9] - '0';
    Hora = (x>23 ? 23 : x);
    x = (texto[10] - '0') * 10 + texto[11] - '0';
    Min = (x>59 ? 59 : x);
    x = (texto[12] - '0') * 10 + texto[13] - '0';
    Seg = (x>59 ? 59 : x);
}

//------------------------------------------------------------------------------
void TVarDataHora::SalvarSav(char * texto)
{
    texto[0] = Ano/1000+'0';
    texto[1] = Ano/100%10+'0';
    texto[2] = Ano/10%10+'0';
    texto[3] = Ano%10+'0';
    texto[4] = Mes/10+'0';
    texto[5] = Mes%10+'0';
    texto[6] = Dia/10+'0';
    texto[7] = Dia%10+'0';
    texto[8] = Hora/10+'0';
    texto[9] = Hora%10+'0';
    texto[10] = Min/10+'0';
    texto[11] = Min%10+'0';
    texto[12] = Seg/10+'0';
    texto[13] = Seg%10+'0';
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

// Obtém dias conforme meses decorridos
    valor += DiasMesTabela[Mes-1];
    if (Mes>2)
        valor += BISSEXTO(Ano);

// Obtém dias conforme anos decorridos
    int a = Ano - 1;
    valor += a*1461/4 - a/100 + a/400;
    // ou:  valor += a*365 + a/4 - a/100 + a/400;
    // ou:  valor += a*1461/4 - (a/100*3+3)/4;
    return valor;
}

//------------------------------------------------------------------------------
void TVarDataHora::NumData(int dias)
{
// Os anos se repetem a cada 400 anos
// Anos divisíveis por 100 são bissextos se forem divisíveis por 400
//     400 anos = 365*400 + 100 - 3 dias = 146097 dias
// Anos 1-100, 101-200, 201-300 contém 36524 dias
// Anos 301-400 contém 36525 dias

// Teste:
// for i in 0 36523 36524 73047 73048 109571 109572 146096 146097
//   do echo $i $[(i*4+3)/146097]
// done
// for ((i=0; i<=4; i++)); do echo $i $[i*146097/4] ; done
    int a = (dias*4+3)/146097;
    dias -= a*146097/4;

// Dentro de 100 anos: A cada 4 anos, 1 é bissexto
// 1,2,3, 5,6,7 não são, 4,8 são, etc.
// O último pode ou não ser bissexto

// Teste:
// for i in 0 364 365 729 730 1094 1095 1460 1461
//   do echo $i $[(i*4+3)/1461]
// done
// for ((i=0; i<=4; i++)); do echo $i $[i*1461/4] ; done
    int b = (dias*4+3)/1461;
    dias -= b*1461/4;

// Anota o ano
    Ano = a*100+b+1;

// Verifica se é bissexto
    int bissexto = (b%4!=3 ? 0 : b!=99 ? 1 : a%4==3);

// Simula fevereiro com 30 dias na contagem de dias
    if (dias >= 31+28+bissexto)
        dias += 2-bissexto;

// O ciclo se repete a cada 7 meses, totalizando 214 dias:
// 31+30+31+30+31+30+31
    int x = (dias*7+3)/214;
    Mes = x+1; // Mês começa no 1

// A fórmula que funcionou para obter o dia é essa
// Teste: for ((x=0; x<=12; x++)); do echo $x $[(x*214+3)/7] ; done
    Dia = 1 + dias - (x*214+3)/7;
}

//------------------------------------------------------------------------------
int TVarDataHora::DiasMes()
{
    if (Mes==2)
        return (Ano%4 ? 28 : Ano%100 ? 29 : Ano%400 ? 28 : 29);
    if (Mes<8)
        return 30 + Mes%2;
    return 31 - Mes%2;
}
