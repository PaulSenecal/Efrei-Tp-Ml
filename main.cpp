#include <QCoreApplication>
#include <QDebug>
#include <algorithm>
#include "stockmarket.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qInfo() << "Chargement des données des actions Apple depuis la base de données...";
    QVector<StockRecord> records = StockMarket::loadFromDatabase();

    if (records.isEmpty()) {
        qWarning() << "Aucune donnée chargée.";
        return 1;
    }

    // Trier par date
    std::sort(records.begin(), records.end(),
              [](const StockRecord& a, const StockRecord& b) {
                  return a.date < b.date;
              });


    StockMarket::displayDatasetOverview(records);
    StockMarket::displayKeyStatistics(records);

    // Calculer les moyennes mobiles sur 50 et 200 jours
    QVector<QPair<QString, double>> ma50 = StockMarket::calculateMovingAverage(records, 50);
    QVector<QPair<QString, double>> ma200 = StockMarket::calculateMovingAverage(records, 200);

    qInfo() << "\n--- Moyennes mobiles ---";

    qInfo() << "Calcule" << ma50.size() << "points de moyenne mobile sur 50 jours";
    qInfo() << "Calcule" << ma200.size() << "points de moyenne mobile sur 200 jours";


    // En mode QCoreApplication, l'application ne se termine pas automatiquement
    // On peut donc ajouter une ligne pour la quitter manuellement
    QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);

    return a.exec();
}
