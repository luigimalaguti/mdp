# Makefile

Questo documento spiega, dalla A alla Z, il funzionamento di `common.mk`, il Makefile universale alla radice del repository. Ogni cartella-programma lo importa con una sola riga e ottiene build, test, esecuzione e supporto clangd senza alcuna configurazione aggiuntiva.

> **Nota di manutenzione**: questo documento descrive `common.mk` così com'è
> oggi. Se modifichi il Makefile, aggiorna anche questo file.

## Indice

1. [Filosofia e panoramica](#1-filosofia-e-panoramica)
2. [Come lavora make: le basi necessarie](#2-come-lavora-make-le-basi-necessarie)
3. [Il layout dei progetti](#3-il-layout-dei-progetti)
4. [Il file, sezione per sezione](#4-il-file-sezione-per-sezione)
5. [I flussi completi: cosa succede quando...](#5-i-flussi-completi-cosa-succede-quando)
6. [Personalizzare un singolo progetto](#6-personalizzare-un-singolo-progetto)
7. [Integrazione con Zed](#7-integrazione-con-zed)
8. [Convenzioni obbligatorie e limiti noti](#8-convenzioni-obbligatorie-e-limiti-noti)
9. [Risoluzione dei problemi](#9-risoluzione-dei-problemi)
10. [Glossario delle funzioni make usate](#10-glossario-delle-funzioni-make-usate)

## 1. Filosofia e panoramica

Il principio guida è **convenzione al posto di configurazione**: ogni progetto rispetta un layout standard e in cambio non deve configurare nulla. Il Makefile di ogni cartella si riduce a:

```make
include ../common.mk
```

Da questa sola riga il progetto ottiene:

- **rilevamento automatico del linguaggio**: C, C++ o misto, senza dichiararlo;
- **rilevamento programma/libreria**: se un sorgente di `src/` definisce `main`, viene prodotto l'eseguibile `bin/main`; altrimenti il progetto è una libreria e vengono prodotti solo gli eseguibili di test;
- **test**: ogni file in `test/` diventa un eseguibile autonomo;
- **librerie vendorizzate**: ogni `external/<lib>/` viene compilata insieme al progetto, con i suoi header nei percorsi di ricerca;
- **build incrementali consapevoli degli header**: toccare un header ricompila solo gli oggetti che lo includono;
- **supporto clangd**: `compile_commands.json` viene rigenerato a ogni build, quindi il language server vede esattamente i flag reali.

Il nome fisso `bin/main` non è un vezzo: è ciò che permette a `.zed/debug.json` di puntare a `$ZED_DIRNAME/../bin/main` per *qualunque* programma, senza configurazione per-cartella (vedi [§7](#7-integrazione-con-zed)).

I target disponibili in ogni progetto:

| Target                  | Effetto                                                          |
| ----------------------- | ---------------------------------------------------------------- |
| `make`, `make all`      | build incrementale di binario e/o test + `compile_commands.json` |
| `make run ARGS="a b c"` | build, poi esegue `bin/main` con gli argomenti dati              |
| `make test`             | build, poi esegue tutti gli eseguibili di test                   |
| `make clean`            | rimuove `build/`, `bin/` e `compile_commands.json`               |
| `make rebuild`          | `clean` + `all`                                                  |

## 2. Come lavora make: le basi necessarie

Per capire il file serve avere chiari cinque concetti. Se li conosci già, salta al [§3](#3-il-layout-dei-progetti).

### 2.1 Le due fasi

Make lavora in due fasi distinte:

1. **Fase di lettura (parsing)**: legge il Makefile dall'alto verso il basso, espande le variabili assegnate con `:=`, esegue le funzioni `$(shell ...)`, valuta i condizionali `ifeq`, e costruisce in memoria il *grafo delle dipendenze* (chi dipende da chi).
2. **Fase di esecuzione**: partendo dal target richiesto, visita il grafo, confronta le date di modifica di target e prerequisiti, e lancia le recipe (i comandi shell) solo per ciò che è scaduto.

Questo spiega perché in `common.mk` i comandi come `find` e `grep` girano *a ogni invocazione di make*, ancora prima che qualsiasi compilazione parta: sono nella fase di lettura.

### 2.2 Regole: target, prerequisiti, recipe

```make
target: prerequisito1 prerequisito2
	comando
```

Make ricostruisce `target` se non esiste o se almeno un prerequisito è più recente di lui. La riga di comando (recipe) **deve iniziare con un carattere TAB**, non spazi. Un `@` a inizio comando ne sopprime l'eco a video.

### 2.3 Tipi di assegnazione

| Sintassi      | Significato                                                                |
| ------------- | -------------------------------------------------------------------------- |
| `X := valore` | espansione **immediata**: il lato destro è valutato subito, una volta sola |
| `X = valore`  | espansione **pigra**: il lato destro è rivalutato a ogni uso di `$(X)`     |
| `X ?= valore` | assegna **solo se `X` non ha già un valore**                               |
| `X += valore` | **appende** al valore esistente                                            |

In `common.mk` quasi tutto usa `:=` (i risultati di `find` non devono essere ricalcolati a ogni uso) e `?=` per tutto ciò che un progetto può voler sovrascrivere.

### 2.4 Variabili automatiche

Dentro una recipe, make mette a disposizione delle variabili che si riferiscono alla regola in corso:

| Variabile | Valore                                                        |
| --------- | ------------------------------------------------------------- |
| `$@`      | il target che si sta costruendo                               |
| `$<`      | il **primo** prerequisito (tipicamente il sorgente)           |
| `$^`      | **tutti** i prerequisiti (tipicamente gli oggetti da linkare) |

### 2.5 Pattern rules e `.PHONY`

Una **pattern rule** usa `%` come jolly e insegna a make una *trasformazione*:

```make
build/%.c.o: %.c
	$(CC) ... -c $< -o $@
```

significa "qualunque `build/X.c.o` si ottiene compilando `X.c`", per qualunque `X`, sottocartelle comprese.

`.PHONY` dichiara che un target è un *comando*, non un file: `make clean` deve girare sempre, anche se esistesse un file chiamato `clean` nella cartella.

## 3. Il layout dei progetti

```
mio-progetto/
├── Makefile            # una riga: include ../common.mk (+ eventuali override)
├── src/                # sorgenti della libreria/programma (*.c, *.cpp)
│   └── qualunque/      # ...le sottocartelle sono supportate
├── include/            # header del progetto (opzionale)
│   └── internal/       # ...anche qui sottocartelle libere
├── test/               # test (opzionale): OGNI file → un eseguibile bin/<nome>
├── external/           # librerie vendorizzate (opzionale)
│   └── <lib>/
│       ├── include/    # header della libreria → aggiunti a -I
│       └── src/        # sorgenti della libreria → compilati insieme
├── build/              # (generata) oggetti .o e file di dipendenza .d
├── bin/                # (generata) eseguibili
└── compile_commands.json  # (generato) database di compilazione per clangd
```

Le tre cartelle marcate "(generata/o)" non vanno mai committate: sono già nel `.gitignore` alla radice e `make clean` le elimina.

## 4. Il file, sezione per sezione

### 4.1 Toolchain - la scelta dei compilatori

```make
ifeq ($(origin CC),default)
CC 			:= gcc
endif
ifeq ($(origin CXX),default)
CXX 		:= g++
endif
```

`CC` e `CXX` sono un caso speciale: make le **predefinisce** (`CC=cc`, `CXX=g++`), quindi un semplice `CC ?= gcc` non funzionerebbe mai - per make la variabile "ha già un valore". La funzione `$(origin CC)` risponde alla domanda *"da dove viene il valore di CC?"*:

- `default` → è il builtin di make: lo sostituiamo con la nostra scelta;
- `file`, `environment`, `command line` → qualcuno l'ha impostata di proposito (il Makefile del progetto, l'ambiente, `make CC=clang`): la rispettiamo.

Risultato: `gcc`/`g++` come default del repository, ma `make CC=clang` o un override nel Makefile di progetto vincono sempre.

### 4.2 Flag di compilazione

```make
CFLAGS   	?= -std=c99   -g -Wall -Wextra -Werror -Wpedantic
CXXFLAGS 	?= -std=c++23 -g -Wall -Wextra -Werror -Wpedantic
LDFLAGS  	?=
```

Significato di ogni flag:

| Flag                     | Effetto                                                               |
| ------------------------ | --------------------------------------------------------------------- |
| `-std=c99`, `-std=c++23` | standard del linguaggio, in modalità strict ISO                       |
| `-g`                     | simboli di debug nel binario - indispensabili per CodeLLDB            |
| `-Wall`                  | il set base di warning "che ogni codice dovrebbe superare"            |
| `-Wextra`                | warning aggiuntivi (parametri inutilizzati, confronti sospetti...)    |
| `-Wpedantic`             | segnala ogni uso di estensioni non-standard del compilatore           |
| `-Werror`                | ogni warning diventa errore: il codice o compila pulito o non compila |

`LDFLAGS` (vuota di default) è il punto in cui i progetti aggiungono le librerie da linkare: `-lm`, `-lpthread`, ecc. Tutte e tre usano `?=`, quindi un progetto può ridefinirle **prima** dell'`include` (vedi [§6](#6-personalizzare-un-singolo-progetto)).

### 4.3 Layout - nomi delle cartelle

```make
SRC_DIR   	?= src
INC_DIR   	?= include
TEST_DIR  	?= test
EXT_DIR   	?= external
BUILD_DIR 	?= build
BIN_DIR   	?= bin
TARGET    	?= $(BIN_DIR)/main
```

Sono i nomi convenzionali del [§3](#3-il-layout-dei-progetti), tutti sovrascrivibili. `TARGET` è il percorso dell'eseguibile del programma: il nome fisso `bin/main` è il contratto con `.zed/debug.json`.

### 4.4 Librerie esterne - scoperta automatica

```make
EXT_INC_DIRS 	:= $(wildcard $(EXT_DIR)/*/include)
EXT_SRC_DIRS 	:= $(wildcard $(EXT_DIR)/*/src)
```

`$(wildcard ...)` espande un glob e - proprietà comoda - **sparisce in silenzio** se non c'è nessuna corrispondenza. Se il progetto non ha `external/`, entrambe le variabili restano vuote e tutto il resto del meccanismo si disattiva da solo. Con `external/utinc/`, invece, `EXT_INC_DIRS = external/utinc/include` e ogni voce diventa un `-I` nella sezione successiva. Librerie multiple funzionano automaticamente: `external/a/`, `external/b/`, ...

### 4.5 CPPFLAGS - preprocessore, dipendenze, POSIX

```make
CPPFLAGS 	+= -I$(INC_DIR) $(addprefix -I,$(EXT_INC_DIRS)) -MMD -MP -D_POSIX_C_SOURCE=200809L
```

`CPPFLAGS` raccoglie i flag del **preprocessore**, condivisi da C e C++. Usa `+=` (non `?=`): se un progetto definisce dei propri `CPPFLAGS` prima dell'`include`, questi vengono estesi, non sostituiti - gli automatismi (-I, -MMD...) non devono mai andare persi.

- **`-I$(INC_DIR)`** e **`$(addprefix -I,$(EXT_INC_DIRS))`**: i percorsi di ricerca degli header. `addprefix` trasforma la lista di cartelle in una lista di flag (`external/utinc/include` → `-Iexternal/utinc/include`). Un `-I` verso una cartella inesistente è innocuo.
- **`-MMD`**: è il cuore della build incrementale. Mentre compila `src/utils.c`, il compilatore scrive *anche* `build/src/utils.c.d`, un mini-makefile che elenca gli header inclusi da quel sorgente:

  ```make
  # contenuto tipico di build/src/utils.c.d
  build/src/utils.c.o: src/utils.c include/linc.h include/internal/shared.h
  ```

  Questi file vengono importati in fondo a `common.mk` (vedi [§4.16](#416-dipendenze--include-deps)); da quel momento make *sa* che toccare `linc.h` rende scaduto `utils.c.o`. La variante `-MD` traccerebbe anche gli header di sistema (`stdio.h`...): inutile, quelli non cambiano.
- **`-MP`**: aggiunge nel `.d` un target fittizio per ogni header. Serve a coprire il caso della *cancellazione* di un header: senza `-MP`, make fallirebbe con "No rule to make target 'include/vecchio.h'" perché il `.d` vecchio lo cita ancora.
- **`-D_POSIX_C_SOURCE=200809L`**: con `-std=c99` (ISO strict) glibc *nasconde* tutto ciò che non è C99 puro: `pthread_rwlock_*`, `sigaction`, `usleep`, `fileno`... Questo *feature-test macro* riabilita l'API POSIX.1-2008 mantenendo strict il linguaggio. Senza, `boilerplate` non compilerebbe (usa i rwlock). L'alternativa `-std=gnu99` avrebbe rilassato anche il linguaggio: scelta scartata.

### 4.6 Sorgenti - la scoperta dei file

```make
SRCS_C   		:= $(shell find $(SRC_DIR) -name '*.c'   2>/dev/null)
SRCS_CXX 		:= $(shell find $(SRC_DIR) -name '*.cpp' 2>/dev/null)

EXT_SRCS_C 		:= $(if $(EXT_SRC_DIRS),$(shell find $(EXT_SRC_DIRS) -name '*.c'))
EXT_SRCS_CXX 	:= $(if $(EXT_SRC_DIRS),$(shell find $(EXT_SRC_DIRS) -name '*.cpp'))

TEST_SRCS_C 	:= $(shell find $(TEST_DIR) -name '*.c'   2>/dev/null)
TEST_SRCS_CXX 	:= $(shell find $(TEST_DIR) -name '*.cpp' 2>/dev/null)
```

Tre dettagli voluti:

1. **`find` invece di `$(wildcard src/*.c)`**: `wildcard` non è ricorsivo, `find` sì - è ciò che fa funzionare `src/c/mathutil.c` in `test-mix` o un ipotetico `src/net/tcp.c`.
2. **`2>/dev/null`**: se la cartella non esiste (progetto senza `test/`), `find` stamperebbe un errore a video; lo si butta via e la variabile resta semplicemente vuota.
3. **`$(if $(EXT_SRC_DIRS),...)`** per le external: qui non basterebbe silenziare l'errore, perché il glob `external/*/src` non espanso arriverebbe letterale a `find`. Si esegue `find` solo se `wildcard` ha trovato davvero delle cartelle.

Seguono le liste aggregate, pura comodità:

```make
LIB_SRCS 		:= $(SRCS_C) $(SRCS_CXX) $(EXT_SRCS_C) $(EXT_SRCS_CXX)
TEST_SRCS 		:= $(TEST_SRCS_C) $(TEST_SRCS_CXX)
ALL_SRCS_C 		:= $(SRCS_C) $(EXT_SRCS_C) $(TEST_SRCS_C)
ALL_SRCS_CXX 	:= $(SRCS_CXX) $(EXT_SRCS_CXX) $(TEST_SRCS_CXX)
```

`LIB_SRCS` = ciò che costituisce la libreria/il programma (src + external); `TEST_SRCS` = i test; `ALL_SRCS_C/CXX` = tutto, diviso per linguaggio (servono alla scelta del linker e al database di compilazione).

### 4.7 La guardia sugli errori

```make
ifeq ($(strip $(LIB_SRCS)$(TEST_SRCS)),)
$(error No C/C++ sources found in $(SRC_DIR)/ or $(TEST_DIR)/)
endif
```

Se non c'è nessun sorgente da nessuna parte, meglio un messaggio chiaro in fase di lettura che un criptico errore del linker dieci righe dopo. `$(strip)` è necessario: concatenare liste vuote produce una stringa di soli spazi, che per `ifeq` *non* sarebbe uguale a "" (questo dettaglio ritorna nel [§4.10](#410-la-scelta-del-linker)).

### 4.8 Programma o libreria? Il rilevamento del `main`

```make
MAIN_PATTERN 	:= int[[:space:]]+main[[:space:]]*\(
MAIN_SRCS 		:= $(if $(strip $(SRCS_C)$(SRCS_CXX)),$(shell grep -lE '$(MAIN_PATTERN)' $(SRCS_C) $(SRCS_CXX) 2>/dev/null))
```

Un `grep` in fase di lettura cerca, **solo nei sorgenti di `src/`**, chi definisce il `main`. `grep -l` restituisce i *nomi dei file* che matchano, non le righe. Il risultato guida due decisioni:

- se `MAIN_SRCS` è non-vuota → il progetto è un **programma**: il target `all` includerà `bin/main`;
- i file elencati vengono **esclusi dal link dei test** ([§4.9](#49-oggetti-e-binari)): così un programma può avere test senza collisione tra il suo `main` e quello dei test.

Tre note:

- il pattern accetta spaziatura libera (`int   main (`), ma il `main` deve stare **su una riga sola** e iniziare con `int` - è la convenzione documentata nel file (vedi [§8](#8-convenzioni-obbligatorie-e-limiti-noti));
- si cerca solo in `src/`: i test hanno *sempre* un main (in `boilerplate` glielo genera la macro `TEST_RUNNER` di utinc), e le external non devono definirne;
- il pattern vive in una variabile separata perché una `(` letterale dentro `$(if ...)` sbilancerebbe il conteggio delle parentesi del parser di make, che si fermerebbe con *"unterminated call to function 'if'"*. In un'assegnazione semplice, invece, la parentesi non dà fastidio.

### 4.9 Oggetti e binari

```make
LIB_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(LIB_SRCS)))
TEST_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(TEST_SRCS)))
MAIN_OBJS 	:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(MAIN_SRCS)))
DEPS 		:= $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

LIB_OBJS_NOMAIN := $(filter-out $(MAIN_OBJS),$(LIB_OBJS))
```

Ogni sorgente diventa un oggetto che **rispecchia il suo percorso completo** sotto `build/`, con l'estensione originale conservata nel nome:

| Sorgente                     | Oggetto                              |
| ---------------------------- | ------------------------------------ |
| `src/utils.c`                | `build/src/utils.c.o`                |
| `src/net/tcp.c`              | `build/src/net/tcp.c.o`              |
| `external/utinc/src/utinc.c` | `build/external/utinc/src/utinc.c.o` |
| `test/test_core.c`           | `build/test/test_core.c.o`           |

Conservare `.c`/`.cpp` nel nome (`.c.o` invece di `.o`) evita che un ipotetico `foo.c` e `foo.cpp` nello stesso posto collidano sullo stesso `foo.o`.

`DEPS` è la lista dei file `.d` corrispondenti (sostituzione di suffisso: `$(VAR:.o=.d)`). `LIB_OBJS_NOMAIN` usa `$(filter-out lista1,lista2)` per togliere dagli oggetti di libreria quelli che contengono un `main`: è il set che i test linkano.

```make
TEST_BINS_C 	:= $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/%,$(TEST_SRCS_C))
TEST_BINS_CXX 	:= $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%,$(TEST_SRCS_CXX))
TEST_BINS 		:= $(TEST_BINS_C) $(TEST_BINS_CXX)
```

`patsubst` mappa ogni sorgente di test sul suo eseguibile: `test/test_core.c` → `bin/test_core`. Un test in sottocartella diventa `test/unit/x.c` → `bin/unit/x`.

### 4.10 La scelta del linker

```make
LD 			:= $(if $(strip $(ALL_SRCS_CXX)),$(CXX),$(CC))
```

Se esiste *almeno un* sorgente C++ ovunque (src, external o test), si linka con `g++`: è il modo corretto di agganciare la runtime C++ (libstdc++) senza doverla nominare. Un progetto interamente C linka con `gcc`.

Lo `$(strip)` è una lezione imparata sul campo: `ALL_SRCS_CXX` nasce concatenando tre liste; se sono tutte vuote il risultato non è la stringa vuota ma `"  "` (due spazi), e `$(if)` considera vero *qualunque* valore non-vuoto, spazi inclusi. Senza `strip`, anche i progetti C puri linkavano con g++.

### 4.11 Il target di default

```make
COMPDB 		:= compile_commands.json
.DEFAULT_GOAL := all

.PHONY: all
all: $(if $(MAIN_SRCS),$(TARGET)) $(TEST_BINS) $(COMPDB)
```

`.DEFAULT_GOAL` fa sì che un `make` nudo equivalga a `make all`. I prerequisiti di `all` sono calcolati con un `$(if)`:

- **programma** (`MAIN_SRCS` non vuota): `bin/main` + eventuali test + compdb;
- **libreria** (`MAIN_SRCS` vuota): il primo termine sparisce → solo test + compdb.

### 4.12 Le regole di link

L'eseguibile del programma:

```make
$(TARGET): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)
```

`$^` = *tutti* gli oggetti di libreria; `$(dir $@)` estrae `bin/` dal percorso del target e `mkdir -p` la crea se manca. Le librerie (`$(LDFLAGS)`) vanno **dopo** gli oggetti: i linker risolvono i simboli da sinistra a destra.

Gli eseguibili di test usano le **static pattern rules**, una forma con tre parti - `elenco-target: pattern: prerequisiti`:

```make
$(TEST_BINS_C): $(BIN_DIR)/%: $(BUILD_DIR)/$(TEST_DIR)/%.c.o $(LIB_OBJS_NOMAIN)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)
```

Per ogni binario dell'elenco, il `%` cattura la parte variabile (`bin/test_core` → `%` = `test_core`) e la usa per selezionare l'oggetto giusto (`build/test/test_core.c.o`). A completare il link ci sono gli oggetti della libreria *senza* l'eventuale main del programma. La regola gemella per `.cpp` copre i test scritti in C++.

Perché non una pattern rule semplice (`$(BIN_DIR)/%: ...`)? Perché catturerebbe *qualunque* richiesta sotto `bin/`, incluso `bin/main`. La forma statica vincola la regola esattamente ai binari elencati in `TEST_BINS_C`.

### 4.13 Le regole di compilazione

```make
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
```

Due sole regole coprono l'intero albero: il `%` matcha percorsi completi (`src/net/tcp`, `external/utinc/src/utinc`, `test/test_core`), quindi funzionano identiche per src, external e test. La divisione per estensione fa sì che ogni file venga compilato dal compilatore e con lo standard giusti. `mkdir -p $(dir $@)` crea al volo la gerarchia sotto `build/` speculare al sorgente.

### 4.14 `run`, `test`, `clean`, `rebuild`

```make
.PHONY: run
run: all
	./$(TARGET) $(ARGS)
```

`make run ARGS="uno 2 tre"` builda e poi esegue `bin/main` con gli argomenti. Ha senso solo per i programmi; in una libreria fallirebbe (non c'è `bin/main`) - per le librerie si usa `make test`.

```make
.PHONY: test
test: $(TEST_BINS)
	@rc=0; for t in $(TEST_BINS); do ./$$t || rc=1; done; exit $$rc
```

Costruisce tutti gli eseguibili di test e li esegue **tutti**, anche se uno fallisce: `rc` memorizza il fallimento e viene restituito alla fine, così vedi l'esito completo ma `make test` esce comunque con codice ≠ 0 se qualcosa è rosso (utile per script e CI). Il `$$t` è un dollaro *escapato*: `$$` diventa `$` per la shell (vedi anche [§4.15](#415-il-database-di-compilazione-per-clangd)).

```make
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(COMPDB)

.PHONY: rebuild
rebuild: clean all
```

`clean` elimina tutto il generato. `rebuild` concatena i due target: prima `clean`, poi `all` (build completa da zero).

### 4.15 Il database di compilazione per clangd

**Il problema**: clangd non legge i Makefile. Senza aiuto, analizza i file con un comando di fallback privo dei nostri `-I`: risultato, la compilazione riesce ma l'editor segnala `'greeter.hpp' file not found`. (Dettaglio subdolo: un include mancante è un errore *fatale* per clang, quindi il parsing si ferma lì e gli include successivi non risultano nemmeno tentati.)

**La soluzione**: lo standard `compile_commands.json` - un array JSON con una voce per sorgente che dichiara con quale comando va compilato. clangd lo cerca da solo risalendo le cartelle dal file aperto.

```make
.PHONY: $(COMPDB)
$(COMPDB):
	@{ \
	    sep='['; \
	    for f in $(ALL_SRCS_C); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CC) $(CFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    for f in $(ALL_SRCS_CXX); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    printf '\n]\n'; \
	} > $(COMPDB)
```

Come leggerla:

- **due passate**: prima tutti i C (con `$(CC) $(CFLAGS)`), poi tutti i C++ (con `$(CXX) $(CXXFLAGS)`) - così clangd analizza `mathutil.c` come C99 e `greeter.cpp` come C++23, esattamente come la build vera;
- **`$$` vs `$`**: make espande le proprie variabili *prima* di consegnare la recipe alla shell. `$(CC)`, `$(CFLAGS)`... finiscono espansi dentro il JSON; `$$f`, `$$sep`, `$$(pwd)` diventano `$f`, `$sep`, `$(pwd)` - variabili e command substitution della shell;
- **il trucco di `sep`**: la prima iterazione stampa `[` (valore iniziale), tutte le successive stampano `,` - è il modo minimale di generare un array JSON valido da un ciclo;
- **`.PHONY`**: il file viene rigenerato a *ogni* build. Costa qualche `printf` e garantisce che aggiungere o togliere un sorgente aggiorni subito il database;
- l'intero blocco `{ ...; } > $(COMPDB)` raggruppa l'output dei printf e lo redirige nel file in un colpo solo.

Esempio di output reale (da `boilerplate`):

```json
[
  {"directory": "/home/luigimalaguti/code/boilerplate", "file": "src/utils.c", "command": "gcc -std=c99   -g -Wall -Wextra -Werror -Wpedantic -Iinclude -Iexternal/utinc/include -MMD -MP -D_POSIX_C_SOURCE=200809L -c src/utils.c"},
  {"directory": "/home/luigimalaguti/code/boilerplate", "file": "test/test_core.c", "command": "gcc ... -c test/test_core.c"}
]
```

### 4.16 Dipendenze - `-include $(DEPS)`

```make
-include $(DEPS)
```

L'ultima riga chiude il cerchio aperto da `-MMD` ([§4.5](#45-cppflags--preprocessore-dipendenze-posix)): importa tutti i file `.d`, che aggiungono a ogni oggetto i suoi header come prerequisiti. Il trattino iniziale (`-include` invece di `include`) dice a make di **ignorare i file mancanti**: alla prima build, o subito dopo un `clean`, i `.d` non esistono ancora - e va bene così, perché in quel caso si compila comunque tutto.

## 5. I flussi completi: cosa succede quando...

### 5.1 `make` in un programma (es. `test-mix`)

1. *Lettura*: `find` trova `src/main.cpp`, `src/c/mathutil.c`, `src/cpp/greeter.cpp`; `grep` trova `int main(` in `src/main.cpp` → `MAIN_SRCS` non vuota → è un programma. C'è un `.cpp` → `LD = g++`.
2. `all` richiede `bin/main` + `compile_commands.json` (niente test).
3. `bin/main` richiede i tre oggetti; per ciascuno make consulta la pattern rule giusta: `mathutil.c` → `gcc -std=c99 ...`, gli altri → `g++ -std=c++23 ...`.
4. Link: `g++ build/src/c/mathutil.c.o build/src/cpp/greeter.cpp.o build/src/main.cpp.o -o bin/main`.
5. Il database di compilazione viene riscritto.

### 5.2 `make` in una libreria con test (es. `boilerplate`)

1. *Lettura*: `grep` non trova nessun `main` in `src/` → `MAIN_SRCS` vuota → niente `bin/main`. `EXT_SRC_DIRS = external/utinc/src`, `EXT_INC_DIRS = external/utinc/include` → `utinc.c` entra in `LIB_OBJS` e `-Iexternal/utinc/include` nei flag. `TEST_BINS = bin/test_core bin/test_modules`.
2. `all` richiede i due binari di test + compdb.
3. Ogni binario di test linka: il *suo* oggetto (es. `build/test/test_core.c.o`) + tutti gli oggetti di `src/` e `external/`. Tutto C → link con `gcc`, più `-lm -lpthread` che il Makefile del progetto ha appeso a `LDFLAGS`.

### 5.3 Tocchi un header (es. `include/linc.h`)

1. I file `.d` (importati con `-include`) dichiarano quali oggetti dipendono da `linc.h`.
2. Al `make` successivo solo quegli oggetti risultano scaduti e vengono ricompilati; poi si rilinkano i binari che li contengono. Gli altri oggetti non si toccano.

### 5.4 Aggiungi un file `src/nuovo.c`

Nessuna modifica a nessun Makefile: al `make` successivo `find` lo trova, diventa `build/src/nuovo.c.o`, entra nel link e nel database di compilazione. Lo stesso vale per un nuovo test (`test/test_nuovo.c` → `bin/test_nuovo`) o una nuova libreria in `external/`.

### 5.5 `make test`

Costruisce ciò che serve ai test (via prerequisiti, non ricompila l'invariato) e li esegue in sequenza. Se `bin/test_core` fallisce, `bin/test_modules` gira comunque; alla fine `make test` esce con codice 1.

## 6. Personalizzare un singolo progetto

Le personalizzazioni vivono nel Makefile del progetto, **prima** dell'`include`:

```make
# Librerie da linkare (il caso di gran lunga più comune)
LDFLAGS += -lm -lpthread

# Cambiare standard o livello di ottimizzazione
CXXFLAGS = -std=c++20 -O2 -g -Wall -Wextra

# Definire simboli per il preprocessore (esteso, non sostituito)
CPPFLAGS += -DDEBUG_MODE

# Cambiare nome/percorso dell'eseguibile (occhio: rompe il contratto
# con .zed/debug.json, che si aspetta bin/main)
TARGET = bin/tool

include ../common.mk
```

Perché *prima*? Le variabili di `common.mk` usano `?=` ("assegna solo se non già impostata"): se il progetto le imposta prima, il default non scatta. La regola semplice che funziona sempre è: **tutto prima dell'`include`**. `CPPFLAGS` è l'eccezione concettuale (usa `+=`, quindi il tuo valore viene esteso coi flag automatici, mai perso).

Da riga di comando si può sovrascrivere qualunque cosa, una tantum:

```sh
make CC=clang CFLAGS="-std=c11 -O2" run ARGS="uno due"
```

## 7. Integrazione con Zed

### 7.1 I task (`.zed/tasks.json`)

Tutti i task usano lo stesso preambolo shell:

```sh
p="$ZED_DIRNAME"; while [ "$p" != "/" ] && [ ! -f "$p/Makefile" ]; do p=$(dirname "$p"); done
```

`$ZED_DIRNAME` è la cartella del file aperto; il ciclo **risale** finché non trova una cartella con un `Makefile` - la radice del progetto. Per questo i task funzionano da *qualunque* file del progetto: un sorgente in `src/cpp/`, un header, un test, il Makefile stesso.

| Task        | Comando (dopo il preambolo)                                     |
| ----------- | --------------------------------------------------------------- |
| Build       | `make -C "$p" --no-print-directory all`                         |
| Rebuild     | `make -C "$p" --no-print-directory rebuild`                     |
| Run program | `cd "$p" && make --no-print-directory all && ./bin/main <args>` |
| Run tests   | `make -C "$p" --no-print-directory test`                        |
| Clean       | `make -C "$p" --no-print-directory clean`                       |

`--no-print-directory` sopprime le righe `make: Entering directory ...` che `-C` produrrebbe (con `-C` make attiva automaticamente l'opzione `-w`). Gli argomenti del programma si cambiano solo negli `args` di "Run program".

### 7.2 Il debug (`.zed/debug.json`)

Due voci, entrambe generiche:

- **Debug program** → `"program": "$ZED_DIRNAME/../bin/main"`. Funziona per qualunque programma grazie al nome fisso `bin/main`. Richiede che il file aperto sia **un livello sotto** la radice del progetto (`src/` o `include/`): nel campo `program` Zed sostituisce variabili ma non esegue shell, quindi il walk-up lì non è possibile.
- **Debug test** → `"program": "$ZED_DIRNAME/../bin/$ZED_STEM"`. `$ZED_STEM` è il nome del file aperto senza estensione: apri `test/test_core.c`, lancia questa voce, e il debugger parte su `bin/test_core`.

Entrambe dichiarano `"build": "Build"`: prima del debug, Zed esegue il task Build (che col walk-up trova sempre il Makefile giusto).

## 8. Convenzioni obbligatorie e limiti noti

1. **Il `main` va scritto `int main(` su una riga** (spaziatura libera). Il rilevamento è un `grep`, non un parser: un main scritto su due righe, o con un typedef di ritorno, non viene visto → il progetto viene trattato da libreria.
2. **Estensioni supportate: `.c` e `.cpp`**. Niente `.cc`, `.cxx`, `.C` (aggiungerle è possibile ma richiede di estendere le coppie di variabili e regole).
3. **Niente spazi nei nomi di file**: le liste di make sono separate da spazi, un nome con spazi le rompe.
4. **Un binario per programma**: due `main` in `src/` finiscono nello stesso link e il linker fallisce con "multiple definition of 'main'". Programmi diversi = cartelle diverse.
5. **Le external hanno layout fisso** `external/<lib>/{include,src}` e vengono compilate con gli stessi flag del progetto (`-Werror` incluso: codice vendorizzato che non compila pulito va sistemato o compilato a parte).
6. **Nomi dei test unici** anche tra sottocartelle diverse, perché l'eseguibile prende il nome del file (`test/a/x.c` e `test/b/x.c` collidono... in realtà no: diventano `bin/a/x` e `bin/b/x` - ma "Debug test" con `$ZED_STEM` risolve solo il nome piatto, quindi per i test in sottocartelle il debug va configurato a mano).
7. **`make run` ha senso solo nei programmi**; nelle librerie usa `make test`.
8. **Il debug da Zed richiede il file aperto al posto giusto**: un livello sotto la radice per "Debug program", dentro `test/` (piatto) per "Debug test". I task Build/Run/Clean invece funzionano ovunque.
9. **`_POSIX_C_SOURCE=200809L` è sempre definito**: se un giorno servisse ISO puro senza POSIX, va rimosso via override di `CPPFLAGS`... che però usa `+=`: in quel caso l'unica è modificare `common.mk`.

## 9. Risoluzione dei problemi

**`*** No C/C++ sources found in src/ or test/`** La guardia del [§4.7](#47-la-guardia-sugli-errori): il progetto non ha nessun sorgente, oppure hai lanciato make nella cartella sbagliata (es. la radice del repo, dove non c'è un Makefile di progetto).

**`multiple definition of 'main'`** Due `main` nello stesso link: o due sorgenti con main in `src/` (vietato, punto 4 del §8), o un main scritto in modo che il grep non riconosca (punto 1) e quindi non escluso dal link dei test.

**`undefined reference to 'pthread_create'` (o `sin`, `sqrt`...)** Manca la libreria al link: aggiungi `LDFLAGS += -lpthread` (o `-lm`) nel Makefile del progetto, prima dell'`include`.

**`implicit declaration of function 'pthread_rwlock_init'` con `-std=c99`** La funzione è POSIX, non C99: è il problema che `-D_POSIX_C_SOURCE=200809L` risolve ([§4.5](#45-cppflags--preprocessore-dipendenze-posix)). Se compare, qualcuno ha tolto quel define.

**clangd segnala `'xxx.h' file not found` ma la build riesce** Il `compile_commands.json` manca o è vecchio: lancia `make` (lo rigenera). Se non basta, riavvia il language server o riapri il file - clangd cacha il comando precedente.

**Il task Zed fallisce con "make: \*\*\* No targets specified and no makefile found"** Il walk-up è arrivato a `/` senza trovare un `Makefile`: il file aperto non appartiene a nessun progetto (es. un file alla radice del repo - `common.mk` di proposito *non* si chiama `Makefile` proprio perché la radice non è un progetto).

**Ho rinominato/cancellato un header e make dà errori strani** Non dovrebbe succedere (ci pensa `-MP`), ma la cura universale è `make rebuild`: i `.d` vengono rigenerati da zero.

**Warning trattato come errore (`-Werror`) su codice che non posso toccare** Sovrascrivi i flag per quel progetto (`CFLAGS = ... senza -Werror`) prima dell'`include`, oppure sistema il codice: il default del repo è volutamente severo.

## 10. Glossario delle funzioni make usate

| Funzione                                       | Cosa fa                                                          | Uso in `common.mk`                                          |
| ---------------------------------------------- | ---------------------------------------------------------------- | ----------------------------------------------------------- |
| `$(wildcard pat)`                              | espande un glob; vuoto se nessun match                           | scoperta di `external/*/include` e `*/src`                  |
| `$(shell cmd)`                                 | esegue un comando shell in fase di lettura e ne cattura l'output | `find` per i sorgenti, `grep` per il main                   |
| `$(if cond,allora[,altrimenti])`               | ramo su stringa vuota/non-vuota                                  | scelta del linker, `all` condizionale, guardie              |
| `$(strip s)`                                   | rimuove spazi in testa/coda e li comprime                        | evitare che liste vuote-ma-con-spazi risultino "vere"       |
| `$(origin var)`                                | dice da dove viene il valore di una variabile                    | distinguere i default builtin di `CC`/`CXX`                 |
| `$(patsubst pat,repl,lista)`                   | sostituzione con pattern `%` su ogni parola                      | `test/x.c` → `bin/x`                                        |
| `$(addprefix p,lista)`, `$(addsuffix s,lista)` | antepone/appende a ogni parola                                   | `-I` sulle include dir; `.o` sui sorgenti                   |
| `$(filter-out lista1,lista2)`                  | toglie da lista2 le parole in lista1                             | oggetti di libreria senza il main                           |
| `$(dir path)`                                  | la parte directory di un percorso                                | `mkdir -p` della cartella di output                         |
| `$(VAR:.o=.d)`                                 | sostituzione di suffisso (scorciatoia di patsubst)               | dai `.o` ai file di dipendenza `.d`                         |
| `$(error msg)`                                 | interrompe make con un messaggio                                 | guardia sui sorgenti mancanti                               |
| `include`, `-include`                          | importa un altro makefile (`-` = ignora se manca)                | il progetto importa `common.mk`; `common.mk` importa i `.d` |
