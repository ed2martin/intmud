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
            ///< Operador de comparação
    void Igual(TVarDataHora * v);
            ///< Operador de atribuição igual
    bool Func(TVariavel * v, const char * nome);
            ///< Função da variável
    static int getTipo(int numfunc);
            ///< Retorna o tipo de variável
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
};

//----------------------------------------------------------------------------

#endif
