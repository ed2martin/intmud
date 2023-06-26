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
#include "instr.h"
#include "classe.h"
#include "misc.h"

using namespace Instr;

//------------------------------------------------------------------------------
/// Mostra como uma instrução está codificada
/**
 *  @param destino Endereço destino (string ASCIIZ)
 *  @param origem  Endereço origem (instrução codificada)
 *  @param tamanho Tamanho do buffer em destino
 *  @retval true Conseguiu codificar com sucesso
 *  @retval false Erro, destino contém a mensagem de erro
 */
bool Instr::Mostra(char * destino, const char * origem, int tamanho)
{
    int  expr=0;   // Índice da expressão numérica, 0=não há
    int  coment=0; // Índice do comentário, 0=não há
    char nome[VAR_NOME_TAM+20]; // Nome da instrução
    char * perro = destino;
    *nome=0;
    *destino=0;

// Comentário em variáveis
    if (origem[2] >= cVariaveis || origem[2] == cRefVar)
    {
        expr = origem[endIndice];
        if (expr == 0)
        {
            for (coment=endNome; origem[coment]; coment++);
            coment++;
        }
    }

// Anota o nome da instrução
    switch (origem[2])
    {
    case cHerda:         // 1 byte = número de classes
        {
            const char * p = origem+endVar+1;
            for (int total=(unsigned char)origem[endVar]; total; total--)
                while (*p++);
            if (p-origem+10 > tamanho)
            {
                copiastr(perro, "Espaço insuficiente");
                return false;
            }
            destino = copiastr(destino, "herda ");
            for (origem+=endVar+1; origem<p; origem++,destino++)
                *destino = (*origem==0 ? ',' : *origem);
            destino[-1]=0;
        }
        return true;
    case cExpr:  // Expressão numérica pura
        strcpy(nome, "=");
        expr=endVar;
        break;
    case cComent: // Comentário
        if ((int)strlen(origem+endVar) + endVar > tamanho)
        {
            copiastr(perro, "Espaço insuficiente");
            return false;
        }
        sprintf(destino, "# %s", origem + endVar);
        return true;

// Constrole de fluxo
    case cSe:        strcpy(nome, "se");        expr=endVar+2; break;
    case cSenao1:    strcpy(nome, "senao");     coment=endVar+2; break;
    case cSenao2:    strcpy(nome, "senao");     expr=endVar+2; break;
    case cFimSe:     strcpy(nome, "fimse");     coment=endVar; break;
    case cEnquanto:  strcpy(nome, "enquanto");  expr=endVar+2; break;
    case cEPara:     strcpy(nome, "epara ");    expr=endVar+6; break;
    case cEFim:      strcpy(nome, "efim");      coment=endVar+2; break;
    case cCasoVar:   strcpy(nome, "casovar ");  expr=endVar+2; break;
    case cCasoSe:
        if (tamanho<11)
        {
            copiastr(perro, "Espaço insuficiente");
            return false;
        }
        strcpy(destino, "casose \"");
        tamanho-=8, destino+=8;
        for (coment=endVar+4; origem[coment]; coment++)
        {
            if (tamanho<3)
            {
                copiastr(perro, "Espaço insuficiente");
                return false;
            }
            switch (origem[coment])
            {
            case ex_barra_n:
                destino[0] = '\\';
                destino[1] = 'n';
                destino+=2, tamanho-=2;
                break;
            case ex_barra_b:
                destino[0] = '\\';
                destino[1] = 'b';
                destino+=2, tamanho-=2;
                break;
            case ex_barra_c:
                destino[0] = '\\';
                destino[1] = 'c';
                destino+=2, tamanho-=2;
                break;
            case ex_barra_d:
                destino[0] = '\\';
                destino[1] = 'd';
                destino+=2, tamanho-=2;
                break;
            case '\"':
            case '\\':
                *destino++='\\', tamanho--;
            default:
                *destino++=origem[coment], tamanho--;
            }
        }
        *destino++ = '\"';
        *destino = 0;
        coment++;
        break;
    case cCasoSePadrao: strcpy(nome, "casose");    coment=endVar; break;
    case cCasoFim:      strcpy(nome, "casofim");   coment=endVar; break;
    case cRet1:         strcpy(nome, "ret");       coment=endVar; break;
    case cRet2:         strcpy(nome, "ret");       expr=endVar; break;
    case cSair1:        strcpy(nome, "sair");      coment=endVar+2; break;
    case cSair2:        strcpy(nome, "sair");      expr=endVar+2; break;
    case cContinuar1:   strcpy(nome, "continuar"); coment=endVar+2; break;
    case cContinuar2:   strcpy(nome, "continuar"); expr=endVar+2; break;
    case cTerminar:     strcpy(nome, "terminar");  coment=endVar; break;

// Variáveis
    case cVariaveis: break;
    case cTxt1:
        sprintf(nome, "txt%d", (unsigned char)origem[endIndice]+1);
        break;
    case cTxt2:
        sprintf(nome, "txt%d", (unsigned char)origem[endIndice]+257);
        break;
    case cInt1:      strcpy(nome, "int1"); break;
    case cInt8:      strcpy(nome, "int8"); break;
    case cUInt8:     strcpy(nome, "uint8"); break;
    case cInt16:     strcpy(nome, "int16"); break;
    case cUInt16:    strcpy(nome, "uint16"); break;
    case cInt32:     strcpy(nome, "int32"); break;
    case cUInt32:    strcpy(nome, "uint32"); break;
    case cIntInc:    strcpy(nome, "intinc"); break;
    case cIntDec:    strcpy(nome, "intdec"); break;
    case cReal:      strcpy(nome, "real"); break;
    case cReal2:     strcpy(nome, "real2"); break;
    case cRef:       strcpy(nome, "ref"); break;
    case cRefVar:    strcpy(nome, "refvar"); coment=0; break;
    case cConstNulo: strcpy(nome, "const (nulo)"); coment=0; break;
    case cConstTxt:  strcpy(nome, "const (txt)"); coment=0; break;
    case cConstNum:  strcpy(nome, "const (num)"); coment=0; break;
    case cConstExpr: strcpy(nome, "const"); coment=0; break;
    case cConstVar:  strcpy(nome, "varconst"); coment=0; break;
    case cFunc:      strcpy(nome, "func"); break;
    case cVarFunc:   strcpy(nome, "varfunc"); break;

// Variáveis extras
    case cListaObj:  strcpy(nome, "listaobj"); break;
    case cListaItem: strcpy(nome, "listaitem"); break;
    case cTextoTxt:  strcpy(nome, "textotxt"); break;
    case cTextoPos:  strcpy(nome, "textopos"); break;
    case cTextoVar:  strcpy(nome, "textovar"); break;
    case cTextoObj:  strcpy(nome, "textoobj"); break;
    case cNomeObj:   strcpy(nome, "nomeobj"); break;
    case cArqDir:    strcpy(nome, "arqdir"); break;
    case cArqLog:    strcpy(nome, "arqlog"); break;
    case cArqProg:   strcpy(nome, "arqprog"); break;
    case cArqExec:   strcpy(nome, "arqexec"); break;
    case cArqSav:    strcpy(nome, "arqsav"); break;
    case cArqTxt:    strcpy(nome, "arqtxt"); break;
    case cArqMem:    strcpy(nome, "arqmem"); break;
    case cIntTempo:  strcpy(nome, "inttempo"); break;
    case cIntExec:   strcpy(nome, "intexec"); break;
    case cTelaTxt:   strcpy(nome, "telatxt"); break;
    case cSocket:    strcpy(nome, "socket"); break;
    case cServ:      strcpy(nome, "serv"); break;
    case cProg:      strcpy(nome, "prog"); break;
    case cDebug:     strcpy(nome, "debug"); break;
    case cIndiceObj: strcpy(nome, "indiceobj"); break;
    case cIndiceItem: strcpy(nome, "indiceitem"); break;
    case cDataHora:  strcpy(nome, "datahora"); break;
    default:
        copiastr(perro, "Instrução não existe");
        return false;
    }

// Copia nome da instrução ou variável
    if (origem[2]>cVariaveis || origem[2]==cRefVar)
    {
        if ((int)strlen(nome) + (int)strlen(origem+endNome) + 2 > tamanho)
        {
            copiastr(perro, "Espaço insuficiente");
            return false;
        }
        sprintf(destino, "%s %s", nome, origem+endNome);
    }
    else if (*nome)
    {
        if ((int)strlen(nome) + 1 > tamanho)
        {
            copiastr(perro, "Espaço insuficiente");
            return false;
        }
        strcpy(destino, nome);
    }
    while (*destino)
       destino++, tamanho--;

// Constantes - obtém o índice
    if (origem[2]==cConstNulo || origem[2]==cConstTxt ||
        origem[2]==cConstNum  || origem[2]==cConstExpr ||
        origem[2]==cConstVar  || origem[2]==cRefVar)
    {
        expr=endNome;
        while (origem[expr++]);
        if (tamanho<3)
        {
            copiastr(perro, "Espaço insuficiente");
            return false;
        }
        strcpy(destino, " =");
        destino += 2;
        tamanho -= 2;
    }

// Verifica se tem comentário
    if (coment)
    {
        int total = Num16(origem) - coment;
        assert(total>=0);
        if (total>0)
        {
            if (total + 5 > tamanho)
            {
                copiastr(perro, "Espaço insuficiente");
                return false;
            }
            destino[0] = ' ';
            destino[1] = '#';
            destino[2] = ' ';
            if (total>0)
                memcpy(destino+3, origem+coment, total);
        }
        destino[3+total]=0;
        return true;
    }

// Verifica se tem expressão numérica
    if (expr==0)
        return true;
    const char * origem_fim = origem + Num16(origem);
    origem += expr;

    while (true)
    {
        *nome=0;
        assert(origem < origem_fim);
        switch (*origem)
        {
        case ex_fim:
            *destino=0;
            return true;
        case ex_coment: // Comentário
            if (tamanho<4)
                return false;
            strcpy(destino, " # ");
            destino+=3, tamanho-=3;
            for (origem++; *origem; origem++)
            {
                if (tamanho<2 || *(unsigned char*)origem < ' ')
                    return false;
                *destino = *origem;
                destino++, tamanho--;
            }
            *destino=0;
            return true;

        // Nome de variável ou função (um texto)
        case ex_varabre:     // Início do texto + abre colchetes
            strcpy(nome, " (varini)[");
            break;
        case ex_varfunc:     // Início do texto
            origem++;
            mprintf(nome, sizeof(nome), " (varfunc)%s",
                    ListaFunc[*(unsigned char*)origem].Nome);
            break;
        case ex_varlocal:
        case ex_varini:      // Início do texto
        case ex_doispontos:  // Dois pontos
        case ex_ponto:       // Fim do nome da variável
        case ex_fecha:       // Fecha colchetes
            if (tamanho<20)
                return false;
            if (*origem==ex_varlocal)
            {
                origem++;
                sprintf(destino, " (varlocal %d)", *(unsigned char*)origem);
            }
            else if (*origem==ex_varini)
                strcpy(destino, " (varini) ");
            else if (*origem==ex_ponto)
                strcpy(destino, " (ponto) ");
            else if (*origem==ex_doispontos)
                strcpy(destino, " (:) ");
            else
                strcpy(destino, " ]");
            while (*destino)
                destino++, tamanho--;
            for (origem++; *(unsigned char*)origem>=' '; )
            {
                if (tamanho<2)
                    return false;
                *destino++ = *origem++;
            }
            origem--;
            break;
        case ex_varfim:      // Fim do texto
            strcpy(nome, "(varfim)"); break;
        case ex_arg:         // Início da lista de argumentos
            strcpy(nome, " (argumentos)"); break;
        case ex_abre:        strcpy(nome, "["); break;

        // Valores fixos
        case ex_nulo:  strcpy(nome, " nulo"); break;
        case ex_txt: // Texto em ASCIIZ
            *destino++ = ' ';
            strcpy(nome, "\"");
            while (true)
            {
                if (tamanho<3)
                {
                    *destino=0;
                    return false;
                }
                if (*origem==0)
                    break;
                switch (*origem)
                {
                case ex_txt:
                    *destino++ = '\"', tamanho--;
                    break;
                case ex_barra_n:
                    destino[0] = '\\';
                    destino[1] = 'n';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_b:
                    destino[0] = '\\';
                    destino[1] = 'b';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_c:
                    destino[0] = '\\';
                    destino[1] = 'c';
                    destino+=2, tamanho-=2;
                    break;
                case ex_barra_d:
                    destino[0] = '\\';
                    destino[1] = 'd';
                    destino+=2, tamanho-=2;
                    break;
                case '\"':
                case '\\':
                    *destino++= '\\', tamanho--;
                default:
                    if (*(unsigned char*)origem >= ' ')
                        *destino++ = *origem, tamanho--;
                    else
                    {
                        *destino=0;
                        return false;
                    }
                }
                origem++;
            }
            break;
        case ex_num0:  strcpy(nome, " 0"); break;
        case ex_num1:  strcpy(nome, " 1"); break;
        case ex_num8p:
            sprintf(nome, " %u", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16p:
            sprintf(nome, " %u", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32p:
            sprintf(nome, " %u", Num32(origem+1));
            origem+=4;
            break;
        case ex_num8n:
            sprintf(nome, " -%u", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16n:
            sprintf(nome, " -%u", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32n:
            sprintf(nome, " -%u", Num32(origem+1));
            origem+=4;
            break;
        case ex_num8hexp:
            sprintf(nome, " 0x%X", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16hexp:
            sprintf(nome, " 0x%X", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32hexp:
            sprintf(nome, " 0x%X", Num32(origem+1));
            origem+=4;
            break;
        case ex_num8hexn:
            sprintf(nome, " -0x%X", (unsigned char)origem[1]);
            origem++;
            break;
        case ex_num16hexn:
            sprintf(nome, " -0x%X", Num16(origem+1));
            origem+=2;
            break;
        case ex_num32hexn:
            sprintf(nome, " -0x%X", Num32(origem+1));
            origem+=4;
            break;
        case ex_div1:        strcpy(nome, " /10"); break;
        case ex_div2:        strcpy(nome, " /100"); break;
        case ex_div3:        strcpy(nome, " /1K"); break;
        case ex_div4:        strcpy(nome, " /10K"); break;
        case ex_div5:        strcpy(nome, " /100K"); break;
        case ex_div6:        strcpy(nome, " /1M"); break;

        // Operadores numéricos
        case exo_ini:        break; // Marca o início dos operadores
        case exo_virgula:      strcpy(nome, " ,"); break;
        case exo_virg_expr:    strcpy(nome, " ,(expr)"); break;
        case exo_neg:          strcpy(nome, " -(unitário)"); break;
        case exo_exclamacao:   strcpy(nome, " !");  break;
        case exo_b_comp:       strcpy(nome, " ~");  break;
        case exo_add_antes:    strcpy(nome, " ++(antes)"); break;
        case exo_sub_antes:    strcpy(nome, " --(antes)"); break;
        case exo_add_depois:   strcpy(nome, " ++(depois)"); break;
        case exo_sub_depois:   strcpy(nome, " --(depois)"); break;
        case exo_add_sub1:     strcpy(nome, " (add_sub1)"); break;
        case exo_add_sub2:     strcpy(nome, " (add_sub2)"); break;
        case exo_mul:          strcpy(nome, " *");  break;
        case exo_div:          strcpy(nome, " /");  break;
        case exo_porcent:      strcpy(nome, " %");  break;
        case exo_add:          strcpy(nome, " +");  break;
        case exo_sub:          strcpy(nome, " -");  break;
        case exo_b_shl:        strcpy(nome, " <<"); break;
        case exo_b_shr:        strcpy(nome, " >>"); break;
        case exo_b_e:          strcpy(nome, " &");  break;
        case exo_b_ouou:       strcpy(nome, " ^");  break;
        case exo_b_ou:         strcpy(nome, " |");  break;
        case exo_menor:        strcpy(nome, " <");  break;
        case exo_menorigual:   strcpy(nome, " <="); break;
        case exo_maior:        strcpy(nome, " >");  break;
        case exo_maiorigual:   strcpy(nome, " >="); break;
        case exo_igual:        strcpy(nome, " =="); break;
        case exo_igual2:       strcpy(nome, " ==="); break;
        case exo_diferente:    strcpy(nome, " !="); break;
        case exo_diferente2:   strcpy(nome, " !=="); break;
        case exo_e:            strcpy(nome, " &&"); break;
        case exo_ou:           strcpy(nome, " ||"); break;
        case exo_atrib:        strcpy(nome, " =");  break;
        case exo_i_mul:        strcpy(nome, " *="); break;
        case exo_i_div:        strcpy(nome, " /="); break;
        case exo_i_porcent:    strcpy(nome, " %="); break;
        case exo_i_add:        strcpy(nome, " +="); break;
        case exo_i_sub:        strcpy(nome, " -="); break;
        case exo_i_b_shl:      strcpy(nome, " <<="); break;
        case exo_i_b_shr:      strcpy(nome, " >>="); break;
        case exo_i_b_e:        strcpy(nome, " &="); break;
        case exo_i_b_ouou:     strcpy(nome, " ^="); break;
        case exo_i_b_ou:       strcpy(nome, " |="); break;
        case exo_fim:        break; // Marca o fim dos operadores
        case exo_ee:           strcpy(nome, " &&início"); break;
        case exo_ouou:         strcpy(nome, " ||início"); break;
        case exo_int1:         strcpy(nome, " ?início"); break;
        case exo_int2:         strcpy(nome, " ?fim"); break;
        case exo_intint1:      strcpy(nome, " ??início"); break;
        case exo_intint2:      strcpy(nome, " ??fim"); break;
        case exo_dponto1:      strcpy(nome, " :início"); break;
        case exo_dponto2:      strcpy(nome, " :fim"); break;

        case ex_barra_n:    // \n - usado em textos
        case ex_barra_b:    // \b - usado em textos
        case ex_barra_c:    // \c - usado em textos
        case ex_barra_d:    // \d - usado em textos
        default:            // Caracter desconhecido
            *destino=0;
            return false;
        }
        origem++;
        if ((int)strlen(nome)+4 > tamanho)
        {
            *destino=0;
            return false;
        }
        strcpy(destino, nome);
        while (*destino)
            destino++, tamanho--;
    }
}
