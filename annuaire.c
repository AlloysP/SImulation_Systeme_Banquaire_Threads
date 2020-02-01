/**
 * Authors: PETIT Alloys, JOSSE Raphaël, COUSTY Jean, NAJMAN Laurent
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "annuaire.h"
#include "alea.h"
#include "lectureEcriture.h"

AnnuaireClients* nouvelAnnuaire(int nbClients)
  /*****************************************************************/
  /* Alloue la m�moire pour un annuaire pouvant contenir nbClients */
  /*****************************************************************/
{
  AnnuaireClients *an = malloc(sizeof(AnnuaireClients));
    
  an->nbClients = nbClients;
  an->donnees = (Client*)malloc(sizeof(Client) * nbClients);
  return an;
}

void libererAnnuaire(AnnuaireClients* an)
  /*****************************************************************/
  /* Lib�re la m�moire r�serv�e pour un annuaire                   */
  /*****************************************************************/
{
  free(an->donnees);
  free(an);
}

AnnuaireClients *annuaireAleatoire(int nbBanques, int nbClients)
  /*****************************************************************/
  /* Retourne un annuaire g�n�r� al�atoirement contenant nbClients */
  /* r�partis dans nbBanques, num�rot�es de 0 � nbBanques - 1      */
  /*****************************************************************/
{

  int i,j;
  char temp[2];
  AnnuaireClients* an = nouvelAnnuaire(nbClients);
  if(an == NULL) return NULL;

  aleainit();

  for(i = 0; i < nbClients; i++){
    // On g�n�re une banque aleatoirement
    sprintf(an->donnees[i].banque, "%04d", alea(0,nbBanques-1));
    
    //On g�n�re le no de carte aleatoirement
    strcpy(an->donnees[i].CB, an->donnees[i].banque);
    for(j=0; j < 12; j++){
      sprintf(temp, "%01d", alea(0,9));
      strcat(an->donnees[i].CB, temp);
    }
    // p, g�n�re al�atoirement le solde en centimes
    an->donnees[i].solde = alea(1, 1000000);
  }
  return an;
}

void afficherAnnuaire(AnnuaireClients *an)
  /*****************************************************************/
  /* Affiche � l'�cran l'annuaire an                               */ 
  /*****************************************************************/
{
  int i;
  char * ligne;
  
  if (an == NULL) 
  {
    printf("Annuaire invalide\n");
    return;
  }
  
  ligne = malloc(sizeof(char) * TAILLEBUF);
  
  printf("Il y a %d clients inscrits\n", an->nbClients);

  for(i = 0; i < an->nbClients ; i++){
    sprintf(ligne,"%s %s %d\n", an->donnees[i].CB, an->donnees[i].banque, an->donnees[i].solde);
    ecritLigne(1, ligne);
  }
}


int sauvegardeAnnuaire(AnnuaireClients *an, char *fileName)
  /*****************************************************************/
  /* Sauvegarde l'annuaire an dans le fichier nomm� fileName       */
  /* Retourne 1 en cas de succ�s et 0 en cas d'�chec (le fichier   */
  /* n'existe pas ou l'annuaire est invalide).                     */
  /*****************************************************************/
{
  int i, fd;
  char * ligne;
  
  if (an == NULL) 
  {
    printf("Annuaire invalide\n");
    return 0;
  }
  
  fd = open(fileName, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU); 
  if(fd == -1){
    printf("Nom de fichier invalide (%s)\n", fileName);
    return 0;
  }
  ligne = malloc(sizeof(char) * TAILLEBUF);
  

  for(i = 0; i < an->nbClients ; i++){
    sprintf(ligne,"%s %s %d\n", an->donnees[i].CB, an->donnees[i].banque, an->donnees[i].solde);
    ecritLigne(fd, ligne);
  }

  close(fd);
  return 1;
}

AnnuaireClients *annuaire(char *fileName)
  /*****************************************************************/
  /* Retourne l'annuaire stock� dans le fichier filName            */
  /*****************************************************************/
{
  int i, fd, nbClients, nb, len;
  AnnuaireClients *an;
  char * ligne;
  char tmp[32];

  fd = open(fileName, O_RDONLY); 
  if(fd == -1){
    fprintf(stderr,"Nom de fichier invalide (%s)\n", fileName);
    return NULL;
  }

  // on compte d'abord le nombre de ligne du fichier
  nbClients = 0;
  ligne = litLigne(fd);
  while(ligne != NULL){
    nbClients++;
    free(ligne);
    ligne = litLigne(fd);
   }

  an = nouvelAnnuaire(nbClients);
  if(an == NULL){
    fprintf(stderr,"Ne peut allouer pas allouer un annuaire\n");
    return NULL;
  }
  
  lseek(fd, SEEK_SET, 0);
  for(i = 0; i < an->nbClients ; i++){
    ligne = litLigne(fd);
    if(ligne == NULL){
      fprintf(stderr, "Ne peut lire la ligne\n");
      return NULL;
    }
    nb=sscanf(ligne, "%s %s %d\n",
    	      an->donnees[i].CB, an->donnees[i].banque, &(an->donnees[i].solde));
    if(nb != 3){
      fprintf(stderr,"Le format du fichier %s n'est pas celui d'un annuaire\n", fileName);
      libererAnnuaire(an);
      free(ligne);
      return NULL;
    }
    free(ligne);
  }

  close(fd);
  return an;
}

AnnuaireClients* annuaireFiltre(AnnuaireClients *anComplet, char *codeBanque)
  /*****************************************************************/
  /* Retourne un nouvel annuaire qui contient toutes les entr�es   */
  /* de anComplet correspondant aux clients de la banque dont le   */
  /* est codeBanque.                                               */
  /*****************************************************************/
{
  int nbFiltres, i, j;
  AnnuaireClients *anFiltre;

  nbFiltres = 0;
  
  for(i = 0; i < anComplet->nbClients; i++){
    if(strcmp(anComplet->donnees[i].banque, codeBanque) == 0)
      nbFiltres ++;
  }

  anFiltre = nouvelAnnuaire(nbFiltres);
  
  if(anFiltre == NULL){
    fprintf(stderr, "Ne peut allouer un annuaire\n");
    return NULL;
  }

  j = 0;

  for(i = 0; i < anComplet->nbClients; i++)
    if(strcmp(anComplet->donnees[i].banque, codeBanque) == 0) {
      strcpy(anFiltre->donnees[j].CB, anComplet->donnees[i].CB);
      strcpy(anFiltre->donnees[j].banque, anComplet->donnees[i].banque);
      anFiltre->donnees[j].solde = anComplet->donnees[i].solde;
      j++;
    }
  return anFiltre;
}


Client *client(AnnuaireClients* an, char* cb)
  /*****************************************************************/
  /* Retourne le (premier) client de l'annuaire dont le numero de  */
  /* carte est cb. Si l'annuaire ne contient pas de client dont la */
  /* carte est cb, alors NULL est retourn�.                        */
  /*****************************************************************/
{
  int i;
  
  for(i = 0; i < an->nbClients; i++)
    if( strcmp(an->donnees[i].CB, cb) == 0) return &(an->donnees[i]);
  return NULL;
}
