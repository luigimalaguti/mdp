# Versione 03

> **Sort int**
>
> *Copy constructor and assignment operator*

## Changelog

Continuando il perfezionamento del nostro codice, come prima cosa possiamo modificare gli include delle standard library C da `#include <stdlib.h>` a `#include <cstdlib>`, è una convenzione che viene utilizzata in C++. Inoltre possiamo estrapolare dal `main` il codice relativo alla scrittura su file per creare una funzione, non della `struct vector`, ma una funzione helper per rendere più pulito il codice stesso:

```cpp
bool write_to_file(const char *filename, const vector &vec) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    for (size_t index = 0; index < vec.size(); index++) {
        fprintf(file, "%d\n", vec.at(index));
    }
    fclose(file);
    return true;
}

int main(int argc, char **argv) {
    // ...

    bool result = write_to_file(output_filename, vec);
    if (result == false) {
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

Soffermiamoci su quanto abbiamo cambiato. Come prima cosa notiamo la signature di `write_to_file`. Questa signature presenta `const char *filename`, fin qua tutto normale, solamente un puntatore a carattere marcato come `const` per indicare che non vogliamo modificare il contenuto della stringa passata come argomento. La seconda parte della signature è `const vector &vec`. Questa parte necessita di approfondimento. La sintassi `vector &vec` indica che stiamo passando un riferimento al `vector`, ovvero non stiamo passando una copia del `vector`, ma stiamo passando un riferimento al `vector` stesso. Questa sintassi è del tutto simile a quella dei puntatori, ma con la differenza che non dobbiamo fare l'operazione di dereferenziazione per accedere al contenuto del `vector`. Infatti, se avessimo passato un puntatore al `vector`, avremmo dovuto scrivere `vec->size()` per accedere alla funzione `size()`, mentre con il riferimento possiamo scrivere direttamente `vec.size()`. Concettualmente, però, il riferimento è un puntatore, quindi se modifichiamo il `vector` all'interno della funzione `write_to_file`, stiamo modificando il `vector` originale. Per evitare questo, abbiamo aggiunto la parola chiave `const`, che indica che non vogliamo modificare il contenuto del `vector` passato come argomento.

Ora supponiamo per un momento di avere la signature della funzione `write_to_file` definita come segue `bool write_to_file(const char *filename, vector vec)`. La funzione definita in questo modo presenta una problematica sottile da comprendere. In questo modo, quando chiamiamo `write_to_file(filename, vec)` stiamo passando una copia del `vector` originale alla funzione `write_to_file`. Quindi appena si entra nella funzione viene chiamato il costruttore di `vector` per costruire la copia di `vec`, la funzione svolge le sue operazioni, ma quando la funzione termina viene chiamato il distruttore di `vector` per la variabile interna appena creata. Tutto normale, ma con una piccola nota. La copia di `vec` originale consiste nel copiare l'esatta struttura dati del `vector` in una nuova variabile. Quindi avremo una copia di `size_`, di `capacity_` e di `data_`, ma l'attributo `data_` è un puntatore a una cella di memoria. Questo vuole dire che la nuova variabile copia solamente questo puntatore, non duplica la zona di memoria puntato da esso. Di conseguenza, quando viene chiamato il distruttore della nuova variabile `vec` interna a `write_to_file`, viene liberata la memoria della zona di memoria relativa a `data_`, la quale rimane comunque la stessa zona di memoria puntata da `data_` del `vector` originale. Questo comporta che quando il `vector` originale viene distrutto, il suo distruttore cerca di liberare la memoria di `data_`, ma questa memoria è già stata liberata dal distruttore della copia interna a `write_to_file`. Questo comporta un errore di doppia liberazione della memoria, che può portare a comportamenti indefiniti del programma. Questa tipologia di copia è chiamata *shallow copy*, ovvero copia superficiale, perché copia solamente i valori degli attributi della `struct`, ma non duplica la memoria puntata da essi.

Per evitare questo problema, abbiamo aggiunto un copy constructor. Infatti, il linguaggio C++ possiede il meccanismo di function overloading, il quale ci consente di creare un secondo costruttore con una signature differente. Questo copy constructor permette di creare una nuova variabile `vector` a partire da un'altra variabile `vector` già esistente. In questo modo, quando chiamiamo `write_to_file(filename, vec)`, viene chiamato il costruttore di copia per creare una nuova variabile `vec` interna a `write_to_file`, ma questa volta il costruttore di copia duplica anche la memoria puntata da `data_`, evitando così il problema della doppia liberazione della memoria. Questo era esattamente quello che veniva fatto nel precedente esempio, con la differenza che il copy constructor era definito di default dal compilatore.

```cpp
struct vector {
        size_t size_;
        size_t capacity_;
        int32_t *data_;

        // Default constructor
        vector() {
            size_ = 0;
            capacity_ = 0;
            data_ = NULL;
        }

        // Copy constructor
        vector(const vector &other) {
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = (int32_t *)malloc(other.capacity_ * sizeof(*other.data_));
            for (size_t index = 0; index < other.size_; index++) {
                data_[index] = other.data_[index];
            }
        }

        // ...
}
```

Adesso con questo costruttore di copia potremmo scrivere qualcosa del tipo `vector vec2 = vec` e il secondo vettore viene copiato per intero, niente shallow copy ma *deep copy*. Un'altra sintassi per ottenere lo stesso risultato è quella di scrivere `vector vec2(vec)`.

Ora facciamo una piccola parentesi per introdurre un nuovo concetto. Supponiamo di avere un `main` così definito:

```cpp
int main(int argc, char **argv) {
    int x = 5;
    x = 5;

    return 0;
}
```

La prima riga di codice `int x = 5;` è un'operazione di inizializzazione, mentre la seconda riga di codice `x = 5;` è un'operazione di assegnamento. La differenza tra le due operazioni è sottile, ma importante. L'operazione di inizializzazione viene eseguita quando una variabile viene dichiarata e inizializzata con un valore, mentre l'operazione di assegnamento viene eseguita quando una variabile già esistente viene assegnata a un nuovo valore. In C++, ma anche in C, l'operatore `=` può essere utilizzato sia per l'inizializzazione che per l'assegnamento, ma il contesto in cui viene utilizzato determina quale operazione viene eseguita. Un altro esempio che lo rende ancora più chiaro può essere:

```cpp
int main(int argc, char **argv) {
    int x[3] = {1, 2, 3};
    x = {4, 5, 6}; // Error

    return 0;
}
```

In questo secondo esempio è ancora più chiara la distinzione dell'operatore `=` in base al contesto in cui viene utilizzato.

Tornando al nostro programma abbiamo detto che il codice di seguito ci permette di utilizzare il copy constructor per creare un nuovo oggetto in deep copy:

```cpp
int main(int argc, char **argv) {
    // ...

    vector vec2 = vec;

    return EXIT_SUCCESS;
}
```

Ma se dopo aver effettuato l'inizializzazione di `vec2`, quindi aver utilizzato `=` durante la dichiarazione della variabile, provassimo a fare un'operazione di assegnamento tra due `vector` già esistenti, ad esempio:

```cpp
int main(int argc, char **argv) {
    // ...

    vector vec2 = vec;
    vec2 = vec;

    return EXIT_SUCCESS;
}
```

Quello che succede è che in fase di inizializzazione della variabile `vec2` viene utilizzato il costruttore di copia per eseguire una deep copy di `vec`. Di conseguenza l'indirizzo di memoria dell'attributo `data_` sarà differente tra `vec` e `vec2`. Ma quando eseguiamo l'operazione di assegnamento `vec2 = vec`, non viene più eseguito il copy constructor, in quanto l'operatore `=` non svolge più la funzione di inizializzazione, ma una funzione di assegnamento. Di conseguenza viene chiamato un metodo di assegnamento generato automaticamente dal compilatore, e come possiamo immaginare questo metodo di default effettua una *shallow copy*. In conclusione ci troveremmo un oggetto `vec2`, che inizialmente possedeva `data_` ad un indirizzo di memoria differente, possedere l'attributo `data_` puntare allo stesso indirizzo di memoria di `data_` di `vec`.

Per risolvere questo problema, dobbiamo definire un nuovo metodo che svolge la funzione di operatore di assegnamento. Questo sarà l'overload dell'operatore `=` per la nostra `struct vector`. La signature di questo metodo sarà la seguente `vector &operator=(const vector &rhs)`. Non bisogna farsi spaventare dalla signature, il nome `operator=` è semplicemente il nome di un metodo e potrebbe essere benissimo utilizzato come `vec2.operator=(vec)`, mentre il valore di ritorno `vector &` indica che il metodo ritorna un riferimento al `vector` stesso, in modo da poter concatenare le operazioni di assegnamento, ad esempio `vec3 = vec2 = vec1`. La parte `const vector &rhs` indica che stiamo passando un riferimento costante al `vector` da cui vogliamo copiare i dati. Il nome `rhs` sta per *Right Hand Side*, ovvero il lato destro dell'operazione di assegnamento.

```cpp
struct vector {
        // ...

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

        // ...
}
```

Il codice del metodo `operator=` è abbastanza semplice, è molto simile al copy constructor con qulche nota aggiuntiva. In primis controlliamo che il `vector` a sinistra dell'operatore di assegnamento non sia lo stesso del `vector` a destra dell'operatore di assegnamento, in quanto se fossero lo stesso oggetto, non avrebbe senso eseguire l'operazione di assegnamento.

```cpp
int main(int argc, char **argv) {
    // ...

    vector vec2 = vec;
    vec2 = vec2;

    return EXIT_SUCCESS;
}
```

In secundis, liberiamo la memoria puntata da `data_` del `vector` a sinistra dell'operatore di assegnamento, in quanto stiamo per sovrascrivere il puntatore con un nuovo puntatore che punta ad una nuova zona di memoria. Se non liberassimo la memoria di `data_` prima di scrivere il nuovo contenuto, perderemmo il riferimento alla zona di memoria puntata da `data_` e non potremmo più liberarla, causando *memory leak*. Il resto del codice è identico al copy constructor, in quanto dobbiamo copiare i valori di `size_` e `capacity_`, allocare una nuova zona di memoria per `data_` e copiare i valori della zona di memoria puntata da `data_` del `vector` a destra dell'operatore di assegnamento.
