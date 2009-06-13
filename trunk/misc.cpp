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
#include <stdarg.h>
#include <string.h>
#include <time.h>
#ifndef __WIN32__
 #include <unistd.h>
#endif
//#include "config.h"
#include "misc.h"
#include "shs.h"

unsigned long TempoIni;
char * arqnome = 0;
char * arqinicio = 0;
char * arqext = 0;
char tabNOMES[256];
char tabCOMPLETO[256];
char tabMAI[256];
char tabMIN[256];

//------------------------------------------------------------------------------
// Prepara tabela ASCII (tabASC)
void tabASCinic(void)
{
    const char especialASCII[] =
        // Cada 3 letras: forma sem acento, minúscula, maiúscula
            "aãÃoõÕ" // Til
            "aáÁeéÉiíÍoóÓuúÚ" // Acento agudo
            "aàÀeèÈiìÌoòÒuùÙ" // Acento grave
            "aâÂeêÊiîÎoôÔuûÛ" // Acento circunflexo
            "ççÇ"; // C cedilha
    const char * cpont;
    int caract;
// Acerta tabNOMES
    memset(tabNOMES,0,256);
    tabNOMES['_'] = '_';
    tabNOMES[' '] = '_';
    for (caract='a'; caract<='z'; caract++) // Letras de A a Z
        tabNOMES[caract-0x20] = tabNOMES[caract] = caract;
    for (caract='0' ; caract<='9'; caract++) // Números de 0 a 9
        tabNOMES[caract] = caract;
    for (cpont=especialASCII; cpont[0] && cpont[1] && cpont[2]; cpont+=3)
    {
        tabNOMES[(unsigned char)cpont[1]] = cpont[0];
        tabNOMES[(unsigned char)cpont[2]] = cpont[0];
    }
// Acerta tabCOMPLETO
    for (caract=0; caract<256; caract++)
        tabCOMPLETO[caract] = (tabNOMES[caract] ? tabNOMES[caract] : caract);
// Acerta tabMAI e tabMIN
    memcpy(tabMAI, tabCOMPLETO, sizeof(tabMAI));
    tabMAI[' '] = ' ';
    memcpy(tabMIN, tabMAI, sizeof(tabMIN));
    for (caract='a'; caract<='z'; caract++) // Letras de A a Z
    {
        tabMAI[caract] = caract-0x20;
        tabMAI[caract-0x20] = caract-0x20;
        tabMIN[caract-0x20] = caract;
    }
    for (cpont=especialASCII; cpont[0] && cpont[1] && cpont[2]; cpont+=3)
    {
        tabMAI[(unsigned char)cpont[1]] = cpont[2];
        tabMIN[(unsigned char)cpont[2]] = cpont[1];
    }
}

//------------------------------------------------------------------------------
// Semelhante a sprintf(), exceto que:
// Só processa caracteres %%, %c, %d, %u e %s
// %t = mensagem de tamanho TAM_NOME (nome de sala ou usuário)
// %S = mensagem como em %s, mas sem espaços finais
// tamanho é o tamanho máximo do buffer destino
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
void move_mem(void * destino, void * origem, unsigned int tamanho)
{
    if (tamanho==0 || origem==destino)
        return;
    if (destino >= origem && (char*)destino - tamanho < (char*)origem)
    {
        char * o = (char*)origem + tamanho - 1;
        char * d = (char*)destino + tamanho - 1;
        while (tamanho--)
            *d-- = *o--;
    }
    else
    {
        char * o = (char*)origem;
        char * d = (char*)destino;
        while (tamanho--)
            *d++ = *o++;
    }
}

//------------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
char * copiastr(char * destino, const char * origem)
{
    while (*origem)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//------------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endereço do byte =0 em destino
// tamanho = número de caracteres que pode escrever em destino
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
int comparaZ(const char * string1, const char * string2)
{
    for (;; string1++, string2++)
    {
        unsigned char ch1,ch2;
        if (*string1==0 || *string2==0)
            return (*string1 ? 2 : *string2 ? -2 : 0);
        ch1=tabCOMPLETO[*(unsigned char *)string1];
        ch2=tabCOMPLETO[*(unsigned char *)string2];
        if (ch1!=ch2)
            return (ch1<ch2 ? -1 : 1);
    }
}

//------------------------------------------------------------------------------
// Compara duas strings terminadas em espaço ou 0
int compara(const char * string1, const char * string2)
{
    unsigned char ch1,ch2;
    while (true)
    {
        ch1=(*string1==' ' ? 0 : tabCOMPLETO[*(unsigned char *)string1]);
        ch2=(*string2==' ' ? 0 : tabCOMPLETO[*(unsigned char *)string2]);
        if (ch1==0)
            return (ch2==0 ? 0 : -2);
        if (ch1!=ch2)
            return (ch2==0 ? 2 : ch1<ch2 ? -1 : 1);
        string1++;
        string2++;
    }
}

//------------------------------------------------------------------------------
// Compara duas strings de tamanho fixo
int compara(const char * string1, const char * string2, int tam)
{
    for (; tam>0; string1++, string2++, tam--)
    {
        unsigned char ch1,ch2;
        ch1=tabCOMPLETO[*(unsigned char *)string1];
        ch2=tabCOMPLETO[*(unsigned char *)string2];
        if (ch1!=ch2)
            return (ch1<ch2 ? -1 : 1);
    }
    return 0;
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
// Verifica se nome de arquivo permitido (está no diretório do programa)
bool arqvalido(const char * nome)
{
    if (nome[0]==0)
        return false;
    const char * p = nome;
#ifdef __WIN32__
    if (nome[0]=='\\' || nome[1]==':')
        return false;
    for (; *p; p++)
        if ((p==nome || p[-1]=='\\') && memcmp(p, "..\\", 3)==0)
            return false;
#else
    if (nome[0]=='/')
        return false;
    for (; *p; p++)
        if ((p==nome || p[-1]=='/') && memcmp(p, "../", 3)==0)
            return false;
#endif
    return true;
}

//------------------------------------------------------------------------------
// Verifica se nome válido para apelido
// Entrada: nome em ASCIIZ (termina com 0 ou ' ')
bool verifNome(const char * nome1)
{
    char tamanho,ch;
    char anterior=0;
    char verifH=0;      // Para não permitir letra antes de h, exceto
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
        if ((ch>='a' && ch<='z') || ch=='ç')
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
// Copia mensagem, como com copiastr(), mas filtrando a mensagem
// tamanho é o tamanho do buffer destino
char * txtFiltro(char * destino, const char * origem, int tamanho)
{
    bool rep=true;
    char * dest=destino;
    int  tam1=0,tam2=0,tam3=0;
    char carac;
    while (*origem==' ') // Ignora espaços iniciais
        origem++;
    while (rep)
    {
        char repete[12]={ 0,0,0,0, 0,0,0,0, 0,0,0,0 };
        char *passo=repete;
        int  consoante=0;
        char conso;
        rep=false;
        tamanho--;
        for (; *origem && tamanho>0; origem++)
        {
            carac=*origem;
            // Caracteres inválidos: ignora
            if ((unsigned char)carac<' ' || (unsigned char)carac==255)
            {
                rep=true;
                continue;
            }
            // Impede certas sequências de caracteres
            passo--;
            if (passo<repete)
                passo=repete+5;
            passo[0]=passo[6]=(carac<'A' || carac>'Z' ? carac : carac|0x20);
            tam1++; tam2++; tam3++;
            // 1 caracter,   exemplo: aaaaaaaaaaaa vira aaaaaa
            if (passo[0]!=passo[1]) tam1=0;
            // 2 caracteres, exemplo: abababababab vira ababab
            if (passo[0]!=passo[2] || passo[1]!=passo[3]) tam2=0;
            // 3 caracteres, exemplo: abcabcabcabc vira abcabcab
            if (passo[0]!=passo[3] || passo[1]!=passo[4] || passo[2]!=passo[5]) tam3=0;
            // Seqüência de espaços
            if (tam1>5 && carac==' ')
                tam1=tam2=tam3=0;
            // Ignora caracteres
            if (tam1>5 || tam2>3 || tam3>3)
            {
                rep=true;
                continue;
            }
            // Ignora muitos caracteres seguidos sem vogais
            // Caracteres ' ' e '0' a '9' não entram na comparaçào
            conso=tabNOMES[(unsigned char)carac];
            if (conso=='a' || conso=='e' || conso=='i' || conso=='o' || conso=='u')
                consoante=0;
            if (carac!=' ' && (carac<'0' || carac>'9'))
                if (consoante++ > 20)
                {
                    rep=true;
                    continue;
                }
            // Anota caracter
            tamanho--;
            *(destino++)=carac;
        }
        // Ignora espaços finais
        for (destino--; destino>=dest; destino--)
            if (*destino!=' ')
                break;
        // Próxima verificação
        destino[1]=0;
        origem=destino=dest;
        tamanho=10000;
    }
    tam1=strlen(dest)/3;
    if (tam1>50) tam1=50;
    for (; tam1>3; tam1--)
    {
        origem=destino=&dest[tam1];
        tam2=0;
        for (; *origem; origem++)
        {
            if (*origem!=origem[-tam1])
                tam2=0;
            if (tam2++ < tam1*2)
                *(destino++)=*origem;
        }
        *destino=0;
        if ((unsigned int)tam1>strlen(dest)/2)
            tam1=strlen(dest)/2;
    }
    while (*dest)
        dest++;
    return dest;
}

//------------------------------------------------------------------------------
// Calcula o número do dia a partir de uma data
// Entrada: string contendo dia, mês, ano no formato: aaaammdd
// Retorna o número de dias, ou 0 se data inválida

//Regra, conforme o Calendário Gregoriano:
// Mês:  J  F  M  A  M  J  J  A  S  O  N  D
// Dias: 31 ?? 31 30 31 30 31 31 30 31 30 31
// Fevereiro só tem 29 dias quando:
// O ano é divisível por 400 ; ou
// O ano é divisível por 4 e não é divisível por 100
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

// Obtém dia, mês, ano
    ano = (data[0]-'0')*1000 + (data[1]-'0')*100 +
          (data[2]-'0')*10   +  data[3]-'0';
    mes = (data[4]-'0')*10   +  data[5]-'0';
    dias= (data[6]-'0')*10   +  data[7]-'0';
    if (dias<=0 || dias>31 || mes<=0 || mes>12 || ano<1900 || ano>9999)
        return 0;

// Obtém dias conforme meses decorridos
    dias+=Tmes[mes-1];
    if (mes>2)
        dias+= (ano%400==0) + (ano%100!=0 && ano%4==0);

// Obtém dias conforme anos decorridos
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
