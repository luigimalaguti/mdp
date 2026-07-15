# Versione 00

> **Sort int**
>
> *Starting point, standard C program*

```c
int main(int argc, char **argv) {
    // ...

    // ...
}
```

## Codice

Il testo dell'esercizio richiede di creare un programma che possa essere utilizzato secondo la seguente sintassi da terminale:

```bash
$ sort_int <filein.txt> <fileout.txt>
```

Quindi il `main` del codice dovrà come prima cosa controllare il numero di argomenti passati da riga di comando:

```c
int main(int argc, char **argv) {
    /*
     * ==================================================
     * CHECK ARGUMENTS
     * ==================================================
     */
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    // ...
}
```

Questo ci permette di ottenere i nomi dei file che dovremo poi andare a leggere e scrivere.

Dopodiché possiamo procedere all'apertura del file di input, il file da cui dovremo andare a leggere il contenuto per poi processarlo.

```c
int main(int argc, char **argv) {
    // ...

    /*
     * ==================================================
     * OPENING INPUT FILE
     * ==================================================
     */
    FILE *input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        printf("Error: Could not open file '%s'\n", input_filename);
        return EXIT_FAILURE;
    }

    // ...
}
```

Se per qualsiasi ragione non si dovesse riuscire ad aprire il file, quindi il puntatore risulterebbe `FILE *input_file` uguale a `NULL`, allora il programma stampa un messaggio di errore e termina con un codice di uscita che indica fallimento.

Se invece il file viene aperto correttamente, allora possiamo procedere a leggere i numeri interi contenuti al suo interno. Per fare questo, possiamo utilizzare un vettore allocato dinamicamente, che ci permetterà di memorizzare tutti i numeri letti senza dover conoscere a priori quanti siano.

```c
int main(int argc, char **argv) {
    // ...

    int32_t *vector = NULL;
    size_t size = 0;
    size_t capacity = 0;

    /*
     * ==================================================
     * READING INPUT FILE
     * ==================================================
     */
    int32_t number;
    while (fscanf(input_file, "%d", &number) == 1) {
        // Resize the vector if necessary
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
        // Store the number in the vector
        vector[size] = number;
        size++;
    }
    fclose(input_file);

    // ...
}
```

Il procedimento è il seguente. La variabile `int32_t number` viene utilizzata per memorizzare temporaneamente il numero letto dal file, attraverso `fscanf`. La funzione `fscanf(input_file, "%d", &number)` legge quindi dal file `input_file` un numero intero `"%d"`, per poi memorizzarlo nella locazione di memoria `&number`.

La lettura di `fscanf(..., "%d", ...)` funziona nel seguente modo. Inizia a leggere dal file un carattere alla volta, continua a leggerli fino a quando non incontra un carattere che non può essere interpretato come parte di un numero intero. A quel punto, `fscanf` restituisce il numero di elementi letti con successo, che in questo caso sarà `1` in quanto stiamo leggendo un solo numero intero. Se invece non riesce a leggere alcun numero, restituirà `0` o `EOF` se si è raggiunta la fine del file.

La condizione `while (fscanf(input_file, "%d", &number) == 1)` permette quindi di continuare a leggere numeri interi dal file fino a quando ce ne sono disponibili. A ogni ciclo, il numero letto viene memorizzato nel vettore `vector`, che viene ridimensionato dinamicamente se necessario. Se la memoria non può essere allocata, il programma stampa un messaggio di errore e interrompe la lettura, mantenendo comunque i numeri già letti fino a quel momento.

Prima di procedere all'ordinamento del vettore, possiamo ridimensionarlo per adattarlo esattamente al numero di elementi letti, evitando così di sprecare memoria. Allo stesso modo di prima, se l'allocazione della memoria fallisce, il programma continuerà a funzionare con il vettore originale, che contiene comunque tutti i numeri letti.

```c
int main(int argc, char **argv) {
    // ...

    /*
     * ==================================================
     * SHRINKING VECTOR TO FIT SIZE
     * ==================================================
     */
    if (size < capacity) {
        int32_t *temp_vector = realloc(vector, size * sizeof(*vector));
        if (temp_vector != NULL) {
            vector = temp_vector;
            capacity = size;
        }
    }

    // ...
}
```

Ora possiamo procedere all'ordinamento del vettore. Per fare questo, possiamo utilizzare la funzione `qsort` della libreria standard C, che richiede un puntatore alla funzione di confronto da utilizzare per determinare l'ordine degli elementi.

```c
/*
 * ==================================================
 * COMPARE FUNCTION
 * ==================================================
 */
int compare_int32(const void *pointer_a, const void *pointer_b) {
    int32_t number_a = *(int32_t *)pointer_a;
    int32_t number_b = *(int32_t *)pointer_b;
    return (number_a > number_b) - (number_a < number_b);
}

int main(int argc, char **argv) {
    // ...

    /*
     * ==================================================
     * SORTING VECTOR
     * ==================================================
     */
    qsort(vector, size, sizeof(*vector), compare_int32);

    // ...
}
```

La funzione `qsort` prende come argomenti il puntatore al primo elemento del vettore, il numero di elementi nel vettore, la dimensione di ciascun elemento e un puntatore alla funzione di confronto. La funzione di confronto `compare_int32` confronta due numeri interi e restituisce un valore negativo, zero o positivo a seconda che il primo numero sia minore, uguale o maggiore del secondo.

```c
int main(int argc, char **argv) {
    // ...

    /*
     * ==================================================
     * OPENING OUTPUT FILE
     * ==================================================
     */
    FILE *output_file = fopen(output_filename, "w");
    if (output_file == NULL) {
        free(vector);
        printf("Error: Could not open file '%s'\n", output_filename);
        return EXIT_FAILURE;
    }

    /*
     * ==================================================
     * WRITING OUTPUT FILE
     * ==================================================
     */
    for (size_t index = 0; index < size; index++) {
        fprintf(output_file, "%d\n", vector[index]);
    }
    fclose(output_file);

    free(vector);
    return EXIT_SUCCESS;
}
```

Il programma si conclude aprendo il file di output in modalità scrittura. Se l'apertura del file fallisce, viene stampato un messaggio di errore, libera la memoria del vettore e il programma termina con un codice di uscita che indica fallimento. Altrimenti, il programma scrive i numeri ordinati nel file di output, uno per riga, utilizzando `fprintf`. Infine, il vettore dinamico viene liberato dalla memoria e il programma termina con successo.
