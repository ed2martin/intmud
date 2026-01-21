#ifndef VAR_DATAHORA_H
#define VAR_DATAHORA_H

class TVariavel;
class TVarInfo;
enum TVarTipo : unsigned char;
class TClasse;
class TObjeto;

//----------------------------------------------------------------------------
class TVarDataHora /// Data e hora
{
public:
    static const TVarInfo * Inicializa();
        ///< Inicializa variável e retorna informações
    void LerSav(const char * texto);
        ///< Atualiza datahora com o valor salvo em arqsav
    void SalvarSav(char * texto);
        ///< Prepara um texto com data e hora para arqsav

private:
    void Mover(TVarDataHora * destino);
        ///< Move para outro lugar
    int  getInt(int numfunc);
        ///< Ler o valor numérico da variável como int
    double getDouble(int numfunc);
        ///< Ler o valor numérico da variável como double

    int DiasMes();          ///< Calcula quantos dias tem no mês
    int DataNum();          ///< Calcula o número de dias desde 1/1/1
    void NumData(int dias); ///< Obtém dia/mês/ano a partir do número de dias
    unsigned short Ano;     ///< Variável ano
    unsigned char Mes;      ///< Variável mes
    unsigned char Dia;      ///< Variável dia
    unsigned char Hora;     ///< Variável hora
    unsigned char Min;      ///< Variável min
    unsigned char Seg;      ///< Variável seg

    static bool FuncAgora(TVariavel * v); ///< Processa função Agora
    static bool FuncAno(TVariavel * v); ///< Processa função Ano
    static bool FuncAntes(TVariavel * v); ///< Processa função Antes
    static bool FuncBissexto(TVariavel * v); ///< Processa função Bissexto
    static bool FuncDepois(TVariavel * v); ///< Processa função Depois
    static bool FuncDia(TVariavel * v); ///< Processa função Dia
    static bool FuncDiaSem(TVariavel * v); ///< Processa função DiaSem
    static bool FuncHora(TVariavel * v); ///< Processa função Hora
    static bool FuncMes(TVariavel * v); ///< Processa função Mes
    static bool FuncMin(TVariavel * v); ///< Processa função Min
    static bool FuncNovaData(TVariavel * v); ///< Processa função NovaData
    static bool FuncNovaHora(TVariavel * v); ///< Processa função NovaHora
    static bool FuncNumDias(TVariavel * v); ///< Processa função NumDias
    static bool FuncNumSeg(TVariavel * v); ///< Processa função NumSeg
    static bool FuncNumTotal(TVariavel * v); ///< Processa função NumTotal
    static bool FuncSeg(TVariavel * v); ///< Processa função Seg


    static int FTamanho(const char * instr);
    static int FTamanhoVetor(const char * instr);
    static TVarTipo FTipo(TVariavel * v);
    static void FRedim(TVariavel * v, TClasse * c, TObjeto * o,
            unsigned int antes, unsigned int depois);
    static void FMoverEnd(TVariavel * v, void * destino,
            TClasse * c, TObjeto * o);
    static bool FGetBool(TVariavel * v);
    static int FGetInt(TVariavel * v);
    static double FGetDouble(TVariavel * v);
    static const char * FGetTxt(TVariavel * v);
    static void FOperadorAtrib(TVariavel * v1, TVariavel * v2);
    static bool FOperadorIgual2(TVariavel * v1, TVariavel * v2);
    static unsigned char FOperadorCompara(TVariavel * v1, TVariavel * v2);
};

//----------------------------------------------------------------------------

#endif
