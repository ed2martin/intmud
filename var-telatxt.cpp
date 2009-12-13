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
#include "var-telatxt.h"
#include "console.h"
#include "variavel.h"
#include "instr.h"
#include "misc.h"

#define CONSOLE_MAX 1024
#define CONSOLE_COR_LINHA 0x74

const char TelaTxtTexto[] = { 8, 0, Instr::cTelaTxt, 0, 0, 0, '=', '0', 0 };
const char TelaTxtTotal[] = { 8, 0, Instr::cTelaTxt, 0, 0, 0, '=', '1', 0 };
const char TelaTxtLinha[] = { 8, 0, Instr::cTelaTxt, 0, 0, 0, '=', '2', 0 };

unsigned int TVarTelaTxt::CorTela = 0x70;
unsigned int TVarTelaTxt::CorBarraN = 0x70;
unsigned char TVarTelaTxt::EditorPosic = 0;
unsigned int TVarTelaTxt::LinhaPosic = 0;
unsigned int TVarTelaTxt::ColEscreve = 0;
unsigned int TVarTelaTxt::ColEditor = 0;
unsigned int TVarTelaTxt::col_linha = 0;
unsigned int TVarTelaTxt::tam_linha = 0;
unsigned int TVarTelaTxt::max_linha = CONSOLE_MAX - 1;
char * TVarTelaTxt::txt_linha = new char[CONSOLE_MAX];
TVarTelaTxt * TVarTelaTxt::Inicio = 0;
TVarTelaTxt * TVarTelaTxt::ObjAtual = 0;

//---------------------------------------------------------------------------
void TVarTelaTxt::Escrever(const char * texto, int tamanho)
{
    if (EditorPosic != 1)
    {
        if (EditorPosic==0)
            Console->CursorLin(-1);      // Cursor sobe uma linha
        else
            Console->CursorLin(LinhaPosic-1);
        EditorPosic = 1;
        Console->CursorIni();        // Cursor no início da linha
        if (ColEscreve && ColEscreve < Console->ColTotal)
            Console->CursorCol(ColEscreve); // Move o cursor para a direita
    }
    Console->CorTxt(CorTela);
    if (tamanho<0)
        tamanho = strlen(texto);
    for (; tamanho>0; texto++,tamanho--)
    {
        if (ColEscreve >= Console->ColTotal)
        {
            Console->EnvTxt("\n\n", 2);  // Dois '\n' para criar uma linha no final
            Console->CursorLin(-1);      // Cursor sobe uma linha
            Console->CorTxt(CorBarraN);
            Console->InsereLin(1);       // Insere uma linha
            Console->CorTxt(CorTela);
            ColEscreve = 0;
            if (LinhaPosic > 0 && LinhaPosic < Console->LinTotal - 1)
                LinhaPosic++;
        }
        if (*texto=='\n')
            ColEscreve = 0xFFFF;
        else
        {
            Console->EnvTxt(texto, 1);
            ColEscreve++;
        }
        CorBarraN = CorTela;
    }
}

//---------------------------------------------------------------------------
void TVarTelaTxt::CursorEditor()
{
    if (EditorPosic != 0)
    {
        if (EditorPosic == 1)
            Console->EnvTxt("\n", 1);
        else
            Console->CursorLin(LinhaPosic);
        EditorPosic = 0;
        Console->CorTxt(CONSOLE_COR_LINHA);
    }
    Console->CursorCol(ColEditor - Console->ColAtual);
}

//---------------------------------------------------------------------------
void TVarTelaTxt::ProcTecla(const char * texto)
{
    if (texto==0)
        return;
// Cursor na linha de edição
    CursorEditor();
// Tecla normal (texto)
    if (texto[0] && texto[1]==0)
    {
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
// Cima
    if (strcmp(texto, "UP")==0)
    {
        if (LinhaPosic < Console->LinTotal-1)
            LinhaPosic++;
        return;
    }
// Baixo
    if (strcmp(texto, "DOWN")==0)
    {
        if (LinhaPosic > 0)
            LinhaPosic--;
        return;
    }
// ENTER
    if (strcmp(texto, "ENTER")==0)
    {
        col_linha = 0;
        tam_linha = 0;
        ColEditor = 0;
        Console->CursorIni();  // Posicina o cursor no início da linha
        Console->LimpaFim();   // Limpa a linha
        return;
    }
// Esquerda
    if (strcmp(texto, "LEFT")==0)
    {
        ProcTeclaCursor(ColEditor + col_linha - 1);
        return;
    }
// Direita
    if (strcmp(texto, "RIGHT")==0)
    {
        ProcTeclaCursor(ColEditor + col_linha + 1);
        return;
    }
// Home
    if (strcmp(texto, "HOME")==0)
    {
        ProcTeclaCursor(0);
        return;
    }
// End
    if (strcmp(texto, "END")==0)
    {
        ProcTeclaCursor(tam_linha);
        return;
    }
// Control + esquerda
    if (strcmp(texto, "C_LEFT")==0)
    {
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
    if (strcmp(texto, "C_RIGHT")==0)
    {
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
// DEL
    if (strcmp(texto, "DEL")==0)
    {
        if (col_linha + ColEditor >= tam_linha)
            return;
    // Altera a linha
        tam_linha--;
        for (unsigned int x=col_linha + ColEditor; x<tam_linha; x++)
            txt_linha[x] = txt_linha[x+1];
    // Resultado na tela
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
    if (strcmp(texto, "BACK")==0)
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
        Console->CursorCol(-1);  // Recua cursor
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
    else if (EditorPosic != 2)
    {
    // Cursor na primeira coluna
        if (Console->ColAtual)
            Console->CursorIni();
    // Obtém número de linhas antes da linha de edição
        unsigned int lintexto = Console->LinAtual;
        if (EditorPosic==1)
            lintexto++;
    // Não sobe antes da primeira linha de texto
        if (LinhaPosic > lintexto)
            LinhaPosic = lintexto;
    // Obtém quantas linhas subir
        int sobe = LinhaPosic;
        if (EditorPosic==1)
            sobe--;
    // Sobe cursor
        Console->CursorLin(-sobe);
        EditorPosic = 2;
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
    move_mem(destino, this, sizeof(TVarTelaTxt));
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
int TVarTelaTxt::getValor(const char * defvar1)
{
    if (Console==0 || defvar1[Instr::endNome]!='=')
        return 0;
    switch (defvar1[Instr::endNome+1])
    {
    case '0':
        return atoi(LerLinha());
    case '1':
        return max_linha;
    case '2':
        return LinhaPosic;
    }
    return 0;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::setValor(const char * defvar1, int valor)
{
    if (Console==0 || defvar1[Instr::endNome]!='=')
        return;
    switch (defvar1[Instr::endNome+1])
    {
    case '0':
        {
            char mens[20];
            sprintf(mens, "%d", valor);
            setTxt(defvar1, mens);
            break;
        }
    case '1':
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
    case '2':
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
const char * TVarTelaTxt::getTxt(const char * defvar1)
{
    static char txtnum[20];
    if (Console==0 || defvar1[Instr::endNome]!='=')
        return "";
    switch (defvar1[Instr::endNome+1])
    {
    case '0':
        return LerLinha();
    case '1':
        sprintf(txtnum, "%u", max_linha);
        return txtnum;
    case '2':
        sprintf(txtnum, "%d", LinhaPosic);
        return txtnum;
    }
    return "";
}

//------------------------------------------------------------------------------
void TVarTelaTxt::setTxt(const char * defvar1, const char * txt)
{
    txt_linha[tam_linha]=0;
    if (Console==0 || defvar1[Instr::endNome]!='=' ||
            strcmp(txt_linha, txt)==0)
        return;
    const char * p;
    switch (defvar1[Instr::endNome+1])
    {
    case '0':
        p = copiastr(txt_linha, txt, max_linha + 1);
        tam_linha = p - txt_linha;
        col_linha = 0xFFF;
        CursorEditor();     // Cursor na linha de edição
        ProcTeclaCursor(0); // Semelhante à tecla HOME
        break;
    case '1':
    case '2':
        setValor(defvar1, atoi(txt));
        break;
    }
}

//------------------------------------------------------------------------------
void TVarTelaTxt::addTxt(const char * defvar1, const char * txt)
{
    if (Console==0 || defvar1[Instr::endNome]!='=' || *txt==0)
        return;
    switch (defvar1[Instr::endNome+1])
    {
    case '0':
        while (tam_linha < max_linha && *txt)
            txt_linha[tam_linha++] = *txt++;
        col_linha = 0xFFF;
        CursorEditor();     // Cursor na linha de edição
        ProcTeclaCursor(0); // Semelhante à tecla HOME
        break;
    case '1':
    case '2':
        setValor(defvar1, atoi(txt));
        break;
    }
}

//------------------------------------------------------------------------------
bool TVarTelaTxt::Inic()
{
    if (Console==0)
        return true;
    if (!Console->Inic())
        return false;
// Acerta linha de edição
    Console->EnvTxt("\n", 1);    // Adiciona uma linha
    Console->CorTxt(CONSOLE_COR_LINHA); // Define frente branca e fundo azul
    Console->LimpaFim();         // Preenche do cursor até o fim da linha
// Acerta o título da janela
    int total = arqext - arqinicio;
    if (total>0)
    {
        char mens[100];
        if (total > (int)sizeof(mens)-1)
            total = (int)sizeof(mens)-1;
        memcpy(mens, arqinicio, total);
        mens[total] = 0;
        Console->MudaTitulo(mens);
    }
    return true;
}

//------------------------------------------------------------------------------
void TVarTelaTxt::Fim()
{
    if (Console==0)
        return;
    if (EditorPosic)
    {
        if (EditorPosic != 1)
            for (int x=0; x<(int)LinhaPosic; x++)
                putchar('\n');
        putchar('\n');
    }
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
        const char * p = Console->LerTecla();
        if (p==0)
            break;
        //Console->Escrever(p);
    // Chama evento que processa tecla
        if (FuncEvento("tecla", p))
            continue;
    // Processa teclas exceto ENTER
        if (strcmp(p, "ENTER")!=0)
        {
            ProcTecla(p);
            continue;
        }
    // Processa enter
        char mens[2048];
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
    } // for (TVarSocket ...
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
                    if (*texto>='0' && *texto<='9')
                    {
                        CorTela = (CorTela & 0x0F) +
                                  (*texto - '0') * 0x10;
                        texto++;
                    }
                    else if ((*texto>='A' && *texto<='F') ||
                        (*texto>='a' && *texto<='f'))
                    {
                        CorTela = (CorTela & 0x0F) +
                                  ((*texto|0x20) - '7') * 0x10;
                        texto++;
                    }
                    break;
                case Instr::ex_barra_d:
                    if (*texto>='0' && *texto<='7')
                    {
                        CorTela = (CorTela & 0xF0) +
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
        return Instr::CriarVarInt(ColEscreve);
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
        return Instr::CriarVarInt(Console ? 5 : 0);
    }
// Gerar um bipe
    if (comparaZ(nome, "bipe")==0)
    {
        if (Console)
            Console->Beep();
        return false;
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
        EditorPosic = 0;
        LinhaPosic = 0;
        ColEscreve = 0;
        int total = tam_linha - col_linha;
        if (total > 0)
            Console->EnvTxt(txt_linha + col_linha,
                            total<(int)Console->ColTotal-1 ? total : Console->ColTotal-1);
        Console->CursorCol(ColEditor - Console->ColAtual);
        return false;
    }
// Variáveis
    const char * x = 0;
    if (comparaZ(nome, "texto")==0)
        x = TelaTxtTexto;
    else if (comparaZ(nome, "total")==0)
        x = TelaTxtTotal;
    else if (comparaZ(nome, "linha")==0)
        x = TelaTxtLinha;
    if (x)
    {
        Instr::ApagarVar(v+1);
        Instr::VarAtual->defvar = x;
        return true;
    }
    return false;
}
