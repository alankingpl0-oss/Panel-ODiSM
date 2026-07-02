/* ========================================================================= *
 * PANEL SERWISANTA ODISM                                                    *
 * Tu jest po prostu panel i nic więcej                                      *
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_TASKS 500000
#define MAX_USERS 100
#define MAX_STR 256
#define MAX_DESC 512
#define REJECT_LIMIT 5
#define REJECT_WINDOW 3600 /* 1 godzina w sekundach */
#define REJECT_TIMEOUT 180 /* 3 minuty kary w sekundach */

/* Struktura przechowujaca zadanie wczytane z pliku */
typedef struct {
    char title[MAX_STR];
    char task[MAX_DESC];
    int points;
} Task;

/* Struktura przechowujaca dane pojedynczego uzytkownika */
typedef struct {
    char username[MAX_STR];
    char password[MAX_STR];
    int points;
    int current_task_index;
} User;

/* Globalna baza użytkowników i indeks aktualnie zalogowanego */
User user_list[MAX_USERS];
int total_users = 0;
int logged_user_index = -1; /* -1 oznacza brak zalogowanego uzytkownika */
int is_logged_in = 0;

/* Tablica zadan i ich licznik */
Task task_list[MAX_TASKS];
int total_tasks = 0;

/* Historia odrzucen dla mechanizmu zabezpieczajacego */
time_t reject_timestamps[MAX_TASKS];
int reject_count = 0;
time_t reject_lock_end_time = 0;

/* Definicje progow poziomow oraz ich nazw (rozbudowane o nowe rangi) */
const int LEVEL_THRESHOLDS[] = {0, 100, 250, 350, 500, 750, 850, 2000, 5000, 10000, 999999};
const char *LEVELS[] = {
    "Mlodszy serwisant", "Serwisant sprzetu", "Diagnosta ODiSM", "Starszy diagnosta",
    "Mistrz optymalizacji", "Inzynier systemowy", "Szef dzialu IT", "Mistrz", 
    "Boski Mistrz", "Lord"
};
const int TOTAL_LEVELS = 10;

/* Prototypy funkcji czyszczacych strumien wejsciowy */
void clear_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Zapisywanie wszystkich profilow do pliku profile.dat */
void save_profile(void) {
    FILE *file = fopen("profile.dat", "w");
    if (file) {
        int i;
        fprintf(file, "%d\n", total_users);
        for (i = 0; i < total_users; i++) {
            fprintf(file, "%s\n", user_list[i].username);
            fprintf(file, "%s\n", user_list[i].password);
            fprintf(file, "%d\n", user_list[i].points);
            fprintf(file, "%d\n", user_list[i].current_task_index);
        }
        fclose(file);
    }
}

/* Wczytywanie wszystkich profilow z pliku profile.dat */
void load_profile(void) {
    FILE *file = fopen("profile.dat", "r");
    if (!file) {
        return; /* Plik jeszcze nie istnieje, to normalne przy pierwszym uruchomieniu */
    }
    
    if (fscanf(file, "%d\n", &total_users) == 1) {
        int i;
        for (i = 0; i < total_users && i < MAX_USERS; i++) {
            if (fgets(user_list[i].username, MAX_STR, file)) {
                user_list[i].username[strcspn(user_list[i].username, "\n")] = 0;
            }
            if (fgets(user_list[i].password, MAX_STR, file)) {
                user_list[i].password[strcspn(user_list[i].password, "\n")] = 0;
            }
            if (fscanf(file, "%d\n", &user_list[i].points) != 1) user_list[i].points = 0;
            if (fscanf(file, "%d\n", &user_list[i].current_task_index) != 1) user_list[i].current_task_index = 0;
        }
    }
    fclose(file);
}

/* Wczytywanie zadan z pliku zada.nia */
int load_tasks(void) {
    FILE *file = fopen("zada.nia", "r");
    if (!file) {
        return 0;
    }
    
    total_tasks = 0;
    while (total_tasks < MAX_TASKS && !feof(file)) {
        if (!fgets(task_list[total_tasks].title, MAX_STR, file)) break;
        task_list[total_tasks].title[strcspn(task_list[total_tasks].title, "\n")] = 0;
        
        if (!fgets(task_list[total_tasks].task, MAX_DESC, file)) break;
        task_list[total_tasks].task[strcspn(task_list[total_tasks].task, "\n")] = 0;
        
        if (fscanf(file, "%d\n", &task_list[total_tasks].points) != 1) break;
        
        total_tasks++;
    }
    
    fclose(file);
    return total_tasks;
}

/* Obliczanie aktualnego poziomu zalogowanego pracownika */
int get_current_level(void) {
    int i;
    int level_index = 0;
    int pts = user_list[logged_user_index].points;
    
    for (i = 0; i < TOTAL_LEVELS - 1; i++) {
        if (pts >= LEVEL_THRESHOLDS[i + 1]) {
            level_index = i + 1;
        } else {
            break;
        }
    }
    return level_index;
}

/* Wyswietlanie paska postepu w wersji tekstowej (TUI) */
void print_progress_bar(void) {
    int lvl = get_current_level();
    int next_lvl = lvl + 1;
    int pts = user_list[logged_user_index].points;
    
    printf("\nPunkty: %d | Poziom: %s\n", pts, LEVELS[lvl]);
    
    if (next_lvl < TOTAL_LEVELS) {
        int current_thresh = LEVEL_THRESHOLDS[lvl];
        int next_thresh = LEVEL_THRESHOLDS[next_lvl];
        int needed = next_thresh - current_thresh;
        int earned = pts - current_thresh;
        int percentage = (earned * 100) / needed;
        if (percentage < 0) percentage = 0;
        if (percentage > 100) percentage = 100;
        
        printf("[");
        int j;
        for (j = 0; j < 20; j++) {
            if (j < (percentage / 5)) printf("#");
            else printf(" ");
        }
        printf("] %d%%\n", percentage);
        printf("Brakuje %d pkt do awansu na: %s\n", next_thresh - pts, LEVELS[next_lvl]);
    } else {
        printf("[####################] MAX\n");
        printf("Gratulacje! Osiagnieto najwyzszy stopien technika ODiSM: %s.\n", LEVELS[TOTAL_LEVELS - 1]);
    }
    printf("--------------------------------------------------\n");
}

/* Filtrowanie starych prob odrzucenia zadania */
void clean_reject_attempts(void) {
    time_t now = time(NULL);
    int i;
    int valid_count = 0;
    for (i = 0; i < reject_count; i++) {
        if (now - reject_timestamps[i] < REJECT_WINDOW) {
            reject_timestamps[valid_count++] = reject_timestamps[i];
        }
    }
    reject_count = valid_count;
}

/* Ekran autoryzacji (Logowanie / Rejestracja) */
void handle_auth_screen(void) {
    int choice;
    char user[MAX_STR];
    char pass[MAX_STR];
    int i;
    int found = 0;
    
    printf("\n=== PANEL SERWISANTA ODiSM ===\n");
    printf("1. Logowanie\n");
    printf("2. Rejestracja pracownika\n");
    printf("3. Wyjscie z programu\n");
    printf("Wybor: ");
    if (scanf("%d", &choice) != 1) {
        clear_buffer();
        return;
    }
    clear_buffer();
    
    if (choice == 3) {
        exit(0);
    }
    
    if (choice == 2) {
        if (total_users >= MAX_USERS) {
            printf("\n[Blad] Osiagnieto maksymalna liczbe kont w systemie!\n"); /* Maks. 100 */
            return;
        }
        
        printf("\n--- REJESTRACJA NOWEGO SERWISANTA ---\n");
        printf("Podaj nazwe użytkownika: ");
        fgets(user, MAX_STR, stdin);
        user[strcspn(user, "\n")] = 0;
        
        /* Sprawdzenie czy użytkownik już istnieje */
        for (i = 0; i < total_users; i++) {
            if (strcmp(user_list[i].username, user) == 0) {
                printf("\n[Blad] Użytkownik o nazwie '%s' juz istnieje!\n", user);
                return;
            }
        }
        
        strcpy(user_list[total_users].username, user);
        
        printf("Podaj bezpieczne haslo: ");
        fgets(user_list[total_users].password, MAX_STR, stdin);
        user_list[total_users].password[strcspn(user_list[total_users].password, "\n")] = 0;
        
        user_list[total_users].points = 0;
        user_list[total_users].current_task_index = 0;
        
        total_users++;
        save_profile(); /* Zapis zaktualizowanej bazy kont */
        printf("\n[Sukces] Konto pracownika utworzone! Mozesz sie zalogowac.\n");
    } 
    else if (choice == 1) {
        if (total_users == 0) {
            printf("\n[Blad] Brak zarejestrowanych profilow. Wybierz najpierw opcje 2.\n");
            return;
        }
        printf("\n--- LOGOWANIE ---\n");
        printf("Uzytkownik: ");
        fgets(user, MAX_STR, stdin);
        user[strcspn(user, "\n")] = 0;
        
        printf("Haslo: ");
        fgets(pass, MAX_STR, stdin);
        pass[strcspn(pass, "\n")] = 0;
        
        for (i = 0; i < total_users; i++) {
            if (strcmp(user, user_list[i].username) == 0 && strcmp(pass, user_list[i].password) == 0) {
                logged_user_index = i;
                is_logged_in = 1;
                found = 1;
                /* Reset licznika odrzucen przy nowej sesji logowania */
                reject_count = 0;
                reject_lock_end_time = 0;
                printf("\nWitamy z powrotem, %s!\n", user_list[i].username);
                sleep(1);
                break;
            }
        }
        
        if (!found) {
            printf("\n[Blad] Niepoprawny login lub haslo!\n");
        }
    }
}

/* Procedura wieloetapowego resetowania postepow zalogowanego uzytkownika */
void handle_reset_progress(void) {
    char confirm_pass[MAX_STR];
    printf("\n⚠️ UWAGA! KROK 1/4: Czy na pewno chcesz zresetowac swoj postep? Stracisz punkty i range. (t/n): ");
    char c1 = getchar(); clear_buffer();
    if (c1 != 't' && c1 != 'T') return;
    
    printf("⚠️ OSTRZEZENIE KROK 2/4: To operacja NIEODWRACALNA. Kontynuowac? (t/n): ");
    char c2 = getchar(); clear_buffer();
    if (c2 != 't' && c2 != 'T') return;
    
    printf("⚠️ OSTRZEZENIE KROK 3/4: Zaraz zwalisz caly swoj progres koncertowo. Dalej? (t/n): ");
    char c3 = getchar(); clear_buffer();
    if (c3 != 't' && c3 != 'T') return;
    
    printf("🔒 KROK 4/4: Wpisz swoje haslo, aby ostatecznie usunac postep: ");
    fgets(confirm_pass, MAX_STR, stdin);
    confirm_pass[strcspn(confirm_pass, "\n")] = 0; /* Po prostu destrukcja... */

/* Nie oczekuje sie od Ciebie, ze to zrozumiesz */
    
    if (strcmp(confirm_pass, user_list[logged_user_index].password) == 0) {
        user_list[logged_user_index].points = 0;
        user_list[logged_user_index].current_task_index = 0;
        reject_count = 0;
        reject_lock_end_time = 0;
        save_profile(); /* Aktualizacja bazy danych w pliku */
        printf("\n[Baza zresetowana] Caly postep usuniety pomyślnie.\n");
    } else {
        printf("\n[Blad] Bledne haslo. Autoryzacja resetu odrzucona!\n");
    }
}

/* Ekran podgladu i decyzji o zleceniu */
void handle_view_task(void) {
    time_t now = time(NULL);
    clean_reject_attempts();

    if (now < reject_lock_end_time) {
        long remaining = (long)(reject_lock_end_time - now);
        printf("\n⛔ BLOKADA AKTYWNA: LIMIT ODRZUCEŃ PRZEKROCZONY! ⛔\n");
        printf("Poczekaj jeszcze: %ld min i %ld sek.\n", remaining / 60, remaining % 60);
        sleep(2);
        return;
    }

    int t_idx = user_list[logged_user_index].current_task_index;
    Task current_task = task_list[t_idx % total_tasks];

    printf("\n--- PODGLĄD ZLECENIA SERWISOWEGO ---\n");
    printf("[Zlecenie nr %d] %s\n", t_idx + 1, current_task.title);
    printf("Specyfikacja: %s\n", current_task.task);
    printf("Wycena: %d punktow\n", current_task.points);
    printf("--------------------------------------------------\n");
    printf("1. Przyjmij zlecenie i sporzadz rekomendacje\n");
    printf("2. Odrzuc audyt (Mozesz odrzucic jeszcze %d w tej godzinie)\n", REJECT_LIMIT - reject_count);
    printf("3. Powrot do menu glownego\n");
    printf("Wybor: ");

    int sub_choice;
    if (scanf("%d", &sub_choice) != 1) {
        clear_buffer();
        return;
    }
    clear_buffer();

    if (sub_choice == 1) {
        char report[MAX_DESC];
        printf("\n[URUCHOMIONO FORMULARZ REKOMENDACJI]\n");
        printf("Wpisz szczegolowa analizę techniczna (min. 10 znakow):\n> ");
        fgets(report, MAX_DESC, stdin);
        report[strcspn(report, "\n")] = 0;
        
        if (strlen(report) < 10) {
            printf("\n❌ Twój opis rekomendacji jest zbyt krotki! Mozesz zepsuc to zlecenie koncertowo.\n");
            sleep(2);
            return;
        }
        
        FILE *rep_file = fopen("reports.txt", "a");
        if (rep_file) {
            fprintf(rep_file, "========================================\n");
            fprintf(rep_file, "Serwisant: %s\n", user_list[logged_user_index].username);
            fprintf(rep_file, "Zlecenie: %s\n", current_task.title);
            fprintf(rep_file, "Punkty: %d\n", current_task.points);
            fprintf(rep_file, "Rekomendacja: %s\n", report);
            fprintf(rep_file, "========================================\n\n");
            fclose(rep_file);
        } else {
            printf("[Ostrzezenie] Nie udalo sie zapisac raportu do pliku reports.txt!\n");
        }

        user_list[logged_user_index].points += current_task.points;
        user_list[logged_user_index].current_task_index++;
        save_profile(); /* Zapisanie postepu bazy danych */
        printf("\n✅ Raport zapisany w pliku i wyslany! Przyznano %d punktow.\n", current_task.points);
        sleep(2);
    }
    else if (sub_choice == 2) {
        now = time(NULL);
        if (reject_count >= REJECT_LIMIT) {
            reject_lock_end_time = now + REJECT_TIMEOUT;
            printf("\nPrzekroczono limit! Blokada aktywowana na 3 minuty.\n");
        } else {
            reject_timestamps[reject_count++] = now;
            printf("\nOdrzucono audyt. Losowanie kolejnego komputera do analizy...\n");
            user_list[logged_user_index].current_task_index++;
            save_profile();
        }
        sleep(1);
    }
}

/* Glowny panel obslugi zlecen serwisowych */
void handle_main_panel(void) {
    if (total_tasks == 0) {
        printf("\n[Blad krytyczny] Brak zadan w pliku zada.nia lub plik nie istnieje!\n");
        is_logged_in = 0;
        logged_user_index = -1;
        return;
    }
    
    printf("\n=================== TWÓJ PANEL ODiSM ===================\n");
    printf("Zalogowany jako: %s\n", user_list[logged_user_index].username);
    print_progress_bar();
    
    printf("Liczba wszystkich dostepnych zadan w bazie: %d\n", total_tasks);
    printf("--------------------------------------------------\n");
    
    printf("Dostepne dzialania:\n");
    printf("1. Zobacz zlecenie\n");
    printf("2. ⚠️ RESETUJ POSTĘP ⚠️\n"); /* dajemy znaki a-30, zeby uzytkownik uwazal */
    printf("3. Wyloguj sie\n");
    printf("Wybor: ");
    
    int choice;
    if (scanf("%d", &choice) != 1) {
        clear_buffer();
        return;
    }
    clear_buffer();
    
    if (choice == 3) {
        is_logged_in = 0;
        logged_user_index = -1;
        printf("\nWylogowano pracownika.\n");
    } 
    else if (choice == 2) {
        handle_reset_progress();
    } 
    else if (choice == 1) {
        handle_view_task();
    }
}

/* Glowna funkcja sterujaca petla programu
 * w sumie nawet krotka, prawda?
 */
int main(void) {
    load_profile(); /* Wczytanie profilu na samym starcie programu */

    if (!load_tasks()) {
        printf("Krytyczny blad systemu: Nie udalo sie zaladowac pliku 'zada.nia'.\n");
        printf("Upewnij sie, ze plik znajduje sie w tym samym katalogu co program.\n");
        return 1;
    }
    
    while (1) {
        if (!is_logged_in) {
            handle_auth_screen();
        } else {
            handle_main_panel();
        }
    }
    
    return 0;
}
