#include <QCoreApplication>
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <QDateTime>
#include <QMap>
#include <QPair>
#include <cmath>
#include <algorithm>
#include <QSqlDatabase>
#include <QTextCodec> //
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFileInfo>

// Structure pour stocker de bourse
struct StockRecord {
    QString date;
    double open;
    double high;
    double low;
    double close;
    double adjClose;
    qint64 volume;
    QString sourceTable;
};

QVector<StockRecord> loadFromDatabase() {
    QVector<StockRecord> records;

    // Vérifier si le fichier de base de données existe
    QString dbPath = "C:/Users/senec/Desktop/datasheet/ml/Database/test.db";
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


    QString tableName = "\"AAPL_1980-12-03_2025-03-15\"";


    QSqlQuery structureQuery;
    structureQuery.prepare("PRAGMA table_info(" + tableName + ")");

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
    testQuery.prepare("SELECT * FROM " + tableName + " LIMIT 1");

    if (!testQuery.exec()) {
        qWarning() << "Erreur lors du test d'accès à la table:" << testQuery.lastError().text();

        tableName = "AAPL_1980-12-03_2025-03-15";
        testQuery.prepare("SELECT * FROM " + tableName + " LIMIT 1");

        if (!testQuery.exec()) {
            qWarning() << "Deuxième tentative echouee:" << testQuery.lastError().text();
            db.close();
            return records;
        } else {
            qInfo() << "Deuxième tentative reussie avec le nom de table sans guillemets.";
        }
    }

    // Obtenir les noms des colonnes
    QSqlRecord rec = testQuery.record();
    QStringList columnNames;

    for (int i = 0; i < rec.count(); i++) {
        columnNames << rec.fieldName(i);
    }

    qInfo() << "Noms des colonnes:" << columnNames.join(", ");

    // Requête SQL pour récupérer toutes les données (avec les noms de colonnes corrects)
    QSqlQuery query;

    if (!columnNames.isEmpty()) {
        query.prepare("SELECT * FROM " + tableName);
    } else {
        query.prepare("SELECT field1, field2, field3, field4, field5, field6, field7 FROM " + tableName);
    }

    if (!query.exec()) {
        qWarning() << "Erreur lors de l'execution de la requete:" << query.lastError().text();
        db.close();
        return records;
    }

    // Traiter les résultats
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
            // Mapping des champs field1-field7 vers les propriétés
            record.date = query.value(0).toString();
            record.open = query.value(1).toDouble();
            record.high = query.value(2).toDouble();
            record.low = query.value(3).toDouble();
            record.close = query.value(4).toDouble();
            record.adjClose = query.value(5).toDouble();
            record.volume = query.value(6).toLongLong();
        }

        record.sourceTable = tableName;
        records.append(record);
        count++;
    }

    qInfo() << "Charge" << count << "enregistrements depuis la table" << tableName;
    db.close();

    return records;
}

// Fonction pour calculer et afficher les principales statistiques
void displayKeyStatistics(const QVector<StockRecord>& records) {
    if (records.isEmpty()) {
        qWarning() << "Pas de donnees pour afficher les statistiques";
        return;
    }

    double sumClose = 0.0;
    double sumVolume = 0.0;
    double minClose = records[0].close;
    double maxClose = records[0].close;
    QDateTime minDate = QDateTime::fromString(records[0].date, "yyyy-MM-dd");
    QDateTime maxDate = minDate;

    if (!minDate.isValid()) {
        minDate = QDateTime::fromString(records[0].date, "yyyy-MM-dd hh:mm:ss");
    }

    if (!minDate.isValid()) {
        minDate = QDateTime::fromString(records[0].date, "MM/dd/yyyy");
    }

    maxDate = minDate;

    // Calculer les moyennes, min et max
    for (const auto& record : records) {
        sumClose += record.close;
        sumVolume += record.volume;

        if (record.close < minClose) minClose = record.close;
        if (record.close > maxClose) maxClose = record.close;

        // parser la date avec différents formats
        QDateTime date = QDateTime::fromString(record.date, "yyyy-MM-dd");

        if (!date.isValid()) {
            date = QDateTime::fromString(record.date, "yyyy-MM-dd hh:mm:ss");
        }

        if (!date.isValid()) {
            date = QDateTime::fromString(record.date, "MM/dd/yyyy");
        }

        if (date.isValid()) {
            if (minDate.isValid() && date < minDate) minDate = date;
            if (maxDate.isValid() && date > maxDate) maxDate = date;
        }
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

    // Calculer rendements quotidiens + volatilité
    QVector<double> dailyReturns;
    for (int i = 1; i < records.size(); i++) {
        double prevClose = records[i-1].close;
        double currentClose = records[i].close;
        if (prevClose > 0) {
            double dailyReturn = (currentClose - prevClose) / prevClose * 100.0;
            dailyReturns.append(dailyReturn);
        }
    }

    // Calculer le rendement moyen et volatilité
    double sumReturns = 0.0;
    for (double ret : dailyReturns) {
        sumReturns += ret;
    }

    double avgReturn = dailyReturns.isEmpty() ? 0.0 : sumReturns / dailyReturns.size();

    double sumReturnDiff = 0.0;
    for (double ret : dailyReturns) {
        double diff = ret - avgReturn;
        sumReturnDiff += diff * diff;
    }
    double volatility = dailyReturns.isEmpty() ? 0.0 : sqrt(sumReturnDiff / dailyReturns.size());

    qInfo() << "\n--- Statistiques des donnees ---";
    if (minDate.isValid() && maxDate.isValid()) {
        qInfo() << "Periode:" << minDate.toString("yyyy-MM-dd") << "à" << maxDate.toString("yyyy-MM-dd");
    } else {
        qInfo() << "Periode: Non determinee (format de date non reconnu)";
        qInfo() << "Premier exemple de date:" << records[0].date;
    }
    qInfo() << "Nombre total d'enregistrements:" << records.size();
    qInfo() << "Prix de clôture moyen:" << avgClose;
    qInfo() << "Prix de clôture minimum:" << minClose;
    qInfo() << "Prix de clôture maximum:" << maxClose;
    qInfo() << "Ecart-type des prix de clôture:" << stdDev;
    qInfo() << "Volume moyen:" << avgVolume;
    qInfo() << "Rendement quotidien moyen:" << avgReturn << "%";
    qInfo() << "Volatilite (ecart-type des rendements quotidiens):" << volatility << "%";
}

QVector<QPair<QString, double>> calculateMovingAverage(const QVector<StockRecord>& records, int period) {
    QVector<QPair<QString, double>> movingAverages;

    if (records.size() < period) {
        qWarning() << "Pas assez de donnees pour calculer la moyenne mobile sur" << period << "jours";
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
    qInfo() << "Chargement des données des actions Apple depuis la base de données...";


    QVector<StockRecord> records = loadFromDatabase();

    if (records.isEmpty()) {
        qWarning() << "Aucune donnée chargée.";
        return 1;
    }

    // Trier par date
    std::sort(records.begin(), records.end(),
              [](const StockRecord& a, const StockRecord& b) {
                  return a.date < b.date;
              });

    // Afficher les statistiques générales
    qInfo() << "\nAperçu du dataset:";
    qInfo() << "Total des enregistrements:" << records.size();

    // Trouver la plage de dates
    QString earliestDate = records.first().date;
    QString latestDate = records.last().date;

    qInfo() << "Plage de dates:" << earliestDate << "à" << latestDate;

    // Afficher les 5 premiers enregistrements
    qInfo() << "\nPremiers 5 enregistrements:";
    qInfo().nospace() << qSetFieldWidth(20)  << "Date"
                      << qSetFieldWidth(10) << "Open"
                      << qSetFieldWidth(10) << "High"
                      << qSetFieldWidth(10) << "Low"
                      << qSetFieldWidth(10) << "Close"
                      << qSetFieldWidth(12) << "Adj Close"
                      << qSetFieldWidth(12) << "Volume";

    for (int i = 0; i < qMin(5, records.size()); ++i) {
        const auto& record = records[i];
        qInfo().nospace() << qSetFieldWidth(20) << record.date
                          << qSetFieldWidth(10) << record.open
                          << qSetFieldWidth(10) << record.high
                          << qSetFieldWidth(10) << record.low
                          << qSetFieldWidth(10) << record.close
                          << qSetFieldWidth(12) << record.adjClose
                          << qSetFieldWidth(12) << record.volume;
    }

    // Calculer et afficher des statistiques détaillées
    displayKeyStatistics(records);

    // Calculer les moyennes mobiles sur 50 et 200 jours
    QVector<QPair<QString, double>> ma50 = calculateMovingAverage(records, 50);
    QVector<QPair<QString, double>> ma200 = calculateMovingAverage(records, 200);

    qInfo() << "\n--- Moyennes mobiles ---";
    qInfo() << "Calcule" << ma50.size() << "points de moyenne mobile sur 50 jours";
    qInfo() << "Calcule" << ma200.size() << "points de moyenne mobile sur 200 jours";

    // En mode QCoreApplication, l'application ne se termine pas automatiquement
    // On peut donc ajouter une ligne pour la quitter manuellement
    QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);

    return a.exec();
}
