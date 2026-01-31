/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
// crypto/kuznechik.h
#ifndef KUZNECHIK_H
#define KUZNECHIK_H

#include <QObject>
#include <QByteArray>

class Kuznechik : public QObject
{
    Q_OBJECT

public:
    explicit Kuznechik(QObject *parent = nullptr);
    ~Kuznechik();

    // Установить ключ (должен быть ровно 32 байта = 256 бит)
    bool setKey(const QByteArray &key);

    // Зашифровать один блок 16 байт
    QByteArray encryptBlock(const QByteArray &block) const;

    // Расшифровать один блок 16 байт
    QByteArray decryptBlock(const QByteArray &block) const;

    // Проверка, установлен ли ключ
    bool isKeySet() const;

private:
    QByteArray m_key;           // 32 байта
    bool m_keySet;

    struct RoundKey {
        quint64 high;
        quint64 low;
    };

    mutable std::array<RoundKey, 10> m_roundKeys;

    // Внутренние методы шифрования (реализуем далее)
    quint64 l(const quint64 &r) const;
    quint64 pi(const quint64 &a) const;
    quint64 pi_inv(const quint64 &a) const;
    void generateRoundKeys() const;
    mutable bool m_roundKeysGenerated;

    // Вспомогательные таблицы (S-блок и др.)
    quint8 Sbox[256];
    quint8 SboxInv[256];

    void initializeSboxes();

public:
        int blockSize() const { return 16; }

};

#endif // KUZNECHIK_H
