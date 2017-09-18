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
#include "var-telatxt.h"
#include "var-arqprog.h"
#include "console.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#define CONSOLE_MAX 1024
#define CONSOLE_COR_LINHA 0x74

enum {
TelaTxtTexto = 1,
TelaTxtTotal,
TelaTxtLinha
};

unsigned int TVarTelaTxt::CorTela = 0x70;
unsigned int TVarTelaTxt::CorBarraN = 0x70;
unsigned int TVarTelaTxt::LinhaFinal = 1;
unsigned int TVarTelaTxt::LinhaPosic = 0;
unsigned int TVarTelaTxt::ColPosic = 0;
unsigned int TVarTelaTxt::ColEscreve = 0;
unsigned int TVarTelaTxt::ColEditor = 0;
char * TVarTelaTxt::TelaBuf = 0;
unsigned int TVarTelaTxt::TelaLin = 0;
unsigned int TVarTelaTxt::col_linha = 0;
unsigned int TVarTelaTxt::tam_linha = 0;
unsigned int TVarTelaTxt::max_linha = CONSOLE_MAX - 1;
char * TVarTelaTxt::txt_linha = new char[CONSOLE_MAX];
TVarTelaTxt * TVarTelaTxt::Inicio = 0;
TVarTelaTxt * TVarTelaTxt::ObjAtual = 0;

//---------------------------------------------------------------------------
void TVarTelaTxt::Escrever(const char * texto, int tamanho)
{
    Console->CursorLin(LinhaFinal - 1 - Console->LinAtual);
    if (ColEscreve && ColEscreve < 0xFFFF)
        Console->CursorCol(ColEscreve - Console->ColAtual);
    else
        Console->CursorIni();
    Console->CorTxt(CorTela);
// Tamanho do texto
    if (tamanho<0)
        tamanho = strlen(texto);
// Envia texto
    char * pbuf = TelaBuf + TelaLin * Console->ColTotal + ColEscreve;
    for (; tamanho>0; texto++,tamanho--)
    {
        if (ColEscreve >= Console->ColTotal)
        {
            Console->EnvTxt("\n\n", 2); // Dois '\n' para criar uma linha no final
            Console->CursorLin(-1);      // Cursor sobe uma linha
            Console->CorTxt(CorBarraN);
            Console->InsereLin(1);       // Insere uma linha
            Console->CorTxt(CorTela);
        // Atualiza LinhaPosic
            if (LinhaPosic > 0 && LinhaPosic < Console->LinTotal - 1)
                LinhaPosic++;
        // Atualiza LinhaFinal
            LinhaFinal++;
            if (LinhaFinal >= Console->LinTotal)
                LinhaFinal = Console->LinTotal - 1;
        // Atualiza buffer com conteúdo da tela
            TelaLin = (TelaLin + 1) % Console->LinTotal;
            pbuf = TelaBuf + TelaLin * Console->ColTotal;
            memset(pbuf, ' ', Console->ColTotal);
        // Atualiza ColEscreve
            if (ColEscreve != 0xFFFF && *texto=='\n')
                texto++,tamanho--;
            ColEscreve = 0;
            if (tamanho==0)
                break;
        }
        //printf("[%c]", *texto);
        if (*texto=='\n')
            ColEscreve = 0xFFFF;
        else
        {
            Console->EnvTxt(texto, 1);
            ColEscreve++;
            *pbuf++ = *texto;
            if (ColEscreve >= Console->ColTotal)
                Console->CursorLin(-1);
        }
        CorBarraN = CorTela;
    }
}

//---------------------------------------------------------------------------
void TVarTelaTxt::CursorEditor()
{
    if (LinhaFinal == Console->LinAtual+1)
        Console->EnvTxt("\n", 1);
    else
        Console->CursorLin(LinhaFinal - Console->LinAtual);
    Console->CursorCol(ColEditor - Console->ColAtual);
    Console->CorTxt(CONSOLE_COR_LINHA);
}

//---------------------------------------------------------------------------
void TVarTelaTxt::ProcTecla(const char * texto)
{
    if (texto==0)
        return;
// Tecla normal (texto)
    if (texto[0] && texto[1]==0)
    {
        CursorEditor();
    // Verifica se tem espaço na linha
        if (tam_linha >= max_linha)
            return;
    // Altera a linha
        tam_linha++;
        char antes = texto[0];
        for (unsigned int x=col_linha + ColEditor; x<tam_linha; x++)
        {
            char valor = txt_linha[x];
            txt_linha[x] = antes;
            antes = valor;
        }
    // Não está no fim da linha na tela
        if (ColEditor < Console->ColTotal-2)
        {
            ColEditor++;
            if (col_linha + ColEditor < tam_linha)
                Console->InsereCol(1);
        }
        else
    // Está no fim da linha na tela
        {
            col_linha++;
            Console->CursorIni(); // Cursor no início da linha
            Console->ApagaCol(1); // Apaga 1 caracter
            Console->CursorCol(ColEditor - 1); // Posiciona cursor
        }
        Console->EnvTxt(txt_linha + col_linha + ColEditor - 1, 1); // Mostra caracter
        return;
    }
// Passa para letras maiúsculas
    char texto1[20];
    for (unsigned int x=0; x<sizeof(texto1)-1; x++)
        texto1[x] = tabMAI[*(unsigned char *)(texto + x)];
    texto1[sizeof(texto1)-1] = 0;
// Mouse
    if (memcmp(texto1, "MOUSE", 5)==0 && texto1[5]>='1' && texto1[5]<='3')
    {
        const char * p = texto1+6;
        long posx=0, posy=0;
        while (*p==' ') p++;
        posx = strtol(p, 0, 10);
        while (*p && *p!=' ') p++;
        while (*p==' ') p++;
        posy = strtol(p, 0, 10);
        LinhaPosic = posy;
        if (posy)
            ColPosic=posx;
        else
        {
            int max = tam_linha - col_linha;
            if (posx>max) posx=max;
            if (posx<0) posx=0;
            ColEditor = posx;
        }
        return;
    }
// Cima
    if (strcmp(texto1, "UP")==0)
    {
        if (LinhaPosic < LinhaFinal)
            LinhaPosic++;
        ColPosic = 0;
        return;
    }
// Baixo
    if (strcmp(texto1, "DOWN")==0)
    {
        if (LinhaPosic > 0)
            LinhaPosic--;
        ColPosic = 0;
        return;
    }
// Esquerda
    if (strcmp(texto1, "LEFT")==0)
    {
        if (LinhaPosic==0)
        {
            CursorEditor();
            ProcTeclaCursor(ColEditor + col_linha - 1);
        }
        else if (ColPosic)
            ColPosic--;
        return;
    }
// Direita
    if (strcmp(texto1, "RIGHT")==0)
    {
        if (LinhaPosic==0)
        {
            ProcTeclaCursor(ColEditor + col_linha + 1);
            CursorEditor();
        }
        else if (ColPosic < Console->ColTotal - 1)
            ColPosic++;
        return;
    }
// ENTER
    if (strcmp(texto1, "ENTER")==0)
    {
        col_linha = 0;
        tam_linha = 0;
        ColEditor = 0;
        CursorEditor();        // Cursor na linha de edição
        Console->CursorIni();  // Posicina o cursor no início da linha
        Console->LimpaFim();   // Limpa a linha
        return;
    }
// Control + esquerda
    if (strcmp(texto1, "C LEFT")==0)
    {
        if (LinhaPosic)
        {
            int linnum = TelaLin + 1 - LinhaPosic + Console->LinTotal;
            linnum %= Console->LinTotal;
            const char * pbuf = TelaBuf + linnum * Console->ColTotal;
            while (ColPosic > 0 && pbuf[ColPosic-1]==' ')
                ColPosic--;
            while (ColPosic > 0 && pbuf[ColPosic-1]!=' ')
                ColPosic--;
            return;
        }
        CursorEditor();
        int col = ColEditor + col_linha - 1;
        for (; col>=0; col--)
            if (txt_linha[col] >= '0')
                break;
        for (; col>=0; col--)
            if (txt_linha[col] < '0')
                break;
        ProcTeclaCursor(col+1);
        return;
    }
// Control + direita
    if (strcmp(texto1, "C RIGHT")==0)
    {
        if (LinhaPosic)
        {
            int linnum = TelaLin + 1 - LinhaPosic + Console->LinTotal;
            linnum %= Console->LinTotal;
            const char * pbuf = TelaBuf + linnum * Console->ColTotal;
            const unsigned int colmax = Console->ColTotal;
            while (ColPosic+1 < colmax && pbuf[ColPosic]!=' ')
                ColPosic++;
            while (ColPosic+1 < colmax && pbuf[ColPosic]==' ')
                ColPosic++;
            return;
        }
        CursorEditor();
        int col = ColEditor + col_linha + 1;
        for (; col<(int)tam_linha; col++)
            if (txt_linha[col] < '0')
                break;
        for (; col<(int)tam_linha; col++)
            if (txt_linha[col] >= '0')
                break;
        ProcTeclaCursor(col);
        return;
    }
// Home
    if (strcmp(texto1, "HOME")==0)
    {
        if (LinhaPosic)
            ColPosic = 0;
        else
            ProcTeclaCursor(0);
        return;
    }
// End
    if (strcmp(texto1, "END")==0)
    {
        if (LinhaPosic)
            ColPosic = Console->ColTotal - 1;
        else
            ProcTeclaCursor(tam_linha);
        return;
    }
// DEL
    if (strcmp(texto1, "DEL")==0)
    {
        if (col_linha + ColEditor >= tam_linha)
            return;
    // Altera a linha
        tam_linha--;
        for (unsigned int x=col_linha + ColEditor; x<tam_linha; x++)
            txt_linha[x] = txt_linha[x+1];
    // Resultado na tela
        CursorEditor();        // Cursor na linha de edição
        Console->ApagaCol(1); // Apaga 1 caracter
        if (col_linha + Console->ColTotal - 2 >= tam_linha)
            return;
        int move = Console->ColTotal - ColEditor - 2;
        Console->CursorCol(move);
        Console->EnvTxt(txt_linha + col_linha + Console->ColTotal - 2, 1); // Mostra caracter
        Console->LimpaFim(); // Limpa da posição do cursor até o final
        Console->CursorCol(-move-1);
        return;
    }
// Backspace
    if (strcmp(texto1, "BACK")==0)
    {
        if (col_linha + ColEditor <= 0)
            return;
    // Altera a linha
        tam_linha--;
        for (unsigned int x=col_linha + ColEditor - 1; x<tam_linha; x++)
            txt_linha[x] = txt_linha[x+1];
    // Resultado na tela
        if (ColEditor==0)
        {
            col_linha--;
            return;
        }
        ColEditor--;
        CursorEditor();        // Cursor na linha de edição
        Console->ApagaCol(1); // Apaga 1 caracter
        if (col_linha + Console->ColTotal - 2 >= tam_linha)
            return;
        int move = Console->ColTotal - ColEditor - 2;
        Console->CursorCol(move);
        Console->EnvTxt(txt_linha + col_linha + Console->ColTotal - 2, 1);
                    // Mostra caracter
        Console->LimpaFim(); // Limpa da posição do cursor até o final
        Console->CursorCol(-move-1);
        return;
    }
}

//---------------------------------------------------------------------------
void TVarTelaTxt::ProcTeclaCursor(int coluna)
{
    bool redesenha = false;
// Acerta limites da coluna
    if (coluna < 0)
        coluna = 0;
    if (coluna > (int)tam_linha)
        coluna = (int)tam_linha;
// Obtém a coluna do cursor na tela
    coluna -= col_linha;
// Mover para esquerda
    if (coluna < 0)
    {
        int move = -coluna;
        coluna = 0;
        col_linha -= move;
        if (move < (int)Console->ColTotal-1) // Cabe na tela
        {
            Console->CursorIni();
            Console->InsereCol(move);
            Console->EnvTxt(txt_linha + col_linha, move);
        }
        else // Não cabe na tela
            redesenha = true;
    }
// Mover para direita
    if (coluna > (int)Console->ColTotal-2)
    {
        int move = coluna - (Console->ColTotal-2);
        coluna = Console->ColTotal-2;
        col_linha += move;
        if (move < (int)Console->ColTotal-1) // Cabe na tela
        {
            Console->CursorIni();
            Console->ApagaCol(move);
            int pos = Console->ColTotal - 1 - move;
            Console->CursorCol(pos);
            pos += col_linha;
            if (move > (int)tam_linha - pos)
                move = (int)tam_linha - pos;
            Console->EnvTxt(txt_linha + pos, move);
        }
        else // Não cabe na tela
            redesenha = true;
    }
// Atualizar toda a linha de edição
    if (redesenha)
    {
        int total = tam_linha - col_linha;
        if (total > (int)Console->ColTotal-1)
            total = (int)Console->ColTotal-1;
        Console->CursorIni();
        Console->EnvTxt(txt_linha + col_linha, total);
        Console->LimpaFim();
    }
// Acerta posição do cursor
    ColEditor = coluna;
    Console->CursorCol(ColEditor - Console->ColAtual);
}

//---------------------------------------------------------------------------
void TVarTelaTxt::ProcFim()
{
    if (Console==0)
        return;
// Posicionar na linha de edição
    if (LinhaPosic == 0)
        CursorEditor();
// Posicionar na linha escolhida pelo usuário
    else
    {
        Console->CursorLin(LinhaFinal - LinhaPosic - Console->LinAtual);
        if (ColPosic==0 && Console->ColAtual)
            Console->CursorIni();
        else
            Console->CursorCol(ColPosic - Console->ColAtual);
    }
    Console->Flush();
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Criar()
{
    Antes = 0;
    Depois = Inicio;
    if (Inicio)
        Inicio->Antes = this;
    Inicio = this;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Apagar()
{
    (Antes ? Antes->Depois : Inicio) = Depois;
    if (Depois)
        Depois->Antes = Antes;
    if (ObjAtual==this)
        ObjAtual=Depois;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Mover(TVarTelaTxt * destino)
{
    (Antes ? Antes->Depois : Inicio) = destino;
    if (Depois)
        Depois->Antes = destino;
    if (ObjAtual == this)
        ObjAtual = destino;
    memmove(destino, this, sizeof(TVarTelaTxt));
}

//------------------------------------------------------------------------------
void TVarTelaTxt::EndObjeto(TClasse * c, TObjeto * o)
{
    if (o)
        endobjeto=o, b_objeto=true;
    else
        endclasse=c, b_objeto=false;
}

//------------------------------------------------------------------------------
int TVarTelaTxt::getTipo(int numfunc)
{
    switch (numfunc)
    {
    case 0: return varOutros;
    case TelaTxtTexto: return varTxt;
    default: return varInt;
    }
}

//------------------------------------------------------------------------------
int TVarTelaTxt::getValor(int numfunc)
{
    if (Console==0)
        return 0;
    switch (numfunc)
    {
    case TelaTxtTexto:
        return TxtToInt(LerLinha());
    case TelaTxtTotal:
        return max_linha;
    case TelaTxtLinha:
        return LinhaPosic;
    }
    return 0;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::setValor(int numfunc, int valor)
{
    if (Console==0)
        return;
    switch (numfunc)
    {
    case TelaTxtTexto:
        {
            char mens[20];
            sprintf(mens, "%d", valor);
            setTxt(numfunc, mens);
            break;
        }
    case TelaTxtTotal:
        if (valor > CONSOLE_MAX - 1)
            valor = CONSOLE_MAX - 1;
        if (valor < 1)
            valor = 1;
        if (valor >= (int)max_linha)
            max_linha = valor;
        else
        {
            max_linha = valor;
            if ((int)tam_linha > valor)
                tam_linha = valor;
            CursorEditor();   // Cursor na linha de edição
            if (col_linha + ColEditor > (unsigned int)valor)
                ProcTeclaCursor(tam_linha); // Semelhante à tecla END
            if (valor - col_linha < Console->ColTotal)
            {
                Console->CursorCol(valor - col_linha - Console->ColAtual);
                Console->LimpaFim();
            }
        }
        break;
    case TelaTxtLinha:
        if (valor < 0)
            valor = 0;
        if (valor == (int)LinhaPosic)
            break;
        CursorEditor();
        if (valor > (int)Console->LinAtual)
            valor = (int)Console->LinAtual;
        LinhaPosic = valor;
        break;
    }
}

//------------------------------------------------------------------------------
const char * TVarTelaTxt::getTxt(int numfunc)
{
    static char txtnum[20];
    if (Console==0)
        return "";
    switch (numfunc)
    {
    case TelaTxtTexto:
        return LerLinha();
    case TelaTxtTotal:
        sprintf(txtnum, "%u", max_linha);
        return txtnum;
    case TelaTxtLinha:
        sprintf(txtnum, "%d", LinhaPosic);
        return txtnum;
    }
    return "";
}

//------------------------------------------------------------------------------
void TVarTelaTxt::setTxt(int numfunc, const char * txt)
{
    txt_linha[tam_linha]=0;
    if (Console==0 || strcmp(txt_linha, txt)==0)
        return;
    switch (numfunc)
    {
    case TelaTxtTexto:
        if (*txt==0)
        {
            tam_linha = 0;
            col_linha = 0xFFF;
            CursorEditor();     // Cursor na linha de edição
            ProcTeclaCursor(0); // Semelhante à tecla HOME
        }
        else
            addTxt(numfunc, txt);
        break;
    case TelaTxtTotal:
    case TelaTxtLinha:
        setValor(numfunc, TxtToInt(txt));
        break;
    }
}

//------------------------------------------------------------------------------
void TVarTelaTxt::addTxt(int numfunc, const char * txt)
{
    if (Console==0 || *txt==0)
        return;
    char * destino;
    switch (numfunc)
    {
    case TelaTxtTexto:
        destino = txt_linha + tam_linha;
        while (*txt && destino < txt_linha + max_linha)
        {
            switch (*txt)
            {
            case Instr::ex_barra_n:
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
            default:
                *destino++ = *txt++;
            }
        }
        tam_linha = destino - txt_linha;
        col_linha = 0xFFF;
        CursorEditor();     // Cursor na linha de edição
        ProcTeclaCursor(0); // Semelhante à tecla HOME
        break;
    case TelaTxtTotal:
    case TelaTxtLinha:
        setValor(numfunc, TxtToInt(txt));
        break;
    }
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::Inic()
{
    if (Console==0)
        return true;
    if (!Console->Inic())
    {
        delete Console;
        Console = 0;
        return false;
    }
// Acerta linha de edição
    Console->EnvTxt("\n", 1);    // Adiciona uma linha
    Console->CorTxt(CONSOLE_COR_LINHA); // Define frente branca e fundo azul
    Console->LimpaFim();         // Preenche do cursor até o fim da linha
// Acerta buffer com cópia da tela
    if (TelaBuf)
        delete[] TelaBuf;
    TelaBuf = new char[Console->LinTotal * Console->ColTotal];
    memset(TelaBuf, ' ', Console->LinTotal * Console->ColTotal);
    TelaLin = 0;
// Acerta o título da janela
    const char * titulo = TArqIncluir::ArqNome();
    Console->MudaTitulo(titulo);
    return true;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Fim()
{
    if (Console==0)
        return;
    if (TelaBuf)
    {
        delete[] TelaBuf;
        TelaBuf = 0;
    }
    for (; LinhaPosic>0; LinhaPosic--)
        putchar('\n');
    Console->Fim();
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Processa()
{
    if (Console==0)
        return;
    while (true)
    {
    // Lê tecla
        char mens[2048];
        const char * p = Console->LerTecla();
        if (p==0)
            break;
        //Console->Escrever(p);
    // Altera posição Y se for evento do mouse
        if (memcmp(p, "MOUSE", 5)==0 && p[5])
        {
            long valor;
            char * d = mens + 5;
            copiastr(mens, p, 100);
            while (*d && *d!=' ') d++;
            while (*d==' ') d++;
            while (*d && *d!=' ') d++;
            while (*d==' ') d++;
            valor = strtol(d, 0, 10); // Obtém a posição Y
            valor = LinhaFinal - valor;
            if (valor<0) valor=0;
            sprintf(d, "%d", (int)valor); // Anota a posição Y
            p = mens;
        }
    // Chama evento que processa tecla
        if (FuncEvento("tecla", p))
            continue;
    // Processa teclas exceto ENTER
        if (comparaZ(p, "ENTER")!=0)
        {
            ProcTecla(p);
            continue;
        }
    // Processa enter
        strcpy(mens, LerLinha());
        ProcTecla(p);
        FuncEvento("msg", mens);
    }
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::FuncEvento(const char * evento, const char * texto)
{
    if (Console==0)
        return false;
    //printf("FuncEvento [%s] [%s]\n", evento, texto); fflush(stdout);
    for (TVarTelaTxt * vobj = Inicio; vobj;)
    {
        bool prossegue = false;
    // Definido em objeto: prepara para executar
        if (vobj->b_objeto)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endobjeto, mens);
        }
    // Definido em classe: prepara para executar
        else if (vobj->endclasse)
        {
            char mens[80];
            sprintf(mens, "%s_%s", vobj->defvar+Instr::endNome, evento);
            prossegue = Instr::ExecIni(vobj->endclasse, mens);
        }
    // Executa
        ObjAtual = vobj->Depois;
        if (prossegue)
        {
            if (texto)
                Instr::ExecArg(texto);
            Instr::ExecArg(vobj->indice);
            if (Instr::ExecX())
                if (Instr::VarAtual >= Instr::VarPilha)
                    prossegue = !Instr::VarAtual->getBool();
            Instr::ExecFim();
            if (!prossegue)
                return true;
        }
    // Passa para próximo objeto
        vobj = ObjAtual;
    } // for (TVarTelaTxt ...
    return false;
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::Func(TVariavel * v, const char * nome)
{
// Envia mensagem
    if (comparaZ(nome, "msg")==0)
    {
        if (Console==0)
            return false;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
        {
            const char * texto = obj->getTxt();
            while (true)
            {
                const char * p = texto;
                while (*(unsigned char*)p >= ' ')
                    p++;
                if (p != texto)
                {
                    Escrever(texto, p-texto);
                    texto = p;
                }
                if (*texto==0)
                    break;
                switch (*texto++)
                {
                case Instr::ex_barra_n:
                    Escrever("\n", 1);
                    break;
                case Instr::ex_barra_b:
                    CorTela = 0x70;
                    break;
                case Instr::ex_barra_c:
                    {
                        char ch = *texto;
                        if (ch>='0' && ch<='9')
                        {
                            CorTela = (CorTela & 0x0F) +
                                    (ch - '0') * 0x10;
                            texto++;
                            break;
                        }
                        switch (ch | 0x20)
                        {
                        case 'a': CorTela=(CorTela|0xF0)-0x50; texto++; break;
                        case 'b': CorTela=(CorTela|0xF0)-0x40; texto++; break;
                        case 'c': CorTela=(CorTela|0xF0)-0x30; texto++; break;
                        case 'd': CorTela=(CorTela|0xF0)-0x20; texto++; break;
                        case 'e': CorTela=(CorTela|0xF0)-0x10; texto++; break;
                        case 'f': CorTela=(CorTela|0xF0);      texto++; break;
                        case 'g': CorTela |=  0x100; texto++; break;
                        case 'h': CorTela &= ~0x100; texto++; break;
                        case 'i': CorTela |=  0x200; texto++; break;
                        case 'j': CorTela &= ~0x200; texto++; break;
                        case 'k': CorTela |=  0x400; texto++; break;
                        case 'l': CorTela &= ~0x400; texto++; break;
                        case 'm':
                        case 'n':
                        case 'o': texto++; break;
                        case 'p': Console->Beep(); texto++; break;
                        }
                        break;
                    }
                case Instr::ex_barra_d:
                    if (*texto>='0' && *texto<='7')
                    {
                        CorTela = (CorTela & ~15) +
                                  (*texto - '0');
                        texto++;
                    }
                } // switch
            } // while
        } // for
        return false;
    }
// Posição do cursor
    if (comparaZ(nome, "posx")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(ColEscreve>=0xFFFF ? 0 : ColEscreve);
    }
// Processa tecla
    if (comparaZ(nome, "tecla")==0)
    {
        if (Console==0)
            return false;
        for (TVariavel * obj=v+1; obj<=Instr::VarAtual; obj++)
        {
            const char * texto = obj->getTxt();
            if (*texto)
                ProcTecla(texto);
        }
        return false;
    }
// Variável proto
    if (comparaZ(nome, "proto")==0)
    {
        Instr::ApagarVar(v);
        return Instr::CriarVarInt(Console ? 1 : 0);
    }
// Limpa a tela
    if (comparaZ(nome, "limpa")==0)
    {
        if (Console==0)
            return false;
        Console->CorTxt(0x70);
        Console->LimpaTela();
        Console->CursorPosic(1,0);
        Console->CorTxt(CONSOLE_COR_LINHA);
        Console->LimpaFim();  // Preenche do cursor até o fim da linha
        LinhaFinal = 0;
        LinhaPosic = 0;
        ColPosic = 0;
        ColEscreve = 0;
        int total = tam_linha - col_linha;
        if (total > 0)
            Console->EnvTxt(txt_linha + col_linha,
                            total<(int)Console->ColTotal-1 ? total : Console->ColTotal-1);
        Console->CursorCol(ColEditor - Console->ColAtual);
        memset(TelaBuf, ' ', Console->LinTotal * Console->ColTotal);
        return false;
    }
// Variáveis
    int x = 0;
    if (comparaZ(nome, "texto")==0)
        x = TelaTxtTexto;
    else if (comparaZ(nome, "total")==0)
        x = TelaTxtTotal;
    else if (comparaZ(nome, "linha")==0)
        x = TelaTxtLinha;
    if (x)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->numfunc = x;
        return true;
    }
    return false;
}
