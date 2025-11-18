#include "config.h"

// Définition des variables
const char* ssid = "Pompe_relevage-75";     // Définie une seule fois
const char* password = "123456789";      // Définie une seule fois
const char* apHostname = "pompe1.com"; // Définie une seule fois

int Led = 2;

bool capteurBloque = false; // Valeur initiale
bool relaisDeclenche = false; // Valeur initiale
int securiteDeclenche; // Valeur initiale
unsigned long tempsDebut = 0;
unsigned long tempsRelaisDeclenche = 0;
bool moteurEnMarche = false;
unsigned long tempsrelai = 0;
unsigned long tempsTotal = 0;
unsigned long jours = 0, heures = 0, minutes = 0, secondes = 0;
unsigned long derDemar = 0;
unsigned long TEMP_FONCTIONNEMENT_MOTEUR = 10000;

const unsigned long TEMPO_ARRET_10MN = 5 * 60 * 1000UL; //finalement 5 minute ca suffit quitte a le relancer
unsigned long tempsDebutMarcheForce = 0;

const int MAX_ENTRIES = 10;
const int MAX_STRING_LENGTH = 100;
unsigned long tempsDepuisderDemar[MAX_ENTRIES] = {0};
unsigned long tempsReelDeFonctionnement[MAX_ENTRIES] = {0};
int indiceDerniereEntreeTempsDepuis = 0;
int indiceDerniereEntreeTempsReel = 0;
int nombreEntries = 0;
int indiceDerniereEntree = 0;

volatile unsigned long tempsDebutReel = 0;
volatile unsigned long tempsFinReel = 0;
volatile bool contacteurDeclenche = false;
int simulateurContacteur = 0;
unsigned long tempsReel = 0;
bool miseAJourEffectuee = false;
bool initialisationVoyant = false;
bool continuerProgramme = true;  
unsigned long debutNiveauHaut = 0;
const unsigned long SEUIL_BLOCAGE = 5000;  // 10 secondes

//bouton presence tension 
const char* couleurTension = "#f44336";  // Valeur par défaut (rouge)
const char* etatTension = "❌ État inconnu";
bool tensionPresente = false ;


// Définition des variables
const long intervalled = 1000;
unsigned long previousMillisled = 0;
bool ledState = false;
unsigned long previousMillis_print = 0; // Variable pour stocker le dernier temps
const unsigned long interval_print = 10000;
bool moteurEnMarcheForce = false;

//pour le wathdog exterieur
unsigned long previousMillisWD = 0; 
const long intervalWD = 200;  //clignotement 500 >1.6hz
bool watchdogState = true;

 unsigned long startTime = 0;
 bool outputSet = false;

//configuration boite noire
int compteurNiveauHaut = 0;
int compteurContacteur = 0;
int compteurRelaisSecurite = 0;
int compteurMoteurEnMarche = 0;
int compteurCapteurBloque = 0;
int compteurMarcheForceOubliee = 0 ;

//variable pour enregister uniquement le changement d'etat
bool precedentNiveauHaut = false;
bool precedentContacteur = false;
bool precedentRelaisSecurite = false;
bool precedentMoteurEnMarche = false;
bool precedentCapteurBloque = false;

void initialiserVariables() {
    securiteDeclenche = 0;
    continuerProgramme = true;
}
