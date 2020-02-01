/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lectureEcriture.h"
#include "message.h"
#include "alea.h"
#include "fonctionsCommunes.h"
#include "annuaire.h"

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

int main(int argc, char *argv[])
{

    char vNomBank[50];

    
    int pipeTerminalEntree0 = atoi(argv[1]);//Entrée terminal (en lecture)
    int pipeTerminalSortie1 = atoi(argv[2]);//Sortie terminal (en écriture)
    sprintf(vNomBank, "%s", argv[3]);
    int vIndiceCBmin = atoi(argv[4]); // Indice minimum puis maximum pour prendre une carte bleue dans l'annuaire (pour éviter que 2 terminaux utilisent en même temps la même carte)
    int vIndiceCBmax = atoi(argv[5]);
    int vAverageTimeBetweenPaiements = atoi(argv[6]); // Temps moyen en secondes en chaque paiement

    int resultat;
    AnnuaireClients *an = annuaire("annuaire.an");
    char *msgToPrint;

    char *msg = malloc(50 * sizeof(char));
    char *cb = malloc(30 * sizeof(char));
    char *type = malloc(30 * sizeof(char));
    char *valeur = malloc(30 * sizeof(char));
    aleainit();

    printf("Terminal %s créé\n", vNomBank);
    while (1)
    {

        sleep(alea(0, vAverageTimeBetweenPaiements * 2)); //Temps d'attente alléatoire entre chaque paiement

        // creation de la demande ----------------------------------

        sprintf(valeur, "%d", alea(1, 1000000)); // Montant demandé

        msg = message(an->donnees[alea(vIndiceCBmin, vIndiceCBmax)].CB, "Demande", valeur);

        // Envoi de la demande -------------------------------------
        ecritLigne(pipeTerminalSortie1, msg);
        msgToPrint = msgToPrintConvert(msg);
        printf("Terminal %s: msg= %s  Demande Envoyée  ---------------------------------------------------------\n", vNomBank, msgToPrint);
        free(msgToPrint);

        // reception de la réponse ----------------------------------
        msg = litLigne(pipeTerminalEntree0);

        resultat = decoupe(msg, cb, type, valeur);
        if (resultat != 1)
        {
            printf("Terminal: Paiement invalide\n");
        }
        else
        {
            msgToPrint = msgToPrintConvert(msg); // On enlève le \n du msg pour plus de lisibilité
            if (atoi(valeur) == 1)
                printf("-----------------------------------------Terminal %s: Réponse= %s Paiement réalisé\n", vNomBank, msgToPrint);
            else
                printf("-----------------------------------------Terminal %s: Réponse= %s Paiement refusé\n", vNomBank, msgToPrint);
            free(msgToPrint);
        }
    }

    // Utile si on enlève la boucle while
    free(msg);
    free(cb);
    free(type);
    free(valeur);

    return 0;
}