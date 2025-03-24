#ifndef STOCKMARKET_H
#define STOCKMARKET_H

#include <QVector>
#include <QString>
#include <QPair>

// Structure pour stocker des données de bourse
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

class StockMarket {
public:
    static QVector<StockRecord> loadFromDatabase(const QString& dbPath = "C:/Users/senec/Desktop/datasheet/ml/Database/test.db",
                                                 const QString& tableName = "\"AAPL_1980-12-03_2025-03-15\"");
    static void displayKeyStatistics(const QVector<StockRecord>& records);
    static QVector<QPair<QString, double>> calculateMovingAverage(const QVector<StockRecord>& records, int period);
    static void displayDatasetOverview(const QVector<StockRecord>& records);
};

#endif // STOCKMARKET_H
