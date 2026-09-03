#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <QObject>
#include <QSettings>
#include <QJsonArray>
#include <QJsonObject>

class Accounts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJsonArray accounts READ accounts WRITE setAccounts NOTIFY accountsChanged)
public:
    explicit Accounts(QObject *parent = nullptr);

    QJsonArray accounts();
    void setAccounts(const QJsonArray &accounts);

    Q_INVOKABLE QJsonObject getAccount(const QString &username, const QString &server, const quint16 &port, const QString &target);
    Q_INVOKABLE void storeAccount(const QJsonObject &account);
    Q_INVOKABLE void removeAccount(const QJsonObject &account);

signals:
    void accountsChanged();

private:
    QSettings *settings;

};

#endif // ACCOUNTS_H
