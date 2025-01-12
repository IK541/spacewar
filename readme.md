g++ serv-threads.cpp Room.cpp Serv.cpp Player.cpp -o st -Wall


polecenia wysyłać zaraz po accept.
Pobieram pierwszy znak, który uruchamia switch case i jest wycinany z dalszej treści.
wiadomości:

Gdy prawidłowo przyjąłem polecenie, zwracam "Y\n", jeżeli nie mogę wykonac, "N\n" jeżeli response jest bardziej skomplikowany, po \n wysyłam resztę (np. pokoje)

nick:
A mynick

get all room info:
B

get specific room info (po dołączeniu, info o drużynach) (niech odpytuje co jakis czas, albo jako refresh):
C room_id

enter room:
D room_id

// IUSSUE
zmien druzyne: 
E

przelacz gotowosc:
F

// TODO not important
rozlacz:
G


//problems TO SOLVE
disconnect

