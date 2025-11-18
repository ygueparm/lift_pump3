#include "capteurs.h"
#include <Arduino.h>  // Nécessaire pour `digitalRead` et `millis`
#include "config.h"



DonneesCapteurs capteurs; 


DonneesCapteurs lireCapteurs() {
    DonneesCapteurs data;
    data.etatContacteur = digitalRead(CAPTEUR_CONTACTEUR);
    data.niveauHaut = digitalRead(CAPTEUR_NIVEAU_HAUT);
    data.etatRelaisSecurite = digitalRead(RELAIS_SECURITE);
    data.tempsActuel = millis();
    data.tension = digitalRead(TENSION);
    return data;
}
