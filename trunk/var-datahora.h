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
    int  getValor(const char * defvar1);
            ///< Ler o valor numérico da variável
    void setValor(const char * defvar1, int valor);
            ///< Mudar o valor numérico da variável

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
