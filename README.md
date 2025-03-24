# Analyseur de données d'actions Apple

## Description
Ce programme en C++ utilisant Qt analyse les données historiques des actions Apple en traitant plusieurs fichiers CSV. Il offre des fonctionnalités d'analyse statistique et génère des indicateurs techniques utiles pour l'analyse financière.

## Fonctionnalités
- Chargement et parsing de multiples fichiers CSV d'actions Apple
- Consolidation des données et élimination des doublons
- Tri chronologique des données
- Calcul de statistiques descriptives:
  - Prix de clôture moyen, minimum et maximum
  - Écart-type des prix de clôture
  - Volume moyen des transactions
  - Rendement quotidien moyen
  - Volatilité (écart-type des rendements quotidiens)
- Calcul d'indicateurs techniques:
  - Moyenne mobile sur 50 jours (MA50)
  - Moyenne mobile sur 200 jours (MA200)
- Exportation des données nettoyées vers un nouveau fichier CSV

## Prérequis
- Qt 5.12 ou supérieur
- Compilateur C++ avec support C++17

## Structure des fichiers
```
├── main.cpp          # Code source principal
├── ml.pro            # Fichier de projet Qt
├── dataset/          # Dossier contenant les fichiers CSV
│   ├── AAPL_*.csv    # Fichiers CSV des données d'actions Apple
├── build/            # Dossier contenant les fichiers générés
    └── apple_stock_clean.csv # Fichier de sortie avec données nettoyées
```

## Installation
1. Clonez ce dépôt:
   ```
   git clone https://github.com/PaulSenecal/Efrei-Tp-Ml/tree/master
   cd https://github.com/PaulSenecal/Efrei-Tp-Ml/tree/master
   ```

2. Compilez le projet:
   ```
   qmake
   make
   ```

## Utilisation
1. Placez vos fichiers CSV d'actions Apple dans le dossier `dataset/`
2. Exécutez le programme:
   ```
   ./build/[NOM_EXECUTABLE]
   ```
3. Les résultats de l'analyse seront affichés dans la console
4. Un fichier CSV consolidé et nettoyé sera généré dans `build/apple_stock_clean.csv`

## Format des fichiers CSV
Les fichiers CSV doivent contenir les colonnes suivantes dans cet ordre:
- Date (YYYY-MM-DD)
- Open (prix d'ouverture)
- High (prix le plus haut)
- Low (prix le plus bas)
- Close (prix de clôture)
- Adj Close (prix de clôture ajusté)
- Volume (nombre d'actions échangées)

## Remarques
- Le programme recherche les fichiers suivant le modèle `AAPL_*.csv`
- Les fichiers peuvent être placés soit dans le dossier `dataset/` soit dans le répertoire courant
- En cas de données dupliquées (même date), seule la dernière entrée est conservée

## Exemple de sortie console
```
Chargement des données des actions Apple...
Chargé 10521 enregistrements depuis ./dataset/AAPL_1980-12-03_2025-02-07.csv
Chargé 10521 enregistrements depuis ./dataset/AAPL_1980-12-13_2025-01-31.csv

Aperçu du dataset:
Total des enregistrements: 21042
Après suppression des doublons: 10521

Plage de dates: 1980-12-12 à 2025-02-07

Premiers 5 enregistrements:
Date        Open      High      Low       Close     Adj Close  Volume     
1980-12-12  0.1288    0.1288    0.1288    0.1288    0.1023     469033600  

--- Statistiques des données ---
Période: 1980-12-12 à 2025-02-07
Nombre total d'enregistrements: 10521
Prix de clôture moyen: 26.3672
Prix de clôture minimum: 0.1288
Prix de clôture maximum: 196.23
Écart-type des prix de clôture: 47.3851
Volume moyen: 86283365
Rendement quotidien moyen: 0.12%
Volatilité (écart-type des rendements quotidiens): 2.45%
```

## Licence
[Insérer informations de licence ici]
