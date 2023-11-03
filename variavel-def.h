#ifndef VARIAVEL_DEF_H
#define VARIAVEL_DEF_H

#define VARIAVEL_MOVER_SIMPLES(classname) \
    classname * o1 = reinterpret_cast<classname*>(v->endvar); \
    classname * d1 = reinterpret_cast<classname*>(destino);   \
    int total = (unsigned char)v->defvar[Instr::endVetor];    \
    if (total == 0)                          \
        o1->Mover(d1);                       \
    else if (d1 > o1)                        \
    {                                        \
        o1 += total - 1;                     \
        d1 += total - 1;                     \
        for (; total; total--,o1--,d1--)     \
            o1->Mover(d1);                   \
    }                                        \
    else                                     \
    {                                        \
        for (; total; total--,o1++,d1++)     \
            o1->Mover(d1);                   \
    }

#define VARIAVEL_MOVER_OBJETO(classname) \
    classname * o1 = reinterpret_cast<classname*>(v->endvar); \
    classname * d1 = reinterpret_cast<classname*>(destino);   \
    int total = (unsigned char)v->defvar[Instr::endVetor];    \
    if (total == 0)                          \
        o1->Mover(d1, o);                    \
    else if (d1 > o1)                        \
    {                                        \
        o1 += total - 1;                     \
        d1 += total - 1;                     \
        for (; total; total--,o1--,d1--)     \
            o1->Mover(d1, o);                \
    }                                        \
    else                                     \
    {                                        \
        for (; total; total--,o1++,d1++)     \
            o1->Mover(d1, o);                \
    }

#define VARIAVEL_MOVER_COMPLETO(classname) \
    classname * o1 = reinterpret_cast<classname*>(v->endvar); \
    classname * d1 = reinterpret_cast<classname*>(destino);   \
    int total = (unsigned char)v->defvar[Instr::endVetor];    \
    if (total == 0)                         \
    {                                       \
        o1->EndObjeto(c, o);                \
        o1->Mover(d1);                      \
    }                                       \
    else if (d1 > o1)                       \
    {                                       \
        o1 += total - 1;                    \
        d1 += total - 1;                    \
        for (; total; total--,o1--,d1--)    \
        {                                   \
            o1->EndObjeto(c, o);            \
            o1->Mover(d1);                  \
        }                                   \
    }                                       \
    else                                    \
    {                                       \
        for (; total; total--,o1++,d1++)    \
        {                                   \
            o1->EndObjeto(c, o);            \
            o1->Mover(d1);                  \
        }                                   \
    }

#define VARIAVEL_MOVERDEF(classname) \
    classname * ref = reinterpret_cast<classname*>(v->endvar); \
    int total = (unsigned char)v->defvar[Instr::endVetor]; \
    if (total == 0)                                        \
        ref[0].defvar = v->defvar;                         \
    else                                                   \
    {                                                      \
        for (int cont = 0; cont < total; cont++)           \
            ref[cont].defvar = v->defvar;                  \
    }

#define VARIAVEL_FGETINT0(classname) \
{ \
    classname * ref = reinterpret_cast<classname*>(v->endvar) + v->indice; \
    return ref->getValor(); \
}

#define VARIAVEL_FGETINT1(classname) \
{ \
    classname * ref = reinterpret_cast<classname*>(v->endvar) + v->indice; \
    return ref->getValor(v->numfunc); \
}

#define VARIAVEL_FGETTXT0(classname) \
{ \
    classname * ref = reinterpret_cast<classname*>(v->endvar) + v->indice; \
    char * buf = TVarInfo::BufferTxt(); \
    sprintf(buf, "%d", ref->getValor()); \
    return buf; \
}

#define VARIAVEL_FGETTXT1(classname) \
{ \
    classname * ref = reinterpret_cast<classname*>(v->endvar) + v->indice; \
    char * buf = TVarInfo::BufferTxt(); \
    sprintf(buf, "%d", ref->getValor(v->numfunc)); \
    return buf; \
}

#endif
