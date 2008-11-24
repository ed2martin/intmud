#include "instr.h"
#include "misc.h"

//------------------------------------------------------------------------------
// Retorna um número que corresponde à prioridade do operador
// Retorna um valor de 2 a 0x1F, ou 0 se operador inválido
int Instr::Prioridade(int operador)
{
    switch (operador)
    {
    case exo_virgula:    return 20;
    case exo_neg:        return 2;
    case exo_exclamacao: return 2;
    case exo_mul:        return 3;
    case exo_div:        return 3;
    case exo_porcent:    return 3;
    case exo_add:        return 4;
    case exo_sub:        return 4;
    case exo_menor:      return 5;
    case exo_menorigual: return 5;
    case exo_maior:      return 5;
    case exo_maiorigual: return 5;
    case exo_igual:      return 6;
    case exo_diferente:  return 6;
    case exo_e:          return 7;
    case exo_ou:         return 8;
    case exo_igualmul:   return 9;
    case exo_igualdiv:   return 9;
    case exo_igualporcent: return 9;
    case exo_igualadd:   return 9;
    case exo_igualsub:   return 9;
    }
    return 0;
}

//------------------------------------------------------------------------------
// Verifica se instrução codificada é herda e contém a classe especificada
bool Instr::ChecaHerda(const char * instr, const char * nomeclasse)
{
    if (instr[0]==0 && instr[1]==0)
        return false;
    if (instr[2] != cHerda)
        return false;
    int x = (unsigned char)instr[3];
    for (instr+=4; x; x--)
    {
        if (comparaZ(instr, nomeclasse)==0)
            return true;
        while (*instr++);
    }
    return false;
}

//------------------------------------------------------------------------------
Instr::ChecaLinha::ChecaLinha()
{
    esperando=0;
}

//------------------------------------------------------------------------------
void Instr::ChecaLinha::Inicio()
{
    esperando=0;
}

//------------------------------------------------------------------------------
const char * Instr::ChecaLinha::Instr(const char * instr)
{
    if (instr[0]==0 && instr[1]==0)
        return 0;
    unsigned char cod = *(unsigned char*)(instr+2);
// Instrução Herda
    if (cod==cHerda)
    {
        if (esperando!=0)
            return "Instrução Herda deve ser a primeira da classe";
        esperando=1;
        return 0;
    }
    if (esperando==0)
        esperando=1;
// Comentário
    if (cod==cComent)
        return 0;
// Função
    if (cod==cFunc || cod==cVarFunc)
    {
        esperando=2;
        return 0;
    }
// Constante
    if (cod==cConstNulo || cod==cConstTxt ||
        cod==cConstNum  || cod==cConstExpr)
    {
        if (esperando!=1)
            esperando=4;
        return 0;
    }
// Variável
    if (cod >= cVariaveis)
    {
        if (esperando == 3)
            return "Variável deve ser definidas no início da função";
        if (esperando > 3)
            return "Variável não pertence a uma classe ou uma função";
        return 0;
    }
// Instrução
    if (esperando<2 || esperando>3)
        return "Instrução não pertence a uma função";
    esperando=3;
    return 0;
}
