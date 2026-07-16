# Versione 02

> **Sort int**
>
> *First OOP in C++*

## Cambiamenti

Il programma precedente era arrivato a un livello di OOP che non poteva essere migliorato. Eppure il codice risultava difficile, lungo da leggere e ripetitivo. Quello che possiamo fare adesso per migliorarlo è passare a un linguaggio che supporti nativamente l'OOP, come C++. In questo modo possiamo eliminare alcune ripetizioni e rendere il codice più leggibile.

La prima prova che possiamo fare è prendere il precedente codice, rinominarlo in `.cpp` e provare a compilarlo. Spoiler, la compilazione fallirà. Questo perché essendo un linguaggio differente il C++ dal C, viene utilizzato un compilatore differente, `g++`, di conseguenza il linguaggio presenterà costrutti propri, come ad esempio `this`. Infatti `this` in C++ è una reserved keyword che rappresenta un puntatore all'oggetto stesso, mentre in C non esiste. Per questo motivo il compilatore segnalerà un errore di sintassi.

Altre differenze rispetto al C è la `struct`. Essa infatti in C++ non ha più la necessità di essere preceduta da `typedef`, il linguaggio fa automaticamente il typedef della `struct` e quindi non è più necessario scrivere `typedef struct vector vector`. In C++ la `struct` è più simile a una `class`, infatti possiamo definire dei metodi all'interno della `struct`. Questi metodi non sono memorizzati in memoria dentro la `struct` stessa, ma sono solamente un meccanismo di sintassi per poter chiamare le funzioni con una sintassi più simile a quella OOP. All'interno della `struct` fisicamente sono memorizzati solamente gli attributi proprio come nella `struct` in C. Nelle `struct` in C++ il costruttore e distruttore sono metodi speciali che vengono definiti senza un tipo di ritorno e con lo stesso nome della classe. Il costruttore viene chiamato automaticamente quando si crea un oggetto della classe, mentre il distruttore viene chiamato automaticamente quando l'oggetto esce dallo scope o viene eliminato. Il distruttore si differenzia dal costruttore da una tilde `~` prima del nome della classe.

Inoltre, ogni metodo riceve implicitamente un puntatore all'oggetto stesso, chiamato `this`, che permette di accedere ai membri dell'oggetto all'interno dei metodi. In realtà non risulta neanche necessario utilizzare `this` per accedere ai membri dell'oggetto, in quanto il compilatore parte a cercare la variabile all'interno della funzione, se non la trova cerca all'interno della `struct` e se non la trova cerca all'interno del file.

```cpp
struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = NULL;
        }

        ~vector() {
            free(data_);
        }

        // ...
};
```

I metodi `vector_new` e `vector_delete`, rispettivamente per creare e distruggere un oggetto di tipo `vector`, vengono automaticamente gestiti dal compilatore stesso, quindi non abbiamo bisogno di definirli. A questo punto possiamo portare tutte le funzioni che fungevano da metodi nella precedente versione, in veri e propri metodi della `struct` in questa versione.

```cpp
struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = NULL;
        }

        ~vector() {
            free(data_);
        }

        int push_back(int32_t number) {
            // ...
        }

        void shrink_to_fit() {
            // ...
        }

        size_t size() const {
            // ...
        }

        int32_t *data() {
            // ...
        }

        int32_t at(size_t index) const {
            // ...
        }

        void sort() {
            // ...
        }
};
```

Una differenza tra il C e il C++ è che nel primo era possibile fare l'assegnazione di un puntatore a `NULL` a un qualsiasi altro puntatore, come nel caso `int32_t *temp_vector = realloc(this->data_, temp_capacity * sizeof(*this->data_))`. In C++, invece, il compilatore non permette più di fare questa assegnazione, a meno che non si espliciti il cast che si vuole ottenere. Quindi il precedente esempio deve essere riscritto come `int32_t *temp_vector = (int32_t *)realloc(this->data_, temp_capacity * sizeof(*this->data_))` per essere accettato dal compilatore.

Una seconda differenza tra C e C++ riguarda la definizione di `const` dopo la dichiarazione di un metodo. In C++ è possibile definire un metodo come `const`, il che significa che il metodo non può modificare lo stato dell'oggetto su cui viene chiamato. Questo è utile per garantire che i metodi non modifichino accidentalmente lo stato dell'oggetto, e permette al compilatore di ottimizzare il codice. Questo è l'analogo di quello che abbiamo fatto nella precedente versione con `size_t vector_size(const vector_t *this)`.

Applicando quindi questi cambiamenti siamo riusciti già ad avere una `struct` più pulita, semplice da leggere e senza le ridondanze della precedente versione:

```cpp
struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = NULL;
        }

        ~vector() {
            free(data_);
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

        size_t size() const {
            return size_;
        }

        int32_t *data() {
            return data_;
        }

        int32_t at(size_t index) const {
            assert(index < size_);
            return data_[index];
        }

        void sort() {
            qsort(data_, size_, sizeof(*data_), compare_int32);
        }
};
```

A questo punto dobbiamo modificare il `main` per adattarlo alla nuova sintassi OOP in C++. In particolare, dobbiamo cambiare la creazione dell'oggetto `vector_t *v = vector_new()` e la distruzione dell'oggetto `vector_delete(v)` con la nuova sintassi OOP in C++.

```cpp
int main(int argc, char **argv) {
    // ...

    vector *vec = new vector;

    // ...

    delete vec;
    return EXIT_SUCCESS;
}
```

Il costruttore svolge automaticamente il compito di allocare e inizializzare l'oggetto, mentre il distruttore svolge automaticamente il compito di liberare la memoria allocata per l'oggetto. In questo modo possiamo eliminare le funzioni `vector_new` e `vector_delete`, rendendo il codice più pulito e leggibile.

```cpp
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

    vector *vec = new vector;

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vec->push_back(number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vec->shrink_to_fit();

    vec->sort();

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        delete vec;
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vec->size(); index++) {
        fprintf(output_file, "%d\n", vec->at(index));
    }
    fclose(output_file);

    delete vec;
    return EXIT_SUCCESS;
}
```

In questo caso abbiamo definito una variabile `vec` di tipo `vector *`, che è un puntatore a un oggetto di tipo `vector`. Di conseguenza l'oggetto `vec` vive nell'heap e non nello stack, quindi dobbiamo utilizzare `new` per allocare la memoria per l'oggetto e `delete` per liberarla. Se invece volessimo creare un oggetto `vector` direttamente nello stack, potremmo scrivere `vector vec`, ma in questo caso non avremmo bisogno di utilizzare `new` e `delete`, in quanto l'oggetto verrebbe automaticamente distrutto quando esce dallo scope.

```cpp
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

    vector vec;

    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        int result = vec.push_back(number);
        if (!result) {
            printf("Error: Memory allocation failed\n");
            break;
        }
    }
    fclose(input_file);

    vec.shrink_to_fit();

    vec.sort();

    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    for (size_t index = 0; index < vec.size(); index++) {
        fprintf(output_file, "%d\n", vec.at(index));
    }
    fclose(output_file);

    return EXIT_SUCCESS;
}
```
