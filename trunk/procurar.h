#ifndef PROCURA_H
#define PROCURA_H

//----------------------------------------------------------------------------
/** Procura a ocorr�ncia de um texto em outro; pode realizar substitu��es */
class TProcurar /// Procura texto
{
public:
    TProcurar();
    bool Padrao(const char * padrao, bool exato);
    int  Proc(const char * texto, int tamanho);

    char * dest;        ///< Aonde colocar o texto modificado, ou 0
    int  tamdest;       ///< Tamanho do buffer em dest
    char troca[4096];   ///< Texto que dever� substituir o padr�o

//private:
    int  tabela[0x100]; ///< Tabela de deslocamento
    char padrao[4096];  ///< Texto procurado
    int  tampadrao;     ///< Tamanho do padr�o
    bool exato;         ///< Se diferencia min�sculas de mai�sculas
    void AddTroca(const char * texto);
    void AddTroca();
    const char * lidoatual; ///< At� aonde j� anotou o texto lido
    const char * lidofim;   ///< Marca o fim do texto em lidoatual
    char * destatual;   ///< Aonde anotar o texto
    char * destfim;     ///< dest + tamdest - 1
};

#endif
