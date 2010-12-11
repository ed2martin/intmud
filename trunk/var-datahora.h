#ifndef VAR_DATAHORA_H
#define VAR_DATAHORA_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarDataHora /// Data e hora
{
public:
    void Criar();
            ///< Chamado ao criar objeto
    void Mover(TVarDataHora * destino);
            ///< Move para outro lugar
    int  Compara(TVarDataHora * v);
            ///< Operador de compara��o
    void Igual(TVarDataHora * v);
            ///< Operador de atribui��o igual
    bool Func(TVariavel * v, const char * nome);
            ///< Fun��o da vari�vel
    static int getTipo(int numfunc);
            ///< Retorna o tipo de vari�vel
    int  getInt(int numfunc);
            ///< Ler o valor num�rico da vari�vel como int
    double getDouble(int numfunc);
            ///< Ler o valor num�rico da vari�vel como double
    void setInt(int numfunc, int valor);
            ///< Mudar o valor num�rico da vari�vel como int
    void setDouble(int numfunc, double valor);
            ///< Mudar o valor num�rico da vari�vel como double
    void LerSav(const char * texto);
            ///< Atualiza datahora com o valor salvo em arqsav
    void SalvarSav(char * texto);
            ///< Prepara um texto com data e hora para arqsav

    int DiasMes();          ///< Calcula quantos dias tem no m�s
    int DataNum();          ///< Calcula o n�mero de dias desde 1/1/1
    void NumData(int dias); ///< Obt�m dia/m�s/ano a partir do n�mero de dias
    unsigned short Ano;     ///< Vari�vel ano
    unsigned char Mes;      ///< Vari�vel mes
    unsigned char Dia;      ///< Vari�vel dia
    unsigned char Hora;     ///< Vari�vel hora
    unsigned char Min;      ///< Vari�vel min
    unsigned char Seg;      ///< Vari�vel seg
};

//----------------------------------------------------------------------------

#endif
