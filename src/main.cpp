
#include "config.h"
#include "pompe.h"
#include "capteurs.h"
#include "securite.h"
//#include <Arduino.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include "web_server.h"
//#define TEST   // laisser pour les test sur planche a pain
#undef TEST  // a activer pour la  production

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
// initialisation serveur AP
String contenuTableau;
DNSServer dnsServer;
AsyncWebServer server(80);  // Déclaration du serveur sur le port 80

char string[16];
uint32_t f;

//instance pour sauvegarder les données
Preferences prefsPompe;
Preferences prefsnewTime1;  // pour stocker le temps de focntionnement de la pompe
unsigned long derDemarRecupere = 0;
// Prototype de la fonction chargerTableau()
void chargerTableau();
void chargerCompteurs(); 
void lireCompteurs(); 
void sauvegarderTableau(); 
void enregistrerCompteurs(int niveauHaut, int etatContacteur, int etatRelaisSecurite, int moteurEnMarche, int capteurBloque, int compteurMarcheForceOubliee);


void setup() {


  Serial.begin(115200);
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password, 9);  // Utilisation du canal 9
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  //reglage de la frequence
  //setCpuFrequencyMhz(80);

  //resistance de tirage pour les entrée :
  pinMode(CAPTEUR_NIVEAU_HAUT, INPUT); //idem ci dessous
  pinMode(RELAIS_SECURITE, INPUT); //pulldown interne trop forte detection intempestive -> 2.2k externe 
  //pinMode(CAPTEUR_CONTACTEUR, INPUT_PULLDOWN);
  pinMode(MOTEUR_POMPE, OUTPUT);
  pinMode(TENSION, INPUT_PULLDOWN);

  pinMode(WATCHDOG_EXT, OUTPUT);  // Configure la broche en sortie
  digitalWrite(MOTEUR_POMPE, HIGH);
  pinMode(MOSFET_PIN, OUTPUT); // sortie pour commandé le bs 170
  digitalWrite(MOSFET_PIN, LOW);  // Initialisation à LOW ls bs170 
  startTime = millis();  // On mémorise le moment du démarrage pour la tempo du bs170

  // Configure le DNS local avec le serveur AP
  //**** affiche le tableau des dernier demarrages
  // Configure le DNS local avec le serveur AP
  dnsServer.start(53, apHostname, WiFi.softAPIP());

  //pour enregistrer les donnés
  prefsPompe.begin("pompe", false);  // Nom de namespace "pompe"
  // initialisation des preference pour le temps de fonctionnement
  prefsnewTime1.begin("newTime1", false);

  //configuration watchdog
  esp_task_wdt_init(WDT_TIMEOUT, true);  //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL);                //add current thread to WDT watch

  //initilaisation de LittleFS pour les log
  if (!LittleFS.begin(true)) {
    Serial.println("Erreur lors de l'initialisation de LittleFS");
    return ;
  }
  Serial.println("LittleFS initialisé avec succès");


  chargerTableau();  //charger les données sauvegardé au demarrage
  Serial.print("⬅️ Chargement de indice_tableau après reboot : ");
  Serial.println(indiceDerniereEntreeTempsDepuis);


    
  // Initialisation des variables


  initialiserVariables();
  initialiserLed();

  // lancement du serveur web
  initWebServer();
  server.begin();
  //lancement de la sauvegarde et chargmeent des valeurs
  chargerCompteurs();
  lireCompteurs();
  // Vérifier si le fichier existe, sinon le créer surtout pour un esp32 neuf
    if (!LittleFS.exists("/compteurs.txt")) {
        Serial.println("Fichier compteurs.txt absent, création en cours...");
        File file = LittleFS.open("/compteurs.txt", FILE_WRITE);
        if (file) {
            file.close();
            Serial.println("Fichier compteurs.txt créé avec succès.");
        } else {
            Serial.println("Erreur lors de la création du fichier compteurs.txt !");
        }
    } 
}
void loop() {

  // watchdog exterieur creer un clignotment <1.6hz
 unsigned long currentMillisWD = millis();
 
   if (currentMillisWD - previousMillisWD >= intervalWD) {
    previousMillisWD = currentMillisWD;
    watchdogState = !watchdogState;  // Inverse l'état
    digitalWrite(WATCHDOG_EXT, watchdogState);
  }

  

  yield(); 
  //partie qui coupe la marche forcée au bout de 5 min finalement pas de modif sur la variable

  if (moteurEnMarcheForce && (millis() - tempsDebutMarcheForce >= TEMPO_ARRET_10MN)) {
    desactiverPompe(); // Éteint la pompe
    moteurEnMarcheForce = false;
    compteurMarcheForceOubliee++; 
    tempsDebutMarcheForce = 0;
    Serial.println("Arrêt automatique après 5 minutes de marche forcée");
    
    enregistrerCompteurs(
        compteurNiveauHaut, 
        compteurContacteur, 
        compteurRelaisSecurite, 
        compteurMoteurEnMarche, 
        compteurCapteurBloque,
        compteurMarcheForceOubliee  // <-- Nouvelle valeur
    );
      yield(); //attente pour soulager l'esp32
}
  //pour permettre au serveur DNS de traiter les requêtes entrantes de manière continue.
  dnsServer.processNextRequest();  
  //reveil du watchdog
  esp_task_wdt_reset();
  unsigned long currentMillisled = millis();
  //recuperation de la valeur sauvegardé du temps de fonctionnement moteur, par defaut 10s
  TEMP_FONCTIONNEMENT_MOTEUR = prefsnewTime1.getULong("timeValue", 10) * 1000;
  capteurs = lireCapteurs();  // Rafraîchir les valeurs avant le test

  //gestion du clignotement de la led sans utiliser delay
  unsigned long intervalActuel = (capteurBloque || relaisDeclenche) ? intervalled / 6 : intervalled;

  if (moteurEnMarche && !capteurBloque || moteurEnMarche && !relaisDeclenche) {
    digitalWrite(Led, HIGH);  // Allumer la LED en continu quand le moteur est en marche et le capteur n'est pas bloqué
   
    } else {
    if (currentMillisled - previousMillisled >= intervalActuel) {
      previousMillisled = currentMillisled;  // Mise à jour du dernier changement
      ledState = !ledState;                  // Inversion de l'état de la LED
      digitalWrite(Led, ledState);           // Mise à jour de la LED
      yield(); //attente pour soulager l'esp32
      }
  }

  // affichage de debug sans utiliser de delay
  if (currentMillisled - previousMillis_print >= interval_print) {
    previousMillis_print = currentMillisled;  // Mise à jour du dernier temps

    // Affichage des informations
    Serial.print("void loop État du capteur de niveau haut : ");
    Serial.println(capteurs.niveauHaut);
    Serial.print("void loop État contacteur : ");
    Serial.println(capteurs.etatContacteur);
    Serial.print("void loop État du relais sécurité : ");
    Serial.println(capteurs.etatRelaisSecurite);
    Serial.print("void loop moteur en marche : ");
    Serial.println(moteurEnMarche);
    Serial.print("etat si le capteur a été bloqué ");
    Serial.println(capteurBloque);
    Serial.print("etat marche forcé moteur 🔧 ⚡");
    Serial.println(moteurEnMarcheForce);
    Serial.print("etat presence tension ");
    Serial.println(capteurs.tension);
    f = getCpuFrequencyMhz();
  sprintf(string, "CPU Freq: %iHz", f);
  Serial.println(string);
    lireCompteurs(); //affiche le contenu du fichier de log
    
    }
    //bouton présence tension sur la page web
tensionPresente = capteurs.tension; // true = tension présente (HIGH), false = absente (LOW)
etatTension = tensionPresente ? "🔌 🔌 Présence tension 🔌 🔌" : "🚧 🚧 🚧 Pas d'energie 🚧 🚧 🚧";
couleurTension = tensionPresente ? "#006400" : "#f44336";

  // Démarrage du moteur de la pompe😍
  if (peutDemarrerPompe(capteurs) && !moteurEnMarche && !capteurBloque) {
    activerPompe();
    moteurEnMarche = true;
    tempsDebut = capteurs.tempsActuel;
    tempsRelaisDeclenche = 0;  // Réinitialise le temps de déclenchement du relais de sécurité
    tempsReel = 0;             // Mise à zéro du temps de fonctionnement réel
    tempsNiveauHautActif = 0;  // Réinitialise le temps où niveauHaut est actif
    Serial.println("Démarrage du moteur.");
    
  } else {
    if (moteurEnMarche) {  // verification si le capteur niveau haut ne reste pas bloqué
      if (capteurs.niveauHaut == HIGH) {
        if (debutNiveauHaut == 0) {
          debutNiveauHaut = millis();  // Premier passage en HIGH
        } else if (millis() - debutNiveauHaut >= SEUIL_BLOCAGE) {
          capteurBloque = true;
          Serial.println("⚠️ Capteur niveauHaut bloqué détecté !");
          debutNiveauHaut = 0;  // Reset pour éviter répétition du message
        }
        
      } else {
        debutNiveauHaut = 0;  // Réinitialiser si le capteur repasse à LOW
      }

      // Gestion sécurité lorsque le moteur fonctionne 😖😖
      if (capteurs.etatRelaisSecurite == HIGH) {
        desactiverPompe();
        moteurEnMarche = false;
        relaisDeclenche = true;
        tempsRelaisDeclenche = capteurs.tempsActuel;  // Enregistre le moment du déclenchement
        // Incrémenter le compteur de sécurité
        securiteDeclenche++;  //incremente le nombre de fois ou cela augmente
        Serial.println("Relais de sécurité déclenché.");
        yield(); //attente pour soulager l'esp32
        }
      // Arrêt automatique après un certain temps💓💓
      else if (capteurs.tempsActuel - tempsDebut >= TEMP_FONCTIONNEMENT_MOTEUR) {
        desactiverPompe();
        moteurEnMarche = false;
        Serial.println("Arrêt automatique du moteur après un certain temps");

        // Calcul du temps écoulé depuis le dernier démarrage
        unsigned long tempsEcoule_1 = capteurs.tempsActuel - derDemar;
        uint32_t jours = tempsEcoule_1 / JOUR;
        uint32_t heures = (tempsEcoule_1 % JOUR) / HEURE;
        uint32_t minutes = ((tempsEcoule_1 % JOUR) % HEURE) / MINUTE;

        // Affichage du temps écoulé
        Serial.print("Dernier démarrage il y a ");
        Serial.print(jours);
        Serial.print("j ");
        Serial.print(heures);
        Serial.print(":");
        if (minutes < 10) {
          Serial.print("0");
        
          yield(); //attente pour soulager l'esp32
          }
        Serial.println(minutes);

        // Sauvegarde des données📥📥
        if (tempsDebut != 0) {
          tempsDepuisderDemar[indiceDerniereEntreeTempsDepuis] = tempsEcoule_1;
          indiceDerniereEntreeTempsDepuis = (indiceDerniereEntreeTempsDepuis + 1) % MAX_ENTRIES;
          Serial.print("Indice tableau temps depuis dernier démarrage : ");
          Serial.println(indiceDerniereEntreeTempsDepuis);
        }

        // Réinitialisation des variables
        derDemar = capteurs.tempsActuel;  // Met à jour le temps du dernier démarrage
        prefsPompe.putULong("derDemar", derDemar);  // Sauvegarde le dernier démarrage dans prefs
        sauvegarderTableau();

        // Réinitialiser les compteurs liés au capteur
        tempsNiveauHautActif = 0;
      }
    }
  }


  // Mettre à jour les compteurs uniquement si l'état change pour sauvegarder dans les log
  bool modification = false;

  if (capteurs.niveauHaut && !precedentNiveauHaut) {
    compteurNiveauHaut++;
    modification = true;
  yield();
  }
  if (capteurs.etatContacteur && !precedentContacteur) {
    compteurContacteur++;
    modification = true;
    yield();
  }
  if (capteurs.etatRelaisSecurite && !precedentRelaisSecurite) {
    compteurRelaisSecurite++;
    modification = true;
    yield();
  }
  if (moteurEnMarche && !precedentMoteurEnMarche) {
    compteurMoteurEnMarche++;
    modification = true;
    yield();
  }
  if (capteurBloque && !precedentCapteurBloque) {
    compteurCapteurBloque++;
    modification = true;
    yield();
  }

  precedentNiveauHaut = capteurs.niveauHaut;
  precedentContacteur = capteurs.etatContacteur;
  precedentRelaisSecurite = capteurs.etatRelaisSecurite;
  precedentMoteurEnMarche = moteurEnMarche;
  precedentCapteurBloque = capteurBloque;

  // Sauvegarde si modification ou toutes les 60 sec
  static unsigned long dernierEnregistrement = 0;
  if (modification) {
    enregistrerCompteurs(compteurNiveauHaut, compteurContacteur, compteurRelaisSecurite, compteurMoteurEnMarche, compteurCapteurBloque, compteurMarcheForceOubliee);
    dernierEnregistrement = millis();
  yield();
  }

  // Si plus de 10s se sont écoulées ET on n'a pas encore activé la sortie pour alumé le mosfet
  //apparement pas utile le WD tolere le redemarrage de l'esp32 ca redemarre au bout de 3 secondes
  if (!outputSet && (millis() - startTime >= DELAY_MS)) {
    digitalWrite(MOSFET_PIN, HIGH);  // On active le MOSFET
    outputSet = true;                // Marque comme activé
  }

}

void sauvegarderDonnees(const char* key, const uint32_t value) {
  prefsPompe.putULong(key, value);
}

uint32_t chargerDonnees(const char* key, uint32_t defaultValue = 0) {
  return prefsPompe.getULong(key, defaultValue);
}

void sauvegarderTableau() {
  for (int i = 0; i < MAX_ENTRIES; i++) {
    sauvegarderDonnees(("temps_" + String(i)).c_str(), tempsDepuisderDemar[i]);
  }
  sauvegarderDonnees("indice_tableau", indiceDerniereEntreeTempsDepuis);
  sauvegarderDonnees("derDemar", derDemar);

  Serial.println("Données du tableau sauvegardées avec succès.");
yield();
}

void chargerTableau() {
  for (int i = 0; i < MAX_ENTRIES; i++) {
    tempsDepuisderDemar[i] = chargerDonnees(("temps_" + String(i)).c_str());
  }
  indiceDerniereEntreeTempsDepuis = chargerDonnees("indice_tableau");
  derDemar = chargerDonnees("derDemar");
  Serial.println("Données du tableau chargées avec succès.");
  yield();
}

//ecrire l'ensemble du fonctionnement de l'esp32 dans les log
void enregistrerCompteurs(int niveauHaut, int etatContacteur, int etatRelaisSecurite, int moteurEnMarche, int capteurBloque,  int marcheForceOubliee) {
  // Ouvrir le fichier en mode écriture (écrase le fichier existant)
  File file = LittleFS.open("/compteurs.txt", FILE_WRITE);
  if (!file) {
    Serial.println("Erreur lors de l'ouverture du fichier compteurs pour ecrire");
    return ;
  }

  // Écrire les compteurs dans le fichier
  file.printf("Niveau haut: %d\nContacteur: %d\nRelais sécurité: %d\nMoteur en marche: %d\nCapteur bloqué: %d\nMarches forcées oubliées: %d\n",
              niveauHaut, etatContacteur, etatRelaisSecurite, moteurEnMarche, capteurBloque,marcheForceOubliee);

  // Fermer le fichier
  file.close();
  Serial.println("Compteurs enregistrés");
  yield();
}

//lire le fichier de log dans la console serie
void lireCompteurs() {
  // Ouvrir le fichier en mode lecture
  File file = LittleFS.open("/compteurs.txt", FILE_READ);
  if (!file) {
    Serial.println("Erreur lors de l'ouverture du fichier compteurs pour lire");
    

      Serial.println("\n=== CONTENU DE compteurs.txt ===");
  while (file.available()) {
    String ligne = file.readStringUntil('\n'); // Lit ligne par ligne
    Serial.println(ligne); // Affiche la ligne complète
    yield(); // Pause courte pour l'ESP32
  }
  Serial.println("============================\n");


    return;
    yield();
  } 
  // Lire et afficher le contenu du fichier
  Serial.println("Compteurs :");
  while (file.available()) {
    Serial.write(file.read());
    yield();
  }

  // Fermer le fichier
  file.close();
}

void chargerCompteurs() {
    File file = LittleFS.open("/compteurs.txt", FILE_READ);
    if (!file) {
        Serial.println("⚠ Aucun fichier compteurs trouvé, initialisation à zéro.");
        return;
        yield();
    }

    // Lire les valeurs depuis le fichier
    int niveauHaut, etatContacteur, etatRelaisSecurite, moteurEnMarche, capteurBloque;
    if (sscanf(file.readString().c_str(), 
               "Niveau haut: %d\nContacteur: %d\nRelais sécurité: %d\nMoteur en marche: %d\nCapteur bloqué: %d\n",
               &niveauHaut, &etatContacteur, &etatRelaisSecurite, &moteurEnMarche, &capteurBloque) == 5) {
        // Charger les valeurs lues dans les compteurs
        compteurNiveauHaut = niveauHaut;
        compteurContacteur = etatContacteur;
        compteurRelaisSecurite = etatRelaisSecurite;
        compteurMoteurEnMarche = moteurEnMarche;
        compteurCapteurBloque = capteurBloque;
        Serial.println("✅ Compteurs chargés depuis LittleFS !");
    yield();
    } else {
        Serial.println("❌ Erreur de lecture des compteurs, valeurs non chargées.");
    }

    file.close();
}

