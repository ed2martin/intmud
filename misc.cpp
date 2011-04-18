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
#include <math.h>
#include <time.h>
#ifndef __WIN32__
 #include <sys/stat.h>
 #include <unistd.h>
#endif
//#include "config.h"
#include "misc.h"
#include "instr.h"
#include "shs.h"

unsigned long TempoIni;
char * arqnome = 0;
char * arqinicio = 0;
char * arqext = 0;
char * tabNOMES = 0;
char * tabCOMPLETO = 0;
char * tabMAI = 0;
char * tabMIN = 0;
char * tabTXTCOD = 0;
char * tabTXTDEC = 0;
char * tab8B = 0;
char * tab7B = 0;
char * tabTXTSEPARA = 0;

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
// Verifica se deve acertar alguma tabela
    if (tabNOMES)
        return;
// Aloca memória
    tabNOMES = new char[0x900];
    tabCOMPLETO = tabNOMES + 0x100;
    tabMAI = tabNOMES + 0x200;
    tabMIN = tabNOMES + 0x300;
    tabTXTCOD = tabNOMES + 0x400;
    tabTXTDEC = tabNOMES + 0x500;
    tab8B = tabNOMES + 0x600;
    tab7B = tabNOMES + 0x700;
    tabTXTSEPARA = tabNOMES + 0x800;
// Acerta tabNOMES
    memset(tabNOMES,0,256);
    tabNOMES[(unsigned char)'_'] = ' ';
    tabNOMES[(unsigned char)' '] = ' ';
    tabNOMES[(unsigned char)'@'] = '@';
    for (caract='a'; caract<='z'; caract++) // Letras de A a Z
        tabNOMES[caract-0x20] = tabNOMES[caract] = caract;
    for (caract='0'; caract<='9'; caract++) // Números de 0 a 9
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
    memcpy(tabMAI, tabCOMPLETO, 0x100);
    tabMAI[(unsigned char)' '] = ' ';
    memcpy(tabMIN, tabMAI, 0x100);
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
// Acerta tabTXTCOD
    memset(tabTXTCOD, 0, 0x100);
    tabTXTCOD[Instr::ex_barra_b] = 'b';
    tabTXTCOD[Instr::ex_barra_c] = 'c';
    tabTXTCOD[Instr::ex_barra_d] = 'd';
    tabTXTCOD[Instr::ex_barra_n] = 'n';
    tabTXTCOD[0x21] = 'e'; // Exclamação
    tabTXTCOD[0x22] = 'a'; // Aspas
    tabTXTCOD[(unsigned char)'@'] = '@';
    caract = 'f';
    for (int x=0x23; x<127; x++)
        if (tabNOMES[x]==0)
        {
            tabTXTCOD[x] = caract;
            caract = (caract=='m' ? 'o' : caract=='z' ? '0' : caract+1);
        }
// Acerta tabTXTDEC
    memset(tabTXTDEC, 0, 0x100);
    for (int x=0; x<0x100; x++)
        if (tabTXTCOD[x])
            tabTXTDEC[(unsigned char)tabTXTCOD[x]] = x;
// Acerta tab8B
    for (int x=0; x<0x100; x++)
        tab8B[x] = x;
// Acerta tab7B
    const char StrNormal[] =
            "AAAAAA C" "EEEEIIII"  // 0xC0
            "DNOOOOOx" " UUUUY  "  // 0xD0
            "aaaaaa c" "eeeeiiii"  // 0xE0
            " nooooo " " uuuuy y"; // 0xF0
    for (int x=0; x<0x80; x++)
        tab7B[x] = x;
    memset(tab7B+0x80, ' ', 0x40);
    memcpy(tab7B+0xC0, StrNormal, 0x40);
// Acerta tabTXTSEPARA
    memset(tabTXTSEPARA,0,256);
    tabTXTSEPARA[0] = 1; // Vazio
    tabTXTSEPARA[(unsigned char)' '] = 8; // Espaço
    for (int x='0'; x<='9'; x++) // Dígitos
        tabTXTSEPARA[x] = 4;
    for (int x='a'; x<='z'; x++) // Letras de A a Z
        tabTXTSEPARA[x-0x20] = tabTXTSEPARA[x] = 0x10;
    for (cpont=especialASCII; *cpont; cpont++) // Letras acentuadas
        tabTXTSEPARA[*(unsigned char*)cpont] = 0x10;
    for (int x=0; x<0x100; x++) // Outros
        if (tabTXTSEPARA[x] == 0)
            tabTXTSEPARA[x] = 0x20;
}

//------------------------------------------------------------------------------
// Semelhante a sprintf(), exceto que:
// Só processa caracteres %%, %c, %d, %u e %s
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
// Verifica se nome de arquivo permitido (está no diretório do programa)
bool arqvalido(char * nome, const char * ext)
{
    if (nome[0]==0)
        return false;
    char * p = nome;
#ifdef __WIN32__
// Acerta nome
    for (; *p; p++)
        if (*p=='/')
            *p='\\';
    p = nome;
// Checa se nome permitido
    if (nome[0]=='\\' || nome[1]==':')
        return false;
    for (; *p; p++)
        if ((p==nome || p[-1]=='\\') &&
            p[0]=='.' && p[1]=='.' && (p[2]==0 || p[2]=='\\'))
            return false;
#else
// Acerta nome
    for (; *p; p++)
        if (*p=='\\')
            *p='/';
    p = nome;
// Checa se nome permitido
    if (nome[0]=='/')
        return false;
    for (; *p; p++)
        if ((p==nome || p[-1]=='/') &&
            p[0]=='.' && p[1]=='.' && (p[2]==0 || p[2]=='/'))
            return false;
#endif
// Acerta final do nome do arquivo
    if (*ext==0)
        return true;
    int tam = strlen(ext);
    if (p+tam < nome)   // Nome muito pequeno: acrescenta extensão
        strcpy(p, ext);
    else if (comparaZ(p-tam, ext)!=0) // Extensão incorreta
        strcpy(p, ext);
    else // Extensão correta: acerta a extensão
        strcpy(p-tam, ext);
    return true;
}

//------------------------------------------------------------------------------
bool arqvalido(char * nome)
{
    if (!arqvalido(nome, ""))
        return false;
// Checa extensão
    char ext[3];
    char * p = nome;
    while (*p)
        p++;
    if (p >= nome+4 && p[-4]=='.')
    {
        ext[0] = p[-3] | 0x20;
        ext[1] = p[-2] | 0x20;
        ext[2] = p[-1] | 0x20;
    // Checa extensão
        if (memcmp(ext, "com", 3)==0 || memcmp(ext, "exe", 3)==0 ||
            memcmp(ext, "bat", 3)==0 || memcmp(ext, "pif", 3)==0 ||
            memcmp(ext, "scr", 3)==0 || memcmp(ext, "log", 3)==0)
            return false;
    }
// Checa se é executável
#ifndef __WIN32__
    struct stat buf;
    if (stat(nome, &buf) < 0) // Obtém dados do arquivo
        return true;
    if (!S_ISDIR(buf.st_mode) && // Não é diretório
         (buf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))) // É executável
        return false;
#endif
    return true;
}

//------------------------------------------------------------------------------
// Verifica se nome válido para apelido
int verifNome(const char * nome1)
{
    char anterior=0;
    char verifH=0;      // Para não permitir letra antes de h, exceto
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
        if ((ch>='a' && ch<='z') || ch=='ç')
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
// Verifica se nome válido para senha
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
// Codifica apelido para comparação
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
        if (ch=='ç' || (ch>='a' && ch<='z'))
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
// Codifica apelido, constituído só de letras, para comparação
// O apelido codificado é sempre menor ou igual ao original
// Retorna o endereço de onde seria o byte =0 no final do nome
char * txtNomeLetras(char * nome, int tamanho)
{
    char ch1,ch2,ch3,chn,chnn;
    char *ori,*des;
    char * fim = nome+tamanho;

// Busca 1: copia apelido passando para letras minúsculas
    ch1=ch2=ch3=0;
    for (ori=des=nome; ori<fim; ori++)
    {
    // Passa para minúsculas, Y=I, W=U
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
    // ão tem som de ãm
        if (ch1=='o' && ch3==1)
            ch1='m';
        ch3 = (*ori=='ã' || *ori=='Ã') ? 1 : 0;
    // Fecha o laço
        *des++ = ch1;
    }
    fim = des;

// Retorna se não tem vogal ou nome muito pequeno
    if (ch2==0 || nome[1]==' ')
        return fim;

// PH tem som de PI em palavras que tenham vogais
    for (ori=nome; ori<fim-1; ori++)
        if (*ori=='p' && ori[1]=='h')
            ori[1]='i';

// Converte ks, kx, cs, cx, xs, xx em x
// Ignora h no final,  h e s entre as letras  e  c, h, k, s ou x antes
    for (des--; *des=='h'; des--); // Nota: o apelido tem também vogais
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
            while (true)  // Pula próximos 'h' da string
            {
                if (ori>=fim)  { chn=' ';  break; }
                if (*ori!='h') { chn=*ori; break; }
                ori++;
            }
        }
    // Obtém: chnn = caracter após chn
        chnn=(ori+1<fim ? ori[1] : ' ');
    // X no início ou antes de N tem som de CH
        if (ch1=='x' && (ch2==0 || ch2=='n'))
            ch1='C';
    // S/X antes de CE/CI é ignorado
        if ((ch1=='s' || ch1=='x') && chn=='c' && (chnn=='e' || chnn=='i'))
            continue;
    // X antes de consoante tem som de S
        if (ch1=='x' && chn && chn!='a' && chn!='e' && chn!='i' && chn!='o' && chn!='u')
            ch1='s';
    // L que não vem antes de vogal tem som de U
        if (ch1=='l' && chn!='a' && chn!='e' && chn!='i' && chn!='o' && chn!='u')
            ch1='u';
    // C exceto CE/CI tem som de K
        if (ch1=='c' && chn!='e' && chn!='i')
            ch1='k';
    // Q tem som de K, em QUE/QUI o U é ignorado
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
        if ( ch1=='m' && (chn=='ç' || (chn>='b' && chn<='z' && chn!='e' &&
                chn!='i' && chn!='o' && chn!='u')) )
            ch1='n';
    // M/N, existindo M/N antes: ignora letra
        if (ch1=='n' && (chn=='m' || chn=='n'))
            continue;
    // N antes de P, B ou no fim do apelido tem som de M
        if (ch1=='n' && (chn=='p' || chn=='b' || (chn<=32 && chn!='ç')))
            ch1='m';
    // Ignora letra se a anterior foi a mesma e não é vogal
        if (ch1==ch2 && ch1>='b' && ch1<='z' && ch1!='e' && ch1!='i'
                && ch1!='o' && ch1!='u' && ch1!='r' && ch1!='s')
            continue;
    // Ignora R/S após não-vogal+R/S
        if ( (ch1=='r' || ch1=='s') && ch1==ch2 && ch3!='a' && ch3!='e' &&
                ch3!='i' && ch3!='o' && ch3!='u')
            continue;
    // S/R entre vogais tem som de Z/R
        if ((ch1=='s' || ch1=='r') &&
                (chn=='a' || chn=='e' || chn=='i' || chn=='o' || chn=='u') &&
                (ch2=='a' || ch2=='e' || ch2=='i' || ch2=='o' || ch2=='u') )
            ch1 = (ch1=='s' ? 'z' : 'R');
    // Ç tem som de S
    // CE/CI tem som de SE/SI (outros C foram convertidos para K)
        if (ch1=='c' || ch1=='ç')
            ch1='s';
    // Ignora R/S repetido
        if (ch1==ch2 && (ch1=='s' || ch1=='r'))
            continue;
    // Anota caracter
        ch3 = ch2;
        *des++ = ch2 = ch1;
    }

// Obtém: des=último caracter do nome
    if (des>nome)
        des--;

// Verifica S/Z no final do apelido
// Entrada: des=último caracter do nome
    ch1=0;
    while (des>nome && (*des=='s' || *des=='z'))
        *des--='s', ch1=1;
    des+=ch1;

// Verifica RR/E/O/AM no final do apelido
// Entrada: des=último caracter do nome
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

// an, am, on, om seguido de não vogal: apaga o n ou m
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
// tamanho é o tamanho do buffer destino
char * txtFiltro(char * destino, const char * origem, int tamanho)
{
    bool rep=true;
    char * dest=destino;
    int  tam1=0,tam2=0,tam3=0;
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
            char carac=*origem;
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
            *destino++ = carac;
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

//------------------------------------------------------------------------------
int DoubleToInt(double valor)
{
//printf("Antes %f Depois %f\n", valor, nearbyint(valor));
    valor = nearbyint(valor);
    if (valor >= (double)0x7FFFFFFFLL)
        return 0x7FFFFFFF;
    if (valor <= (double)-0x80000000LL)
        return -0x80000000;
//printf("Conv %f para %d\n", valor, (int)valor);
    return (int)valor;
}

//------------------------------------------------------------------------------
TAddBuffer::TAddBuffer()
{
    PosAtual=0;
    Total=0;
    Inicio = Atual = 0;
    Buf = 0;
}

//------------------------------------------------------------------------------
TAddBuffer::~TAddBuffer()
{
    if (Buf)
        delete[] Buf;
    while (Inicio)
    {
        TAddBufferBloco * p = Inicio->Proximo;
        delete Inicio;
        Inicio = p;
    }
}

//------------------------------------------------------------------------------
void TAddBuffer::Limpar()
{
    Atual = Inicio;
    PosAtual=0;
    Total=0;
}

//------------------------------------------------------------------------------
void TAddBuffer::Add(const char * origem, int tamanho)
{
    if (tamanho <= 0)
        return;
    if (Inicio==0)
    {
        Inicio = Atual = new TAddBufferBloco;
        Inicio->Proximo = 0;
    }
    Total += tamanho;
    while (tamanho > 0)
    {
        int copiar = (int)sizeof(Atual->buf) - PosAtual;
        if (copiar <= 0) // Se o bloco está cheio
        {
            copiar = sizeof(Atual->buf);
            PosAtual = 0;
            if (Atual->Proximo==0)
            {
                Atual->Proximo = new TAddBufferBloco;
                Atual->Proximo->Proximo = 0;
            }
            Atual = Atual->Proximo;
        }
        if (copiar > tamanho) // Acerta quantos bytes anotar no bloco
            copiar = tamanho;
        memcpy(Atual->buf + PosAtual, origem, copiar);
        PosAtual += copiar;
        origem += copiar;
        tamanho -= copiar;
    }
}

//------------------------------------------------------------------------------
int TAddBuffer::Anotar(char * destino, int tamanho)
{
    if (tamanho <= 0)
        return 0;
    TAddBufferBloco * bl = Inicio;
    int total = 0;
    while (bl != Atual)
    {
        if (tamanho < (int)sizeof(bl->buf))
        {
            memcpy(destino, bl->buf, tamanho);
            return total + tamanho;
        }
        memcpy(destino, bl->buf, sizeof(bl->buf));
        tamanho -= sizeof(bl->buf);
        total += sizeof(bl->buf);
        destino += sizeof(bl->buf);
        bl = bl->Proximo;
    }
    if (tamanho > (int)PosAtual)
        tamanho = PosAtual;
    if (tamanho > 0)
        memcpy(destino, bl->buf, tamanho);
    return total + tamanho;
}

//------------------------------------------------------------------------------
void TAddBuffer::AnotarBuf()
{
    if (Buf)
    {
        delete[] Buf;
        Buf = 0;
    }
    if (Total)
    {
        Buf = new char[Total];
        Anotar(Buf, Total);
    }
}
