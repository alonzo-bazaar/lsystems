# Format json per l-system

## Obbiettivi del formato

## Specifica del formato
ho messo un campo `"version"` nel json per diciamo scaramanzia
mi va di essere forward compatible

tutti i campi toplevel oltre a `version` sono tipi di pianta, quindi ad esempio
```json
{
    "version" : 1,
    "cipresso" : "metti l-system cipresso",
    "cespuglio" : "metti l-system cespuglio",
    "e via dicendo" : "..."
}
```
Ogni l-system di pianta è un oggetto che può descrivere un l-system plain o parametric, vi sono campi in comune tra i due tipi di l-system e campi specifici dell'uno o altro tipo di L-system

## Campi Comuni
Per entrambi i tipi di l-system vi sono i campi
- `"type"` : può essere `"plain"` o `"parametric"`
- `"color_table"` : tabella di colori usata dall'albero, può essere
  - una lista di colori espressi in formato `"0x<rr><gg><bb>"`, per esempio `"0x110011"` (viola), `"0xff8800"` (giallo)
  - un oggetto con le seguenti campi
    - `"start"` : colore iniziale (di tronco), espresso come `"0x<rr><gg><bb>"`
    - `"end"` : colore finale (di foglia novella), espresso come `"0x<rr><gg><bb>"`
    - `"steps"` : numero intero di quanti valori intermedi interpolat mettere tra start e end 
    qualora espressa in questo secondo formato, `color-table` sarà trasformata in una lista lunga `steps` che interpola linearmente tra `start` e `end`
- `"thickness_table"` : tabella di spessori usata dall'albero, può essere
  - una lista di numeri che rappresentano i varii spessori
  - uno oggetto con i seguenti campi
    - `"start"`: spessore iniziale (di tronco)
    - `"end"`: spessore finale (di rametto)
    - `"steps"`: numero intero di quanti valori intermedi interpolati mettere tra start e end
    qualora espressa in questo secondo formato, `thickness-table` sarà trasformata in una lista lunga `steps` che interpola linearmente tra `start` e `end`
- `"axiom"` : stringa iniziale da cui si inizierà a fare la riscrittura tot volte secondo le regole degli l-system
- `"rewrite_rules"` : regole di riscrittra utilizzate, un l-system `"plain"` e uno `"parametric"` hanno formati diversi con cui sono espresse le rewrite rules dell'uno o l'altro tipo, ma il nome della chiave è lo stesso per entrambi

> TODO: per il colore forse si potrebbe introdurre un formato anche `{"r" : ..., "g": ..., "b":... }`, e poi durante la lettura i due possibili formati vengono "normalizzati" entrambi a un `Color` di raylib, boh
Le tabelle `color-table` e `thickness-table` possono avere lunghezze arbitrarie (purchè diverse da 0), se durante la generazione dell'albero viene richiesto un elemento della tabella che va oltre la lunghezza di questa verrà usato automaticametne l'ultiomo elemento della tabella

## Plain l-system
Un plain l-system ha, apparte quelli elencati sopra, i campi:
- `"stride"`: stride usata della tartaruga nell'eseguire un'istruzione `F` o `f`, sarebbe a dire, di quanto va avanti la tartaruga quando incontra una delle suddette istruzoni
- `"angle"`: angolo (espresso in gradi) usato dalla tartaruga nell'eseguire un'istruzione `+`, `-`, `&`, `^`, `\`, o `/`, sarebbe a dire, di quanto gira la tartaruga (nella direzione indicata dall'istruzione) quando questa incontra una delle suddette istruzioni

> Da notare come, dalla definizione qui data di `"stride"` e `"angle"`, risulti che tutte le istruzioni `F`/`f`, o `+`/`-`/`&`/`^`/`\`/`/`, seguite dalla tartaruga durante la creazione dell'albero saranno effettuate con lo stesso passo, (o angolo, a seconda dell'istruzione).

### `"rewrite_rules"` per un l-system plain
In un plain l-system le rewrite rule sono espresse come un oggetto avente come chiavi delle stringhe lunghe 1 contenente il carattere da riscrivere, e come valori
- la stringa con cui sostituire la chiave in questione (per riscritture deterministiche)
- una lista di coppie `[probabilità , "stringa con cui sositituire]` per riscritture stocastiche

### esempio di l-system plain
Ai fini di rendere quanto sopra scritto più chiaro è sotto presentato un documento json riportante un l-system plain (chiamato `"bush"`) rappresentato secondo lo schema sopra documentato.  
L'l-system ivi descritto è ripreso dal libro "The algorithmic beauty of plants".
```json
{
    "version" : 1,
    "bush" : {
        "type" : "plain",

        "color_table": {
            "start" : "0x00ff00",
            "end" : "0x884400",
            "steps" : 10,
        },
        "thickness_table": {
            "start" : 0.05
            "end" : 0.01
            "steps" : 10,
        },

        "stride" : 0.3,
        "angle" : 22.5,

        "axiom" : "A",
        "rewrite_rules" : {
            "A" : "[&FL!A]/////'[&FL!A]///////'[&FL!A]",
            "F" : [[0.9, "S////FF"],
                   [0.1, "S/////F]"]],
            "S": "FL",
            "L": "['''^^{-f+f+f-|-f+f+f}]"
        }
    }
}
```

## Parametric l-system

### `"rewrite_rules"` per un l-system parametrico
{
parameter_names
expansion
}

parameter names opzionale
se assente possibile anche mettere il valore di expansion e basta

il valore di expansion:
può esse una stringa
può essere una lista di elementi
può essere una lista di coppie [probabilità - expansion]

qualora presente `parameter_names` famo, in pratica, variabili locali che legano i parametri presenti in `parameter_names` con i valori presenti nei parametri della regola che stiamo espandendo




### espressioni matematiche espresse in json
- `["+", ...args]`: si aspetta che tutti i parametri siano numerici, ritorna la sommatoria di questi
- `["-", arg1, arg2]`: si aspetta che `arg1` e `arg2` siano entrambi numerici, ritorna `arg1 - arg2`
- `["*", ...args]`: si aspetta che tutti i parametri siano numerici, ritorna la produttoria di questi
- `["/", arg1, arg2]`: si aspetta che `arg1` e `arg2` siano entrambi numerici, ritorna `arg1 / arg2`, se `arg2` è pari a 0 tira un'eccezione
- `["if", check, then, else]`: se l'espressione `check` rende `false` o `0` allora si esegue `else`, altrimenti si esegue `then`.
- `[">", arg1, arg2]`: si aspetta che `arg1` e `arg2` siano numerici, ritorna `arg1>arg2?true:false`
- `["<", arg1, arg2]`: si aspetta che `arg1` e `arg2` siano numerici, ritorna `arg1<arg2?true:false`
- `["==", arg1, arg2, eps]`: si aspetta che `arg1`, `arg2`, e `eps` siano numerici, ritorna `true` se `abs(arg1-arg2)<eps`, altrimenti ritorna `false`.
- `["==", arg1, arg2]`: equivalente a `["==", arg1, arg2, 0.001]`

Un qualsiasi letterale numerico in un'espressinoe aritmetica è self evaluating.

Una stringa diversa da `"+"`, `"-"`, `"*"`, `"/"`, `"if"`, `">"`, `"<"`, o `"=="` all'inizio di una lista, o una qualsiasi stringa che non sia all'inizio di una lista, viene interpretata come un riferimento a una variabile presente in `"globals"`, e il suo valore sarà quindi quello del valore in `"globals"` con chiave pari alla stringa.

### Accorgimenti di quality of life
A fini quali semplicità di scrittura e di trasformazione di l-system plain in l-system parametrici sono stati presi inoltre i seguenti accorgimenti:
- Qualora espresso come singola stringa o come lista di coppie numero-stringa, un target di riscrittura sarà interpretato con le stesse regole con cui viene interpretato per un l-system plain, con i dovuti accorgimenti su come passare parametri impliciti a istruzioni che, nell'ambito di un l-system parametrico, non sono interpretabili senza un parametro di accompagnamento
- In assenza di un campo `"globals"`, sarà costruito un `"globals"` implicito contenente tutti i campi presenti nell'oggetto al di fuori di quelli con semantica sopra specificata (cioè con tutti i campi tranne quelli descritti nella sezione [Campi Comuni](#campi-comuni), qualora sia però presente un campo `"globals"` la presenza dei suddetti ulteriori campi è considerata un errore.
- Qualora inizializzate senza alcun parametro, le istruzioni `F` e `f` verrano create con una lista parametri lunga 1 contenente il valore alla chiave `"stride"` di `"globals"` (o del `"globals"` implicito), per le istruzioni `+`, `-`, `&`, `^`, `\`, o `/` verrà fatta la stessa cosa usando invece valore alla chiave `"angle"` di `"globals"` (o del `"globals"` implicito).
  La presenza di una regola `F`, `f`, `+`, `-`, `&`, `^`, `\`, o `/` in assenza di `"stride"` o `"angle"` in globals è considerata un errore.

# Instruction Alphabet per la tartaruga
Le istruzioni per la tartaruga sono espresse secondo la sintassi utilizzata nel libro "The algorithmic beauty of plants", sono qui riportati i comandi eseguiti da questa per ogni carattere nel caso di l-system sia plain che parametrici

- `[`
- `]`
- `{`
- `}`
- `!`
- `'`
- `|`

- `F`
- `f`
- `+`
- `-`
- `&`
- `^`
- `/`
- `\`
