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
#include <assert.h>
#ifdef __WIN32__
 #include <windows.h>
#endif
#include "classe.h"
#include "arqmapa.h"
#include "console.h"
#include "var-arqprog.h"
#include "var-arqexec.h"
#include "instr.h"
#include "instr-enum.h"
#include "misc.h"

//#define DEBUG

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Inicio = nullptr;
TArqMapa * TArqMapa::Fim = nullptr;
unsigned short TArqMapa::ParamLinha = 4000;
unsigned char TArqMapa::ParamN = 0;
unsigned char TArqMapa::ParamIndent = 2;
unsigned char TArqMapa::ParamClasse = 1;
unsigned char TArqMapa::ParamFunc = 1;
unsigned char TArqMapa::ParamVar = 0;

//------------------------------------------------------------------------------
TArqMapa::TArqMapa(const char * arqnome)
{
    copiastr(Arquivo, arqnome, sizeof(Arquivo));
    Anterior = Fim, Proximo = nullptr;
    (Fim ? Fim->Proximo : Inicio)=this;
    Fim=this;
    Mudou=false;
    ClInicio = nullptr, ClFim = nullptr;
    Existe=false;
#ifdef DEBUG
    printf("TArqMapa( %s )\n", Arquivo); fflush(stdout);
#endif
}

//------------------------------------------------------------------------------
TArqMapa::~TArqMapa()
{
#ifdef DEBUG
    printf("~TArqMapa( %s )\n", Arquivo); fflush(stdout);
#endif
    (Anterior ? Anterior->Proximo : Inicio) = Proximo;
    (Proximo ? Proximo->Anterior : Fim) = Anterior;
    assert(ClInicio == nullptr);
}

//----------------------------------------------------------------------------
bool TArqMapa::NomeValido(const char * nome, bool incluir)
{
    if (*nome == '.')
        return false;
    const char *o=nome;
    for (; *o; o++)
    {
        if (*o == '\\' || *o == '/')
        {
            if (o[1] == '.')
                return false;
            continue;
        }
        if ((*o < '0' || *o > '9') && (*o < 'a' || *o > 'z') &&
                *o != '-' && *o != '_' && *o != '.')
            return false;
    }
    if (o-nome >= INT_NOME_TAM)
        return false;
    if (!incluir && *nome)
    {
        o=nome+1;
        while (*o) o++;
        if (o[-1] == '/' || o[-1] == '\\')
            return false;
    }
    return true;
}

//------------------------------------------------------------------------------
TArqMapa * TArqMapa::Procura(const char * nome)
{
    for (TArqMapa * arqmapa = Inicio; arqmapa; arqmapa = arqmapa->Proximo)
        if (strcmp(nome, arqmapa->Arquivo) == 0)
            return arqmapa;
    return 0;
}

//------------------------------------------------------------------------------
void TArqMapa::SalvarArq(bool tudo)
{
    TArqMapa * arqmapa = Fim;
    char char_lf[100];
    memset(char_lf, '\n', sizeof(char_lf));
    //if (Inicio->Mudou) puts("-------------"); fflush(stdout);
    while (arqmapa)
    {
    // Checa se arquivo foi alterado
        if (!arqmapa->Mudou && !tudo)
        {
            arqmapa = arqmapa->Anterior;
            continue;
        }
    // Obtém o nome do arquivo
        mprintf(arqinicio, INT_NOME_TAM, "%s." INT_EXT,
                *arqmapa->Arquivo ? arqmapa->Arquivo
                                  : TArqIncluir::ArqNome());
        for (char * p = arqinicio; *p; p++)
#ifdef __WIN32__
            if (*p == '/') *p = '\\';
#else
            if (*p == '\\') *p = '/';
#endif
    // Verifica se arquivo vazio
        if (arqmapa->ClInicio == nullptr && *arqmapa->Arquivo)
        {
            if (arqmapa->Existe)
                remove(arqnome);
            TArqMapa * antes = arqmapa->Anterior;
            delete arqmapa;
            arqmapa = antes;
            continue;
        }
    // Cria arquivo
        FILE * arq = fopen("intmud-temp.txt", "w");
        if (arq == nullptr)
        {
            arqmapa = arqmapa->Anterior;
            continue;
        }
        if (*arqmapa->Arquivo == 0)
        {
            TArqIncluir * obj = TArqIncluir::IncluirIni();
            if (obj)
            {
                fprintf(arq, "# Nomes dos outros arquivos que compõem "
                             "o programa começam com:\n");
                for (; obj; obj = obj->IncluirProximo())
                    fprintf(arq, "incluir = %s\n", obj->IncluirNome());
                fprintf(arq, "\n");
            }
            fprintf(arq,
                "# Quantas instruções uma função chamada pelo programa pode\n"
                "# executar antes do controle retornar ao programa\n"
                "exec = %d\n\n"
                "# Se deve abrir uma janela de texto - variável telatxt\n"
                "telatxt = %d\n\n"
                "# Aonde apresentar mensagens de erro no programa\n"
                "log = %d\n\n"
                "# Erros em blocos de instruções:\n"
                "# 0=ignorar, 1=permitir apenas FimSe sem Se, 2=checar tudo\n"
                "err = %d\n\n",
                Instr::VarExecIni, Console != nullptr, opcao_log,
                Instr::ChecaLinha::ChecaErro);
            fprintf(arq,
                "# Se o programa roda sem restrições (0=não, 1=sim)\n"
                "# Em caso de dúvida, deixar 0. Nesse caso, as restrições são:\n"
                "# Não pode acessar arquivos fora do diretório e subdiretórios do programa\n"
                "# Não pode acessar executáveis e .bat .com .exe .pif .scr\n"
                "# Arquivos .int só podem ser alterados via PROG\n"
                "# Arquivos .log só podem ser alterados via ARQLOG\n"
                "# ArqExec só podem executar programas conforme as opções ArqExec\n"
                "completo = %d\n\n", opcao_completo);
            TArqExec * exec = TArqExec::ExecIni();
            if (exec)
            {
                fprintf(arq, "# Comandos que podem ser executados por ArqExec "
                    "(se completo=0)\n"
                    "# Um asterisco no final do comando significa qualquer texto\n"
                    "# Exemplo: arqexec=wget *     "
                    "Significa wget seguido de qualquer texto\n");
                for (; exec; exec = exec->ExecProximo())
                    fprintf(arq, "arqexec = %s\n", exec->ExecNome());
                fprintf(arq, "\n");
            }
        }
        for (TClasse * cl = arqmapa->ClInicio; cl; cl=cl->ArqDepois)
        {
            int linhas = 0;
        // Nome da classe
            fprintf(arq, "classe %s\n", cl->Nome);
            for (const char * p = cl->Comandos; p[0] || p[1]; p += Num16(p))
            {
        // Obtém tipo de espaçamento de linhas
                int indent = (unsigned char)p[Instr::endAlin];
                int tipo=-1;
                switch (p[2])
                {
                case Instr::cConstNulo:
                case Instr::cConstTxt:
                case Instr::cConstNum:
                case Instr::cConstExpr:
                case Instr::cConstVar:
                    tipo=ParamVar;
                    break;
                case Instr::cFunc:
                case Instr::cVarFunc:
                    tipo=ParamFunc;
                    break;
                default:
                    if (indent == 0 && p[2] >= Instr::cVariaveis)
                        tipo = ParamVar;
                }
        // Linhas entre funções e entre variáveis
                if (tipo >= 0)
                {
                    if (linhas < tipo)
                        linhas = tipo;
                    if (linhas > 0)
                        fwrite(char_lf, 1, linhas, arq);
                    linhas = tipo;
                }
        // Obtém texto correspondente à instrução
                char mens[BUF_MENS + BUF_MENS/2];
                char * o = mens + sizeof(mens) - BUF_MENS;
                int r = Instr::Decod(o, p, BUF_MENS);
                assert(r!=0);
                int espaco = (p[2]!=Instr::cComent ? indent : 0) * ParamIndent;
                if (espaco)
                {
                    if (espaco > 40)
                        espaco = 40;
                    o -= espaco;
                    memset(o, ' ', espaco);
                }
        // Divide em linhas
                char barran = 0;
                switch (p[2])
                {
                default:                if (ParamN < 3) break;
                case Instr::cConstNulo:
                case Instr::cConstNum:
                case Instr::cConstExpr:
                case Instr::cConstVar:  if (ParamN < 2) break;
                case Instr::cConstTxt:  if (ParamN < 1) break;
                    if ((int)strlen(o) >= ParamLinha - 10)
                        barran = 'n';
                }
                char * d = mens;
                char * dfim = d + ParamLinha - 10;
                while (*o)
                {
                // Divide linha
                    while (d >= dfim)
                    {
                        int x, total = dfim - d + 8;
                        if (total > 0 && o[-1] != ' ')
                            break;
                        for (x=0; x < total; x++)
                            if (o[x] == ' ')
                                break;
                        if (x<total)
                            break;
                        *d++ = '\\';
                        *d++ = '\n';
                        dfim = d+ParamLinha-10;
                        break;
                    }
                // Anota caracter se não começa com barra invertida
                    if (o[0] != '\\' || o[1] == 0)
                    {
                        *d++ = *o++;
                        continue;
                    }
                // Anota dois caracteres
                    memcpy(d, o, 2);
                    d += 2, o += 2;
                // Verifica se deve dividir em \n
                    if (o[-1] != barran || o[0] == '\"' || memcmp(o,"\\n", 2)==0)
                        continue;
                // Checa se está no final da linha
                    int x;
                    for (x = 0; x < 8; x++)
                    {
                        if (o[x] == 0 || o[x] == '\"')
                            break;
                        else if (o[x] == '\\' && o[x + 1])
                            x++;
                    }
                    if (x < 8 && d + x + 2 < dfim)
                        continue;
                // Divide
                    *d++ = '\\';
                    *d++ = '\n';
                    dfim = d+ParamLinha-10;
                }
                *d++ = '\n';
                *d = 0;
        // Anota instrução
                fprintf(arq, "%s", mens);
            }
        // Linhas entre classes
            if (cl->ArqDepois && ParamClasse)
                fwrite(char_lf, 1, ParamClasse, arq);
        }
        fclose(arq);
    // Renomeia arquivo
#ifdef DEBUG
        printf("TArqMapa::Salvar( %s )\n", arqnome); fflush(stdout);
#endif
#ifdef __WIN32__
        bool salvou = MoveFileEx("intmud-temp.txt", arqnome,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED);

#else
        bool salvou = (rename("intmud-temp.txt", arqnome) >= 0);
#endif
        if (salvou)
        {
            arqmapa->Existe = true;
            arqmapa->Mudou = false;
        }
        else
            remove("intmud-temp.txt");
        arqmapa = arqmapa->Anterior;
    }
}
