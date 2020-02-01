Authors: PETIT Alloys, JOSSE Raphaël

# Simulation_Systeme_Banquaire_Threads
Il est question de simuler un système de paiement par carte bancaire en langage C, ce projet permet de s’entraîner sur la synchronisation des threads.

Pour lancer le programme de simulation bancaire:
-faire un make
-tapper ./interbancaire avec les paramètres voulus (voir en-dessous pour plus de détails)


Les paramètres possibles(possibilité de mettre 0, 1, 2, 3 ou 4 paramètres):
1er paramètre: 
Nombre de banques à créer.

2ème paramètre: 
Nombre de terminaux à créer pour chacune des banques.

3ème paramètre: 
Nombre de clients à créer dans l'annuaire.

4ème paramètre:
Temps d’attente moyen entre chaque paiement.


- Faire 'make' pour tout compiler.  
  Si warning/problème sur malloc, modifier HAVE_MALLOC dans les
  fichiers

- Faire 'make' cleanall pour tout nettoyer, avant de faire une archive
  * NOTE * : les Mac-users doivent utiliser la commande suivante
  COPYFILE_DISABLE=1 tar zcvf libCarteBancaire.tgz libCarteBancaire
  pour faire une archive sans les ennuyeux ._* fichiers spéciaux du Mac.

- TestLectureEcriture.c / lectureEcriture.(c,h) : Fonction de lecture/ecriture 
  d'une ligne/message

- alea.(c,h) : générateur aléatoire - testé dans TestMessage

- TestMessage.c message.(c,h) : Fonction de gestion des messages

- TestRedirection.c : example de redirection, celui donné en annexe dans le sujet
