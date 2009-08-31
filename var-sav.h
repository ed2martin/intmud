#ifndef VAR_SAV_H
#define VAR_SAV_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarSav /// Variável arqsav
{
public:
    static void ProcEventos();
        ///< Processa eventos
    static bool Func(TVariavel * v, const char * nome);
        ///< Funções da variável arqsav
    static int Dia;  ///< Dia em relação ao início do ano, começa no 1
    static int Hora; ///< Hora atual
    static int Min;  ///< Minuto atual

private:
    static void Senha(char * senhacodif, const char * senha, char fator);
        ///< Codifica senha
    static int Tempo(const char * arqnome);
        ///< Obtém a quantidade de minutos para expirar
        /// Retorna: quantidade de minutos ou 0=expirou, -1=nunca expira
    static bool InicVar;
        ///< Se já inicializou variáveis
    static int HoraReg;
        ///< Tempo que corresponde a dia, hora e minuto
};

//----------------------------------------------------------------------------

#endif
