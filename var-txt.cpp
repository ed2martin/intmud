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
#include <fcntl.h>
#include <unistd.h>
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
    // Lê o texto do arquivo
        if (arq) // Se o arquivo está aberto...
        {
            if (Instr::VarAtual > v) // Ler o tamanho especificado
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
    // Acerta variáveis
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
                            (txt[1]>='A' && txt[1]<='F') ||
                            (txt[1]>='a' && txt[1]<='f'))
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
// Abrir arquivo
    if (comparaZ(nome, "abrir")==0)
    {
        FILE * descr = 0;
    // Obtém nome do arquivo
        while (Instr::VarAtual == v+2)
        {
            char nome[300]; // Nome do arquivo
            char * p = copiastr(nome, v[1].getTxt(), sizeof(nome)-4);
            if (!arqvalido(nome))
                break;
    // Acrescenta ".txt" se necessário
            if (p <= nome+4)
                strcpy(p, ".txt");
            else if (comparaZ(p-4, ".txt")!=0)
                strcpy(p, ".txt");
            else
                strcpy(p-3, "txt");
    // Abre arquivo
            switch (v[2].getInt())
            {
            case 0: descr = fopen(nome, "r"); break;
            case 1: descr = fopen(nome, "r+"); break;
            case 2: descr = fopen(nome, "w"); break;
            case 3: descr = fopen(nome, "a"); break;
            }
#ifdef DEBUG
            printf("Abrindo [%s] : %s\n", nome, descr ? "OK" : "ERRO");
            fflush(stdout);
#endif
            break;
        }
    // Variável int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
        {
            if (descr)
                fclose(descr);
            return false;
        }
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
// Truncar
    if (comparaZ(nome, "truncar")==0)
    {
        int descr = -1;
        int tamanho = 0;
    // Obtém nome e novo tamanho do arquivo
        while (Instr::VarAtual == v+2)
        {
            char nome[300]; // Nome do arquivo
            char * p = copiastr(nome, v[1].getTxt(), sizeof(nome)-4);
            tamanho = v[2].getInt();
            if (!arqvalido(nome))
                break;
    // Acrescenta ".txt" se necessário
            if (p <= nome+4)
                strcpy(p, ".txt");
            else if (comparaZ(p-4, ".txt")!=0)
                strcpy(p, ".txt");
            else
                strcpy(p-3, "txt");
    // Abre arquivo
            descr = open(nome, O_RDWR);
            break;
        }
    // Variável int no topo da pilha
        Instr::ApagarVar(v);
        if (!Instr::CriarVarInt(0))
        {
            if (descr >= 0)
                close(descr);
            return false;
        }
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
