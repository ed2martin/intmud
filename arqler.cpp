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
#include <unistd.h>
#include <fcntl.h>
#include "config.h"
#include "arqler.h"

//----------------------------------------------------------------------------
TArqLer::TArqLer()
{
    arq = -1;
    pler = ptotal = buf;
}

//----------------------------------------------------------------------------
TArqLer::~TArqLer()
{
    if (arq >= 0)
        close(arq);
}

//----------------------------------------------------------------------------
bool TArqLer::Abrir(const char * arquivo)
{
    if (arq >= 0)
        close(arq);
#ifdef __WIN32__
    arq=open(arquivo, O_RDONLY|O_BINARY);
#else
    arq=open(arquivo, O_RDONLY);
#endif
    if (arq < 0)
        return false;
    pler = ptotal;
    linhanum = 0;
    linhaCRLF = 0;
    return true;
}

//----------------------------------------------------------------------------
void TArqLer::Fechar()
{
    if (arq < 0)
        return;
    close(arq);
    arq = -1;
    pler = ptotal;
}

//----------------------------------------------------------------------------
int TArqLer::Linha(char * destino, int tamanho, bool barra_junta)
{
    const char * destinoini = destino;
    linhanum++;
    while (true)
    {
    // Verifica se precisa ler arquivo
        if (pler >= ptotal)
        {
            if (arq < 0)  // Fim do arquivo
                break;
            int x = read(arq, buf, sizeof(buf));
            if (x<0)    // Erro
                return -1;
            if (x<(int)sizeof(buf)) // Atingiu fim do arquivo
                Fechar();
            if (x == 0)   // Não leu nada
                break;
            pler = buf;
            ptotal = buf+x;
        }

    // Lê próximo byte
        unsigned char ch = *pler++;

    // Transforma CR(Mac), LF(Unix), CRLF(Win) ou LFCR em '\n'
        if (ch != 13 && ch != 10)
            linhaCRLF = 0;
        else if (linhaCRLF == 0)
            linhaCRLF = ch, ch = '\n';
        else if (ch != linhaCRLF)
        {
            linhaCRLF=0;
            continue;
        }
        else
            ch='\n';

    // Anota caracter na linha
        if (ch!='\n')
        {
            if (ch==' ' && destino == destinoini)
                continue;
            if (ch<' ' || tamanho <= 1)
                continue;
            *destino++ = ch, tamanho--;
            continue;
        }

    // Fim da linha
        if (destinoini != destino) // Linha não vazia
        {
            if (!barra_junta || destino[-1] != '\\')
                break;
            destino--, tamanho++;
        }
        linhanum++;
    }
    // Verifica se leu alguma coisa
    if (destinoini == destino)
        return 0;
    // Retira espaços no fim da linha e coloca um 0
    for (destino--; destino>destinoini; destino--)
        if (*destino!=' ')
            break;
    destino[1] = 0;
    return linhanum;
}
