# Versione 04

> **Sort int**
>
> *Move constructor and move assignment*

## Changelog

In questa nuova versione del programma `sort_int` andremo innanzitutto a fare una prima pulizia dei costrutti C non più necessari, cercando un pò alla volta di sostituirli con costrutti C++ più moderni. La prima modifica che possiamo apportare riguarda la `malloc` e la `realloc`.

In C++ l'allocazione dinamica della memoria può essere effettuata tramite il costrutto `new` e la deallocazione tramite il costrutto `delete`. Prendendo come esempio l'allocazione di un singolo intero nella heap memory potremmo scrivere `int *number = new int`. Questo è l'equivalente di `malloc(sizeof(int))` in C. Mentre per liberare la memoria dinamica appena creata possiamo semplicemente utilizzare `delete number`. Per allocare dinamicamente più di un intero, in C++ possiamo scrivere `int *array = new int[10]`. Risulta una buona sintassi, ma introduce un problema, **non possiamo** deallocare la memoria come abbiamo visto precedentemente, ovvero con `delete array`. La sintassi corretta per deallocare memoria dinamica allocata con `new[]` è `delete[] array`. Il motivo di queste due differenti sintassi riguarda il fatto di chiamare il distruttore per ogni elemento dell'array, cosa che non avviene con `delete array`. Se chiamassimo `delete array`, il distruttore verrebbe chiamato una sola volta, mentre con `delete[] array` il distruttore viene chiamato per ogni elemento dell'array.

Fatta questa piccola parentesi, possiamo procedere con la rimozione dei costrutti C per sostituirli con quelli più C++-like. Di conseguenza il codice passa da questo:

```cpp
struct vector {
        // ...

        // Copy constructor
        vector(const vector &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = (int32_t *)malloc(other.capacity_ * sizeof(*other.data_));
            for (size_t index = 0; index < other.size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        ~vector() {
            free(data_);
        }

        // Assignment operator
        vector &operator=(const vector &rhs) {
            if (this != &rhs) {
                size_ = rhs.size_;
                capacity_ = rhs.capacity_;
                free(data_);
                data_ = (int32_t *)malloc(rhs.capacity_ * sizeof(*rhs.data_));
                for (size_t index = 0; index < rhs.size_; index++) {
                    data_[index] = rhs.data_[index];
                }
            }
            return *this;
        }

        int push_back(int32_t number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
                int32_t *temp_vector = (int32_t *)realloc(data_, temp_capacity * sizeof(*data_));
                if (temp_vector == NULL) {
                    return 0;
                }
                data_ = temp_vector;
                capacity_ = temp_capacity;
            }
            data_[size_] = number;
            size_++;
            return 1;
        }

        void shrink_to_fit() {
            if (size_ < capacity_) {
                int32_t *temp_vector = (int32_t *)realloc(data_, size_ * sizeof(*data_));
                if (temp_vector != NULL) {
                    data_ = temp_vector;
                    capacity_ = size_;
                }
            }
        }

        // ...
};

// ...
```

A questa versione:

```cpp
struct vector {
        // ...

        // Copy constructor
        vector(const vector &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new int32_t[other.capacity_];
            for (size_t index = 0; index < other.size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        ~vector() {
            delete[] data_;
        }

        // Assignment operator
        vector &operator=(const vector &rhs) {
            if (this != &rhs) {
                size_ = rhs.size_;
                capacity_ = rhs.capacity_;
                delete[] data_;
                data_ = new int32_t[rhs.capacity_];
                for (size_t index = 0; index < rhs.size_; index++) {
                    data_[index] = rhs.data_[index];
                }
            }
            return *this;
        }

        int push_back(int32_t number) {
            if (size_ >= capacity_) {
                size_t temp_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
                int32_t *temp_vector = new int32_t[temp_capacity];
                for (size_t index = 0; index < size_; index++) {
                    temp_vector[index] = data_[index];
                }
                delete[] data_;
                data_ = temp_vector;
                capacity_ = temp_capacity;
            }
            data_[size_] = number;
            size_++;
            return 1;
        }

        void shrink_to_fit() {
            if (size_ < capacity_) {
                int32_t *temp_vector = new int32_t[size_];
                for (size_t index = 0; index < size_; index++) {
                    temp_vector[index] = data_[index];
                }
                delete[] data_;
                data_ = temp_vector;
                capacity_ = size_;
            }
        }

        // ...
};

// ...
```

Una seconda miglioria che possiamo applicare al codice rispetto alla precedente versione è estrarre la logica di lettura del file per creare una funzione helper che ci consente di creare direttamente un `vector` a partire da un file. In questo modo potremo semplificare il `main` ulteriormente per renderlo più leggibile.

```cpp
// ...

vector read_from_file(const char *filename) {
    vector vec;
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int32_t number;
        while (fscanf(file, "%d", &number) == 1) {
            int result = vec.push_back(number);
            if (result == false) {
                break;
            }
        }
        vec.shrink_to_fit();
        fclose(file);
    }
    return vec;
}

// ...

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    vector vec;
    vec = read_from_file(input_filename);
    if (vec.empty() == true) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vec.sort();

    bool result = write_to_file(output_filename, vec);
    if (result == false) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

Ma prestiamo piuttosto attenzione a questa versione del codice, perché in una sola riga di codice `vec = read_from_file(input_filename)` succedono davvero tante cose. Una prima analisi che possiamo fare è che abbiamo definito una variabile `vector vec` senza inizializzarla subito, quindi facciamo solo successivamente l'assegnazione con `vec = read_from_file(input_filename)`. Questa scelta è un'ottima occasione per analizzarne il comportamento del codice con il debugger. Innanzitutto aggiungiamo al Makefile di progetto questa riga `CXXFLAGS += -fno-elide-constructors` alla fine del file, ci permette di disabilitare una ottimizzazione che approfondiremo. Di seguito possiamo vedere il diagramma di chiamate che avvengono durante l'esecuzione del programma sopra, successivamente lo commenteremo.

```
int main(int argc, char **argv) {
│
├── vector vec;
│   │
│   └── vector() {
│       └── this = 0x0000ffffeb1f27c8 [LABEL vec01]
│
├── vec = read_from_file(input_filename);
│   │
│   ├── vector read_from_file(const char *filename) {
│   │   │
│   │   ├── vector vec;
│   │   │   │
│   │   │   └── vector() {
│   │   │       └── this = 0x0000ffffeb1f2760 [LABEL vec02]
│   │   │
│   │   └── return vec;
│   │       │
│   │       ├── vector(const vector &other) {
│   │       │   ├── this  = 0x0000ffffeb1f27e0 [LABEL vec03]
│   │       │   └── other = 0x0000ffffeb1f2760 [LABEL vec02]
│   │       │
│   │       └── ~vector() {
│   │           └── this = 0x0000ffffeb1f2760 [LABEL vec02]
│   │
│   ├── vector &operator=(const vector &rhs) {
│   │   ├── this = 0x0000ffffeb1f27c8 [LABEL vec01]
│   │   └── rhs  = 0x0000ffffeb1f27e0 [LABEL vec03]
│   │
│   └── ~vector() {
│       └── this = 0x0000ffffeb1f27e0 [LABEL vec03]
│
└── return EXIT_SUCCESS;
    │
    └── ~vector() {
        └── this = 0x0000ffffeb1f27c8 [LABEL vec01]
```

L'esecuzione parte ovviamente dal `main`, vengono fatte un paio di cose, e poi si arriva alla riga `vector vec`. Come possiamo immaginare, in questa riga viene chiamato il costruttore di default di `vector` per allocare dinamicamente la memoria dell'oggetto e inizializzarne gli attributi. Dal diagramma possiamo vederne l'indirizzo di memoria in cui questo primo vettore viene salvato, in modo da poterlo distinguere univocamente rispetto ad altri oggetti `vector`. Oltre all'indirizzo di memoria possiamo vedere anche una label, anch'essa univoca, in modo da identificare gli oggetti con più semplicità.

L'esecuzione procede quindi alla riga successiva, ovvero `vec = read_from_file(input_filename);`. Facciamo uno step-in con il debugger per entrare dentro la funzione `read_from_file` e notiamo immediatamente che viene definito un nuovo oggetto `vector vec` all'interno della funzione. Come abbiamo visto precedentemente, verrà chiamato il costruttore di default e otteniamo quindi un secondo oggetto `vec02`. Questo secondo vettore risulta ovviamente diverso da quello creato precedentemente, come mostrato dagli indirizzi di memoria e dalle label.

La funzione `read_from_file` procede con la sua esecuzione, come previsto, fino a quando si arriva alla sua ultima riga, `return vec`. A questo punto il debugging inizia a farsi interessante. Facendo nuovamente step-in sulla riga di codice, succede qualcosa non proprio così intuitivo. Anziché ritornare il controllo del programma al `main`, il programma entra nel costruttore di copia di `vector`. Questo processo avviene perché il secondo vettore `vec02`, quello nello scope locale della funzione `read_from_file`, viene deallocato dalla memoria al termine appunto di `read_from_file`, quindi il programma crea un terzo vettore `vec03` attraverso la copia di `vec02` per poterlo restituire come valore di ritorno della funzione. Possiamo infatti notare come tutti e tre i vettori abbiano indirizzi di memoria differenti, seppur nel codice abbiamo definito esplicitamente soltanto due vettori.

Infatti, dopo aver terminato l'esecuzione di `read_from_file` e aver deallocato il vettore locale `vec02`, il controllo dell'esecuzione torna al `main`. Contrariamente a quanto potremmo pensare, l'esecuzione non procede con la riga di codice successiva, bensì procede con l'operatore di assegnazione. L'assegnazione avviene tra il vettore `vec01`, ovvero `this` colui a cui deve essere assegnato un valore, e il vettore `vec03`, il vettore temporaneo `rhs` creato come valore di ritorno della funzione. Solamente al termine di questa assegnazione, il vettore temporaneo `vec03` termina il suo scope e quindi viene deallocato dalla memoria.

A questo punto l'esecuzione di `read_from_file` è ufficialmente terminata e il primo vettore che abbiamo definito ha finalmente il suo contenuto aggiornato con i valori letti dal file. Possiamo quindi procedere con le righe successive del `main`, come la chiamata a `vec.sort()` e così via. Il programma termina correttamente e il vettore `vec01` viene deallocato dalla memoria al termine del `main`.

Da questo processo di debugging possiamo notare che il codice così come è stato scritto ha creato ben tre vettori differenti, quando il nostro intento era semplicemente quello di leggere un file e assegnare il contenuto al vettore `vec`. Questo comportamento non è per niente ottimale, è stato fatto un sacco di lavoro in appena due righe di codice. Infatti, dal 2011 definire una funzione che restituisca un oggetto è considerata una pessima pratica per i motivi appena analizzati. Viene creato un oggetto locale alla funzione, questo oggetto viene copiato in un oggetto temporaneo, e quest'ultimo oggetto temporaneo viene restituito dalla funzione per poi essere assegnato a un oggetto già esistente. La pratica che si è iniziata a utilizzare da C++11 in poi è quella utilizzare un approccio ormai standard in tutti i compilatori, questo costrutto prende il nome di RVO, **Return Value Optimization**.

Concettualmente ci potremmo chiedere, perché dobbiamo copiare il vettore locale `vec02` in un nuovo vettore `vec03` per poi restituirlo come valore di ritorno della funzione per poi copiare nuovamente il contenuto di `vec03` in `vec01` attraverso l'operatore di assegnazione? Perché non possiamo semplicemente restituire un puntatore al vettore locale `vec02` dato che vive nell'heap memory? La risposta è semplice, perché il vettore locale `vec02` andrà in out-of-scope al termine della funzione, quindi non potremmo più accedere ai suoi dati. La pratica che si è iniziata ad utilizzare è definire e inizializzare direttamente il vettore `vec01` con la funzione di lettura, e quindi sfruttare la RVO.

```cpp
// ...

vector read_from_file(const char *filename) {
    vector vec;
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int32_t number;
        while (fscanf(file, "%d", &number) == 1) {
            int result = vec.push_back(number);
            if (result == false) {
                break;
            }
        }
        vec.shrink_to_fit();
        fclose(file);
    }
    return vec;
}

// ...

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    vector vec = read_from_file(input_filename);
    if (vec.empty() == true) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vec.sort();

    bool result = write_to_file(output_filename, vec);
    if (result == false) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

In questo modo, quello che succede è il seguente flusso.

```
int main(int argc, char **argv) {
│
├── vector vec = read_from_file(input_filename);
│   │
│   └── vector read_from_file(const char *filename) {
│       │
│       ├── vector vec;
│       │   │
│       │   └── vector() {
│       │       └── this = 0x0000fffffc208ee0 [LABEL vec01]
│       │
│       └── return vec;
│           │
│           ├── vector(const vector &other) {
│           │   ├── this  = 0x0000fffffc208f50 [LABEL vec02]
│           │   └── other = 0x0000fffffc208ee0 [LABEL vec01]
│           │
│           └── ~vector() {
│               └── this = 0x0000fffffc208ee0 [LABEL vec01]
│
└── return EXIT_SUCCESS;
    │
    └── ~vector() {
        └── this = 0x0000fffffc208f50 [LABEL vec02]
```

In questo caso, il `main` non crea più nessun vettore, ma semplicemente riceve il vettore `vec02` creato dalla funzione `read_from_file` che è un vettore temporaneo anch'esso, creato sempre dal vettore locale della funzione, ma viene evitata una copia attraverso il metodo di assegnazione. In questo caso, il vettore temporaneo `vec02` viene creato direttamente con il costruttore di copia, e quindi non viene più chiamato l'operatore di assegnamento. Però il problema di fondo rimane, ovvero il fatto che viene comunque creato un vettore temporaneo `vec02` che viene deallocato al termine del `main`. Per avere un meccanismo ulteriore di "rubare" le risorse del vettore temporaneo, possiamo implementare due ulteriori metodi, ovvero il **move constructor** e il **move operator**.

```cpp
struct vector {
        // ...

        // Move constructor
        vector(vector &&other) {
            printf("vector(vector &&other)\n");
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = other.data_;
            other.data_ = nullptr;
        }

        // ...

        // Move operator
        vector &operator=(vector &&rhs) {
            size_ = rhs.size_;
            capacity_ = rhs.capacity_;
            delete[] data_;
            data_ = rhs.data_;
            rhs.data_ = nullptr;
            return *this;
        }
}
```

La sintassi di questi due metodi è molto simile a quella del costruttore di copia e dell'operatore di assegnamento, ma la differenza fondamentale è il parametro passato, definito da `vector &&other` e `vector &&rhs`. Questo parametro prende il nome di **r-value reference**, e ci permette di distinguere tra un oggetto che è un l-value, ovvero un oggetto che ha un nome e quindi può essere modificato, e un oggetto che è un r-value, ovvero un oggetto temporaneo che non ha un nome e quindi non può essere modificato. In questo caso, il vettore temporaneo `vec02` creato dalla funzione `read_from_file` è un r-value, quindi viene chiamato il costruttore di move e non quello di copia. In questo modo, possiamo "rubare" le risorse del vettore temporaneo senza doverle copiare, migliorando le performance del programma. Se poi modificassimo nuovamente il `main` per avere definizione e assegnazione distinti di `vec`, potremo vedere come viene chiamato l'operatore di move invece di quello di assegnamento per lo stesso principio.
