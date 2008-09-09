#ifndef ARQMAPA_H
#define ARQMAPA_H

//------------------------------------------------------------------------------
// Lista de arquivos que compõem o mapa
// Se não houver arquivo, significa que o mapa está em um único arquivo
class TArqMapa
{
public:
    TArqMapa(const char * classe);
    ~TArqMapa();
    static void Salvar(const char * classe);
                 // Indica que deve salvar arquivo
                 // Deve ser chamado quando uma classe for criada ou apagada
    bool Mudou; // Se o arquivo foi alterado
    char Arquivo[32];   // ??? de intmud-???.map
                        // Se for nulo, é o arquivo intmud.map
    TArqMapa *Anterior, *Proximo;
    static TArqMapa *Inicio,*Fim;
};

//------------------------------------------------------------------------------

#endif
