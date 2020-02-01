/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void usage(char * basename) {
    fprintf(stderr,
        "usage : %s [<programme 1> [<programme 2>]]\n",
        basename);
    exit(1);
}

int main(int argc, char *argv[]) {
    int pid;       /* permet d'identifier qui on est*/
    int fdpipe[2]; /* sera utilis� pour lier les processus */

    if (argc != 3) usage(argv[0]);

    /* on cr�� le pipe  qui sera utilis� pour relier 
       la sortie du premier processus 
       vers l'entr�e du second 
    */
    if ( pipe(fdpipe) == -1 ) {
        perror("pipe");
        exit(-1);
    }

    switch(pid = fork()) {
        case -1:
            /* le fork a �chou� */
            perror("fork");
            exit(-1);       
        case 0:
            /* code du fils */
            /* on fait en sorte que lorsque le processus 
               �crira sur l'entr�e standard (1) 
               il le fera en fait dans le pipe (fdpipe[1])
            */
            dup2(fdpipe[1], 1);
            /* on ferme tout, m�me le pipe... 
               on n'en a plus besoin 
            */
            close(fdpipe[0]);
            close(fdpipe[1]);
            execlp(argv[1], argv[1],NULL);
            /* pas besoin de break,
               ce code n'existe d�j� plus � l'ex�cution 
            */
        default :
            /* code du p�re */  
            /* on fait en sorte que lorsque le processus 
               lira sur la sortie standard (0) 
               il le fera en fait dans le pipe (fdpipe[0])
            */
            dup2(fdpipe[0], 0);
            close(fdpipe[0]);
            close(fdpipe[1]);
            execlp(argv[2],argv[2], NULL);
    }
   /*
   cette portion de code ne sera jamais ex�cut�e, 
   puisque les processus ont d�j�
   �t� remplac�. On met n�anmoins un 
   return sinon le compilateur proteste.
   */
   return 0;
}
