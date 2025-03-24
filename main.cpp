#include <QCoreApplication>
#include <QDebug>
#include <algorithm>
#include "stockmarket.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    qInfo() << "Chargement des données des actions Apple depuis la base de données...";
    QVector<StockRecord> records = StockMarketML::loadFromDatabase();

    if (records.isEmpty()) {
        qWarning() << "Aucune donnée chargée.";
        return 1;
    }

    // Trier par date
    std::sort(records.begin(), records.end(),
              [](const StockRecord& a, const StockRecord& b) {
                  return a.date < b.date;
              });

    // Prétraiter les données (calcul des features)
    StockMarketML::preprocessData(records);

    // Diviser les données en ensembles d'entraînement et de test
    auto datasets = StockMarketML::splitTrainTest(records);
    QVector<StockRecord> trainData = datasets.first;
    QVector<StockRecord> testData = datasets.second;


    StockMarketML::LinearRegressionModel model;
    model.train(trainData);

    StockMarketML::evaluateModel(model, testData);
    StockMarketML::displayTopResults(testData);

    QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);
    return a.exec();
}
