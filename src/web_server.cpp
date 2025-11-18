#include "web_server.h"
#include <ESPAsyncWebServer.h>
#include "config.h"
#include <Preferences.h>
#include "capteurs.h"
#include <LittleFS.h>
extern Preferences prefsPompe;
extern Preferences prefsnewTime1;

extern AsyncWebServer server;


void initWebServer() {
  Serial.print("capteurBloque: ");
  Serial.println(capteurBloque);
  Serial.print("relaisDeclenche: ");
  Serial.println(relaisDeclenche);
  Serial.print("securiteDeclenche: ");
  Serial.println(securiteDeclenche);
  Serial.print("MAX_SECURITE: ");
  Serial.println(MAX_SECURITE);

  server.on("/", [](AsyncWebServerRequest* request) {
    if (capteurBloque == true) {
      Serial.println("🚨 Condition capteurBloque OK");
      Serial.print("🔍 Valeur brute : ");
      Serial.println((int)capteurBloque);
      char html[3500];
      snprintf(html, sizeof(html),
               "<!DOCTYPE html><html><head>"
               "<meta charset='UTF-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
               "<title>Alerte - Problème détecté</title>"
               "<style>"
               "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
               "h1 { color: #d9534f; }"
               "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
               ".btn-retour { background-color: #0070ff; }"
               ".btn-reset { background-color: #FF8C00; }"  // Bouton pour réinitialiser relaisDeclenche
               "</style>"
               "</head><body>"
               "<h1>⚠️ Alerte - capteur niveau haut</h1>"
               "<p>Un problème est survenu. la capteur niveau haut semble bloqué en position haute, il faut le nettoyer pour le debloquer</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='resetCapteur()'>Réinitialiser le capteur de niveau haut</button>"
               "<script>"
               "function resetCapteur() {"
               "    document.body.style.cursor = 'wait';"
               "    fetch('/reset-capteur', { method: 'GET' })"
               "        .then(response => {"
               "            if (!response.ok) {"
               "                throw new Error('Erreur lors de la réinitialisation du capteur.');"
               "            }"
               "            return response.text();"
               "        })"
               "        .then(data => {"
               "            alert(data);"
               "            setTimeout(() => {"
               "                window.location.href = '/';"
               "                document.body.style.cursor = 'default';"
               "            }, 2000);"
               "        })"
               "        .catch(error => {"
               "            console.error('Erreur réseau :', error);"
               "            alert('Une erreur est survenue lors de la réinitialisation du capteur.');"
               "            document.body.style.cursor = 'default';"
               "        });"
               "}"
               "</script>"
               "</body></html>");
      Serial.println("🚀 Envoi de la page web");
      request->send(200, "text/html", html);
    
    yield(); 
    } else if (relaisDeclenche) {
      Serial.println("⚡ Condition relaisDeclenche OK");
      char html[2000];
      snprintf(html, sizeof(html),
               "<!DOCTYPE html><html><head>"
               "<meta charset='UTF-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
               "<title>Alerte - Problème détecté</title>"
               "<style>"
               "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
               "h1 { color: #d9534f; }"
               "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
               ".btn-retour { background-color: #0070ff; }"
               ".btn-reset { background-color: #FF8C00; }"  // Bouton pour réinitialiser relaisDeclenche
               "</style>"
               "</head><body>"
               "<h1>⚠️ Alerte - le rotor est bloqué</h1>"
               "<p>Un problème est survenu. Le relais de controle de courant à detecté une anomalie veuillez verifer la pompe. En particulier si il n'y a pas un corps etranger qui bloque le rotor, temps d'attente environ 2 minute avant le prochain demarrage</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='resetRelais()'>Réinitialiser le relais de securité</button>"  // Nouveau bouton
               "<script>"
               "function resetRelais() {"
               "    fetch('/reset-relais')"  // Envoyer une requête pour réinitialiser relaisDeclenche
               "        .then(response => response.text())"
               "        .then(data => {"
               "            alert(data);"                 // Afficher un message de confirmation
               "            window.location.href = '/';"  // Recharger la page
               "        });"
               "}"
               "</script>"
               "</body></html>");
      request->send(200, "text/html", html);
    yield(); 
    }
    else if (securiteDeclenche >= MAX_SECURITE) {
      Serial.println("🔒 Condition securiteDeclenche OK");
      char html[3500];
      snprintf(html, sizeof(html),
               "<!DOCTYPE html><html><head>"
               "<meta charset='UTF-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
               "<title>Alerte - Problème détecté</title>"
               "<style>"
               "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
               "h1 { color: #d9534f; }"
               "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
               ".btn-retour { background-color: #0070ff; }"
               ".btn-reset { background-color: #FF8C00; }"  // Bouton pour réinitialiser relaisDeclenche
               "</style>"
               "</head><body>"
               "<h1>⚠️ Alerte - Trop de sur/sous courant détecté</h1>"
               "<p>Un problème est survenu. le relais de surveillance de courant c'est déclenché trop souvent😫, la pompe est peut etre bloqué😢 😭</p>"
               "<button class='btn-retour' onclick='window.location.href=\"/\"'>Retour à la page principale</button>"
               "<button class='btn-reset' onclick='resetSecurite()'>Réinitialiser la securité</button>"
               "<script>"
               "function resetSecurite()  {"
               "    fetch('/reset-securite')"  // Envoyer une requête pour réinitialiser relaisDeclenche
               "        .then(response => response.text())"
               "        .then(data => {"
               "            alert(data);"                 // Afficher un message de confirmation
               "            window.location.href = '/';"  // Recharger la page
               "        });"
               "}"
               "</script>"
               "</body></html>");
      request->send(200, "text/html", html);
    yield(); 
    } else {
      Serial.println("✅");
      char html[5000];  // Ajustez la taille selon vos besoins
      snprintf(html, sizeof(html),
               "<!DOCTYPE html><html><head>"
               "<meta charset='UTF-8'>"
               "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
               "<title>Contrôle de la pompe</title>"
               "<style>"
               "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; padding: 20px; }"
               "button { width: 100%%; padding: 15px; font-size: 18px; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 0; }"
               ".btn-tension { background-color: %s; }"
               ".btn-allumer { background-color: #4CAF50; }"
               ".btn-eteindre { background-color: #f44336; }"
               ".btn-modifier  { background-color: #0070ff; }"
               ".btn-temps { background-color: #0070ff; }"
               ".btn-tableau { background-color: #006400; }"  // Vert foncé pour le nouveau bouton
               ".btn-fermer { background-color: #7a7a7a; }"   // Gris foncé pour le bouton "Fermer"
               ".btn-reset { background-color: #006400; }"    //Vert foncé
               "#modalTableau { display: none; position: fixed; top: 50%%; left: 50%%; transform: translate(-50%%, -50%%); background: white; padding: 20px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); z-index: 1000; }"
               "</style>"
               "</head><body>"
               
               "<h2> ⚙️ Gestion de la pompe relevage </h2>"
               "<button class='btn-tension'> %s </button>"
               "<button class='btn-temps' onclick='getTempsFonctionnement()'>⏱️ durée pour vider la cuve ⏱️</button>"
               "<p id='tempsFonctionnement'></p>"
               "<button class='btn-modifier' onclick='modifierTemps()'>✍️ Modification du temps de vidange ✍️</button>"
               "<p id='messageModifier'></p>"
               "<button class='btn-tableau' onclick='afficherTableauFonctionnement()'>📊 Délai entre marche moteur 📊</button>"  // Nouveau bouton
               "<button class='btn-reset' onclick='resetTableau()'>🆑 Mise à 0 du tableau 🆑 </button>"                                             // Bouton "Mise à 0" sur la page principale
               "<div id='modalTableau'>"                                                                                          // Modal pour afficher le tableau
               "<h2>Delai entre marche pompe</h2>"
               "<pre id='tableauData'></pre>"
               "<button class='btn-fermer' onclick='document.getElementById(\"modalTableau\").style.display = \"none\";'>Fermer</button>"
               "</div>"
               
               "<h1>⚠️Contrôle Manuel de la pompe⚠️ aucune protection moteur</h1>"
               "<button class='btn-eteindre' onclick='toggleMoteur(\"eteindre\")'>Éteindre le moteur</button>"
               "<button class='btn-allumer' onclick='toggleMoteur(\"allumer\")'>Allumer le moteur</button>"
               "<script>"
               "function toggleMoteur(action) { fetch('/' + action); location.reload(); }"
               "function getTempsFonctionnement() {"
               "    fetch('/temps')"
               "        .then(response => response.text())"
               "        .then(data => {"
               "            document.getElementById('tempsFonctionnement').innerText = 'Durée de fonctionnement : ' + data;"
               "        });"
               "}"
               "function resetTemps() {"
               "    fetch('/reset');"
               "    location.reload();"
               "}"
               "function modifierTemps() {"
               "    let newTime = prompt('Entrez la nouvelle durée de la pompe pour vider la cuve (en secondes) :');"
               "    if (newTime && !isNaN(newTime)) {"
               "        fetch('/set-time?new-time=' + encodeURIComponent(newTime))"
               "            .then(response => response.text())"
               "            .then(data => {"
               "                document.getElementById('messageModifier').innerText = data;"
               "                location.reload();"
               "            });"
               "    } else {"
               "        alert('Veuillez entrer un nombre valide.');"
               "    }"
               "}"
               "function afficherTableauFonctionnement() {"  // Fonction pour afficher le tableau
               "    fetch('/tableau-fonctionnement')"
               "        .then(response => response.text())"
               "        .then(html => {"
               "             document.getElementById('tableauData').innerHTML = html; "
               "            document.getElementById('modalTableau').style.display = 'block';"
               "        });"
               "}"
               "function resetTableau() {"
               "    if (confirm('Êtes-vous sûr de vouloir réinitialiser le tableau ?')) {"
               "        fetch('/reset-tableau')"
               "            .then(response => response.text())"
               "            .then(data => {"
               "                alert(data);"
               "                location.reload();"
               "            })"
               "            .catch(error => {"
               "                console.error('Erreur lors de la réinitialisation du tableau :', error);"
               "                alert('Une erreur est survenue lors de la réinitialisation du tableau.');"
               "            });"
               "    }"
               "}"
               "</script>"
               "</body></html>",
        couleurTension,  // Premier %s (couleur)
        etatTension  
       );    
      request->send(200, "text/html; charset=utf-8", html);
   yield(); 
    }
  });  //cloture de la focntion

  // marche forcé moteur
  server.on("/allumer", [](AsyncWebServerRequest* request) {
    digitalWrite(MOTEUR_POMPE, LOW);  // Allume la pompe
    moteurEnMarcheForce = true ;
    tempsDebutMarcheForce = millis(); // Enregistre le moment du démarrage
    request->send(200, "text/plain; charset=utf-8", "Moteur allumé");
  });

  //arret forcé moteur
  server.on("/eteindre", [](AsyncWebServerRequest* request) {
    digitalWrite(MOTEUR_POMPE, HIGH);  // Éteint la pompe
    moteurEnMarcheForce = false;
    request->send(200, "text/plain; charset=utf-8", "Moteur éteint");
  });

  //affichage du temps de focntionnement
  server.on("/temps", [](AsyncWebServerRequest* request) {
    float tempsSecondes = TEMP_FONCTIONNEMENT_MOTEUR / 1000.0;
    String tempsFonctionnement = String(tempsSecondes, 1) + " secondes";
    request->send(200, "text/plain; charset=utf-8", tempsFonctionnement.c_str());
  });

  //mise a jour du temps de fonctionnement
  server.on("/set-time", HTTP_GET, [](AsyncWebServerRequest* request) {
    String newTimeStr = request->arg("new-time");  // Récupère le paramètre "new-time"
    if (newTimeStr.length() > 0) {
      unsigned long newTime = newTimeStr.toInt();      // Convertit en entier
      if (newTime >= 0) {                              // Vérifie que la valeur est positive
        prefsnewTime1.putULong("timeValue", newTime);  // Sauvegarde dans les préférences
        request->send(200, "text/plain; charset=utf-8", "Temps mis à jour avec succès !");
      } else {
        request->send(400, "text/plain; charset=utf-8", "Valeur incorrecte pour le temps.");
      }
    } else {
      request->send(400, "text/plain; charset=utf-8", "Paramètre 'new-time' manquant.");
    }
  });

    // ------------------------------------------------------------------
  //  /tableau-fonctionnement  –  réponse HTML complète
  // ------------------------------------------------------------------
  server.on("/tableau-fonctionnement", HTTP_GET, [](AsyncWebServerRequest *request) {

    /* ----- récupération des données (votre code inchangé) ---------- */
    int indiceRecent = indiceDerniereEntreeTempsDepuis;
    int dernierEntree = (indiceRecent > 0) ? (indiceRecent - 1) : (MAX_ENTRIES - 1);

    char dernierKey[15];
    snprintf(dernierKey, sizeof(dernierKey), "temps_%d", dernierEntree);
    unsigned long derniereValeur = tempsDepuisderDemar[dernierEntree];

    unsigned long jours_dernier   = derniereValeur / JOUR;
    unsigned long heures_dernier  = (derniereValeur % JOUR) / HEURE;
    unsigned long minutes_dernier = ((derniereValeur % JOUR) % HEURE) / MINUTE;
    String derniereValeurFormattee = String(jours_dernier)   + " j " +
                                     String(heures_dernier)  + " h " +
                                     String(minutes_dernier) + " min";

    unsigned long delaiDernier = millis() - tempsDebut;
    unsigned long jourdelaiDernier        = delaiDernier / JOUR;
    unsigned long heuresdelaiDernier      = (delaiDernier % JOUR) / HEURE;
    unsigned long minutesdelaiDernier     = ((delaiDernier % JOUR) % HEURE) / MINUTE;
    String delaiFormate = String(jourdelaiDernier)   + " j " +
                          String(heuresdelaiDernier) + " h " +
                          String(minutesdelaiDernier)+ " min";

    /* ----- construction de la page HTML ----------------------------- */
     String html = "<!DOCTYPE html><html lang='fr'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Tableau de fonctionnement</title><style>body{font-family:Arial,sans-serif;margin:0;padding:1em;background:#f5f5f5;color:#333;}.container{max-width:100%;}h2{color:#0066cc;text-align:center;}table{width:100%;border-collapse:collapse;background:white;}th,td{padding:8px;border:1px solid #ddd;}th{background:#e8e8e8;}th:nth-child(1),td:nth-child(1){width:20%;text-align:center;}th:nth-child(2),td:nth-child(2){width:80%;}@media(max-width:480px){body{padding:0.5em;}table{font-size:12px;}th,td{padding:6px;}}</style></head><body><div class='container'><h2>Tableau de fonctionnement</h2><p>Dernière entrée: <strong>" + String(dernierEntree) + "</strong></p><p>Durée: <strong>" + delaiFormate + "</strong></p><table><thead><tr><th>Indice</th><th>Temps écoulé</th></tr></thead><tbody>";
     
    /* ----- lignes du tableau --------------------------------------- */
    for (int i = 0; i < MAX_ENTRIES; i++) {
      char key[15];
      snprintf(key, sizeof(key), "temps_%d", i);
      unsigned long temps_tab = tempsDepuisderDemar[i];
      unsigned long jours_tab   = temps_tab / JOUR;
      unsigned long heures_tab  = (temps_tab % JOUR) / HEURE;
      unsigned long minutes_tab = ((temps_tab % JOUR) % HEURE) / MINUTE;
      String tempsFormatte = String(jours_tab)   + " j " +
                             String(heures_tab)  + " h " +
                             String(minutes_tab) + " min";

      html += "<tr><td>" + String(i) + "</td><td>" + tempsFormatte + "</td></tr>\n";
    }

    html +=
        "</tbody>"
        "</table>"
        "</body>"
        "</html>";

    /* ----- envoi au client ----------------------------------------- */
    request->send(200, "text/html", html);
  });

  //reinitialiser la memoire du tableau
  server.on("/reset-tableau", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Effacer toutes les préférences liées au tableau
    prefsPompe.begin("pompe", false);  // Ouvrir le namespace "pompe"
    prefsPompe.clear();                // Effacer toutes les clés dans ce namespace
    prefsPompe.end();                  // Fermer le namespace

    // Réinitialiser manuellement l'indice du tableau
    prefsPompe.putInt("indice_tableau", 0);

    request->send(200, "text/plain", "Tableau réinitialisé avec succès !");
  });

  
  // Route pour réinitialiser relais securité declenche
  server.on("/reset-relais", HTTP_GET, [](AsyncWebServerRequest* request) {
    relaisDeclenche = false;  // Réinitialiser relaisDeclenche
    request->send(200, "text/plain", "Relais réinitialisé avec succès !");
  });

  // Route pour réinitialiser capteur bloque
  server.on("/reset-capteur", HTTP_GET, [](AsyncWebServerRequest* request) {
     capteurBloque = false;  // Réinitialiser capteurBloque
    // Créer une réponse HTML avec une balise <h1>
    const char* responseHtml =
      "Capteur réinitialisé avec succès !";
    request->send(200, "text/html;charset=UTF-8", responseHtml);
  });

  // Page secrète accessible via /log
server.on("/log", HTTP_GET, [](AsyncWebServerRequest* request) {
  Serial.println("📋 Accès à la page /log");
  
  // Lire le contenu du fichier
  String contenu = "";
  if (LittleFS.exists("/compteurs.txt")) {
    File file = LittleFS.open("/compteurs.txt", FILE_READ);
    if (file) {
      while (file.available()) {
        contenu += (char)file.read();
      }
      file.close();
    } else {
      contenu = "Erreur lors de l'ouverture du fichier.";
    }
  } else {
    contenu = "Fichier compteurs.txt introuvable.";
  }
  
  // Récupérer la fréquence CPU
  int f = getCpuFrequencyMhz();
  
  // Construire l'affichage des variables
  String variables = "=== ÉTAT DU SYSTÈME ===\n\n";
  variables += "Capteur niveau haut : " + String(capteurs.niveauHaut) + "\n";
  variables += "État contacteur : " + String(capteurs.etatContacteur) + "\n";
  variables += "État relais sécurité : " + String(capteurs.etatRelaisSecurite) + "\n";
  variables += "Moteur en marche : " + String(moteurEnMarche) + "\n";
  variables += "Capteur bloqué : " + String(capteurBloque) + "\n";
  variables += "Marche forcée moteur : " + String(moteurEnMarcheForce) + "\n";
  variables += "Présence tension : " + String(capteurs.tension) + "\n";
  variables += "CPU Freq : " + String(f) + " MHz\n";
  variables += "\n=== CONTENU FICHIER ===\n\n";
  
  // Construction de la page HTML
  String html = "<!DOCTYPE html><html lang='fr'><head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<title>Logs système</title>"
                "<style>"
                "body { font-family: 'Courier New', monospace; background-color: #1e1e1e; color: #00ff00; padding: 20px; }"
                "h1 { color: #00ff00; text-align: center; }"
                "pre { background-color: #2d2d2d; padding: 15px; border-radius: 5px; overflow-x: auto; white-space: pre-wrap; }"
                "button { padding: 10px 20px; background-color: #0070ff; color: white; border: none; border-radius: 5px; cursor: pointer; margin: 10px 5px; }"
                "button:hover { background-color: #0056cc; }"
                "</style>"
                "</head><body>"
                "<h1>📋 Logs système</h1>"
                "<pre>" + variables + contenu + "</pre>"
                "<button onclick='window.location.href=\"/\"'>🏠 Retour</button>"
                "<button onclick='location.reload()'>🔄 Actualiser</button>"
                "</body></html>";
  
  request->send(200, "text/html", html);
});

  server.on("/reset-securite", HTTP_GET, [](AsyncWebServerRequest* request) {
    relaisDeclenche = false;
    securiteDeclenche = 0;  // Réinitialiser capteurBloque
  relaisDeclenche = false;

    const char* responseHtml =
      "relais réinitialisé avec succès !";
    request->send(200, "text/plain", "Relais réinitialisé avec succès !");
  });

  server.begin();
}
