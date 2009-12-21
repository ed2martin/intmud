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
        // Cada 3 letras: forma sem acento, min�scula, mai�scula
            "a��o��" // Til
            "a��e��i��o��u��" // Acento agudo
            "a��e��i��o��u��" // Acento grave
            "a��e��i��o��u��" // Acento circunflexo
            "���"; // C cedilha
    const char * cpont;
    int caract;
// Acerta tabNOMES
    memset(tabNOMES,0,256);
    tabNOMES[(unsigned char)'_'] = '_';
    tabNOMES[(unsigned char)' '] = '_';
    for (caract='a'; caract<='z'; caract++) // Letras de A a Z
        tabNOMES[caract-0x20] = tabNOMES[caract] = caract;
    for (caract='0'; caract<='9'; caract++) // N�meros de 0 a 9
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
    tabMAI[(unsigned char)' '] = ' ';
    memcpy(tabMIN, tabMAI, sizeof(tabMIN));
    for (caract='A'; caract<='Z'; caract++) // Letras de A a Z
    {
        tabMAI[caract+0x20] = caract;
        tabMAI[caract] = caract;
        tabMIN[caract] = caract+0x20;
    }
    for (cpont=especialASCII; cpont[0] && cpont[1] && cpont[2]; cpont+=3)
    {
        tabMAI[(unsigned char)cpont[1]] = cpont[2];
        tabMAI[(unsigned char)cpont[2]] = cpont[2];
        tabMIN[(unsigned char)cpont[1]] = cpont[1];
        tabMIN[(unsigned char)cpont[2]] = cpont[1];
    }
}

//------------------------------------------------------------------------------
// Semelhante a sprintf(), exceto que:
// S� processa caracteres %%, %c, %d, %u e %s
// %S = mensagem como em %s, mas sem espa�os finais
// tamanho � o tamanho m�ximo do buffer destino
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
// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
char * copiastr(char * destino, const char * origem)
{
    while (*origem)
        *(destino++) = *(origem++);
    *destino=0;
    return destino;
}

//------------------------------------------------------------------------------
// Semelhante a strcpy(), mas retorna endere�o do byte =0 em destino
// tamanho = n�mero de caracteres que pode escrever em destino
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
// Compara duas strings terminadas em espa�o ou 0
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
// Verifica se nome de arquivo permitido (est� no diret�rio do programa)
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
// Verifica se nome v�lido para apelido
int verifNome(const char * nome1)
{
    char anterior=0;
    char verifH=0;      // Para n�o permitir letra antes de h, exceto
                        // em ch, lh, nh, quando apelido tiver vogal
    for (int tamanho=0;; tamanho++,nome1++)
    {
        if (*nome1==0)
            return (tamanho<2 ? 1 : 0);
        char ch = tabNOMES[*(unsigned char *)nome1];
        if (ch==0 || ch=='_')
            return 2;
        if (ch=='h' && anterior>='a' && anterior<='z' &&
                  anterior!='c' && anterior!='l' && anterior!='n' &&
                  anterior!='a' && anterior!='e' && anterior!='i' &&
                  anterior!='o' && anterior!='u' && anterior!='y' &&
                  anterior!='w' && anterior!='p')
            if ( (verifH|=1) ==3 )
                return 2;
        if ((ch>='a' && ch<='z') || ch=='�')
        {
            if (ch=='a' || ch=='e' || ch=='i' || ch=='o' || ch=='u')
                if ( (verifH|=2) ==3 )
                    return 2;
        }
        else
            verifH=0;
        anterior=ch;
    }
}

//------------------------------------------------------------------------------
// Verifica se nome v�lido para senha
int verifSenha(const char * nome1)
{
    int letranum = 0;
    for (int tamanho=0;; tamanho++,nome1++)
    {
        if (*(unsigned char*)nome1 <= ' ')
            return (*nome1 ? 2 : tamanho<5 ? 1 : letranum!=3 ? 3 : 0);
        char ch = tabCOMPLETO[*(unsigned char*)nome1];
        if (ch)
        {
            if (ch>='0' && ch<='9')
                letranum|=1;
            if (ch>='a' && ch<='z')
                letranum|=2;
        }
    }
}

//------------------------------------------------------------------------------
// Codifica apelido para compara��o
// Dois apelidos iguais ou parecidos produzem o mesmo texto
char * txtNome(char * destino, const char * origem, int tamanho)
{
    if (*origem==0 || tamanho<=1)
    {
        if (tamanho>=1)
            *destino=0;
        return destino;
    }
    const char * fim = destino + tamanho - 1;
    int copiar = 0;
    for (; *origem && destino<fim; origem++)
    {
        char ch = tabNOMES[*(unsigned char*)origem];
        if (ch=='�' || (ch>='a' && ch<='z'))
        {
            *destino++ = *origem;
            copiar++;
            continue;
        }
        if (copiar)
        {
            destino = txtNomeLetras(destino - copiar, copiar);
            copiar=0;
        }
        if (ch)
            *destino++ = ch;
    }
    if (copiar)
        destino = txtNomeLetras(destino - copiar, copiar);
    *destino=0;
    return destino;
}

//------------------------------------------------------------------------------
// Codifica apelido, constitu�do s� de letras, para compara��o
// O apelido codificado � sempre menor ou igual ao original
// Retorna o endere�o de onde seria o byte =0 no final do nome
char * txtNomeLetras(char * nome, int tamanho)
{
    char ch1,ch2,ch3,chn,chnn;
    char *ori,*des;
    char * fim = nome+tamanho;

// Busca 1: copia apelido passando para letras min�sculas
    ch1=ch2=ch3=0;
    for (ori=des=nome; ori<fim; ori++)
    {
    // Passa para min�sculas, Y=I, W=U
        char ch=tabNOMES[*(unsigned char *)ori];
        if (ch==0)
            break;
        ch1=ch;
        if (ch1=='y')
            ch1='i';
        else if (ch1=='w')
            ch1='u';
        if (ch1=='a' || ch1=='e' || ch1=='i' || ch1=='o' || ch1=='u')
            ch2=1;
    // �o tem som de �m
        if (ch1=='o' && ch3==1)
            ch1='m';
        ch3 = (*ori=='�' || *ori=='�') ? 1 : 0;
    // Fecha o la�o
        *des++ = ch1;
    }
    fim = des;

// Retorna se n�o tem vogal ou nome muito pequeno
    if (ch2==0 || nome[1]==' ')
        return fim;

// PH tem som de PI em palavras que tenham vogais
    for (ori=nome; ori<fim-1; ori++)
        if (*ori=='p' && ori[1]=='h')
            ori[1]='i';

// Converte ks, kx, cs, cx, xs, xx em x
// Ignora h no final,  h e s entre as letras  e  c, h, k, s ou x antes
    for (des--; *des=='h'; des--); // Nota: o apelido tem tamb�m vogais
    if (*des=='s' || *des=='x')
    {
        while (*des=='h' || *des=='s')
            des--;
        if (*des=='x' || *des=='k' || *des=='c')
        {
            for (des--; *des=='c' || *des=='h' || *des=='k' ||
                        *des=='s' || *des=='x'; des--);
            des[1]='x';
            fim = des+2;
        }
    }

// Busca 2: filtro
    ch2=ch3=0;
    ori=des=nome;
    while (ori<fim)
    {
        ch1 = *ori++;
        chn = (ori<fim ? *ori : ' ');
    // Ignora H
        if (ch1=='h')
            continue;
    // Letras seguidas de H: ch=C, sh=C, lh=L, nh=N, ph=F
        if (chn=='h')
        {
            if (ch1=='c') // || ch1=='s')
                ch1='C';
            else if (ch1=='l')
                ch1='L';
            else if (ch1=='n')
                ch1='N';
         //   else if (ch1=='p')
         //       ch1='f';
            while (true)  // Pula pr�ximos 'h' da string
            {
                if (ori>=fim)  { chn=' ';  break; }
                if (*ori!='h') { chn=*ori; break; }
                ori++;
            }
        }
    // Obt�m: chnn = caracter ap�s chn
        chnn=(ori+1<fim ? ori[1] : ' ');
    // X no in�cio ou antes de N tem som de CH
        if (ch1=='x' && (ch2==0 || ch2=='n'))
            ch1='C';
    // S/X antes de CE/CI � ignorado
        if ((ch1=='s' || ch1=='x') && chn=='c' && (chnn=='e' || chnn=='i'))
            continue;
    // X antes de consoante tem som de S
        if (ch1=='x' && chn && chn!='a' && chn!='e' && chn!='i' && chn!='o' && chn!='u')
            ch1='s';
    // L que n�o vem antes de vogal tem som de U
        if (ch1=='l' && chn!='a' && chn!='e' && chn!='i' && chn!='o' && chn!='u')
            ch1='u';
    // C exceto CE/CI tem som de K
        if (ch1=='c' && chn!='e' && chn!='i')
            ch1='k';
    // Q tem som de K, em QUE/QUI o U � ignorado
        else if (ch1=='q')
        {
            if (chn=='u' && (chnn=='e' || chnn=='i'))
                ori++;
            ch1='k';
        }
    // G seguido de E/I tem som de J
        else if (ch1=='g' && (chn=='e' || chn=='i'))
            ch1='j';
    // GUE/GUI tem som de GE/GI
        else if (ch1=='g' && chn=='u' && (chnn=='e' || chnn=='i'))
            ori++;
    // M antes de consoante tem som de N
        if ( ch1=='m' && (chn=='�' || (chn>='b' && chn<='z' && chn!='e' &&
                chn!='i' && chn!='o' && chn!='u')) )
            ch1='n';
    // M/N, existindo M/N antes: ignora letra
        if (ch1=='n' && (chn=='m' || chn=='n'))
            continue;
    // N antes de P, B ou no fim do apelido tem som de M
        if (ch1=='n' && (chn=='p' || chn=='b' || (chn<=32 && chn!='�')))
            ch1='m';
    // Ignora letra se a anterior foi a mesma e n�o � vogal
        if (ch1==ch2 && ch1>='b' && ch1<='z' && ch1!='e' && ch1!='i'
                && ch1!='o' && ch1!='u' && ch1!='r' && ch1!='s')
            continue;
    // Ignora R/S ap�s n�o-vogal+R/S
        if ( (ch1=='r' || ch1=='s') && ch1==ch2 && ch3!='a' && ch3!='e' &&
                ch3!='i' && ch3!='o' && ch3!='u')
            continue;
    // S/R entre vogais tem som de Z/R
        if ((ch1=='s' || ch1=='r') &&
                (chn=='a' || chn=='e' || chn=='i' || chn=='o' || chn=='u') &&
                (ch2=='a' || ch2=='e' || ch2=='i' || ch2=='o' || ch2=='u') )
            ch1 = (ch1=='s' ? 'z' : 'R');
    // � tem som de S
    // CE/CI tem som de SE/SI (outros C foram convertidos para K)
        if (ch1=='c' || ch1=='�')
            ch1='s';
    // Ignora R/S repetido
        if (ch1==ch2 && (ch1=='s' || ch1=='r'))
            continue;
    // Anota caracter
        ch3 = ch2;
        *des++ = ch2 = ch1;
    }

// Obt�m: des=�ltimo caracter do nome
    if (des>nome)
        des--;

// Verifica S/Z no final do apelido
// Entrada: des=�ltimo caracter do nome
    ch1=0;
    while (des>nome && (*des=='s' || *des=='z'))
        *des--='s', ch1=1;
    des+=ch1;

// Verifica RR/E/O/AM no final do apelido
// Entrada: des=�ltimo caracter do nome
    if (des>nome)
    {
        if (*des=='r' && *(des-1)=='r')
            des--;
        else if (*des=='o')
            *des='u';
        else if (*des=='e')
            *des='i';
        else if (*des=='m' && des[-1]=='a')
            des--;
    }

// Acerta fim
    fim = des+1;

// an, am, on, om seguido de n�o vogal: apaga o n ou m
    for (des=nome; des<=fim-2; des++)
        if ( (*des=='a' || *des=='o') && (des[1]=='n' || des[1]=='m') )
        {
            if (des+2<fim)
                if (des[2]=='a' || des[2]=='e' || des[2]=='i' ||
                                   des[2]=='o' || des[2]=='u')
                    continue;
            fim--;
            for (ori=des+1; ori<fim; ori++)
                ori[0]=ori[1];
        }

// Transforma C,L,N,R em c,w,y,h
    for (des=nome; des<fim; des++)
        switch (*des)
        {
        case 'C': *des='c'; break;
        case 'L': *des='w'; break;
        case 'N': *des='y'; break;
        case 'R': *des='h'; break;
        }
    return fim;
}

//------------------------------------------------------------------------------
// Copia mensagem, como com copiastr(), mas filtrando a mensagem
// tamanho � o tamanho do buffer destino
char * txtFiltro(char * destino, const char * origem, int tamanho)
{
    bool rep=true;
    char * dest=destino;
    int  tam1=0,tam2=0,tam3=0;
    while (*origem==' ') // Ignora espa�os iniciais
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
            char carac=*origem;
            // Caracteres inv�lidos: ignora
            if ((unsigned char)carac<' ' || (unsigned char)carac==255)
            {
                rep=true;
                continue;
            }
            // Impede certas sequ�ncias de caracteres
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
            // Seq��ncia de espa�os
            if (tam1>5 && carac==' ')
                tam1=tam2=tam3=0;
            // Ignora caracteres
            if (tam1>5 || tam2>3 || tam3>3)
            {
                rep=true;
                continue;
            }
            // Ignora muitos caracteres seguidos sem vogais
            // Caracteres ' ' e '0' a '9' n�o entram na compara��o
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
            *destino++ = carac;
        }
        // Ignora espa�os finais
        for (destino--; destino>=dest; destino--)
            if (*destino!=' ')
                break;
        // Pr�xima verifica��o
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
// Calcula o n�mero do dia a partir de uma data
// Entrada: string contendo dia, m�s, ano no formato: aaaammdd
// Retorna o n�mero de dias, ou 0 se data inv�lida

//Regra, conforme o Calend�rio Gregoriano:
// M�s:  J  F  M  A  M  J  J  A  S  O  N  D
// Dias: 31 ?? 31 30 31 30 31 31 30 31 30 31
// Fevereiro s� tem 29 dias quando:
// O ano � divis�vel por 400 ; ou
// O ano � divis�vel por 4 e n�o � divis�vel por 100
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

// Obt�m dia, m�s, ano
    ano = (data[0]-'0')*1000 + (data[1]-'0')*100 +
          (data[2]-'0')*10   +  data[3]-'0';
    mes = (data[4]-'0')*10   +  data[5]-'0';
    dias= (data[6]-'0')*10   +  data[7]-'0';
    if (dias<=0 || dias>31 || mes<=0 || mes>12 || ano<1900 || ano>9999)
        return 0;

// Obt�m dias conforme meses decorridos
    dias+=Tmes[mes-1];
    if (mes>2)
        dias+= (ano%400==0) + (ano%100!=0 && ano%4==0);

// Obt�m dias conforme anos decorridos
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
