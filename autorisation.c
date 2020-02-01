/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "alea.h"
#include "fonctionsCommunes.h"
#include "message.h"
#include "lectureEcriture.h"
#include "annuaire.h"

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */

#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

bool isAutorized(char *numCard, char *montant, AnnuaireClients *pAnnuDeLaBanque) //Vérifie si le client a assez de fonds pour autoriser la transaction
{
    int accountMoney = client(pAnnuDeLaBanque, numCard)->solde;
    printf("  solde actuel: %d", accountMoney);
    int myMontant = atoi(montant);
    if (myMontant <= accountMoney)
    {
        accountMoney = accountMoney - myMontant;
        printf("  nouveau solde: %d", accountMoney);
        return true;
    }
    else
    {
        printf("   solde insuffisant pour debiter %d", myMontant);
        return false;
    }
}

int main(int argc, char *argv[])
{
    char vNomBank[50];
    char vNomFilePourAnnu[50]; 

    int pipeAutorisationEntree0 = atoi(argv[1]); // Entrée Autorisation (en lecture)
    int pipeAutorisationSortie1 = atoi(argv[2]); // Sortie Autorisation (en écriture)
    sprintf(vNomFilePourAnnu, "%s", argv[3]); //Annuaire contenant les soldes des clients de la banque
    sprintf(vNomBank, "%s", argv[4]);

    char *msgToPrint; //Variable utilisée pour print le msg

    char *msg;
    char *cb;
    char *type;
    char *valeur;
    int resultat;

    AnnuaireClients *vAnnuDeLaBanque = annuaire(vNomFilePourAnnu); //Création de l'annuaire à partir du fichier .an

    msg = malloc(50 * sizeof(char));
    cb = malloc(30 * sizeof(char));
    type = malloc(30 * sizeof(char));
    valeur = malloc(30 * sizeof(char));

    printf("Autorisation %s créé\n", vNomBank);
    while (1)
    {
        msg = litLigne(pipeAutorisationEntree0); //Réception du message

        msgToPrint = msgToPrintConvert(msg);// Pour print le msg
        printf("Autorisation %s: message lu %s", vNomBank, msgToPrint);
        free(msgToPrint);

        resultat = decoupe(msg, cb, type, valeur);
        if (resultat == 1)
        {
            if (isAutorized(cb, valeur, vAnnuDeLaBanque)) //Vérification du solde
            {
                valeur = strdup("1");
            }
            else
            {
                valeur = strdup("0");
            }

            type = strdup("Reponse");
            msg = message(cb, type, valeur); //Création de la réponse
            ecritLigne(pipeAutorisationSortie1, msg); //Réponse envoyée à l'acquisition de la banque
            msgToPrint = msgToPrintConvert(msg); //Pour print le msg
            printf("  Réponse envoyée= %s\n", msgToPrint);
            free(msgToPrint);
        }
        else
        {
            printf("Autorisation: Paiement invalide\n");
        }
    }

    return 0;
}