#ifndef VAR_SAV_H
#define VAR_SAV_H

//----------------------------------------------------------------------------
class TVariavel;
class TVarSav /// Vari�vel arqsav
{
public:
    static void ProcEventos();
        ///< Processa eventos
    static bool Func(TVariavel * v, const char * nome);
        ///< Fun��es da vari�vel arqsav
    static int Dia;  ///< Dia em rela��o ao in�cio do ano, come�a no 1
    static int Hora; ///< Hora atual
    static int Min;  ///< Minuto atual

private:
    static void Senha(char * senhacodif, const char * senha, char fator);
        ///< Codifica senha
    static int Tempo(const char * arqnome);
        ///< Obt�m a quantidade de minutos para expirar
        /// Retorna: quantidade de minutos ou 0=expirou, -1=nunca expira
    static bool InicVar;
        ///< Se j� inicializou vari�veis
    static int HoraReg;
        ///< Tempo que corresponde a dia, hora e minuto
};

//----------------------------------------------------------------------------

#endif
