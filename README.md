# projekt_zso_1

- program ma być bezpieczny wielowątkowo (C/C++) i ma symulować rozwiązanie problemu przedstawionego  scenariusza
- projekty piszemy w C/C++ (biblioteka thread) - preferowane C, a jeżeli w C++ to proszę unikać obiektowości, dziedziczenia, wszelki aspektów utrudniających weryfikację - skupiamy się na wątkach.
- program powinien dać się sparametryzować - robimy to na poziomie kompilacji przez consty oraz z wykorzystaniem #ifdef w przypadku wyświetlania informacji na ekranie i wprowadzeniu opóźnień. Generalnie program powinien dać się skompilować w dwóch wersjach bez wyświetlania informacji na ekranie i bez opóźnień (żadnych sleepów) oraz z informacjami i ze sleepami i do tego proszę użyć #ifdef. Ma to pokazać, ze program jest merytorycznie poprawny.
- program będzie testowany przy użyciu narzędzia Valgrind. Opis narzędzia jest w wykładzie 11. Mamy sprawdzić wycieki pamięci, blokowanie się wątków, warunki hazardu  itp... 
- aby program napisać poprawnie trzeba wykorzystać mechanizmy synchronizacji opisane w wykładach 1 – 6 czyli głównie muteksy i obowiązkowo zmienne decyzyjne. 
- zadanie trzeba obronić, to znaczy wstawić do gakko i umówić się na pogawędkę
- w zadaniach proszę unikać elementów losowości
- termin oddania pierwszego projektu jest w zadaniach na gakko - proszę sprawdzić czy to jest zadanie jest widoczne.
- jeżeli ktoś ma problem ze zrozumieniem treści projektu, lub ze zrozumieniem działania tych mechanizmów synchronizujących to proszę o kontakt.

 

Uwaga, program implementujemy w funkcji o nazwie projekt_zso(...) i ta funkcja powinna zawierać kompletne rozwiązanie problemu. Funkcja ta ma zostać uruchomiona 10 razy. Wszystkie wątki mają powstać w tej funkcji i zostać poprawnie zakończone.

Problem miejsca w RESTAURACJI.
Mamy restaurację, a w niej dziesięć 4 osobowych stolików i  4 kelnerów w restauracji.
Klienci i klientki wchodzą do restauracji grupkami nie większymi niż 4 osoby.
Kelnerzy chcą obsłużyć jak największą liczbę klientów i klientek, więc jeżeli do restauracji wchodzi grupa, to jak jest wolny stolik to grupie jest przydzielany ten stolik.
Kelnerzy chcą obsłużyć jak największą ilość gości, więc jeżeli dla grupy dla której nie ma wolego stolika, to może być ona posadzona przy zajętym stoliku o ile jest jest wystarczająco dużo wolnych miejsc.
* Jeżeli przy stoliku siedzi para – klient i klientka – wówczas do takiego stolika nikogo się nie dosadza.
Klient może odejść od stolika i wyjść z restauracji, jeżeli wszyscy goście jedzący przy danym stoliku podejmą taką decyzję.
