/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#include "magma.h"
#include <QtEndian>
#include <cstring>

// S-блок замены по ГОСТ Р 34.12-2015
const quint8 Magma::S[8][16] = {
    { 0xC, 0x5, 0x6, 0xB, 0x9, 0x0, 0xA, 0xD, 0x3, 0xE, 0xF, 0x8, 0x4, 0x7, 0x1, 0x2 },
    { 0x6, 0x8, 0x2, 0x3, 0x9, 0xA, 0x5, 0xC, 0x1, 0xE, 0x4, 0x7, 0xB, 0xD, 0xF, 0x0 },
    { 0xD, 0x3, 0x4, 0xF, 0x8, 0x1, 0xA, 0x6, 0x9, 0x0, 0x7, 0xC, 0xB, 0x5, 0xE, 0x2 },
    { 0xB, 0x0, 0x6, 0x1, 0x5, 0xA, 0x8, 0xE, 0x3, 0xD, 0x7, 0xC, 0xF, 0x9, 0x2, 0x4 },
    { 0x7, 0x2, 0xC, 0x5, 0x8, 0x4, 0x6, 0xB, 0x1, 0xA, 0x9, 0xE, 0x3, 0xF, 0x0, 0xD },
    { 0x5, 0xB, 0x0, 0xC, 0x7, 0xA, 0x4, 0x8, 0xE, 0x2, 0xD, 0x6, 0xF, 0x1, 0x3, 0x9 },
    { 0xA, 0xD, 0x0, 0x3, 0x6, 0x9, 0x2, 0x8, 0xB, 0x1, 0x7, 0x5, 0xF, 0x4, 0xE, 0xC },
    { 0xC, 0x8, 0x2, 0x1, 0xD, 0x4, 0xF, 0x6, 0x7, 0x0, 0xA, 0x5, 0x3, 0xE, 0x9, 0xB }
};

// Циклический сдвиг влево на shift битов
quint32 Magma::rotateLeft(quint32 value, int shift) {
    return ((value << shift) | (value >> (32 - shift))) & 0xFFFFFFFF;
}

// Раундовая функция f
quint32 Magma::f(quint32 half, quint32 key) {
    quint32 sum = (half + key) & 0xFFFFFFFF;  // Сложение по модулю 2^32
    quint32 output = 0;

    // Преобразование через S-блоки (по 4 бита)
    for (int i = 0; i < 8; ++i) {
        quint8 byte = (sum >> (4 * i)) & 0xF;
        output |= static_cast<quint32>(S[i][byte]) << (4 * i);
    }

    return rotateLeft(output, 11);  // Сдвиг на 11 бит влево
}

// Конструктор
Magma::Magma() {
    keySchedule.fill(0);
}

// Установка ключа (ожидается 32 байта = 256 бит)
bool Magma::setKey(const QByteArray &key) {
    if (key.size() != 32) {
        return false;  // Ключ должен быть ровно 32 байта
    }

    // Используем 8 ключей по 32 бита (в порядке little-endian)
    for (int i = 0; i < 8; ++i) {
        quint32 k = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(key.constData()) + i * 4);
        keySchedule[i] = k;
    }

    return true;
}

// Шифрование 8-байтного блока
QByteArray Magma::encryptBlock(const QByteArray &block) {
    if (block.size() != 8) {
        return QByteArray();  // Ошибка: блок не 8 байт
    }

    // Читаем левую и правую части (32 бита каждая, little-endian)
    quint32 left = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(block.constData()));
    quint32 right = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(block.constData() + 4));

    // 32 раунда шифрования
    for (int i = 0; i < 32; ++i) {
        quint32 temp = right;
        right = left ^ f(right, keySchedule[i % 8]);
        left = temp;
    }

    // Результат: right || left (в порядке следования байт)
    QByteArray result;
    result.resize(8);
    qToLittleEndian<quint32>(right, reinterpret_cast<uchar*>(result.data()));
    qToLittleEndian<quint32>(left, reinterpret_cast<uchar*>(result.data() + 4));

    return result;
}

// Расшифрование 8-байтного блока
QByteArray Magma::decryptBlock(const QByteArray &block) {
    if (block.size() != 8) {
        return QByteArray();
    }

    quint32 left = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(block.constData()));
    quint32 right = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(block.constData() + 4));

    // 32 раунда расшифрования (в обратном порядке ключей)
    for (int i = 31; i >= 0; --i) {
        quint32 temp = left;
        left = right ^ f(left, keySchedule[i % 8]);
        right = temp;
    }

    // Результат: left || right
    QByteArray result;
    result.resize(8);
    qToLittleEndian<quint32>(left, reinterpret_cast<uchar*>(result.data()));
    qToLittleEndian<quint32>(right, reinterpret_cast<uchar*>(result.data() + 4));

    return result;
}

