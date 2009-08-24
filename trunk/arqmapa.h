#ifndef ARQMAPA_H
#define ARQMAPA_H

#define MAPA_NOME_TAM 32 // Tamanho m�ximo do prefixo dos nomes dos arquivos

//------------------------------------------------------------------------------
class TClasse;
/** Lista de arquivos que comp�em o mapa.
    Se n�o houver arquivo, significa que o mapa est� em um �nico arquivo */
class TArqMapa /// Arquivos que comp�em o programa interpretado
{
public:
    TArqMapa(const char * classe);
    ~TArqMapa();
    static bool NomeValido(const char * nome);
        ///< Verifica se nome � um nome v�lido para arquivo
        /**<
         *  @param nome Nome a pesquisar
         *  @return true se o nome � valido */
    static TArqMapa * Procura(const char * nome);
        ///< Procura um arquivo a partir do nome
        /**<
         *  @param nome Nome a pesquisar
         *  @return Endere�o do objeto, ou 0 se n�o foi encontrado */
    static void SalvarArq(bool tudo);
            ///< Salva classes em arquivo
            /**< @param tudo true=salvar todas as classes,
                             false=salvar s� o que foi modificado */
    static unsigned char ParamIndent; ///< Quantos espa�os para indenta��o
    static unsigned char ParamClasse; ///< Linhas entre classes
    static unsigned char ParamFunc;   ///< Linhas entre fun��es
    static unsigned char ParamVar;    ///< Linhas entre vari�veis
    static bool MapaGrande; ///< Se est� trabalhando com v�rios arquivos

    bool Mudou;             ///< Se o arquivo foi alterado
    bool Existe;            ///< Se o arquivo j� existe
    char Arquivo[MAPA_NOME_TAM];   /**< ??? de intmud-???.map;
                             Se for nulo, � o arquivo intmud.map */

// Objetos TClasse
    TClasse * ClInicio;     ///< Primeira classe do arquivo
    TClasse * ClFim;        ///< �ltima classe do arquivo

// Objetos TArqMapa
    TArqMapa *Anterior;     ///< Lista ligada; objeto anterior
    TArqMapa *Proximo;      ///< Lista ligada; pr�ximo objeto
    static TArqMapa *Inicio;///< Lista ligada; primeiro objeto
    static TArqMapa *Fim;   ///< Lista ligada; �ltimo objeto
};

//------------------------------------------------------------------------------

#endif
