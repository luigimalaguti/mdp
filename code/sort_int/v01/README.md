# Versione 01

> **Sort int**
>
> *Moved standard C program to OOP*

## Changelog

In questa versione proviamo a prendere il codice scritto precedentemente e trasformarlo in una versione più vicina alla OOP. Come prima cosa possiamo creare la struttura più vicina a una classe in C, ovvero una `struct`.

```c
typedef struct vector {
    // ...
} vector_t;

// ...
```

Questa `struct vector` la dovremo trattare proprio come una classica classe in linguaggi OO. Quindi procederemo inserendo quelli che sono gli attributi relativi al nostro vettore, di conseguenza `vector`, `size` e `capacity`. Per ragioni di convenzione, gli attributi della `struct` li chiameremo con il suffisso `_`, questa convenzione ci aiuterà a distinguere gli attributi della `struct` da eventuali metodi che andremo a definire successivamente.

```c
typedef struct vector {
    size_t size_;
    size_t capacity_;
    int32_t *data_;
} vector_t;

// ...
```

A questo punto andremo a definire i metodi della classe. Ovviamente il linguaggio C non è orientato agli oggetti, di fatto non esistono le classi, di conseguenza dovremo definire i metodi come funzioni che prendono come primo parametro un puntatore alla `struct vector`. In questo modo possiamo simulare il concetto di `this` presente nei linguaggi OO. Inoltre, dovremo definire manualmente i costruttori e i distruttori della nostra `struct vector`, in modo da poter allocare e deallocare la memoria necessaria per il nostro vettore.

```c
typedef struct vector {
    size_t size_;
    size_t capacity_;
    int32_t *data_;
} vector_t;

vector_t *vector_constructor(vector_t *this) {
    this->size_ = 0;
    this->capacity_ = 0;
    this->data_ = NULL;
    return this;
}

vector_t *vector_destructor(vector_t *this) {
    free(this->data_);
    return this;
}

// ...
```

Una nota riguardo la signature delle funzioni che abbiamo definito è il tipo di dato che viene restituito. In C è una best practice restituire un puntatore al dato che stiamo manipolando, in questo caso un puntatore alla `struct vector`. In questo modo possiamo concatenare le chiamate ai metodi della nostra `struct vector`, come se fossero dei metodi di una classe in un linguaggio OO.

Una seconda nota importante riguarda alla `free`. La `free` viene chiamata solamente su `data_` e non su `this` stesso. Questo perché la `struct vector` non è detto che sia necessariamente allocata dinamicamente, quindi non possiamo fare l'assunzione che sia sempre corretto chiamare `free` su `this`. In questo modo, se la `struct vector` è allocata dinamicamente, sarà compito del chiamante liberare la memoria della `struct vector` stessa. La `free` su `this` stesso la utilizzeremo successivamente quando definiremo `vector_new` e `vector_delete`.

Quello che possiamo fare adesso è cercare tutto il codice nella precedente versione che può essere considerato come un metodo della nostra `struct vector` e spostarlo all'interno di questa nuova struttura. In questo modo possiamo rendere il codice più modulare e più facile da gestire.

```c
// ...

int vector_push_back(vector_t *this, int32_t number) {
    if (this->size_ >= this->capacity_) {
        size_t temp_capacity = (this->capacity_ == 0) ? 1 : this->capacity_ * 2;
        int32_t *temp_vector = realloc(this->data_, temp_capacity * sizeof(*this->data_));
        if (temp_vector == NULL) {
            return 0;
        }
        this->data_ = temp_vector;
        this->capacity_ = temp_capacity;
    }
    this->data_[this->size_] = number;
    this->size_++;
    return 1;
}

void vector_shrink_to_fit(vector_t *this) {
    if (this->size_ < this->capacity_) {
        int32_t *temp_vector = realloc(this->data_, this->size_ * sizeof(*this->data_));
        if (temp_vector != NULL) {
            this->data_ = temp_vector;
            this->capacity_ = this->size_;
        }
    }
}

size_t vector_size(const vector_t *this) {
    return this->size_;
}

int32_t *vector_data(vector_t *this) {
    return this->data_;
}

int32_t vector_at(const vector_t *this, size_t index) {
    assert(index < this->size_);
    return this->data_[index];
}
```

Abbiamo aggiunto alcuni metodi. Il primo è `vector_push_bash`, il quale fa essenzialmente la stessa cosa del codice della versione precedente, ovvero aggiunge un elemento alla fine del vettore. La differenza risiede solamente nella sezione di codice in cui si controlla l'allocazione dinamica della memoria. Nel metodo, se l'allocazione fallisce, viene restituito un valore di errore, il quale poi verrà controllato dal chiamante.

Il secondo metodo è `vector_shrink_to_fit`, il quale riduce la capacità del vettore alla sua dimensione attuale. Essenzialmente analogo a quanto abbiamo scritto nella versione precedente.

Il terzo metodo è `vector_size`. Questo metodo funge da classico getter delle classi orientate agli oggetti. La sua funzione è quella di incapsulare gli attributi della `struct vector`, secondo i principi della programmazione OO. La cosa da notare è che questa volta il parametro della funzione viene definito come `const vector_t *this`. Questo vuole dire al compilatore e allo sviluppatore che il metodo non modificherà lo stato della `struct vector`, quindi non modificherà gli attributi della `struct vector`. In questo modo possiamo garantire che il metodo non modificherà lo stato della `struct vector`, rendendo il codice più sicuro e più facile da capire. Discorso differente per il secondo getter `vector_data`. In questo caso non definiamo il parametro come `const` perché il metodo restituisce un puntatore al vettore, quindi il chiamante potrà modificare gli elementi del vettore. In questo caso, il metodo non modifica lo stato della `struct vector`, ma permette al chiamante di farlo.

Allo stesso modo abbiamo definito il metodo `vector_at`, il quale restituisce l'elemento alla posizione `index` del vettore. Anche in questo caso, il parametro della funzione viene definito come `const vector_t *this`, garantendo che il metodo non modificherà lo stato della `struct vector`. Inoltre, viene utilizzata la funzione `assert` per garantire che l'indice passato come parametro sia valido, ovvero che sia minore della dimensione del vettore. La nota interessante è che l'`assert` viene utilizzata solamente in fase di debug, quindi non verrà eseguita in fase di release. Ultima nota riguardo questo metodo è che non risulta necessario verificare che l'indice sia positivo. Questo perché l'indice è definito come `size_t`, il quale è un tipo di dato senza segno, quindi non può essere negativo.

A questo punto possiamo effettuare la prima riscrittura del codice. La funzione `main` passerà da questo codice:

```c
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    int32_t *vector = NULL;
    size_t size = 0;
    size_t capacity = 0;

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        if (size >= capacity) {
            size_t temp_capacity = (capacity == 0) ? 1 : capacity * 2;
            int32_t *temp_vector = realloc(vector, temp_capacity * sizeof(*vector));
            if (temp_vector == NULL) {
                printf("Error: Memory allocation failed\n");
                break;
            }
            vector = temp_vector;
            capacity = temp_capacity;
        }
        vector[size] = number;
        size++;
    }
    fclose(input_file);

    if (size < capacity) {
        int32_t *temp_vector = realloc(vector, size * sizeof(*vector));
        if (temp_vector != NULL) {
            vector = temp_vector;
            capacity = size;
        }
    }

    qsort(vector, size, sizeof(*vector), compare_int32);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        free(vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < size; index++) {
        fprintf(output_file, "%d\n", vector[index]);
    }
    fclose(output_file);

    free(vector);
    return EXIT_SUCCESS;
}
```

A questo codice:

```c
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vector vector;
    vector_constructor(&vector);

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vector_push_back(&vector, number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vector_shrink_to_fit(&vector);

    qsort(vector_data(&vector), vector_size(&vector), sizeof(*vector_data(&vector)), compare_int32);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        vector_destructor(&vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vector_size(&vector); index++) {
        fprintf(output_file, "%d\n", vector_at(&vector, index));
    }
    fclose(output_file);

    vector_destructor(&vector);
    return EXIT_SUCCESS;
}
```

Che è a sua volta uguale a questo codice:

```c
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    vector_t *vector = malloc(sizeof(vector_t));
    if (vector == NULL) {
        fclose(input_file);
        printf("Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    vector_constructor(vector);

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vector_push_back(vector, number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vector_shrink_to_fit(vector);

    qsort(vector_data(vector), vector_size(vector), sizeof(*vector_data(vector)), compare_int32);

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        vector_destructor(vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vector_size(vector); index++) {
        fprintf(output_file, "%d\n", vector_at(vector, index));
    }
    fclose(output_file);

    vector_destructor(vector);
    free(vector);
    return EXIT_SUCCESS;
}
```

La differenza tra le due versioni è che nella prima versione la `struct vector` viene allocata sullo stack, mentre nella seconda versione viene allocata dinamicamente. La scelta tra le due versioni dipende dalle esigenze del programma. Questo è il motivo di cui parlavamo inizialmente riguardo la `free` su `this`. In questo caso, se la `struct vector` è allocata dinamicamente, sarà compito del chiamante liberare la memoria della `struct vector` stessa.

Per semplificare l'utilizzo della struttura vettore, quindi, possiamo definire due ulteriori metodi che gestiscono l'intero ciclo di vita della struttura stessa, `vector_new` e `vector_delete`.

```c
// ...

vector_t *vector_new(void) {
    vector_t *this = malloc(sizeof(vector_t));
    if (this != NULL) {
        return vector_constructor(this);
    }
    return NULL;
}

void vector_delete(vector_t *this) {
    free(vector_destructor(this));
}

// ...
```

Questi due nuovi metodi ci permettono quindi di allocare e deallocare la memoria della `struct vector` in modo più semplice e sicuro. In questo modo, il chiamante non dovrà preoccuparsi di allocare e deallocare la memoria della `struct vector`, ma potrà semplicemente utilizzare i metodi `vector_new` e `vector_delete`. Il codice del programma principale diventa quindi più semplice e leggibile.

```c
int main(int argc, char **argv) {
    // ...

    vector_t *vector = vector_new();
    if (vector == NULL) {
        fclose(input_file);
        printf("Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }

    // ...

    vector_delete(vector);
    return EXIT_SUCCESS;
}
```

L'ultima cosa che possiamo fare per questa versione è incapsulare anche la funzione di ordinamento all'interno della nostra `struct vector`. In questo modo, il chiamante non dovrà preoccuparsi di passare i parametri corretti alla funzione `qsort`, ma potrà semplicemente chiamare il metodo `vector_sort`.

```c
// ...

void vector_sort(vector_t *this) {
    qsort(this->data_, this->size_, sizeof(*this->data_), compare_int32);
}

// ...

int main(int argc, char **argv) {
    // ...

    vector_sort(vector);

    // ...
}
```
