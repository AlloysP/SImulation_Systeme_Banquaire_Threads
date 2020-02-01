/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <math.h>

#include "lectureEcriture.h"
#include "message.h"
#include "fonctionsCommunes.h"
#include "annuaire.h"
#include "hashMap.h"

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */
#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

struct thread_args //Pour stocker les arguments des threads
{
    int first;
    int second;
};

sem_t semPInterBankEntreeWrite;    //Semaphore pour écrire dans l'entrée du serveur enterBancaire, sur le même principe que acquisition les threads de interBancaire vont écrire dans le pipe de interBancaire grâce à ce sémaphore
int pipeInterbankLectureSortie[2]; //Pipe dans lequel les threads vont écrire

//Création des threads qui ont pour but de lire les sorties des banques (leur acquisition)
void *threadInterBankLecture(void *_args)
{
    struct thread_args *args = (struct thread_args *)_args;

    int pipeAcquisitionSortie0 = args->first;
    int vNumBankInt = args->second; //Numéro de la banque

    free(args); // On libère la mémoire de la structure une fois les arguments stockés

    char *msgToPrint;
    char *msg = malloc(50 * sizeof(char));

    char *vNumBankChar = malloc(50 * sizeof(char));
    sprintf(vNumBankChar, "%d\n", vNumBankInt);

    printf("InterBancaire Thread créé\n");
    while (1)
    {
        msg = litLigne(pipeAcquisitionSortie0); // Entrée InterBankLecture Thread (en lecture)

        sem_wait(&semPInterBankEntreeWrite);                     //Semaphore
        ecritLigne(pipeInterbankLectureSortie[1], msg);          // Entrée du serveur InterBancaire: on y écrit le msg
        ecritLigne(pipeInterbankLectureSortie[1], vNumBankChar); // Entrée du serveur InterBancaire: on y écrit le numéro de la banque associée à ce thread pour ensuite l'enregistrer dans la hashMap
        sem_post(&semPInterBankEntreeWrite);

        msgToPrint = msgToPrintConvert(msg);
        printf("InterBancaire Thread: message lu %s  Demande transmise au serveur interBancaire\n", msgToPrint);
        free(msgToPrint);
    }
}

void annuaireBuilder(long int nbBanques, long int nbCartes)
{
    //***************************************************************************
    //Création d'un annuaire global puis "unibancaire" (à tej quand on aura interbancaire)
    AnnuaireClients *an;
    //long int nbBanques, nbCartes;
    AnnuaireClients *anFiltre;
    //Client *cl;
    an = annuaireAleatoire(nbBanques, nbCartes);
    if (an == NULL)
    {
        fprintf(stderr, "Ne peut allouer un annuaire \n");
        exit(0);
    }
    // afficher l'annuaire an
    afficherAnnuaire(an);

    // sauvegarder l'annuaire an dans le fichier "annuaire"
    if (!sauvegardeAnnuaire(an, "annuaire.an"))
        fprintf(stderr, "Ne peut pas sauvegarder l'annuaire\n");

    // lib�rer la m�moire de l'annuaire an
    libererAnnuaire(an);

    // Charger un annuaire sauvegard� dans le fichier "annuaire"
    an = annuaire("annuaire.an");
    if (an == NULL)
    {
        fprintf(stderr, "ne peut lire l'annuaire depuis le fichier%s\n", "annuaire");
        exit(0);
    }

    char vNumBankPourAnnu[5];
    char vNomFilePourAnnu[50];
    for (int i = 0; i < nbBanques; i++)
    {
        // G�n�rer un annuaire ne contenant que les cartes de la banque "i"

        if (i < 10)
            sprintf(vNumBankPourAnnu, "000%d", i);
        else if (i < 100)
            sprintf(vNumBankPourAnnu, "00%d", i);
        else if (i < 1000)
            sprintf(vNumBankPourAnnu, "0%d", i);
        else
            sprintf(vNumBankPourAnnu, "%d", i);

        sprintf(vNomFilePourAnnu, "annuaire%s.an", vNumBankPourAnnu);

        anFiltre = annuaireFiltre(an, vNumBankPourAnnu);

        if (anFiltre == NULL)
        {
            fprintf(stderr, "ne peut filtrer l'annuaire demande\n");
            libererAnnuaire(an);
            exit(0);
        }

        // Afficher l'annuaire filtr�
        afficherAnnuaire(anFiltre);

        // Sauvegarder l'annuaire filtr�
        if (!sauvegardeAnnuaire(anFiltre, vNomFilePourAnnu))
            fprintf(stderr, "Ne peut pas sauvegarder l'annuaire\n");

        // Lib�re l'annuaire filtr�
        libererAnnuaire(anFiltre);
    }

    // Lib�rer la m�moire de l'annuaire initial
    libererAnnuaire(an);

    //***************************************************************************
}

void createAcquisitions(int pipeAcquisitionEntree[][2], int pipeAcquisitionSortie[][2], int pNbAcquisition, int pNbTerminaux, int pNbCB, int vAverageTimeBetweenPaiements)
{
    int vNbTotalTerminal = pNbAcquisition * pNbTerminaux;
    float vNbCBparBanqueFloat = pNbCB / vNbTotalTerminal;
    int vNbCBparBanqueInt = (int)floorf(vNbCBparBanqueFloat);

    char vNomBanque[50];
    char vNumBankPourAnnu[5];
    char vNomFilePourAnnu[50];
    int pid; // permet d'identifier qui on est
    for (int i = 0; i < pNbAcquisition; i++)
    {

        pid = fork();
        if (pid == -1) // le fork a échoué
        {
            perror("fork");
            exit(-1);
        }
        else if (pid == 0)
        {
            //Selection du nom de la banque à créer
            if (i == 0)
                sprintf(vNomBanque, "%s", "Banque Bénépé");
            else if (i == 1)
                sprintf(vNomBanque, "%s", "Crédit Chaton");
            else
                sprintf(vNomBanque, "Banque %d", i);

            //Selection du numéro de la banque à créer
            if (i < 10)
                sprintf(vNumBankPourAnnu, "000%d", i);
            else if (i < 100)
                sprintf(vNumBankPourAnnu, "00%d", i);
            else if (i < 1000)
                sprintf(vNumBankPourAnnu, "0%d", i);
            else
                sprintf(vNumBankPourAnnu, "%d", i);

            sprintf(vNomFilePourAnnu, "annuaire%s.an", vNumBankPourAnnu);

            char *vArgument1 = malloc(50 * sizeof(char));
            char *vArgument2 = malloc(50 * sizeof(char));
            char *vArgument3 = malloc(50 * sizeof(char));
            char *vArgument4 = malloc(50 * sizeof(char));
            char *vArgument5 = malloc(50 * sizeof(char));
            char *vArgument6 = malloc(50 * sizeof(char));
            char *vArgument7 = malloc(50 * sizeof(char));
            char *vArgument8 = malloc(50 * sizeof(char));
            char *vArgument9 = malloc(50 * sizeof(char));
            char *vArgument10 = malloc(50 * sizeof(char));
            char *vArgument11 = malloc(50 * sizeof(char));

            sprintf(vArgument1, "%d", pipeAcquisitionEntree[i][0]);
            sprintf(vArgument2, "%d", pipeAcquisitionSortie[i][1]);
            sprintf(vArgument3, "%d", pNbTerminaux);
            sprintf(vArgument4, "%s", vNomBanque);
            sprintf(vArgument5, "%d", i);                             //numéro de la banque (sous forme de int)
            sprintf(vArgument6, "%s", vNomFilePourAnnu);              //Nom donné au fichier .an qui contient les clients de la banque
            sprintf(vArgument7, "%d", pipeAcquisitionEntree[i][1]);   //Pour écrire dans l'entrée de acquisition, est utilisé par les threads de acquisition
            sprintf(vArgument8, "%d", i * vNbCBparBanqueInt);         //Indice de la 1ère case de l'annuaire auquel les terminaux de la banque pourront avoir accès
            sprintf(vArgument9, "%d", vNbCBparBanqueInt);             // Nombre des clients que les terminaux de la banque peuvent simuler (à partager entre chaque terminal)
            sprintf(vArgument10, "%d", vAverageTimeBetweenPaiements); //Temps moyen entre 2 paiements, temps de pause aléatoire de 0 seconde à 2*vAverageTimeBetweenPaiements secondes
            vArgument11 = NULL;

            char *argum[12];
            argum[0] = "./acquisition";
            argum[1] = vArgument1;
            argum[2] = vArgument2;
            argum[3] = vArgument3;
            argum[4] = vArgument4;
            argum[5] = vArgument5;
            argum[6] = vArgument6;
            argum[7] = vArgument7;
            argum[8] = vArgument8;
            argum[9] = vArgument9;
            argum[10] = vArgument10;
            argum[11] = vArgument11;

            execvp("./acquisition", argum);
        }
    }
}

int main(int argc, char *argv[])
{

    int vNbAcquisition;
    int vNbTerminaux;
    int vNbCB;
    int vAverageTimeBetweenPaiements;
    if (argc >= 2)
        vNbAcquisition = atoi(argv[1]); //Argument 1: Nombre de banques à créer
    else
        vNbAcquisition = 2; //valeur par défaut

    if (argc >= 3)
        vNbTerminaux = atoi(argv[2]); //Argument 2: Nombre de terminaux par banque à créer
    else
        vNbTerminaux = 2; //valeur par défaut

    if (argc >= 4)
        vNbCB = atoi(argv[3]); //Argument 3: Nombre de clients à créer dans l'annuaire
    else
        vNbCB = 4; //valeur par défaut

    if (argc >= 5)
        vAverageTimeBetweenPaiements = atoi(argv[4]); //Temps moyen entre chaque paiement
    else
        vAverageTimeBetweenPaiements = 4; //valeur par défaut. Chaque terminal va envoyer une demande toutes les 1 à 4*2=8 secondes (sans compter le temps de traitement des demandes)

    if (vNbCB <= vNbAcquisition * vNbTerminaux)    // Si il y a plus de terminaux que de cartes bleues, alors on remplace par une valeur suffisante
        vNbCB = vNbAcquisition * vNbTerminaux * 3; // <==> 2 à 3 cartes bleues par terminal (les divisions sont réalisées avec des float, et en prenant la partie entière, 1.9 qui est presque égal à 2 devient 1)

    struct table *vHashMapCodeBankEtTubes;          //HashMap pour mémoriser le pipe de sortie en lecture des banques créées
    struct table *vHashMapCodeCBanswerEtBankToSend; //HashMap pour mémoriser le pipe d'entré en écriture du terminal auquel envoyé une demande associée à une CB

    int pipeAcquisitionEntree[vNbAcquisition][2];
    int pipeAcquisitionSortie[vNbAcquisition][2];

    char *msgToPrint;

    sem_init(&semPInterBankEntreeWrite, 0, 1);
    pipe(pipeInterbankLectureSortie);
    vHashMapCodeBankEtTubes = createTable(vNbAcquisition);
    vHashMapCodeCBanswerEtBankToSend = createTable(vNbTerminaux);

    annuaireBuilder(vNbAcquisition, vNbCB); //Création des annuaires

    //On crée et stocke les pipes pour communiquer avec les banques, et on crée les processus chargés de lire ces pipes
    for (int i = 0; i < vNbAcquisition; i++)
    {
        pipe(pipeAcquisitionEntree[i]);
        pipe(pipeAcquisitionSortie[i]);

        insert(vHashMapCodeBankEtTubes, (long int)i, (long int)pipeAcquisitionEntree[i][1]);

        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->first = pipeAcquisitionSortie[i][0];
        args->second = i;

        pthread_t threadIntLect;
        pthread_create(&threadIntLect, NULL, threadInterBankLecture, args);
    }

    //Création des processus acquisitions avec les pipes précédemment stockés
    createAcquisitions(pipeAcquisitionEntree, pipeAcquisitionSortie, vNbAcquisition, vNbTerminaux, vNbCB, vAverageTimeBetweenPaiements);

    char *cb = malloc(30 * sizeof(char));
    char *type = malloc(30 * sizeof(char));
    char *valeur = malloc(30 * sizeof(char));
    char *msg = malloc(50 * sizeof(char));
    char *vNumBankCompar = malloc(30 * sizeof(char));
    int resultat;
    char *vNumBankChar = malloc(50 * sizeof(char));
    long int vCodeBankToSend;
    int vPipeAcquisitionEntreeOfBankToSend;

    printf("InterBancaire créé\n");
    while (1)
    {

        msg = litLigne(pipeInterbankLectureSortie[0]);          // Sortie InterBankLecture: On lit le prochain message qui a été envoyé par un des threads
        vNumBankChar = litLigne(pipeInterbankLectureSortie[0]); // Sortie InterBankLecture: On lit le numéro de banque associé au thread qui a envoyé le message

        resultat = decoupe(msg, cb, type, valeur);
        if (resultat != 1)
            printf("InterBancaire: Paiement invalide\n");
        else
        {
            if (strcmp(type, "Demande") == 0)
            { // Il s'agit d'une demande
                memcpy(vNumBankCompar, cb, 4 * sizeof(char));
                vPipeAcquisitionEntreeOfBankToSend = lookup(vHashMapCodeBankEtTubes, atol(vNumBankCompar)); //On cherche dans la hashMap le pipe de la banque associé à la CB de la demande
                if (vPipeAcquisitionEntreeOfBankToSend != -1)
                {
                    ecritLigne(vPipeAcquisitionEntreeOfBankToSend, msg);                    // Entrée Acquisition de la banque en question
                    insert(vHashMapCodeCBanswerEtBankToSend, atol(cb), atol(vNumBankChar)); //On mémorise la banque à laquelle envoyée la réponse pour cette CB quand celle-ci arrivera
                    msgToPrint = msgToPrintConvert(msg);
                    printf("InterBancaire: message lu %s  Demande envoyée à la banque numéro %s\n", msgToPrint, vNumBankCompar);
                    free(msgToPrint);
                }
                else
                    printf("InterBancaire: La banque %s n'existe pas\n", vNumBankCompar);
            }
            else
            { // Il s'agit d'une réponse
                vCodeBankToSend = lookup(vHashMapCodeCBanswerEtBankToSend, atol(cb)); //On regarde à quelle banque il faut envoyer la réponse pour cette CB
                if (vCodeBankToSend != -1)
                {
                    vPipeAcquisitionEntreeOfBankToSend = lookup(vHashMapCodeBankEtTubes, vCodeBankToSend);
                    if (vPipeAcquisitionEntreeOfBankToSend != -1)
                    {
                        ecritLigne(vPipeAcquisitionEntreeOfBankToSend, msg); // Entrée Acquisition de la banque en question
                        msgToPrint = msgToPrintConvert(msg);
                        printf("InterBancaire: message lu %s  Réponse envoyée à la banque numéro %ld\n", msgToPrint, vCodeBankToSend);
                        free(msgToPrint);
                    }
                    else
                        printf("InterBancaire: La CB %s n'est pas dans la liste des CB\n", cb);
                }
                else
                    printf("InterBancaire: La CB %s n'est pas dans la liste des CB attendant une réponse\n", cb);
            }
        }
    }

    return 0;
}
