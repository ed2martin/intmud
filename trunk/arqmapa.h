#ifndef ARQMAPA_H
#define ARQMAPA_H

//------------------------------------------------------------------------------
/** Lista de arquivos que comp�em o mapa.
    Se n�o houver arquivo, significa que o mapa est� em um �nico arquivo */
class TArqMapa /// Arquivos que comp�em o programa interpretado
{
public:
    TArqMapa(const char * classe);
    ~TArqMapa();
    static void Salvar(const char * classe);
            /**< Indica que deve salvar arquivo
                 @note Deve ser chamado quando uma classe for criada ou apagada */
    bool Mudou;
            /**< Se o arquivo foi alterado */
    char Arquivo[32];   /**< ??? de intmud-???.map;
                             Se for nulo, � o arquivo intmud.map */
    TArqMapa *Anterior;     ///< Lista ligada; objeto anterior
    TArqMapa *Proximo;      ///< Lista ligada; pr�ximo objeto
    static TArqMapa *Inicio;///< Lista ligada; primeiro objeto
    static TArqMapa *Fim;   ///< Lista ligada; �ltimo objeto
};

//------------------------------------------------------------------------------

#endif
