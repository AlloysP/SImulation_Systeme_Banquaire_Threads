/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>

#include "message.h"
#include "alea.h"

/**
 * Programme de test des fonctions messages
 */
int main(int argc, char **argv)
{
  char emeteur[255], type[255], valeur[255];
  int decoupeOk;

  aleainit();
  // Creation d'une demande de paiement pour un montant aleatoire
  
  sprintf(valeur,"%d",alea(1,50000));
  printf("Demande de paiement de %s euros\n",valeur);
  
  char *msg = message("0001000000000000", "Demande", valeur);
  printf("Le message est : *%s*\n", msg);

  decoupeOk = decoupe(msg, emeteur, type, valeur);
  if (!decoupeOk) {
    printf("Erreur de d�coupage!!\n");
    exit(0);
  }

  printf("La carte bacaire est : %s\n", emeteur);
  printf("Le type du message est : %s\n", type);
  printf("La valeur est : %s\n", valeur);
  
  printf("\n\nMaintenant, on coupe le message pour avoir une erreur.\n");
  msg[5] = '\0';
  printf("message = %s\n", msg);
  decoupeOk = decoupe(msg, emeteur, type, valeur);
  if (!decoupeOk) {
    printf("Erreur de d�coupage!!\n");
    perror("TestMessage (decoupe)");
    exit(0);
  }

  free(msg);
  return 0;
}

