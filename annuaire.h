/**
 * Authors: PETIT Alloys, JOSSE Raphaël, COUSTY Jean, NAJMAN Laurent
 */

typedef struct Client{
  char CB[17];
  char banque[5];
  int solde;
} Client;

typedef  struct AnnuaireClients
{
  int nbClients;
  Client * donnees;
} AnnuaireClients;


AnnuaireClients* nouvelAnnuaire(int nbClients);
AnnuaireClients* annuaireAleatoire(int nbBanques, int nbClients);
void afficherAnnuaire(AnnuaireClients *an);
int sauvegardeAnnuaire(AnnuaireClients *an, char *fileName);
AnnuaireClients *annuaire(char *fileName);
AnnuaireClients* annuaireFiltre(AnnuaireClients *anComplet, char *codeBanque);
void libererAnnuaire(AnnuaireClients* an);
Client *client(AnnuaireClients* an, char* cb);
