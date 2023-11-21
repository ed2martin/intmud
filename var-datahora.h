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
    void Mover(TVarDataHora * destino);
            ///< Move para outro lugar
    int  Compara(TVarDataHora * v);
            ///< Operador de comparação
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável
    int  getInt(int numfunc);
            ///< Ler o valor numérico da variável como int
    double getDouble(int numfunc);
            ///< Ler o valor numérico da variável como double
    void setInt(int numfunc, int valor);
            ///< Mudar o valor numérico da variável como int
    void setDouble(int numfunc, double valor);
            ///< Mudar o valor numérico da variável como double
    void LerSav(const char * texto);
            ///< Atualiza datahora com o valor salvo em arqsav
    void SalvarSav(char * texto);
            ///< Prepara um texto com data e hora para arqsav

    int DiasMes();          ///< Calcula quantos dias tem no mês
    int DataNum();          ///< Calcula o número de dias desde 1/1/1
    void NumData(int dias); ///< Obtém dia/mês/ano a partir do número de dias
    unsigned short Ano;     ///< Variável ano
    unsigned char Mes;      ///< Variável mes
    unsigned char Dia;      ///< Variável dia
    unsigned char Hora;     ///< Variável hora
    unsigned char Min;      ///< Variável min
    unsigned char Seg;      ///< Variável seg

private:
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
    static void FOperadorAtrib(TVariavel * v);
};

//----------------------------------------------------------------------------

#endif
