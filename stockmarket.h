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
    // Attributs dérivés pour les features
    double ma50;
    double ma200;
    double volatility;
    double dailyReturn;
    // Pour l'évaluation du modèle
    double predictedClose;
    bool predictedDirection;
    bool actualDirection;
};

// Classe pour les métriques d'évaluation
class EvaluationMetrics {
public:
    // Matrice de confusion
    int truePositives;
    int trueNegatives;
    int falsePositives;
    int falseNegatives;

    // Métriques d'erreur
    double mse;
    double rmse;
    double accuracy;
    double precision;
    double recall;
    double f1Score;

    EvaluationMetrics();
    void calculateMetrics(const QVector<StockRecord>& testData);
    void displayMetrics() const;
};

class StockMarketML {
public:
    // Classe de régression linéaire
    class LinearRegressionModel {
    private:
        double intercept;
        QVector<double> coefficients;

    public:
        LinearRegressionModel();
        void train(const QVector<StockRecord>& trainData);
        double predict(const StockRecord& record) const;
    };

    // Méthodes de chargement et prétraitement des données
    static QVector<StockRecord> loadFromDatabase(
        const QString& dbPath = "C:/Users/senec/Desktop/datasheet/ml/Database/test.db",
        const QString& tableName = "\"AAPL_1980-12-03_2025-03-15\""
        );

    static void preprocessData(QVector<StockRecord>& records);
    static void calculateMovingAveragesForRecords(QVector<StockRecord>& records, int period);
    static void calculateVolatility(QVector<StockRecord>& records, int period);

    // Méthodes de division des données
    static QPair<QVector<StockRecord>, QVector<StockRecord>> splitTrainTest(
        const QVector<StockRecord>& records,
        double trainRatio = 0.8
        );

    // Méthodes d'évaluation
    static void evaluateModel(const LinearRegressionModel& model, QVector<StockRecord>& testData);
    static void displayTopResults(const QVector<StockRecord>& testResults, int numToShow = 10);
};

#endif // STOCKMARKET_H
