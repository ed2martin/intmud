# Funcionamento da opção CRASH

Colocando uma linha `CRASH = 1` no início do arquivo *int* principal,
o programa passará a gerar automaticamente um arquivo de log sempre que
encerrar por erro fatal (crash).

Esse arquivo de log é bastante útil para identificar onde ocorreu a falha
e, em muitos casos, ajudar a resolver o problema sem a necessidade de
reproduzi-lo novamente.

---

## Nome do arquivo de log

O nome do arquivo de log é o nome do arquivo *int* principal acrescido
da palavra crash e a data do ocorrido.

Por exemplo, ao executar o programa com o comando:
```bash
./mud
```

o arquivo de log gerado poderá ter um nome como `mud-crash-2026-01-01.log`,
onde `2026-01-01` corresponde à data em que o erro ocorreu.

---

## Conteúdo do relatório (UNIX-like)

O formato do relatório pode variar conforme o sistema operacional.
Em sistemas UNIX-like (como Linux e BSD), o log inclui um *backtrace*
com endereços de memória, semelhante ao exemplo abaixo (os valores são
apenas ilustrativos):

```texto
===== 2026-01-15 12:40 intmud V1.16a Signal 11 SIGSEGV =====
./mud(+0x323de) [0x55d6185783de]
./mud(+0x912a) [0x55d61854f12a]
/lib/x86_64-linux-gnu/libc.so.6(+0x2a1ca) [0x7ffb6c62a1ca]
/lib/x86_64-linux-gnu/libc.so.6(+0x2a28b) [0x7ffb6c62a28b] <__libc_start_main+0x8b>
./mud(+0x9615) [0x55d61854f615]
```

---

## Identificando a linha do erro (UNIX-like)

É possível descobrir em qual arquivo e linha ocorreu o erro utilizando
o programa `addr2line`, passando como parâmetro os endereços que aparecem
entre parênteses.

Isso funciona desde que o executável **não tenha sido reduzido com o
programa `strip`** e tenha sido compilado com informações de depuração.
Isso é padrão quando quando se compila o programa.

Exemplo de comandos:

```bash
addr2line -e mud 0x323de
addr2line -e mud 0x912a
addr2line -e mud 0x9615
```

Resultado esperado (exemplo fictício):

```texto
/home/eu/mud/src/var-telatxt.cpp:632
/home/eu/mud/src/main.cpp:340
??:?
```

A linha `??:?` indica código sem informações de depuração disponíveis.

---

## Conteúdo do relatório (Windows)

No Windows, o arquivo de log possui formato semelhante, porém com
informações específicas da plataforma. Um exemplo típico é:

```text
===== 2026-01-15 12:40 intmud V1.16a EXCEPTION_ACCESS_VIOLATION em 004259C9 =====
004259C9
00401386
76C5ED6C BaseThreadInitThunk
772A37EB RtlInitializeExceptionChain
772A37BE RtlInitializeExceptionChain
```

---

## Identificando a linha do erro (Windows)

Assim como nos sistemas UNIX-like, é possível usar `addr2line` para
traduzir os endereços para arquivo e linha, desde que o executável
contenha informações de depuração.

Exemplo:

```bash
addr2line -e mud/mud.exe 004259c9
addr2line -e mud/mud.exe 00401386
```

Resultado ilustrativo:

```text
C:\msys32\home\eu\intmud\mud\src\var-telatxt.cpp:632
E:/mingwbuild/mingw-w64-crt-git/src/mingw-w64/mingw-w64-crt/crt/crtexe.c:341
```

---

## Observações importantes

* O executável distribuído junto ao pacote foi compilado em 32 bits e
  reduzido com o programa `strip`, com o objetivo de diminuir seu tamanho.
* Mesmo sem informações completas de depuração, o arquivo de log ainda
  fornece dados úteis para análise do problema.
