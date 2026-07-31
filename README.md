# mdp

> Corso: **Multimedia data processing**
>
> Argomento: *C++ programming*

Questo repository contiene tutta la sezione pratica del corso *Multimedia data processing* riguardo **C++ programming**. Il repository è impostato come un workspace, ogni package del workspace corrisponde a un singolo esercizio di laboratorio o prova d'esame. Di conseguenza, al loro interno saranno presenti tutte le informazioni riguardo il dominio applicativo.

## Struttura

Di seguito viene mostrata la struttura generale del repository.

```
.
├── .gitignore                          # Git-Ignore generale del workspace
├── .clangd                             # Configurazione del server clangd
├── .clang-tidy                         # Configurazione del linter clang-tidy
├── .clang-format                       # Configurazione del formatter clang-format
├── .devcontainer                       # Configurazione del devcontainer per lo sviluppo in container
│   └── ...
├── scripts                             # Folder contenente script di utilità per la gestione del workspace
│   └── ...
├── docs                                # Documentazione con tutte le informazioni non riguardanti i singoli esercizi o esami
│   └── ...
├── .zed                                # Configurazione dell'editor Zed
│   └── ...
├── ...
└── packages                            # Folder contenente i package del workspace
    ├── common.mk                       # Makefile comune a tutti i package, con regole e variabili condivise
    ├── boilerplate                     # Package di boilerplate, da cui partire per ogni nuovo esercizio o esame
    │   ├── README.md                   # Traccia dell'esercizio o esame in formato testuale
    │   ├── Makefile                    # Makefile specifico per la gestione dello sviluppo
    │   ├── ...                         # Altri file generati dall'esecuzione del `Makefile`
    │   ├── .zed                        # Configurazione dell'editor Zed per il package, come tasks e debug
    │   │   └── ...
    │   └── src                         # Folder contenente il codice sorgente del package
    │       └── ...
    └── ...                             # Altri package, ognuno corrispondente a un esercizio o esame, con la stessa struttura del boilerplate
```
