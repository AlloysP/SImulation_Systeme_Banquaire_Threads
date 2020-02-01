all:	clean TestMessage TestRedirection TestLectureEcriture terminal autorisation acquisition interbancaire 

message.o: message.c message.h
	gcc -Wall -c message.c

alea.o: alea.h alea.c
	gcc -Wall -c alea.c

TestMessage: message.o alea.o TestMessage.c
	gcc -Wall TestMessage.c message.o alea.o -o  TestMessage

TestRedirection: TestRedirection.c
	gcc -Wall TestRedirection.c -o  TestRedirection

lectureEcriture.o: lectureEcriture.c lectureEcriture.h
	gcc -c -Wall lectureEcriture.c

TestLectureEcriture: lectureEcriture.o TestLectureEcriture.c
	gcc lectureEcriture.o TestLectureEcriture.c -o TestLectureEcriture

fonctionsCommunes.o: fonctionsCommunes.c
	gcc -Wall -c fonctionsCommunes.c

terminal.o: terminal.c
	gcc -Wall -c terminal.c

terminal: fonctionsCommunes.o lectureEcriture.o alea.o message.o annuaire.o terminal.o
	gcc fonctionsCommunes.o lectureEcriture.o alea.o message.o annuaire.o terminal.o -o terminal

acquisition.o: acquisition.c
	gcc -Wall -c acquisition.c

acquisition: fonctionsCommunes.o lectureEcriture.o  message.o hashMap.o  acquisition.o 
	gcc -lpthread fonctionsCommunes.o lectureEcriture.o message.o hashMap.o acquisition.o -lm -o acquisition

autorisation.o: autorisation.c
	gcc -Wall -c autorisation.c

autorisation: fonctionsCommunes.o lectureEcriture.o message.o alea.o annuaire.o autorisation.o 
	gcc fonctionsCommunes.o lectureEcriture.o message.o alea.o annuaire.o autorisation.o -o autorisation

hashMap.o: hashMap.c
	gcc -Wall -c hashMap.c

interbancaire.o: interbancaire.c
	gcc -Wall -c interbancaire.c

interbancaire: fonctionsCommunes.o lectureEcriture.o  message.o annuaire.o alea.o hashMap.o interbancaire.o 
	gcc -lpthread fonctionsCommunes.o lectureEcriture.o message.o annuaire.o alea.o hashMap.o interbancaire.o -lm -o interbancaire

annuaire.o: annuaire.c annuaire.h
	gcc -c annuaire.c




clean:	
	rm -f *.o *~ *.an

cleanall: clean
	rm TestRedirection TestMessage TestLectureEcriture terminal acquisition autorisation hashMap Test interbancaire
