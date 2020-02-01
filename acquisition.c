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
#include "hashMap.h"

//#include "hashMap.h"

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#define HAVE_MALLOC 1

/* Define to 1 if you have the <malloc.h> header file. */
/* #undef HAVE_MALLOC_H */
#if defined(HAVE_MALLOC_H)
#include <malloc.h>
#endif

sem_t semPAcquisitionEntreeWrite; //Semaphore pour écrire dans le pipe d'entrée de acquisition, tous les threads de lecture du processus acquisition écrivent dans ce pipe.

struct table *vHashMapCodeCBEtPipesEntree1TerminauxWaiting; //HashMap (grâce à l'utilisation de structures) qui relie les codes de CB aux pipes des terminaux dont provient la demande

char *vNomBank;

struct thread_args //Pour stocker les arguements des threads
{
    int first;
    int second;
    int third;
};

void *threadAcquisition(void *_args) //Threads qui ont pour but lire tous les messages à destination de acquisition (tous les terminaux, autorisation, et le serveur interBancaire)
{
    char *msgToPrint; //Pour print msg
    struct thread_args *args = (struct thread_args *)_args;

    int pipeTerminalEntree1 = args->first;
    int pipeTerminalSortie0 = args->second;
    int pipeAcquisitionEntree1 = args->third;

    free(args);

    char *msg;
    char *cb;
    char *type;
    char *valeur;
    int resultat;

    msg = malloc(50 * sizeof(char));
    cb = malloc(30 * sizeof(char));
    type = malloc(30 * sizeof(char));
    valeur = malloc(30 * sizeof(char));

    printf("Acquisition Thread %s créé\n", vNomBank);
    while (1)
    {
        msg = litLigne(pipeTerminalSortie0); // Sortie du terminal (en lecture)
        resultat = decoupe(msg, cb, type, valeur);
        if (resultat != 1)
            printf("Acquisition Thread: Paiement invalide\n");
        else
        {
            sem_wait(&semPAcquisitionEntreeWrite);   //Sémaphore pour éviter que le message soit corompu par l'écriture simultanée de plusieurs threads acquisition
            ecritLigne(pipeAcquisitionEntree1, msg); // Entrée acquisition (en écriture)
            sem_post(&semPAcquisitionEntreeWrite);
            if (pipeTerminalEntree1 != 9999)
                insert(vHashMapCodeCBEtPipesEntree1TerminauxWaiting, atol(cb), (long int)pipeTerminalEntree1); //On mémorise le pipe du terminal pour lui renvoyer la réponse quand celle-ci arrive
            msgToPrint = msgToPrintConvert(msg);
            printf("Acquisition Thread %s: message lu %s  Message envoyé à Acquisition\n", vNomBank, msgToPrint);
            free(msgToPrint);
        }
    }

    pthread_exit(NULL); // Pas très utile car jamais atteinte
}

//Crée les processus terminal avec des fork( ) et execvp( )
void createTerminal(int pipeTerminalEntree0, int pipeTerminalSortie1, char *vNomBank, int pIndiceCBmin, int pIndiceCBmax, int vAverageTimeBetweenPaiements)
{

    int pid;

    pid = fork();
    if (pid == -1) // le fork a échoué
    {
        perror("fork");
        exit(-1);
    }
    else if (pid == 0)
    {
        char *vArgument1 = malloc(50 * sizeof(char));
        char *vArgument2 = malloc(50 * sizeof(char));
        char *vArgument3 = malloc(50 * sizeof(char));
        char *vArgument4 = malloc(50 * sizeof(char));
        char *vArgument5 = malloc(50 * sizeof(char));
        char *vArgument6 = malloc(50 * sizeof(char));
        char *vArgument7 = malloc(50 * sizeof(char));

        sprintf(vArgument1, "%d", pipeTerminalEntree0);
        sprintf(vArgument2, "%d", pipeTerminalSortie1);
        sprintf(vArgument3, "%s", vNomBank);
        sprintf(vArgument4, "%d", pIndiceCBmin);
        sprintf(vArgument5, "%d", pIndiceCBmax);
        sprintf(vArgument6, "%d", vAverageTimeBetweenPaiements);
        vArgument7 = NULL;

        char *argum[8];
        argum[0] = "./terminal";
        argum[1] = vArgument1;
        argum[2] = vArgument2;
        argum[3] = vArgument3;
        argum[4] = vArgument4;
        argum[5] = vArgument5;
        argum[6] = vArgument6;
        argum[7] = vArgument7;

        execvp("./terminal", argum);
    }
}

//crée le processus autorisation
void createAutorisation(int pipeAutorisationEntree0, int pipeAutorisationSortie1, char *vNomFilePourAnnu, char *vNomBank)
{

    char *vArgumentAuto1 = malloc(50 * sizeof(char));
    char *vArgumentAuto2 = malloc(50 * sizeof(char));
    char *vArgumentAuto3 = malloc(50 * sizeof(char));
    char *vArgumentAuto4 = malloc(50 * sizeof(char));
    char *vArgumentAuto5 = malloc(50 * sizeof(char));

    int pid;

    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(-1);
    }
    else if (pid == 0)
    {

        sprintf(vArgumentAuto1, "%d", pipeAutorisationEntree0);
        sprintf(vArgumentAuto2, "%d", pipeAutorisationSortie1);
        sprintf(vArgumentAuto3, "%s", vNomFilePourAnnu);
        sprintf(vArgumentAuto4, "%s", vNomBank);
        vArgumentAuto5 = NULL;

        char *argvAuto[6];
        argvAuto[0] = "./autorisation";
        argvAuto[1] = vArgumentAuto1;
        argvAuto[2] = vArgumentAuto2;
        argvAuto[3] = vArgumentAuto3;
        argvAuto[4] = vArgumentAuto4;
        argvAuto[5] = vArgumentAuto5;

        execvp("./autorisation", argvAuto);
    }
}

int main(int argc, char *argv[])
{
    int pipeAcquisitionEntree0 = atoi(argv[1]); //Entrée acquisition (en lecture)
    int pipeAcquisitionSortie1 = atoi(argv[2]); //Sortie acquisition (en écriture)
    int vNbTerminal = atoi(argv[3]);            // Nombre de terminaux à créer
    vNomBank = argv[4];
    char *vNumBank = argv[5];
    char *vNomFilePourAnnu = argv[6];
    int pipeAcquisitionEntree1 = atoi(argv[7]); //Entrée asquisition (en écriture), est utilisé par les threads acquisition pour ajouter les messages dans la "file d'attente"
    int vIndiceCBmin = atoi(argv[8]);
    int vNbCB = atoi(argv[9]);
    int vAverageTimeBetweenPaiements = atoi(argv[10]);

    int pipeAutorisationEntree[2];
    int pipeAutorisationSortie[2];

    sem_init(&semPAcquisitionEntreeWrite, 0, 1);

    vHashMapCodeCBEtPipesEntree1TerminauxWaiting = createTable(vNbTerminal);

    float vNbCBparTerminalFloat = vNbCB / vNbTerminal;            //Pour determiner combien de CB chaque terminal aura accès
    int vNbCBparTerminalInt = (int)floorf(vNbCBparTerminalFloat); //On prends la partie entière. Il y a donc potentiellement des cartes bleues "oubliées", si on poursuivait encore plus loin ce projet ce serait un des points à améliorer

    int pipeTerminalEntree[2];
    int pipeTerminalSortie[2];
    int pipeTerminalEntree0;
    int pipeTerminalSortie1;

    //Création des terminaux ainsi que des threads qui leur sont associés (grâce aux pipes). Un thread lit la sortie du terminal qui lui est associé
    for (int i = 0; i < vNbTerminal; i++)
    {
        struct thread_args *args = malloc(sizeof(struct thread_args));
        pthread_t threadAcq;

        pipe(pipeTerminalEntree);
        pipe(pipeTerminalSortie);

        pipeTerminalEntree0 = pipeTerminalEntree[0];
        pipeTerminalSortie1 = pipeTerminalSortie[1];

        createTerminal(pipeTerminalEntree0, pipeTerminalSortie1, vNomBank, vIndiceCBmin + i * vNbCBparTerminalInt, vIndiceCBmin + (i + 1) * vNbCBparTerminalInt - 1, vAverageTimeBetweenPaiements);

        args->first = pipeTerminalEntree[1];
        args->second = pipeTerminalSortie[0];
        args->third = pipeAcquisitionEntree1;

        pthread_create(&threadAcq, NULL, threadAcquisition, args); //Création du thread
    }

    pipe(pipeAutorisationEntree);
    pipe(pipeAutorisationSortie);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //De la même façon on crée le processus autorisation ainsi que le thread qui lui est associé
    createAutorisation(pipeAutorisationEntree[0], pipeAutorisationSortie[1], vNomFilePourAnnu, vNomBank);

    struct thread_args *args = malloc(sizeof(struct thread_args));
    args->first = 9999;
    args->second = pipeAutorisationSortie[0];
    args->third = pipeAcquisitionEntree1;

    pthread_t threadAcq;
    pthread_create(&threadAcq, NULL, threadAcquisition, args);

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    char *msg = malloc(50 * sizeof(char));
    char *cb = malloc(30 * sizeof(char));
    char *type = malloc(30 * sizeof(char));
    char *valeur = malloc(30 * sizeof(char));
    char *vNumBankCompar = malloc(30 * sizeof(char));
    int resultat;
    int vPipeTerminalEntree1ToSend; //On y stocke avec l'utilisation de la hashMap le pipe dans lequel envoyer la réponse
    char *msgToPrint;

    printf("Acquisition %s créé\n", vNomBank);
    while (1)
    {
        msg = litLigne(pipeAcquisitionEntree0); // Entrée Acquisition (en lecture)
        resultat = decoupe(msg, cb, type, valeur);
        if (resultat != 1)
            printf("Acquisition: Paiement invalide\n");
        else
        {
            msgToPrint = msgToPrintConvert(msg);
            if (strcmp(type, "Demande") == 0)
            { // Il s'agit d'une demande

                memcpy(vNumBankCompar, cb, 4 * sizeof(char)); //On récupère le numéro de banque associé à la CB en copiant dans la variable vNumBankCompar les 4 premiers chars de la CB
                if (atoi(vNumBankCompar) == atoi(vNumBank))   // Si meme bank
                {
                    ecritLigne(pipeAutorisationEntree[1], msg); // Entrée Autorisation
                    printf("Acquisition %s: message lu %s  Demande transmise à la banque\n", vNomBank, msgToPrint);
                }
                else
                {
                    ecritLigne(pipeAcquisitionSortie1, msg); // Entrée interBankLecture
                    printf("Acquisition %s: message lu %s  Demande transmise au serveur interBancaire\n", vNomBank, msgToPrint);
                }
            }
            else
            { // Il s'agit d'une Réponse
                vPipeTerminalEntree1ToSend = lookup(vHashMapCodeCBEtPipesEntree1TerminauxWaiting, atol(cb));
                if ((vPipeTerminalEntree1ToSend != -1) && (vPipeTerminalEntree1ToSend != 99999999999999999)) //Si égal à -1, l'entrée n'existe pas encore dans la hashMap, si égal à 99999999999999999 alors l'entrée existe mais n'est plus valable
                {                                                                                            // La réponse est à envoyée à l'un des terminaux de cette banque

                    ecritLigne(vPipeTerminalEntree1ToSend, msg);                                                  // Entrée Acquisition de la banque en question
                    insert(vHashMapCodeCBEtPipesEntree1TerminauxWaiting, atol(cb), 99999999999999999);            //On "enlève" la CB de la liste maintenant que la réponse a été envoyée, pour cela on met la valeur 99999999999999999
                    printf("Acquisition %s: message lu %s  Réponse envoyée au terminal\n", vNomBank, msgToPrint); //Suite:    qui ne peut pas correspondre à une carte bleue car ce char[] contient 17 chiffres
                }
                else
                {                                            // La réponse est à envoyée à un terminal appartenant à une autre banque
                    ecritLigne(pipeAcquisitionSortie1, msg); // Entrée interBankLecture
                    printf("Acquisition %s: message lu %s  Réponse transmise au serveur interBancaire\n", vNomBank, msgToPrint);
                }
            }
            free(msgToPrint);
        }
    }

    return 0;
}