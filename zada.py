import random

# No kurde dlaczego w Python nie ma komentarzy blokowych /* ... */!
# To jest generator zadań.

# Definicje cegiełek (szablonów) dla różnych części zadań

TYPY_ZADAN = [
    "Audyt stanowiska dyspozytora (Komputer biurowy)",
    "Diagnostyka terminalu diagnostycznego w warsztacie",
    "Modernizacja laptopa terenowego agronoma",
    "Optymalizacja komputera sterującego wagą najazdową",
    "Odzyskiwanie stacji roboczej CAD w biurze projektowym",
    "Przegląd serwera dokumentacji w archiwum",
    "Konfiguracja terminala kasowego w punkcie obsługi",
    "Reanimacja komputera diagnostycznego przy silosie"
]

PROBLEMY_CHLODZENIE_ZASILANIE = [
    "Procesor osiąga 92°C pod małym obciążeniem.",
    "Kurz całkowicie zablokował wentylator zasilacza.",
    "Brak reakcji na przycisk Power. Po podmianie zasilacza komputer wstaje, ale piszczy (błąd GPU).",
    "Wiatrak na karcie graficznej wydaje metaliczne dźwięki i cyklicznie staje.",
    "Zasilacz pod obciążeniem piszczy cewkami, komputer samoczynnie się restartuje.",
    "Napuchnięte kondensatory na płycie głównej w sekcji zasilania."
]

PROBLEMY_HARDWARE_DYSK = [
    "Dysk HDD wykazuje relokowane sektory.",
    "System operacyjny zgłasza błędy pamięci RAM.",
    "Błędy zapisu bazy na karcie SD.",
    "Obudowa pęknięta przy zawiasie. Matryca miga przy poruszaniu klapą.",
    "Taśma sygnałowa dysku SSD wykazuje błędy CRC w SMART.",
    "Brak wolnego miejsca na partycji systemowej NVMe."
]

PROBLEMY_SOFTWARE = [
    "System Windows działa potwornie wolno.",
    "System zaśmiecony aplikacjami startowymi, brakuje RAMu (4 GB).",
    "Systemd spowalnia bootowanie o dwie minuty przez wiszącą usługę sieciową.",
    "Złośliwe oprogramowanie (adware) blokuje dostęp do narzędzi administracyjnych.",
    "Sterowniki karty sieciowej powodują losowe błędy Blue Screen (BSOD).",
    "Baza danych PostgreSQL uszkodzona po nagłym zaniku napięcia."
]

def generuj_zadanie():
    # Losowanie poszczególnych komponentów zadania
    typ = random.choice(TYPY_ZADAN)
    p1 = random.choice(PROBLEMY_CHLODZENIE_ZASILANIE)
    p2 = random.choice(PROBLEMY_HARDWARE_DYSK)
    p3 = random.choice(PROBLEMY_SOFTWARE)

    # Losowanie punktacji (np. od 15 do 50, z krokiem 5)
    punkty = random.choice([15, 20, 25, 30, 35, 40, 45, 50])

    # Składanie wyników w format tekstowy
    wynik = f"{typ}\nWyniki przeglądu: {p1} {p2} {p3}\n{punkty}"
    return wynik

def generuj_zestaw_zadan(ilosc=5):
    zestaw = []
    for _ in range(ilosc):
        zestaw.append(generuj_zadanie())
    return "\n\n".join(zestaw)

# Uruchomienie generatora i wyświetlenie 5 przykładowych zadań
if __name__ == "__main__":
    print(generuj_zestaw_zadan(5))
