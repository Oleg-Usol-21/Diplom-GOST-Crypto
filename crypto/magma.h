/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#ifndef MAGMA_H
#define MAGMA_H

#include <QByteArray>
#include <array>

class Magma {
private:
    std::array<quint32, 8> keySchedule;  // Раундовые ключи (8 * 32 бита = 256 бит)
    static const quint8 S[8][16];       // S-блок замены (ГОСТ Р 34.12-2015)
    quint32 f(quint32 half, quint32 key); // Функция f (раундовая)
    quint32 rotateLeft(quint32 value, int shift); // Циклический сдвиг влево


public:
    Magma();
    bool setKey(const QByteArray &key);  // Установка 32-байтного ключа
    QByteArray encryptBlock(const QByteArray &block); // Шифрование 8 байт
    QByteArray decryptBlock(const QByteArray &block); // Расшифрование 8 байт
    int blockSize() const { return 8; }
};

#endif // MAGMA_H
