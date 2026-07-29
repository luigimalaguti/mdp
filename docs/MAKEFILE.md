# Makefile

Questo documento vuole spiegare il funzionamento di `common.mk`, il Makefile condiviso alla radice del repository. Ogni progetto interno a `packages` definisce quindi un proprio Makefile che importa `common.mk` e ottiene così le regole comuni a tutti i progetti. In questo modo, ogni progetto può definire le proprie regole specifiche, ma allo stesso tempo beneficiare delle regole generali definite in `common.mk`.

## common.mk

Il Makefile `common.mk` parte definendo alcune variabili di default per il compilatore C e C++. Se l'utente non ha specificato un compilatore, vengono utilizzati `gcc` per C e `g++` per C++. Questo permette di avere un comportamento coerente tra i vari progetti senza dover specificare manualmente il compilatore ogni volta.

```make
ifeq ($(origin CC),default)
CC				:= gcc
endif
ifeq ($(origin CXX),default)
CXX				:= g++
endif
```

`CC` e `CXX` sono variabili speciali. Make le predefinisce, `CC = cc` e `CXX = g++`, quindi un semplice `CC ?= gcc` non funzionerebbe, per make la variabile *ha già un valore*. La funzione `$(origin CC)` risponde alla domanda **da dove viene il valore di CC?**:

- `default`, è il builtin di make, lo sostituiamo con la nostra scelta
- `file`, `environment`, `command line`, qualcuno l'ha impostata di proposito, come ad esempio `make CC=clang` o nel Makefile di progetto, quindi la rispettiamo

Il risultato è `gcc` e `g++` come default del repository, ma gli override vincono sempre.

### Compiler flags

```make
CFLAGS			?= -std=c99   -g -Wall -Wextra -Werror -Wpedantic
CXXFLAGS		?= -std=c++23 -g -Wall -Wextra -Werror -Wpedantic
```

| Flag                     | Significato                                                        |
| ------------------------ | ------------------------------------------------------------------ |
| `-std=c99`, `-std=c++23` | Standard del linguaggio, modalità strict ISO                       |
| `-g`                     | Simboli di debug nel binario, servono al debugger                  |
| `-Wall`                  | Il set base di warning che ogni codice dovrebbe superare           |
| `-Wextra`                | Warning aggiuntivi, parametri inutilizzati, confronti sospetti...  |
| `-Wpedantic`             | Segnala ogni estensione non-standard del compilatore               |
| `-Werror`                | Ogni warning è un errore, o compila pulito o non compila           |

Il `?=` permette a ogni progetto di ridefinirli **prima** dell'`include` del `common.mk`.

### Linker flags

```make
LDFLAGS			?=
```

Vuota di default. È il punto in cui un esercizio aggiunge le librerie da linkare, ad esempio `LDFLAGS += -lm` per la matematica, `-lpthread` per i thread, e così via. Nel link le librerie compaiono **dopo** gli oggetti, perché i linker risolvono i simboli da sinistra a destra.

### Sources

```make
SRC_DIR			?= src
INC_DIR			?= include
BUILD_DIR		?= build
BIN_DIR			?= bin
EXEC			?= main
TARGET			?= $(BIN_DIR)/$(EXEC)

SRCS_C			:= $(shell find $(SRC_DIR) -name '*.c'   2>/dev/null)
SRCS_CXX		:= $(shell find $(SRC_DIR) -name '*.cpp' 2>/dev/null)
SRCS			:= $(SRCS_C) $(SRCS_CXX)
```

Prima troviamo i nomi convenzionali delle cartelle, tutti eventualmente sovra-scrivibili, con l'eseguibile spezzato in due variabili, `EXEC`, il nome dell'eseguibile, e `TARGET`, il percorso completo. `EXEC` è *pensato* per essere sovrascritto, ad esempio `EXEC := sort_int` prima di `common.mk`.

Poi l'ottenimento' dei sorgenti. Due dettagli:

- **`find` invece di `$(wildcard src/*.c)`**: `wildcard` non è ricorsivo, mentre `find` sì. Un domani `src/util/parser.c` viene preso senza toccare nulla
- **`2>/dev/null`**: Se `src/` non esiste, `find` stamperebbe un errore a ogni make, lo si scarta e la lista resta semplicemente vuota

### Objects

```make
OBJS			:= $(addprefix $(BUILD_DIR)/,$(addsuffix .o,$(SRCS)))
DEPS			:= $(OBJS:.o=.d)

LD				:= $(if $(strip $(SRCS_CXX)),$(CXX),$(CC))
```

`OBJS`, ogni sorgente diventa un oggetto che rispecchia il suo percorso, con l'estensione originale conservata nel nome:

| Sorgente              | Oggetto                       |
| --------------------- | ----------------------------- |
| `src/main.c`          | `build/src/main.c.o`          |
| `src/util/parser.cpp` | `build/src/util/parser.cpp.o` |

Tenere `.c` e `.cpp` nel nome evita che un ipotetico `foo.c` e `foo.cpp` collidano sullo stesso `foo.o`, e permette alle due pattern rules di distinguersi.

`DEPS`, la lista dei file `.d` accanto agli oggetti. Sono le dipendenze dagli header generate da `-MMD`. Il cerchio si chiude con l'`-include` in fondo al `common.mk`.

`LD`, il compilatore usato per il **link**. Se c'è almeno un sorgente C++ si usa `g++`, che aggancia automaticamente il runtime C++. Un programma solo C linka con `gcc`.

### Pre-processor flags

```make
CPPFLAGS		+= -MMD -MP -I$(INC_DIR)
```

Flag del **preprocessore**, condivisi da C e C++, la sigla `CPP` sta per *C PreProcessor*. Sono importanti per la gestione degli header:

- **`-I$(INC_DIR)`**: Dove cercare gli header con `#include "..."`. Un `-I` verso una cartella inesistente è innocuo
- **`-MMD`**: Mentre compila, il compilatore scrive *anche* un file `.d` accanto all'oggetto. Un mini-makefile che elenca gli header inclusi da quel sorgente:

    ```make
    # build/src/main.c.d (esempio)
    build/src/main.c.o: src/main.c include/miolib.h
    ```

    È ciò che rende `make build` consapevole degli header. Tocchi `mylib.h` allora `main.c.o` risulta scaduto, quindi si ricompila. La variante `-MD` traccerebbe anche gli header di sistema, anche se inutile perché non cambiano
- **`-MP`**: Aggiunge nel `.d` un target fittizio per ogni header, così *cancellare* un header non blocca make con "No rule to make target"

Viene utilizzato `+=`. Se un esercizio definisce propri `CPPFLAGS`, ad esempio `-DDEBUG` o `-D_POSIX_C_SOURCE=...`, i flag automatici si sommano ai suoi invece di perdersi.

### Clangd variables

```make
COMP_DB			:= compile_commands.json
CACHE_DIR		:= .cache
```

`COMP_DB` è il *database di compilazione*, il file standard da cui clangd impara con quale comando va compilato ogni sorgente. `CACHE_DIR` è la cache dell'indice di clangd, che compare nella cartella dei progetti, viene eliminata dal `clean`.

### Build rules

Il link dell'eseguibile:

```make
$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(LDFLAGS)
```

`$^` indica tutti gli oggetti, `$(dir $@)` estrae `bin/` dal percorso del target e `mkdir -p` la crea se manca. Mentre `$@` è il target stesso, quindi `bin/main` o `bin/sort_int`.

La compilazione, con due pattern rules gemelle:

```make
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
```

Il `%` è un *wildcard*, fa da placeholder per il percorso del sorgente. `$<` è il primo prerequisito, quindi il sorgente stesso. La regola dice, "per ogni file `.c` o `.cpp`, crea l'oggetto corrispondente in `build/` usando il compilatore giusto e i flag giusti".

La divisione per estensione fa usare a ogni file il compilatore e lo standard giusto. Qui il `mkdir -p $(dir $@)` è essenziale, gli oggetti vivono in `build/src/...` e la gerarchia va creata al volo.

### Phony rules

```make
.PHONY: all
all: rebuild

.PHONY: build
build: $(TARGET) $(COMP_DB)

.PHONY: rebuild
rebuild: clean build

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(COMP_DB) $(CACHE_DIR)
```

La catena delle scelte:

- `build`: Il build vero e proprio, eseguibile più database per clangd. È **incrementale**, riutilizza gli oggetti validi
- `rebuild`: `clean` più `build`, riparte da zero
- `all`: `rebuild`, quindi **il `make` nudo ricostruisce sempre tutto da zero**. È una scelta deliberata, progetti piccoli, build di pochi decimi di secondo, zero rischi di stato sporco. Se un giorno i progetti crescono, basta ripuntare `all` a `build`
- `clean`: Elimina tutto il generato, cache di clangd compresa

### Clang rules

**Il problema**, clangd non legge i Makefile. Senza aiuto clangd analizza i file con un comando di fallback privo dei nostri `-I` e dello standard giusto, quindi Zed segnala `'file.h' file not found` anche se la build riesce.

**La soluzione**, generare `compile_commands.json`, un array JSON con una voce per sorgente. Clangd lo trova da solo risalendo le cartelle dal file aperto.

```make
.PHONY: $(COMP_DB)
$(COMP_DB):
	@{ \
	    sep='['; \
	    for f in $(SRCS_C); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CC) $(CFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    for f in $(SRCS_CXX); do \
	        printf '%s\n  {"directory": "%s", "file": "%s", "command": "$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c %s"}' "$$sep" "$$(pwd)" "$$f" "$$f"; \
	        sep=','; \
	    done; \
	    printf '\n]\n'; \
	} > $(COMP_DB)
```

Come leggere la recipe:

- **Due cicli**: Prima i sorgenti C, comando con `$(CC) $(CFLAGS)`, poi i C++, con `$(CXX) $(CXXFLAGS)`. Clangd analizzerà ogni file esattamente come lo compila make, standard incluso
- **`$$` contro `$`**: Make espande le *proprie* variabili prima di passare la recipe alla shell. `$(CC)`, `$(CFLAGS)` arrivano già espansi dentro il JSON. `$$f`, `$$sep`, `$$(pwd)` diventano `$f`, `$sep`, `$(pwd)`, ovvero variabili e command substitution *della shell*
- **il trucco di `sep`**: Alla prima iterazione stampa `[`, a tutte le successive `,`, il modo più semplice di produrre un array JSON valido da un ciclo
- **`.PHONY`**: Il file si rigenera a ogni `build`. Costa qualche `printf` e garantisce che aggiungere o togliere sorgenti aggiorni subito il database
- Il blocco `{ ...; } > $(COMP_DB)` raccoglie tutti i printf e li redirige nel file in un colpo solo

### Includes

```make
-include $(DEPS)
```

L'ultima riga chiude il cerchio aperto da `-MMD`. Importa tutti i file `.d`, che aggiungono a ogni oggetto i suoi header come prerequisiti. Il trattino iniziale dice a make di **ignorare i file mancanti**, alla prima build, o dopo un `clean`, i `.d` non esistono ancora, e va bene perché in quel caso si compila comunque tutto.

## Integrazione con Zed

Il `common.mk` e i Makefile di progetto non tornano utili solamente per l'utilizzo da riga di comando, ma anche per l'integrazione con Zed. Zed utilizza clangd come language server per fornire funzionalità avanzate come completamento automatico, navigazione del codice e refactoring.

Infatti possiamo sfruttare i `tasks.json` e `debug.json` per definire task e configurazioni di debug personalizzate, che si integrano perfettamente con i Makefile. In questo modo, possiamo eseguire build, test e debug direttamente dall'editor senza dover passare alla riga di comando.

### Tasks globali

```json
[
  {
    "label": "Build",
    "command": "make build",
    "cwd": "$ZED_DIRNAME/.."
  },
  {
    "label": "Rebuild",
    "command": "make rebuild",
    "cwd": "$ZED_DIRNAME/.."
  },
  {
    "label": "Clean",
    "command": "make clean",
    "cwd": "$ZED_DIRNAME/.."
  }
]
```

La variabile `$ZED_DIRNAME` è una variabile speciale di Zed che rappresenta la directory del file aperto. In questo caso, puntiamo alla cartella del progetto padre con `..`, in quanto la convenzione di questo repository è avere il Makefile esterno alla cartella `src` che contiene appunto il `main`.

Notare come la semantica dei tasks rispecchi quella dei target del Makefile, con `Build` che esegue `make build`, `Rebuild` che esegue `make rebuild` e `Clean` che esegue `make clean`. In questo modo, possiamo facilmente eseguire le operazioni comuni a tutti i progetti direttamente dall'editor.

### Tasks per progetto

Ogni progetto può, e solitamente ha, i propri tasks di esecuzione. L'esecuzione dei progetti ha tasks propri in quanto ogni progetto potrebbe aver necessità differenti.

```json
[
  {
    "label": "Run sort_int",
    "command": "make rebuild && bin/sort_int",
    "args": [
      "inputs/input01.txt",
      "outputs/output01.txt",
    ],
    "cwd": "$ZED_WORKTREE_ROOT/packages/sort_int",
  },
]
```

In questo caso l'eseguibile non ha il nome standard `main`, ma nel Makefile è stato sovrascritto con `sort_int`. Inoltre, i singoli progetti potrebbero richiedere degli argomenti specifici, rendendo complicato avere un task generico di esecuzione.

Zed accoda `args` al `command`, quindi il comando finale è
`make rebuild && bin/sort_int inputs/input01.txt outputs/output01.txt`, eseguito dalla radice del progetto.

### Debug per progetto

Per la stessa ragione espressa precedentemente riguardo i tasks per progetto, anche il debug risulta essere singolo per ogni progetto.

```json
[
  {
    "label": "Debug sort_int",
    "adapter": "CodeLLDB",
    "request": "launch",
    "build": "Rebuild",
    "program": "$ZED_WORKTREE_ROOT/packages/sort_int/bin/sort_int",
    "args": [
      "inputs/input01.txt",
      "outputs/output01.txt",
    ],
    "cwd": "$ZED_WORKTREE_ROOT/packages/sort_int",
  }
]
```

Tre pezzi che si incastrano:

- `"build": "Build"` esegue il task Build prima di partire, quello definito nei tasks generali
- `"program"` punta a `bin/<EXEC>` del progetto, nel campo `program` Zed sostituisce le variabili ma **non esegue shell**, quindi serve il file aperto un livello sotto la radice dell'esercizio
- Il `-g` nei flag di compilazione è ciò che dà a CodeLLDB i simboli per breakpoint e variabili, il quale viene impostato nel `common.mk`
