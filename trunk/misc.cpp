/* Este programa È software livre; vocÍ pode redistribuir e/ou
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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifndef __WIN32__
 #include <unistd.h>
#endif
//#include "config.h"
#include "misc.h"
#include "shs.h"

char * arqnome = 0;
char * arqinicio = 0;
char * arqext = 0;
char tabNOMES[256];
char tabCOMPLETO[256];

//------------------------------------------------------------------------------
// Prepara tabela ASCII (tabASC)
void tabASCinic(void)
{
    char * especialASCII[] = { "aeiouÁ_", "„·‡‚¬√¡¿", "ÍÈË …»", "ÓÌÏŒÕÃ",
                        "ÙıÛÚ‘”“", "˚˙˘⁄⁄Ÿ", "Á«", "_ " };
    char outrosASCII[] = "0123456789";
    int  caract;
    char ch,*cpont;
    memset(tabNOMES,0,256);           // Limpa todos os caracteres
    for (caract='a'; caract<='z'; caract++)
        tabNOMES[caract-0x20]=tabNOMES[caract]=caract;  // Letras de A a Z
    for (caract=0; especialASCII[0][caract]; caract++)  // Caracteres especiais
        for (cpont=especialASCII[caract+1]; *cpont; cpont++)
        {
            ch=especialASCII[0][caract];
            tabNOMES[*(unsigned char *)cpont]=ch;
        }
    for (cpont=outrosASCII; *cpont; cpont++)
        tabNOMES[*(unsigned char *)cpont]=*cpont;
    for (caract=0; caract<256; caract++)
        tabCOMPLETO[caract] = (tabNOMES[caract] ? tabNOMES[caract] : caract);
}

//------------------------------------------------------------------------------
// Semelhante a sprintf(), exceto que:
// SÛ processa caracteres %%, %c, %d, %u e %s
// %t = mensagem de tamanho TAM_NOME (nome de sala ou usu·rio)
// %S = mensagem como em %s, mas sem espaÁos finais
// tamanho È o tamanho m·ximo do buffer destino
char * mprintf(char * destino, int tamanho, const char * mens, ...)
{
    const char * fim=&destino[tamanho<=0 ? 0 : tamanho-1];
    const char * p;
    char * p2;
    char numero[20];
    bool zerofinal=(tamanho>0);
    unsigned int utamanho;
    va_list argp;
    va_start(argp, mens);
    for (; *mens; mens++)
    {
        if (*mens!='%')
        {
            if (destino<fim)
                *(destino++)=*mens;
            continue;
        }
        mens++;
        switch (*mens)
        {
        case '%':
            if (destino<fim)
                *(destino++)='%';
            break;
        case 'c':
            if (destino<fim)
                *(destino++)=va_arg(argp, int);
            break;
        case 's':
            for (p=va_arg(argp, char *); *p && destino<fim; p++,destino++)
                *destino=*p;
            break;
        case 'S':
            for (p=va_arg(argp, char *),p2=destino; *p && p2<fim; p++,p2++)
            {
                *p2=*p;
                if (*p!=' ')
                    destino=p2+1;
            }
            break;
        case 'd':
            tamanho=va_arg(argp, int);
            if (destino>=fim)
                break;
            if (tamanho<0)
            {
                *destino++='-';
                tamanho=-tamanho;
            }
            for (p2=numero; p2<&numero[sizeof(numero)] && tamanho; tamanho/=10)
                *p2++=tamanho%10+'0';
            if (p2==numero)
                *p2++='0';
            for (p=p2-1; p>=numero && destino<fim; p--)
                *destino++=*p;
            break;
        case 'u':
            utamanho=(unsigned int)va_arg(argp, unsigned int);
            if (destino>=fim)
                break;
            for (p2=numero; p2<&numero[sizeof(numero)] && utamanho; utamanho/=10)
                *p2++=utamanho%10+'0';
            if (p2==numero)
                *p2++='0';
            for (p=p2-1; p>=numero && destino<fim; p--)
                *destino++=*p;
            break;
        default:
            mens--;
        }
    }
    if (zerofinal)
        *destino=0;
    va_end(argp);
    return destino;
}

//------------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereÁo do byte =0 em destino
char * copiastr(char * destino, const char * origem)
{
    while (*origem)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//------------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereÁo do byte =0 em destino
// tamanho = n˙mero de caracteres que pode escrever em destino
char * copiastr(char * destino, const char * origem, int tamanho)
{
    if (*origem==0 || tamanho<=1)
    {
        if (tamanho>=1)
            *destino=0;
        return destino;
    }
    *(destino++) = *(origem++);
    tamanho-=2;  // -2 por causa do zero no final
    for (; tamanho && *origem; tamanho--)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//------------------------------------------------------------------------------
// Compara duas strings ASCIIZ
// Retorna: 0=strings iguais  -1=string1<string2   1=string1>string2
int comparaZ(const char * string1, const char * string2)
{
    for (;; string1++, string2++)
    {
        unsigned char ch1,ch2;
        if (*string1==0 || *string2==0)
            return (*string1 ? 1 : *string2 ? -1 : 0);
        ch1=tabCOMPLETO[*(unsigned char *)string1];
        ch2=tabCOMPLETO[*(unsigned char *)string2];
        if (ch1!=ch2)
            return (ch1<ch2 ? -1 : 1);
    }
}

//------------------------------------------------------------------------------
// Compara duas strings terminadas em espaÁo ou 0
// Retorna: 0=strings iguais  -1=string1<string2  1=string1>string2
int compara(const char * string1, const char * string2)
{
    unsigned char ch1,ch2;
    while (true)
    {
        ch1=(*string1==' ' ? 0 : tabCOMPLETO[*(unsigned char *)string1]);
        ch2=(*string2==' ' ? 0 : tabCOMPLETO[*(unsigned char *)string2]);
        if (ch1==0 || ch1!=ch2)
            break;
        string1++;
        string2++;
    }
    return (ch1==ch2 ? 0 : ch1<ch2 ? -1 : 1);
}

//------------------------------------------------------------------------------
void gerasenha(const char * senha, unsigned long codif[5])
{
    int i;
    SHS_INFO shsInfo;
    shsInit(&shsInfo);
    shsUpdate(&shsInfo, (unsigned char *)senha, strlen(senha));
    shsFinal(&shsInfo);
    for (i=0; i<5; i++)
        codif[i]=shsInfo.digest[i];
}

//------------------------------------------------------------------------------
bool senhavazia(bool limpar, unsigned long codif[5])
{
    static unsigned long semsenha[5];
    static bool ini=true;
    if (ini)
    {
        gerasenha("", semsenha);
        ini=false;
    }
    if (limpar)
    {
        memcpy(codif, semsenha, sizeof(semsenha));
        return true;
    }
    return (memcmp(codif, semsenha, sizeof(semsenha))==0);
}

//------------------------------------------------------------------------------
// Verifica se nome v·lido para apelido
// Entrada: nome em ASCIIZ (termina com 0 ou ' ')
bool verifNome(const char * nome1)
{
    char tamanho,ch;
    char anterior=0;
    char verifH=0;      // Para n„o permitir letra antes de h, exceto
                        // em ch, lh, nh, quando apelido tiver vogal
    for (tamanho=0; tamanho<=20; tamanho++)
    {
        if (*nome1==0 || *nome1==' ')
            break;
        ch=tabNOMES[*(unsigned char *)nome1];
        if (ch==0)
            return 0;
        if (ch=='h' && anterior>='a' && anterior<='z' &&
                  anterior!='c' && anterior!='l' && anterior!='n' &&
                  anterior!='a' && anterior!='e' && anterior!='i' &&
                  anterior!='o' && anterior!='u' && anterior!='y' &&
                  anterior!='w' && anterior!='p')
            if ( (verifH|=1) ==3 )
                return 0;
        if (ch>='a' && ch<='z' || ch=='Á')
        {
            if (ch=='a' || ch=='e' || ch=='i' || ch=='o' || ch=='u')
                if ( (verifH|=2) ==3 )
                    return 0;
        }
        else
            verifH=0;
        anterior=ch;
        nome1++;
    }
    return (tamanho>=2 && tamanho<20);
}

//------------------------------------------------------------------------------
// Calcula o n˙mero do dia a partir de uma data
// Entrada: string contendo dia, mÍs, ano no formato: aaaammdd
// Retorna o n˙mero de dias, ou 0 se data inv·lida

//Regra, conforme o Calend·rio Gregoriano:
// MÍs:  J  F  M  A  M  J  J  A  S  O  N  D
// Dias: 31 ?? 31 30 31 30 31 31 30 31 30 31
// Fevereiro sÛ tem 29 dias quando:
// O ano È divisÌvel por 400 ; ou
// O ano È divisÌvel por 4 e n„o È divisÌvel por 100
long numdata(const char * data)
{
    int ano,mes,dias;
    const int Tmes[]={ 0,  31,  31+28,  31+28+31,// J F M A
                 31+28+31+30,                    // M
                 31+28+31+30+31,                 // J
                 31+28+31+30+31+30,              // J
                 31+28+31+30+31+30+31,           // A
                 31+28+31+30+31+30+31+31,        // S
                 31+28+31+30+31+30+31+31+30,     // O
                 31+28+31+30+31+30+31+31+30+31,  // N
                 31+28+31+30+31+30+31+31+30+31+30 }; // D

// ObtÈm dia, mÍs, ano
    ano = (data[0]-'0')*1000 + (data[1]-'0')*100 +
          (data[2]-'0')*10   +  data[3]-'0';
    mes = (data[4]-'0')*10   +  data[5]-'0';
    dias= (data[6]-'0')*10   +  data[7]-'0';
    if (dias<=0 || dias>31 || mes<=0 || mes>12 || ano<1900 || ano>9999)
        return 0;

// ObtÈm dias conforme meses decorridos
    dias+=Tmes[mes-1];
    if (mes>2)
        dias+= (ano%400==0) + (ano%100!=0 && ano%4==0);

// ObtÈm dias conforme anos decorridos
    ano--;
    dias+=ano*365+ano/4-ano/100+ano/400;
    return dias;
}

//------------------------------------------------------------------------------
unsigned short Num16(const char * x)
{
    return ((unsigned int)(unsigned char)x[1]<<8)+(unsigned char)x[0];
}

//----------------------------------------------------------------------------
unsigned int Num24(const char * x)
{
    return ((unsigned int)(unsigned char)x[2]<<16)+
           ((unsigned int)(unsigned char)x[1]<<8)+
           (unsigned char)x[0];
}

//------------------------------------------------------------------------------
unsigned int Num32(const char * x)
{
    return ((unsigned int)(unsigned char)x[3]<<24)+
           ((unsigned int)(unsigned char)x[2]<<16)+
           ((unsigned int)(unsigned char)x[1]<<8)+
           (unsigned char)x[0];
}
