Costin Didoaca 


Tema 4- Client WEB. Comunicatie cu REST API

    In aceasta tema am ales sa folosesc biblioteca parson primita 
in linkul afiliat in enunt.Pe scurt, in fisierul client.c, 
inaintea fiecarei comenzi primite de la tastatura din enunt adica
login, logout, add_book, delete_book, enter_library, register,  
deschid cate o conexiune noua cu severul de fiecare data cand o 
comanda este primita la stdin iar in momentul cand primesc exit 
inchid conexiunea cu serverul.Pentru implementare am ales sa imi 
creez o structura Connection care imi retine in special 
raspunsul de la server, mesajul, tokenul, cookieurile si o 
structura Book.Structura book retine toate informatiile primite 
de la server pentru o anumita carte.

Functiile implementate in requests.c ajuta la trimiterea de 
cereri POST, DELETE si GET catre server. Helpers.c, este folosit 
pentru a trimite si pentru a primi mesaje de la server iar 
pentru a interactiona cu serverul si a putea conecta mesajul, am 
folosit buffer.c, fisiere disponibile in laboratorul inititulat 
Protocolul HTTP.

Scurta detaliere a implementarii comenzilor folosite:

1.login
    Comanda este utilizata pentru a se conecta la server.
    Utilizatorul introduce un nume de utilizator si o parola 
    care sunt apoi trimise la server prin intermediul unei 
    cereri HTTP POST. Daca serverul gaseste credentialele, 
    clientul este logat si primeste un cookie de sesiune.

2.register
    Comanda este utilizata pentru a inregistra un utilizator nou 
    in sistem.Utilizatorul furnizeazaa un username si o parola 
    care sunt trimise la server printr-o cerere HTTP POST. Daca 
    username-ul nu este deja inregistrat, serverul creeaza un 
    cont nou si trimite un raspuns de succes.

3.logout
    Comanda este utiliozata pentru a se deconecta un utilizator 
    de la server.Se trimite o cerere HTTP GET catre server 
    pentru a informa daca clientul doreste sa se deconecteze.
    Dupa aceasta, cookie-ul este sters.

4.enter_library
    Comanda este utilizata pentru a obtine accesul la biblioteca 
    dupa o autentificare.Se trimite o cerere HTTP GET catre 
    server pentru a se obtine accesul.DAca clientul este 
    autentificat, se obtine un token JWT care este utilizat 
    pentrtu a face alte cereri.

5.add_book
    
    Comanda este utilizata pentru a adauga o noua carte in 
    biblioteca.Utilizatorul introduce detaliile cartii(titlul, 
    autorul,genul, editorul si numarul de pagini). Sunt trimise 
    mai apoi catre server printr-o cerere HTTP POST.Daca exista 
    drepturi de adaugare a unei carti adica daca a intrat in 
    biblioteca cu comanda enter_library si detaliile sunt 
    valide, cartea este adaugata.

6.delete_book
    Comanda este utilizata pentru a sterge o carte existenta din 
    biblioteca avand la cerere id-ul acesteia.Se trimite catre 
    server prin HTTP DELETE.

Feedback:
O tema foarte interesanta, modul de lucru cu comenzile primite pentru
a fi implementate fiind unul creativ impreuna cu folosirea protocolului
HTTP.Keep up the good work, PCOM Team!!