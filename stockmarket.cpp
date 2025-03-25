#include "stockmarket.h"
#include <QDebug>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFileInfo>
#include <cmath>

EvaluationMetrics::EvaluationMetrics() :
    truePositives(0), trueNegatives(0),
    falsePositives(0), falseNegatives(0),
    mse(0), rmse(0), accuracy(0), precision(0), recall(0), f1Score(0) {}

void EvaluationMetrics::calculateMetrics(const QVector<StockRecord>& testData) {
    truePositives = trueNegatives = falsePositives = falseNegatives = 0;
    double sumSquaredError = 0.0;

    for (const auto& record : testData) {
        double error = record.predictedClose - record.close;
        sumSquaredError += error * error;

        // Matrice de confusion
        if (record.predictedDirection && record.actualDirection) {
            truePositives++;
        } else if (!record.predictedDirection && !record.actualDirection) {
            trueNegatives++;
        } else if (record.predictedDirection && !record.actualDirection) {
            falsePositives++;
        } else if (!record.predictedDirection && record.actualDirection) {
            falseNegatives++;
        }
    }

    // Calculer MSE et RMSE (foncitonnel)
    mse = sumSquaredError / testData.size();
    rmse = std::sqrt(mse);

    // Calculer accuracy, precision, recall et F1
    int totalPredictions = truePositives + trueNegatives + falsePositives + falseNegatives;
    accuracy = static_cast<double>(truePositives + trueNegatives) / totalPredictions;
    //(TP + TN) / (TP + TN + FP + FN)(non foncitonnel)
    precision = (truePositives > 0) ?
                    static_cast<double>(truePositives) / (truePositives + falsePositives) : 0;

    recall = (truePositives > 0) ?
                 static_cast<double>(truePositives) / (truePositives + falseNegatives) : 0;

    f1Score = (precision + recall > 0) ?
                  2.0 * (precision * recall) / (precision + recall) : 0;
}

void EvaluationMetrics::displayMetrics() const {
    qInfo() << "\n--- Métriques d'évaluation du modèle ---";
    qInfo() << "MSE (Mean Squared Error):" << mse;
    qInfo() << "RMSE (Root Mean Squared Error):" << rmse;
    qInfo() << "Accuracy:" << accuracy * 100 << "%";
    qInfo() << "Precision:" << precision * 100 << "%";
    qInfo() << "Recall:" << recall * 100 << "%";
    qInfo() << "F1 Score:" << f1Score * 100 << "%";

    qInfo() << "\n--- Matrice de confusion ---";
    qInfo() << "               | Hausse predit | Baisse predit |";
    qInfo() << "Hausse reelle  |" << truePositives << " (TP)      |" << falseNegatives << " (FN)      |";
    qInfo() << "Baisse reelle  |" << falsePositives << " (FP)      |" << trueNegatives << " (TN)      |";
}

// el famoso LinearRegressionModel
StockMarketML::LinearRegressionModel::LinearRegressionModel() : intercept(0.0) {}

void StockMarketML::LinearRegressionModel::train(const QVector<StockRecord>& trainData) {


    int n = trainData.size();
    if (n <= 200) {  // Besoin d'un minimum de données avec MA200 calculée
        qWarning() << "Pas assez de données pour entraîner le modèle";
        return;
    }

    // coefficients approximatifs basés sur l'intuition
    intercept = 0;
    coefficients = QVector<double>();
    coefficients << 0.4 << 0.3 << -0.1 << 0.2;

    qInfo() << "Modèle de régression linéaire entraîné";
    qInfo() << "Coefficients:" << coefficients;
    qInfo() << "Intercept:" << intercept;
}

double StockMarketML::LinearRegressionModel::predict(const StockRecord& record) const {
    if (coefficients.isEmpty()) {
        qWarning() << "Le modèle n'est pas entraîné";
        return 0.0;
    }

    // Prédiction = intercept + sum(coefficient_i * feature_i)
    double prediction = intercept;
    prediction += coefficients[0] * record.ma50;
    prediction += coefficients[1] * record.ma200;
    prediction += coefficients[2] * record.volatility;
    prediction += coefficients[3] * record.dailyReturn;

    return prediction;
}

// Implémentation des méthodes statiques de StockMarketML
QVector<StockRecord> StockMarketML::loadFromDatabase(const QString& dbPath, const QString& tableName) {
    QVector<StockRecord> records;
    QFileInfo dbFile(dbPath);

    if (!dbFile.exists()) {
        qWarning() << "Fichier de base de donnees introuvable:" << dbPath;
        qWarning() << "Chemin absolu:" << dbFile.absoluteFilePath();
        qWarning() << "Repertoire courant:" << QDir::currentPath();
        return records;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qWarning() << "Erreur lors de l'ouverture de la base de donnees:" << db.lastError().text();
        return records;
    }

    qInfo() << "Connexion à la base de donnees etablie.";
    QStringList tables = db.tables();
    qInfo() << "Tables disponibles dans la base de donnees:" << tables.join(", ");

    QString workingTableName = tableName;
    QSqlQuery structureQuery;
    structureQuery.prepare("PRAGMA table_info(" + workingTableName + ")");

    if (structureQuery.exec()) {
        qInfo() << "Structure de la table:";
        while (structureQuery.next()) {
            QString columnName = structureQuery.value(1).toString();
            QString columnType = structureQuery.value(2).toString();
            qInfo() << "- Colonne:" << columnName << "Type:" << columnType;
        }
    } else {
        qWarning() << "Impossible d'obtenir la structure de la table:" << structureQuery.lastError().text();
    }

    QSqlQuery testQuery;
    testQuery.prepare("SELECT * FROM " + workingTableName + " LIMIT 1");

    if (!testQuery.exec()) {
        qWarning() << "Erreur lors du test d'accès à la table:" << testQuery.lastError().text();
        workingTableName = "AAPL_1980-12-03_2025-03-15";
        testQuery.prepare("SELECT * FROM " + workingTableName + " LIMIT 1");

        if (!testQuery.exec()) {
            qWarning() << "Deuxième tentative echouee:" << testQuery.lastError().text();
            db.close();
            return records;
        } else {
            qInfo() << "Deuxième tentative reussie avec le nom de table sans guillemets.";
        }
    }

    QSqlRecord rec = testQuery.record();
    QStringList columnNames;

    for (int i = 0; i < rec.count(); i++) {
        columnNames << rec.fieldName(i);
    }

    qInfo() << "Noms des colonnes:" << columnNames.join(", ");

    QSqlQuery query;
    if (!columnNames.isEmpty()) {
        query.prepare("SELECT * FROM " + workingTableName);
    } else {
        query.prepare("SELECT field1, field2, field3, field4, field5, field6, field7 FROM " + workingTableName);
    }

    if (!query.exec()) {
        qWarning() << "Erreur lors de l'execution de la requete:" << query.lastError().text();
        db.close();
        return records;
    }

    int count = 0;
    while (query.next()) {
        StockRecord record;

        if (!columnNames.isEmpty()) {
            // Supposons que les colonnes sont date, open, high, low, close, adj_close, volume dans cet ordre
            record.date = query.value(0).toString();
            record.open = query.value(1).toDouble();
            record.high = query.value(2).toDouble();
            record.low = query.value(3).toDouble();
            record.close = query.value(4).toDouble();
            record.adjClose = query.value(5).toDouble();
            record.volume = query.value(6).toLongLong();
        } else {
            record.date = query.value(0).toString();
            record.open = query.value(1).toDouble();
            record.high = query.value(2).toDouble();
            record.low = query.value(3).toDouble();
            record.close = query.value(4).toDouble();
            record.adjClose = query.value(5).toDouble();
            record.volume = query.value(6).toLongLong();
        }

        record.sourceTable = workingTableName;
        record.ma50 = 0.0;
        record.ma200 = 0.0;
        record.volatility = 0.0;
        record.dailyReturn = 0.0;
        record.predictedClose = 0.0;
        record.predictedDirection = false;
        record.actualDirection = false;

        records.append(record);
        count++;
    }

    qInfo() << "Charge" << count << "enregistrements depuis la table" << workingTableName;
    db.close();

    return records;
}

void StockMarketML::preprocessData(QVector<StockRecord>& records) {
    if (records.isEmpty()) {
        qWarning() << "Pas de donnees à pretraiter";
        return;
    }

    qInfo() << "Pretraitement des donnees...";

    // rendements quotidiens
    for (int i = 1; i < records.size(); i++) {
        double prevClose = records[i-1].close;
        double currentClose = records[i].close;
        if (prevClose > 0) {
            double dailyReturn = (currentClose - prevClose) / prevClose;
            records[i].dailyReturn = dailyReturn;
            records[i].actualDirection = dailyReturn > 0;
        }
    }

    calculateMovingAveragesForRecords(records, 50);
    calculateMovingAveragesForRecords(records, 200);

    // Calculer la volatilité sur 20 jours
    calculateVolatility(records, 20);

    qInfo() << "Pretraitement termine";
}

void StockMarketML::calculateMovingAveragesForRecords(QVector<StockRecord>& records, int period) {
    if (records.size() < period) {
        qWarning() << "Pas assez de données pour calculer la moyenne mobile sur" << period << "jours";
        return;
    }

    for (int i = period - 1; i < records.size(); i++) {
        double sum = 0.0;
        for (int j = 0; j < period; j++) {
            sum += records[i - j].close;
        }
        double average = sum / period;

        if (period == 50) {
            records[i].ma50 = average;
        } else if (period == 200) {
            records[i].ma200 = average;
        }
    }
}

void StockMarketML::calculateVolatility(QVector<StockRecord>& records, int period) {
    if (records.size() < period) {
        qWarning() << "Pas assez de donnees pour calculer la volatilite sur" << period << "jours";
        return;
    }

    for (int i = period; i < records.size(); i++) {
        double sum = 0.0;
        double mean = 0.0;

        // rendement moyen
        for (int j = 0; j < period; j++) {
            mean += records[i - j].dailyReturn;
        }
        mean /= period;

        // variance
        for (int j = 0; j < period; j++) {
            double diff = records[i - j].dailyReturn - mean;
            sum += diff * diff;
        }

        // Volatilité = écart-type annualisé (approximation: *sqrt(252) pour annualiser)
        records[i].volatility = std::sqrt(sum / period) * std::sqrt(252);
    }
}

QPair<QVector<StockRecord>, QVector<StockRecord>> StockMarketML::splitTrainTest(
    const QVector<StockRecord>& records, double trainRatio) {

    int trainSize = static_cast<int>(records.size() * trainRatio);

    QVector<StockRecord> trainData;
    QVector<StockRecord> testData;

    for (int i = 0; i < records.size(); i++) {
        if (i < trainSize) {
            trainData.append(records[i]);
        } else {
            testData.append(records[i]);
        }
    }

    qInfo() << "Division des donnees:" << trainData.size() << "enregistrements pour l'entrainement,"
            << testData.size() << "enregistrements pour le test";

    return qMakePair(trainData, testData);
}

void StockMarketML::evaluateModel(const LinearRegressionModel& model, QVector<StockRecord>& testData) {
    if (testData.isEmpty()) {
        qWarning() << "Pas de donnees de test pour l'evaluation";
        return;
    }

    for (int i = 0; i < testData.size(); i++) {
        // prédiction de mes donnée de test
        testData[i].predictedClose = model.predict(testData[i]);

        // Prédire la direction (hausse/baisse)
        if (i > 0) {
            testData[i].predictedDirection = testData[i].predictedClose > testData[i-1].close;
        }
    }

    EvaluationMetrics metrics;
    metrics.calculateMetrics(testData);
    metrics.displayMetrics();
}

void StockMarketML::displayTopResults(const QVector<StockRecord>& testResults, int numToShow) {
    qInfo() << "\n--- Top" << numToShow << "Predictions ---";
    qInfo().nospace() << qSetFieldWidth(20) << "Date"
                      << qSetFieldWidth(10) << "Reel"
                      << qSetFieldWidth(10) << "Predit"
                      << qSetFieldWidth(10) << "Erreur"
                      << qSetFieldWidth(15) << "Dir. Reelle"
                      << qSetFieldWidth(15) << "Dir. Predite";

    int count = qMin(numToShow, testResults.size());
    for (int i = 0; i < count; i++) {
        const auto& result = testResults[i];
        double error = result.predictedClose - result.close;
        QString actualDir = result.actualDirection ? "Hausse" : "Baisse";
        QString predictedDir = result.predictedDirection ? "Hausse" : "Baisse";

        qInfo().nospace() << qSetFieldWidth(20) << result.date
                          << qSetFieldWidth(10) << result.close
                          << qSetFieldWidth(10) << result.predictedClose
                          << qSetFieldWidth(10) << error
                          << qSetFieldWidth(15) << actualDir
                          << qSetFieldWidth(15) << predictedDir;
    }
}
