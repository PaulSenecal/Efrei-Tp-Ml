#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <QDateTime>
#include <QFileInfo>
#include <QMap>
#include <QPair>
#include <cmath>
#include <algorithm>

// Structure pour stocker les données d'une action
struct StockRecord {
    QString date;
    double open;
    double high;
    double low;
    double close;
    double adjClose;
    qint64 volume;
    QString sourceFile;
};

// Analyse un fichier CSV return vecteur de StockRecord
QVector<StockRecord> parseCSVFile(const QString& filePath) {
    QVector<StockRecord> records;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Erreur lors de l'ouverture du fichier:" << filePath;
        return records;
    }

    QTextStream in(&file);

    if (!in.atEnd()) {
        in.readLine();
    }

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() >= 7) {
            StockRecord record;
            record.sourceFile = QFileInfo(filePath).fileName();

            // Date
            record.date = fields[0];

            // Open
            record.open = fields[1].toDouble();

            // High
            record.high = fields[2].toDouble();

            // Low
            record.low = fields[3].toDouble();

            // Close
            record.close = fields[4].toDouble();

            // Adj Close
            record.adjClose = fields[5].toDouble();

            // Volume
            record.volume = fields[6].toLongLong();

            records.append(record);
        }
    }

    file.close();
    qInfo() << "Chargé" << records.size() << "enregistrements depuis" << filePath;
    return records;
}

QVector<StockRecord> loadLocalCSVFiles() {
    QVector<StockRecord> allRecords;
    QStringList fileList;

    QDir dir("C:/Users/senec/Desktop/datasheet/ml/dataset");
    if (!dir.exists()) {
        dir = QDir(".");
    }

    // Obtenir tous les fichiers CSV commençant par "AAPL_"
    QStringList filters;
    filters << "AAPL_*.csv";//AAPL_1980-12-03_2025-02-07.csv
    // filters << "AAPL_1980-12-03_2025-02-07.csv";
    dir.setNameFilters(filters);

    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& fileInfo : files) {
        fileList << fileInfo.filePath();
    }

    // Analyser chaque fichier moyen en qt
    for (const QString& file : fileList) {
        QVector<StockRecord> records = parseCSVFile(file);
        allRecords.append(records);
    }

    return allRecords;
}

// Fonction pour calculer des statistiques sur les données d'actions
void calculateStatistics(const QVector<StockRecord>& records) {
    if (records.isEmpty()) {
        qWarning() << "Pas de données pour calculer les statistiques";
        return;
    }

    // Valeurs pour les calculs
    double sumClose = 0.0;
    double sumVolume = 0.0;
    double minClose = records[0].close;
    double maxClose = records[0].close;
    QDateTime minDate = QDateTime::fromString(records[0].date, "yyyy-MM-dd");
    QDateTime maxDate = minDate;

    // Calculer les moyennes, min et max
    for (const auto& record : records) {
        sumClose += record.close;
        sumVolume += record.volume;

        if (record.close < minClose) minClose = record.close;
        if (record.close > maxClose) maxClose = record.close;

        QDateTime date = QDateTime::fromString(record.date, "yyyy-MM-dd");
        if (date < minDate) minDate = date;
        if (date > maxDate) maxDate = date;
    }

    double avgClose = sumClose / records.size();
    double avgVolume = sumVolume / records.size();

    // Calculer l'écart-type des prix de clôture
    double sumSquareDiff = 0.0;
    for (const auto& record : records) {
        double diff = record.close - avgClose;
        sumSquareDiff += diff * diff;
    }
    double stdDev = sqrt(sumSquareDiff / records.size());

    // Calculer la volatilité (pourcentage de variation quotidienne)
    QVector<double> dailyReturns;
    for (int i = 1; i < records.size(); i++) {
        double prevClose = records[i-1].close;
        double currentClose = records[i].close;
        double dailyReturn = (currentClose - prevClose) / prevClose * 100.0;
        dailyReturns.append(dailyReturn);
    }

    // Calculer la volatilité moyenne
    double sumReturns = 0.0;
    for (double ret : dailyReturns) {
        sumReturns += ret;
    }
    double avgReturn = sumReturns / dailyReturns.size();

    double sumReturnDiff = 0.0;
    for (double ret : dailyReturns) {
        double diff = ret - avgReturn;
        sumReturnDiff += diff * diff;
    }
    double volatility = sqrt(sumReturnDiff / dailyReturns.size());

    qInfo() << "\n--- Statistiques des données ---";
    qInfo() << "Période:" << minDate.toString("yyyy-MM-dd") << "à" << maxDate.toString("yyyy-MM-dd");
    qInfo() << "Nombre total d'enregistrements:" << records.size();
    qInfo() << "Prix de clôture moyen:" << avgClose;
    qInfo() << "Prix de clôture minimum:" << minClose;
    qInfo() << "Prix de clôture maximum:" << maxClose;
    qInfo() << "Écart-type des prix de clôture:" << stdDev;
    qInfo() << "Volume moyen:" << avgVolume;
    qInfo() << "Rendement quotidien moyen:" << avgReturn << "%";
    qInfo() << "Volatilité (écart-type des rendements quotidiens):" << volatility << "%";
}

// Fonction pour exporter les données traitées vers un nouveau fichier CSV
bool exportToCSV(const QVector<StockRecord>& records, const QString& filePath) {
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Erreur lors de l'ouverture du fichier pour l'écriture:" << filePath;
        return false;
    }

    QTextStream out(&file);

    // Écrire l'en-tête
    out << "Date,Open,High,Low,Close,Adj Close,Volume,Source File\n";

    // Écrire les données
    for (const auto& record : records) {
        out << record.date << ","
            << record.open << ","
            << record.high << ","
            << record.low << ","
            << record.close << ","
            << record.adjClose << ","
            << record.volume << ","
            << record.sourceFile << "\n";
    }

    file.close();
    qInfo() << "Données exportées avec succès vers" << filePath;
    return true;
}

// Fonction pour calculer les moyennes mobiles
QVector<QPair<QString, double>> calculateMovingAverage(const QVector<StockRecord>& records, int period) {
    QVector<QPair<QString, double>> movingAverages;

    if (records.size() < period) {
        qWarning() << "Pas assez de données pour calculer la moyenne mobile sur" << period << "jours";
        return movingAverages;
    }

    for (int i = period - 1; i < records.size(); i++) {
        double sum = 0.0;

        for (int j = 0; j < period; j++) {
            sum += records[i - j].close;
        }

        double average = sum / period;
        movingAverages.append(qMakePair(records[i].date, average));
    }

    return movingAverages;
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    qInfo() << "Chargement des données des actions Apple...";

    QVector<StockRecord> records = loadLocalCSVFiles();

    if (records.isEmpty()) {
        qWarning() << "Aucune donnée chargée.";
        return 1;
    }

    // Trier par date
    std::sort(records.begin(), records.end(),
              [](const StockRecord& a, const StockRecord& b) {
                  return a.date < b.date;
              });

    // Supprimer les doublons basés sur la date
    QMap<QString, StockRecord> uniqueRecordsMap;
    for (const auto& record : records) {
        uniqueRecordsMap[record.date] = record;
    }

    QVector<StockRecord> uniqueRecords;
    for (const auto& record : uniqueRecordsMap) {
        uniqueRecords.append(record);
    }

    // Afficher les statistiques générales
    qInfo() << "\nAperçu du dataset:";
    qInfo() << "Total des enregistrements:" << records.size();
    qInfo() << "Après suppression des doublons:" << uniqueRecords.size();

    // Trouver la plage de dates
    QString earliestDate = uniqueRecords.first().date;
    QString latestDate = uniqueRecords.last().date;

    qInfo() << "Plage de dates:" << earliestDate << "à" << latestDate;

    // Afficher les 5 premiers enregistrements
    qInfo() << "\nPremiers 5 enregistrements:";
    qInfo().nospace() << qSetFieldWidth(12)  << "Date"
                      << qSetFieldWidth(10) << "Open"
                      << qSetFieldWidth(10) << "High"
                      << qSetFieldWidth(10) << "Low"
                      << qSetFieldWidth(10) << "Close"
                      << qSetFieldWidth(12) << "Adj Close"
                      << qSetFieldWidth(12) << "Volume";

    for (int i = 0; i < qMin(5, uniqueRecords.size()); ++i) {
        const auto& record = uniqueRecords[i];
        qInfo().nospace() << qSetFieldWidth(12) << record.date
                          << qSetFieldWidth(10) << record.open
                          << qSetFieldWidth(10) << record.high
                          << qSetFieldWidth(10) << record.low
                          << qSetFieldWidth(10) << record.close
                          << qSetFieldWidth(12) << record.adjClose
                          << qSetFieldWidth(12) << record.volume;
    }

    // Calculer et afficher des statistiques plus détaillées
    calculateStatistics(uniqueRecords);

    // Calculer les moyennes mobiles sur 50 et 200 jours
    QVector<QPair<QString, double>> ma50 = calculateMovingAverage(uniqueRecords, 50);
    QVector<QPair<QString, double>> ma200 = calculateMovingAverage(uniqueRecords, 200);

    qInfo() << "\n--- Moyennes mobiles ---";
    qInfo() << "Calculé" << ma50.size() << "points de moyenne mobile sur 50 jours";
    qInfo() << "Calculé" << ma200.size() << "points de moyenne mobile sur 200 jours";

    // Exporter les données nettoyées
    exportToCSV(uniqueRecords, "./build/apple_stock_clean.csv");
    //"C:/Users/senec/Desktop/datasheet/ml/build/apple_stock_clean.csv");

    // En mode QCoreApplication, l'application ne se termine pas automatiquement
    // On peut donc ajouter une ligne pour la quitter manuellement
    QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);

    return a.exec();
}
