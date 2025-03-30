# Problem Jedzących Filozofów

## Wprowadzenie

Problem jedzących filozofów jest klasycznym problemem synchronizacji w programowaniu wielowątkowym. Grupa filozofów siedzi przy okrągłym stole, a każdy z nich cyklicznie myśli, staje się głodny, a następnie je. Proces ten jest powtarzany w nieskończoność. Problem polega na tym, że każdy z nich potrzebuje dwóch widelców - lewego i prawego. Celem implementacji jest rozwiązanie tego problemu języku C++, bez użycia gotowych mechanizmów synchronizacji takich jak mutexy czy semafory.

## Opis Problemu

Filozofowie siedzą przy stole, potrzebując dwóch widelców, aby jeść. Jeśli wszyscy filozofowie jednocześnie podniosą widelec z lewej strony, żaden nie będzie mógł podnieść widelca z prawej, co doprowadzi do zakleszczenia - sytuacji, w której żaden filozof nie będzie mógł kontynuować. Celem jest zaprojektowanie systemu, w którym każdy filozof będzie mógł *próbować* jeść, unikając zakleszczenia. Nie gwarantujemy, że każdy filozof zje, ale program nie może się zawiesić.

## Wymagania

* **Kompilator C++11** (np. GCC, Clang, MSVC)
* **CMake** (wersja 3.10 lub nowsza)
* **Make** 
* **(Opcjonalnie)** Python 3 (z `pip` lub `pipx`) do instalacji `cpplint` (do sprawdzania stylu)

## Uruchamianie

1.  **Pobierz repozytorium:**
    ```bash
    git clone https://github.com/Petter77/Systemy-Operacyjne-2-Projekt
    cd Systemy-Operacyjne-2-Projekt
    ```

2.  **Skompiluj projekt:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3.  **Uruchom program:**
    ```bash
    ./dining_philosophers <liczba_filozofow> [czas_symulacji_s]
    ```
    *Przykład:* `./dining_philosophers 5` lub `./dining_philosophers 7 60`

## Wątki i Synchronizacja

### Wątki

W programie wykorzystywane są następujące typy wątków:

1.  **Wątek Główny:**
    * **Odpowiedzialność:** Inicjalizacja, uruchamianie wątków filozofów, zarządzanie pętlą wyświetlającą interfejs użytkownika (UI), sygnalizowanie zakończenia symulacji oraz oczekiwanie na zakończenie pracy wątków filozofów (`join`).
2.  **Wątki Filozofów (N wątków):**
    * **Odpowiedzialność:** Każdy z N wątków reprezentuje jednego filozofa. Wykonuje logikę cyklu życia filozofa tj. myślenie, głód, próba jedzenia, jedzenie, odkładanie widelców. Aktualizuje swój stan w dedykowanej strukturze danych, aby funkcja displayUI mogła go odczytać.

### Sekcje Krytyczne

Sekcja krytyczna to fragmenty kodu, w których do danych zasobów dostęp próbuje uzyskać kilka wątków jednocześnie.

1.  **Dostęp do Widelców:**
    * **Problem:** Widelec jest zasobem współdzielonym przez dwóch sąsiadujących filozofów. Tylko jeden filozof może trzymać dany widelec w tym samym czasie. Brak zarządzania widelcami mógłby doprowadzić do sytuacji, w której dwóch filozofów trzyma ten sam widelec, lub każdy trzyma widelec po swojej lewej stronie, przez co każdy z nich będzie czekał w nieskończoność na zwolnienie się prawego widelca, co nigdy nie nastąpi.
    * **Rozwiązanie:**
        * Każdy widelec jest chroniony przez dedykowany obiekt `SpinlockLock` (przechowywany w `std::vector<SpinlockLock> forks`).
        * Filozof musi wywołać `forks[id_widelca].Lock()` przed użyciem widelca i `forks[id_widelca].Unlock()` po zakończeniu.
        * Aby zapobiec **zakleszczeniu**, zastosowana została strategia **hierarchii zasobów**. Filozof zawsze podnosi w pierwszej kolejności widelec o niższym indeksie. Pozwala to pozbyć się swego rodzaju "pętli", gdyż kolejność podnoszenia widelców uzależniona jest od jego ID, a nie od tego, po której stronie od filozofa się znajduje. 

2.  **Dostęp do Stanu Filozofów (`philosopher_states`):**
    * **Problem:** Wektor `philosopher_states` przechowuje informacje o statusie filozofa, trzymanych przez niego widelcach i czasie spędzonym w statusie **HUNGRY**. Jest on modyfikowany przez N wątków filozofów oraz odczytywany przez wątek główny (w pętli UI). Jednoczesny zapis przez jednego filozofa i odczyt przez UI (lub zapis przez innego filozofa) stanowi **wyścig danych (data race)** i może prowadzić do wyświetlania niespójnych lub nieprawidłowych informacji.
    * **Rozwiązanie:**
        * Wprowadzono **jedną wspólną blokadę** `SpinlockLock state_lock` dedykowaną do ochrony *całego* wektora `philosopher_states`.
        * **Każda operacja** (zarówno odczyt w `display_ui`, jak i zapis w `philosopher_life`), która uzyskuje dostęp do elementów `philosopher_states`, musi najpierw zdobyć blokadę `state_lock.Lock()`, a po zakończeniu operacji ją zwolnić `state_lock.Unlock()`. Zapewnia to atomowość operacji na współdzielonym stanie z perspektywy UI i innych filozofów.

### SpinlockLock - Podstawowa Blokada

Ze względu na wymów niekorzystania ze standardowych mechanizmów synchronizacji jak `std::mutex`, zaimplementowana została blokada typu **Spinlock** w klasie `SpinlockLock`.

* **Działanie:** Spinlock realizuje wzajemne wykluczanie poprzez tza **aktywne oczekiwanie**. Wątek próbujący zdobyć zajętą blokadę kręci się w pętli, sprawdzając flagę `std::atomic_flag`, aż stanie się dostępna. W momencie gdy stanie się dostępna, wątek przejmuje kontrolę nad sekcją krytyczną, wykonuje swoje zadanie, a następnie opuszcza flagę, przez co inny wątek może zrealizować swoje zadanie wewnątrz sekcji krytycznej.

### Interfejs

W celu wizualizacji działania projektu zaimplementowano czytelny interfejs:

| ID  | Status    | Forks | Waiting (ms) |
| --- | --------- | ----- | ------------ |
| 0   | Hungry    | 0 -   | 396          |
| 1   | Eating    | 1 2   | -            |
| 2   | Hungry    | - -   | 496          |
| 3   | Thinking  | - -   | -            |
| 4   | Hungry    | 4 -   | 658          |

Zawiera on takie pola jak:
* ID każdego filozofa
* Status
* Podniesione w danej chwili widelce
* Czas oczekiwania w statusie HUNGRY

## Dodatkowe elementy programu
* Niemal cały kod spełnia wymagania lintera (cpplinter)
* System automatyzacji budowy projektu (CMake)
