/* Este arquivo � software livre; voc� pode redistribuir e/ou
 * modificar nos termos das licen�as GPL ou LGPL. Vide arquivos
 * COPYING e COPYING2.
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GPL or the LGP licenses.
 * See files COPYING e COPYING2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "var-txt.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

//#define DEBUG

//------------------------------------------------------------------------------
void TVarTxt::Criar()
{
    arq=0;
}

//------------------------------------------------------------------------------
void TVarTxt::Apagar()
{
    if (arq)
        fclose(arq);
}

//------------------------------------------------------------------------------
void TVarTxt::Fechar()
{
    if (arq==0)
        return;
    fclose(arq);
    arq=0;
}

//------------------------------------------------------------------------------
int TVarTxt::getValor()
{
    return (arq!=0);
}

//------------------------------------------------------------------------------
bool TVarTxt::Func(TVariavel * v, const char * nome)
{
// Ler
    if (comparaZ(nome, "ler")==0)
    {
        char mens[4096];
        int pmens=0;
    // L� o texto do arquivo
        if (arq) // Se o arquivo est� aberto...
        {
            if (ModoBinario)
            {
                int tam = 1;
                if (Instr::VarAtual > v)
                    tam = v[1].getInt();
                if (tam < 0)
                    tam = 0;
                if (tam > (int)sizeof(mens)/2)
                    tam = (int)sizeof(mens)/2;
                tam *= 2;
                while (pmens<tam)
                {
                    int lido = fgetc(arq);
                    if (lido==EOF)
                        break;
                    char ch = ((lido >> 4) & 15);
                    mens[pmens++] = (ch < 10 ? ch+'0' : ch+'a'-10);
                    ch = (lido & 15);
                    mens[pmens++] = (ch < 10 ? ch+'0' : ch+'a'-10);
                }
            }
            else if (Instr::VarAtual > v) // Ler o tamanho especificado
            {
                int tam = v[1].getInt();
                if (tam > (int)sizeof(mens))
                    tam = (int)sizeof(mens);
                while (pmens<tam)
                {
                    int lido = fgetc(arq);
                    if (lido==EOF)
                        break;
                    if (lido=='\n')
                        mens[pmens++] = Instr::ex_barra_n;
                    else if (lido >= ' ')
                        mens[pmens++] = lido;
                }
            }
            else // Nenhum argumento: ler uma linha
                while (pmens < (int)sizeof(mens))
                {
                    int lido = fgetc(arq);
                    if (lido==EOF || lido=='\n')
                        break;
                    if (lido >= ' ')
                        mens[pmens++] = lido;
                }
        }
    // Acerta vari�veis
        Instr::ApagarVar(v);
        return Instr::CriarVarTexto(mens, pmens);
    }
// Escrever
    if (comparaZ(nome, "msg")==0)
    {
        if (arq==0)
            return false;
        for (TVariavel * v1 = v+1; v1<=Instr::VarAtual; v1++)
        {
            const char * txt = v1->getTxt();
            char mens[2048];
            int pmens=0;
            if (ModoBinario)
            {
                int valor = 1;
                while (true)
                {
                    unsigned char ch = *txt++;
                    if (ch == 0)
                        break;
                    else if (ch >= '0' && ch <= '9')
                        valor = valor * 16 + ch - '0';
                    else if (ch >= 'A' && ch <= 'F')
                        valor = valor * 16 + ch - 'A' + 10;
                    else if (ch >= 'a' && ch <= 'f')
                        valor = valor * 16 + ch - 'a' + 10;
                    else
                        continue;
                    if (valor < 0x100)
                        continue;
                    mens[pmens++] = valor, valor=1;
                    if (pmens >= (int)sizeof(mens))
                    {
                        fwrite(mens, 1, pmens, arq);
                        pmens = 0;
                    }
                }
                if (pmens)
                    fwrite(mens, 1, pmens, arq);
                continue;
            }
            while (*txt)
            {
                if (pmens >= (int)sizeof(mens)-2)
                {
                    fwrite(mens, 1, pmens, arq);
                    pmens = 0;
                }
                switch (*txt)
                {
                case Instr::ex_barra_b:
                    txt++;
                    break;
                case Instr::ex_barra_c:
                    if ((txt[1]>='0' && txt[1]<='9') ||
                            (txt[1]>='A' && txt[1]<='J') ||
                            (txt[1]>='a' && txt[1]<='j'))
                        txt += 2;
                    else
                        txt++;
                    break;
                case Instr::ex_barra_d:
                    if (txt[1]>='0' && txt[1]<='7')
                        txt += 2;
                    else
                        txt++;
                    break;
                case Instr::ex_barra_n:
                    mens[pmens++] = '\n';
                    txt++;
                    break;
                default:
                    mens[pmens++] = *txt++;
                }
            }
            if (pmens)
                fwrite(mens, 1, pmens, arq);
        }
        return false;
    }
// EOF
    if (comparaZ(nome, "eof")==0)
    {
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
        if (arq==0 || feof(arq))
            Instr::VarAtual->setInt(1);
#ifdef DEBUG
        printf("EOF = %d\n", Instr::VarAtual->getInt());
        fflush(stdout);
#endif
        return true;
    }
// Pos
    if (comparaZ(nome, "pos")==0)
    {
        if (arq && Instr::VarAtual >= v+1)
        {
            int pos = v[1].getInt();
            int modo = 0;
            if (Instr::VarAtual >= v+2)
                modo = v[2].getInt();
            switch (modo)
            {
            case 0: fseek(arq, pos, SEEK_SET); break;
            case 1: fseek(arq, pos, SEEK_CUR); break;
            case 2: fseek(arq, pos, SEEK_END); break;
            }
        }
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
        if (arq)
            Instr::VarAtual->setInt(ftell(arq));
        return true;
    }
// Fechar arquivo
    if (comparaZ(nome, "fechar")==0)
    {
        Fechar();
        return false;
    }
// Fflush
    if (comparaZ(nome, "flush")==0)
    {
        if (arq)
            fflush(arq);
        return false;
    }
// Obt�m o nome do arquivo
    char arqnome[300]; // Nome do arquivo; nulo se n�o for v�lido
    bool escrita = false;
    *arqnome=0;
    if (Instr::VarAtual >= v+1)
    {
        copiastr(arqnome, v[1].getTxt(), sizeof(arqnome)-4);
    // Verifica se nome permitido
        if (arqvalido(arqnome, true))
            escrita = true;
        else if (!arqvalido(arqnome, false))
            *arqnome=0;
    }
// Checa se nome de arquivo � v�lido
    if (comparaZ(nome, "valido")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Checa se arquivo existe
    if (comparaZ(nome, "existe")==0)
    {
        struct stat buf;
        Instr::ApagarVar(v);
        if (*arqnome && stat(arqnome, &buf)<0)
            *arqnome = 0;
        return Instr::CriarVarInt(*arqnome != 0);
    }
// Abrir arquivo
    if (comparaZ(nome, "abrir")==0)
    {
        FILE * descr = 0;
        int modo = -1;
        if (*arqnome && Instr::VarAtual >= v+2)
            modo = v[2].getInt();
    // Vari�vel int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
    // Checa permiss�es
        if (modo >= 2 && escrita == false)
            return true;
    // Abre arquivo
        switch (modo)
        {
        case 0: ModoBinario = false; descr = fopen(arqnome, "r"); break;
        case 1: ModoBinario = false; descr = fopen(arqnome, "r+"); break;
        case 2: ModoBinario = false; descr = fopen(arqnome, "w"); break;
        case 3: ModoBinario = false; descr = fopen(arqnome, "a"); break;
        case 4: ModoBinario = true;  descr = fopen(arqnome, "rb"); break;
        case 5: ModoBinario = true;  descr = fopen(arqnome, "rb+"); break;
        case 6: ModoBinario = true;  descr = fopen(arqnome, "wb"); break;
        case 7: ModoBinario = true;  descr = fopen(arqnome, "ab"); break;
        }
#ifdef DEBUG
        printf("Abrindo [%s] : %s\n", nome, descr ? "OK" : "ERRO");
        fflush(stdout);
#endif
    // Se conseguiu abrir arquivo...
        if (descr != 0)
        {
            if (arq)
                fclose(arq);
            arq = descr;
            Instr::VarAtual->setInt(1);
        }
        return true;
    }
// Truncar
    if (comparaZ(nome, "truncar")==0)
    {
        int descr = -1;
        int tamanho = 0;
    // Obt�m novo tamanho do arquivo
        if (Instr::VarAtual >= v+2)
            tamanho = v[2].getInt();
    // Vari�vel int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
            return false;
    // Abre arquivo
        if (*arqnome)
            descr = open(arqnome, O_RDWR);
    // Se conseguiu abrir arquivo...
        if (descr >= 0)
        {
#ifdef __WIN32__
            chsize(descr, tamanho);
#else
            ftruncate(descr, tamanho);
#endif
            close(descr);
            Instr::VarAtual->setInt(1);
        }
        return true;
    }
    return false;
}
