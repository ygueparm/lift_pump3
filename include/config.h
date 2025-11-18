#ifndef CONFIG_H
#define CONFIG_H

// Broches
#define MOTEUR_POMPE         22 
#define CAPTEUR_NIVEAU_HAUT  23
#define RELAIS_SECURITE      21 
#define CAPTEUR_CONTACTEUR   4
#define WATCHDOG_EXT         25
#define MOSFET_PIN 14   // Broche connectée à la gate du BS170 pas utilisé ici
#define TENSION 15  //broche presence tension avec relais finder

// watchdog en seconde : 
#define WDT_TIMEOUT 5
#define DELAI_ATTENTE_REDEMARRAGE 30000

#define DELAY_MS 10000   // 10 secondes

extern int Led;  // Déclaration externe de la variable Led

// Constantes de temps
const unsigned long MILLISECONDE = 1;
const unsigned long SECONDE = 1000 * MILLISECONDE;
const unsigned long MINUTE = 60 * SECONDE;
const unsigned long HEURE = 60 * MINUTE;
const unsigned long JOUR = 24 * HEURE;

//gestion led clignotante
extern const long intervalled;
extern unsigned long previousMillisled;
extern bool ledState;

// Configuration
const unsigned long TEMP_FONCTIONNEMENT_MOTEUR_CONFIG = 0;
const unsigned long TEMP_ATTENTE_SECURITE = 10000UL;
const int MAX_SECURITE = 6;
// unsigned long MAX_ENTRIES;
// extern const int MAX_STRING_LENGTH ;

//constante pour aficher les message de debug gentiment
extern unsigned long previousMillis_print ; // Variable pour stocker le dernier temps
extern const unsigned long interval_print ; // Intervalle de temps en millisecondes (3 secondes)
extern unsigned long tempsDebutMarcheForce ;
extern const unsigned long TEMPO_ARRET_10MN ;

// Options
#define SIMULATEUR_CONTACTEUR 1
#define SCRIPT 1

// Réseau
extern const char* ssid;
extern const char* password;
extern const char* apHostname; // Le nom d'hôte que vous souhaitez utiliser

//bouton presence tension 
extern bool tensionPresente;
extern const char* etatTension;
extern const char* couleurTension; 

// Variables d'état
extern bool capteurBloque;
extern bool relaisDeclenche;
extern unsigned long tempsDebut;
extern unsigned long tempsRelaisDeclenche;
extern bool moteurEnMarche;
extern bool moteurEnMarcheForce;
extern unsigned long tempsrelai;
extern unsigned long tempsTotal;
extern unsigned long jours, heures, minutes, secondes;
extern unsigned long derDemar;
extern unsigned long TEMP_FONCTIONNEMENT_MOTEUR;

// Tableaux de stockage

extern unsigned long tempsDepuisderDemar[];
extern unsigned long tempsReelDeFonctionnement[];
extern const int MAX_ENTRIES;

extern int indiceDerniereEntreeTempsDepuis;
extern int indiceDerniereEntreeTempsReel;
extern int nombreEntries;
extern int indiceDerniereEntree;
extern unsigned long debutNiveauHaut;
extern const unsigned long SEUIL_BLOCAGE;  // 10 secondes

extern int compteurMarcheForceOubliee ; //ajout log de marche forcé

//config watchdog

extern bool watchdogState ;
extern unsigned long previousMillisWD ;
extern const long intervalWD ;

extern unsigned long startTime ;
extern bool outputSet ;



// Calcul du temps réel de fonctionnement avec retour du contacteur
extern volatile unsigned long tempsDebutReel;
extern volatile unsigned long tempsFinReel;
extern volatile bool contacteurDeclenche;
extern int simulateurContacteur; // Simulation retour contacteur
extern unsigned long tempsReel; // Variable du temps réel de fonctionnement
extern bool miseAJourEffectuee; // Indicateur de mise à jour tableau
extern bool initialisationVoyant;

extern bool moteurEnMarche;
extern int securiteDeclenche;
extern bool continuerProgramme;

//boite noire log
extern int compteurNiveauHaut;
extern int compteurContacteur;
extern int compteurRelaisSecurite;
extern int compteurMoteurEnMarche;
extern int compteurCapteurBloque;
extern bool precedentNiveauHaut;
extern bool precedentContacteur;
extern bool precedentRelaisSecurite;
extern bool precedentMoteurEnMarche;
extern bool precedentCapteurBloque;

void initialiserVariables();


#endif
