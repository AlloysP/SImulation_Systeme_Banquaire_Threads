/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#ifndef _MESSAGEH_
#define _MESSAGEH_

/**
 * Decoupe du message 
 * Retourne 1 si il n'y a pas eu de probl�me
 * Retourne 0 si il y a eu un probl�me
 * ATTENTION:
 *   Les variables cb, type et valeur doivent 
 * �tre allou�es avant l'appel � decoupe
 */

int decoupe(char *message,      /* Source a d�couper */
	    char *cb,           /* no de carte concernee */
	    char *type,         /* Type du message */
	    char *valeur        /* Valeur associ�e au message */
	    );

/** 
 * Construction du message
 * Retourne le message construit suivant le protocole
 * a partir des arguments
 * Le message est allou� dans la fonction
 */
char* message(char *cb, char *type, char * valeur);

#endif // _MESSAGEH_
