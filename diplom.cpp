/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
include "diplom.h"
#include "./ui_diplom.h"
#include "settings.h"
#include <QScreen>
#include <QGuiApplication>
#include <QSettings>
#include <QFileDialog>
#include <QLabel>
#include <QFileInfo>
#include <QDir>
#include <algorithm>  // для std::sort
#include <utility>  // IWYU pragma: keep
#include <QDirIterator>
#include <QRandomGenerator>
#include "crypto/kuznechik.h"
#include "crypto/striborg.h"
#include "crypto/magma.h"
#include <QCryptographicHash>

Diplom::Diplom(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Diplom)
{
    ui->setupUi(this);
    ui->tableView->setIconSize(QSize(20, 20));
    // Опционально: подгонка ширины столбцов
    ui->tableView->resizeColumnToContents(0);  // Номер
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // Путь — растягивается
    connect(ui->lineEdit_vod, &QLineEdit::textChanged, this, &Diplom::validatePassword);
    setObjectName("Diplom");

    // Получаем экран
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = static_cast<int>(screenGeometry.width() * 0.7);
    int height = static_cast<int>(screenGeometry.height() * 0.7);

    // Устанавливаем размер и фиксируем его
    resize(width, height);
    setFixedSize(size());  // 🔒 Нельзя изменять размер

    // Центрируем
    move(screenGeometry.center() - rect().center());

    // Создаём Settings
    settingsWindow = new Settings(nullptr);

    // Загружаем цвет
    QSettings settings("MyCompany", "DiplomApp");
    QString savedColor = settings.value("BackgroundColor", "Красный").toString();
    settingsWindow->applyStyle(savedColor);

    // Инициализация модели
    fileModel = new QStandardItemModel(0, 4, this);  // 0 строк, 4 столбца
    fileModel->setHorizontalHeaderLabels({ "Номер", "Путь", "Состояние", "Метод" });
    ui->tableView->setModel(fileModel);

    // Настройка таблицы
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    fileCounter = 0;  // Начинаем с 1 при добавлении
    ui->lineEdit_vod->setPlaceholderText("Введите пароль (без пробелов)");
    ui->lineEdit_vod->setStyleSheet(
        "QLineEdit {"
        "   background: #333;"
        "   color: white;"
        "   border: 1px solid #555;"
        "   padding: 2px;"
        "}"
        );
    ui->lineEdit_vod->setMaxLength(64); // Ограничение длины
    // Где-то при инициализации (например, в конструкторе)
    ui->comboBox_algoritm->clear();
    ui->comboBox_algoritm->addItem("Кузнечик");
    ui->comboBox_algoritm->addItem("Магма");
}

Diplom::~Diplom()
{
    delete ui;
}
void Diplom::setupMessageBoxStyle(QMessageBox &msgBox) {
    msgBox.setStyleSheet(
        "QMessageBox {"
        "   background-color: #2b2b2b;"
        "   border: none;"
        "   font-family: 'Times New Roman';"
        "   font-size: 20px;"
        "}"
        "QLabel {"
        "   color: white;"
        "   font-family: 'Times New Roman';"
        "   font-size: 20px;"
        "   margin: 15px;"
        "   qproperty-alignment: AlignLeft | AlignVCenter;"
        "}"
        "QLabel#qt_msgbox_label {"
        "   color: white;"
        "   background-color: transparent;"
        "}"
        "QLabel#qt_msgboxtitlebar {"
        "   color: white;"
        "   background-color: #252525;"
        "   font-weight: bold;"
        "   padding: 8px;"
        "   border-bottom: 1px solid #444;"
        "}"
        "QTextEdit {"
        "   background-color: #333;"
        "   color: white;"
        "   font-family: 'Times New Roman';"
        "   font-size: 18px;"
        "   border: 1px solid #555;"
        "   padding: 10px;"
        "   border-radius: 4px;"
        "}"
        "QPushButton {"
        "   background-color: #4a4a4a;"
        "   color: white;"
        "   border: 1px solid #555;"
        "   padding: 8px 16px;"
        "   font-family: 'Times New Roman';"
        "   font-size: 20px;"
        "   min-width: 100px;"
        "   min-height: 30px;"
        "   border-radius: 6px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a5a5a;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3a3a3a;"
        "}"
        );
}
void Diplom::on_pushButton_5_clicked()  // Кнопка "Настройки"
{
    settingsWindow->showWindow();
}

void Diplom::on_pushButton_clicked()
{
    // Например, открытие окна настроек
    if (settingsWindow) {
        settingsWindow->showWindow();
    }
}


void Diplom::on_pushButton_port1_clicked()
{
        qDebug() << "Вызван port1";
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Выберите файлы",
        QDir::homePath(),
        "Все файлы (*.*)"
        );

    for (const QString &filePath : std::as_const(files)) {
        if (QFileInfo(filePath).isFile()) {
            addFileToTable(filePath);
        }
    }
}
void Diplom::on_pushButton_port2_clicked()
{
        qDebug() << "Вызван port2";
    QString dirPath = QFileDialog::getExistingDirectory(this, "Выберите папку", QDir::homePath());
    if (!dirPath.isEmpty()) {
        addFileToTable(dirPath, true);  // true = это папка
    }
}
void Diplom::processDirectory(const QString &dirPath, bool encrypt, const QString &algorithm)
{
    QDir dir(dirPath);
    if (!dir.exists()) return;

    // Рекурсивный обход
    QDirIterator it(dirPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();
        QString result = processFile(filePath, encrypt, algorithm);
        if (!result.isEmpty()) {
            // Удаляем старый путь, добавляем новый
            for (int row = 0; row < fileModel->rowCount(); ++row) {
                QStandardItem *item = fileModel->item(row, 1);
                if (item && item->toolTip() == filePath) {
                    updateTableRowWithPath(row, result, "", "");
                    break;
                }
            }
        }
    }
}
void Diplom::addFileToTable(const QString &path, bool isDir)
{
    fileCounter++;

    QFileInfo info(path);
    QString displayPath = isDir ? path + "/" : path;

    QList<QStandardItem*> row;

    // Колонка 0 — Номер
    QStandardItem *numberItem = new QStandardItem(QString::number(fileCounter));
    numberItem->setEditable(false);
    row << numberItem;

    // Колонка 1 — Путь с иконкой
    QStandardItem *pathItem = new QStandardItem(displayPath);
    QString iconPath = isDir ? ":/image/papka.png" : ":/image/file.png";
    pathItem->setIcon(QIcon(iconPath));
    pathItem->setEditable(false);
    pathItem->setToolTip(path);
    row << pathItem;

    // Колонка 2 — Состояние (пусто — обновится при обновлении)
    QStandardItem *statusItem = new QStandardItem("—");
    statusItem->setEditable(false);
    row << statusItem;

    // Колонка 3 — Метод
    QStandardItem *methodItem = new QStandardItem("—");
    methodItem->setEditable(false);
    row << methodItem;

    fileModel->appendRow(row);
}
void Diplom::on_pushButton_port3_clicked()
{
    QModelIndexList selection = ui->tableView->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() > b.row();
    });

    for (const QModelIndex &index : std::as_const(selection)) {
        fileModel->removeRow(index.row());
    }

    renumberRows();
}
void Diplom::on_pushButton_port4_clicked()
{
    fileModel->setRowCount(0);  // Очистить всё
    fileCounter = 0;  // Сбросить счётчик
}
void Diplom::renumberRows()
{
    for (int i = 0; i < fileModel->rowCount(); ++i) {
        fileModel->item(i, 0)->setText(QString::number(i + 1));
    }
    fileCounter = fileModel->rowCount();
}
void Diplom::on_pushButton_obn_clicked()
{
    for (int i = 0; i < fileModel->rowCount();) {
        QStandardItem *pathItem = fileModel->item(i, 1);
        if (!pathItem) {
            ++i;
            continue;
        }

        QString currentPath = pathItem->toolTip();
        QFileInfo info(currentPath);

        if (info.exists()) {
            // Файл существует — обновляем статус по его расширению
            updateRowStatus(i);
            ++i;
            continue;
        }

        // Файл не существует — ищем зашифрованную версию
        QString base = info.path() + "/" + info.completeBaseName();
        QString kuzPath = base + ".kuz";
        QString magPath = base + ".mag";
        QString encPath = base + ".enc";

        QString foundPath, alg;

        if (QFile::exists(kuzPath)) {
            foundPath = kuzPath;
            alg = "Кузнечик";
        } else if (QFile::exists(magPath)) {
            foundPath = magPath;
            alg = "Магма";
        } else if (QFile::exists(encPath)) {
            foundPath = encPath;
            alg = "Неизвестный";
        }

        if (!foundPath.isEmpty()) {
            // Удаляем старую строку
            fileModel->removeRow(i);

            // Вставляем новую на то же место
            QFileInfo newInfo(foundPath);
            QList<QStandardItem*> newRow;

            QStandardItem *numberItem = new QStandardItem(QString::number(i + 1));
            numberItem->setEditable(false);
            newRow << numberItem;

            QStandardItem *newPathItem = new QStandardItem(newInfo.fileName());
            newPathItem->setIcon(QIcon(":/image/file.png"));
            newPathItem->setEditable(false);
            newPathItem->setToolTip(foundPath);
            newRow << newPathItem;

            QStandardItem *statusItem = new QStandardItem("Зашифровано (" + alg + ")");
            statusItem->setIcon(QIcon(":/image/lock.png"));
            statusItem->setEditable(false);
            newRow << statusItem;

            QStandardItem *methodItem = new QStandardItem(alg);
            methodItem->setEditable(false);
            newRow << methodItem;

            fileModel->insertRow(i, newRow);
            ++i;
        } else {
            // Не найден и шифра нет
            QStandardItem *statusItem = fileModel->item(i, 2);
            statusItem->setText("Не найден");
            statusItem->setIcon(QIcon(":/image/error.png"));
            statusItem->setData(QColor(0xd32f2f), Qt::ForegroundRole);
            fileModel->item(i, 3)->setText("—");
            ++i;
        }
    }

    qDebug() << "Обновление завершено";
}
void Diplom::updateRowStatus(int row)
{
    QStandardItem *pathItem = fileModel->item(row, 1);
    if (!pathItem) return;

    QString path = pathItem->toolTip();
    QFileInfo info(path);
    QStandardItem *statusItem = fileModel->item(row, 2);
    QStandardItem *methodItem = fileModel->item(row, 3);

    QString suffix = info.suffix().toLower();

    if (suffix == "kuz") {
        statusItem->setText("Зашифровано (Кузнечик)");
        statusItem->setIcon(QIcon(":/image/lock.png"));
        methodItem->setText("Кузнечик");
    } else if (suffix == "mag") {
        statusItem->setText("Зашифровано (Магма)");
        statusItem->setIcon(QIcon(":/image/lock.png"));
        methodItem->setText("Магма");
    } else {
        QString base = info.path() + "/" + info.completeBaseName();
        if (QFile::exists(base + ".kuz")) {
            statusItem->setText("Расшифровано (был Кузнечик)");
            statusItem->setIcon(QIcon(":/image/unlock.png"));
            methodItem->setText("Кузнечик");
        } else if (QFile::exists(base + ".mag")) {
            statusItem->setText("Расшифровано (была Магма)");
            statusItem->setIcon(QIcon(":/image/unlock.png"));
            methodItem->setText("Магма");
        } else {
            statusItem->setText("Актуален");
            statusItem->setIcon(QIcon(":/image/ok.png"));
            methodItem->setText("—");
        }
    }
}
void Diplom::updateTableRowWithPath(int row, const QString &newPath, const QString & /*status*/, const QString & /*method*/)
{
    if (row < 0 || row >= fileModel->rowCount()) {
        qDebug() << "Некорректный номер строки:" << row;
        return;
    }

    QStandardItem *pathItem = fileModel->item(row, 1);
    if (!pathItem) {
        qDebug() << "Не найдена ячейка пути в строке" << row;
        return;
    }

    QFileInfo info(newPath);

    // Обновляем отображаемое имя и полный путь
    pathItem->setText(info.fileName());
    pathItem->setToolTip(newPath);

    // ✅ Пересчитываем статус и метод на основе нового расширения
    updateRowStatus(row);

    qDebug() << "Строка" << row << "обновлена:" << newPath;
}

void Diplom::on_pushButton_procedure_clicked()
{
    startProcedure();
}
void Diplom::on_pushButton_shifr_clicked()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        qDebug() << "Нет выбранных элементов для шифрования";
        return;
    }

    QString algorithm = ui->comboBox_algoritm->currentText();
    if (algorithm.isEmpty()) {
        qDebug() << "Не выбран алгоритм шифрования";
        return;
    }

    for (const QModelIndex &index : selected) {
        int row = index.row();
        QStandardItem *pathItem = fileModel->item(row, 1);
        if (!pathItem) continue;

        QString path = pathItem->toolTip();
        QFileInfo info(path);

        if (info.isDir()) {
            // 🔐 Это папка — обрабатываем рекурсивно
            processDirectory(path, true, algorithm);
        } else if (info.isFile()) {
            // 📄 Это файл — обычное шифрование
            QString suffix = info.suffix().toLower();
            if (suffix == "kuz" || suffix == "mag") {
                qDebug() << "Файл уже зашифрован:" << path;
                continue;
            }

            QString encryptedPath = processFile(path, true, algorithm);
            if (!encryptedPath.isEmpty()) {
                updateTableRowWithPath(row, encryptedPath, "", "");
            }
        }
    }
}


// ВАЖНО: Эту функцию нужно добавить в .cpp, но НЕ внутри processFile
QByteArray Diplom::generateRandom(int length) {
    QByteArray data(length, 0);
    for (int i = 0; i < length; ++i) {
        data[i] = static_cast<char>(QRandomGenerator::global()->generate() % 256);
    }
    return data;
}
template<typename Cipher>
QByteArray encryptCTR(const QByteArray &data, Cipher &cipher, const QByteArray &iv)
{
    QByteArray result;
    QByteArray counter = iv;
    int blockSize = cipher.blockSize(); // Должен быть метод blockSize()

    for (int i = 0; i < data.size(); i += blockSize) {
        QByteArray block = data.mid(i, blockSize);
        QByteArray encryptedCounter = cipher.encryptBlock(counter);
        if (encryptedCounter.isEmpty()) return QByteArray();

        for (int j = 0; j < block.size(); ++j) {
            result.append(encryptedCounter[j] ^ block[j]);
        }

        // Увеличиваем счётчик (младшие байты)
        for (int k = counter.size() - 1; k >= 0; --k) {
            counter[k]++;
            if (counter[k] != 0) break;
        }
    }
    return result;
}

template<typename Cipher>
QByteArray decryptCTR(const QByteArray &data, Cipher &cipher, const QByteArray &iv)
{
    return encryptCTR(data, cipher, iv); // CTR симметричен
}
QByteArray Diplom::hmacStreebog(const QByteArray &data, const QByteArray &key)
{
    // Ключ должен быть 32 байта (Streebog-256)
    QByteArray k = key;
    if (k.size() > 32) {
        Streebog hash(256);
        unsigned char* h = hash.hash((unsigned char*)k.data(), k.size());
        k = QByteArray((char*)h, 32);
    } else if (k.size() < 32) {
        k.resize(32, 0);
    }

    // iPad: 32 байта: key XOR 0x36
    QByteArray iPad(32, 0x36);
    for (int i = 0; i < 32; ++i) {
        iPad[i] ^= k[i];
    }

    // oPad: key XOR 0x5C
    QByteArray oPad(32, 0x5C);
    for (int i = 0; i < 32; ++i) {
        oPad[i] ^= k[i];
    }

    // HMAC = H(oPad || H(iPad || data))
    Streebog hash(256);
    QByteArray innerInput = iPad + data;
    unsigned char* innerHash = hash.hash((unsigned char*)innerInput.data(), innerInput.size());

    QByteArray outerInput = oPad + QByteArray((char*)innerHash, 32);
    unsigned char* outerHash = hash.hash((unsigned char*)outerInput.data(), outerInput.size());

    return QByteArray((char*)outerHash, 32);
}

QString Diplom::processFile(const QString &filePath, bool encrypt, const QString &algorithm)
{
    QFileInfo info(filePath);
    QString outPath;

    QString algExt;
    QString cleanAlg = algorithm.trimmed();
    qDebug() << "Алгоритм:" << algorithm << "→ clean:" << cleanAlg;

    if (cleanAlg == "Кузнечик") {
        algExt = ".kuz";
    } else if (cleanAlg == "Магма") {
        algExt = ".mag";
    } else {
        qDebug() << "Неизвестный алгоритм:" << algorithm;
        return QString();
    }

    if (encrypt) {
        outPath = info.path() + "/" + info.fileName() + algExt;
    } else {
        QString fileName = info.fileName();
        if (fileName.endsWith(".kuz") || fileName.endsWith(".mag")) {
            outPath = info.path() + "/" + fileName.left(fileName.length() - algExt.length());
        } else {
            return QString();
        }
    }

    QFile inFile(filePath);
    QFile outFile(outPath);

    if (!inFile.open(QIODevice::ReadOnly) || !outFile.open(QIODevice::WriteOnly)) {
        if (inFile.isOpen()) inFile.close();
        if (outFile.isOpen()) outFile.close();
        return QString();
    }

    QString password = ui->lineEdit_vod->text().trimmed();
    if (password.isEmpty()) {
        inFile.close();
        outFile.close();
        return QString();
    }

    QByteArray key;
    QByteArray salt, iv;

    if (encrypt) {
        // Генерация salt и IV
        salt = generateRandom(16);
        iv = generateRandom(cleanAlg == "Кузнечик" ? 16 : 8);

        // Запись в начало файла
        outFile.write(salt);
        outFile.write(iv);
    } else {
        // Чтение salt и IV из начала файла
        salt = inFile.read(16);
        iv = inFile.read(cleanAlg == "Кузнечик" ? 16 : 8);

        if (salt.size() != 16 || iv.size() != (cleanAlg == "Кузнечик" ? 16 : 8)) {
            inFile.close();
            outFile.close();
            return QString();
        }
    }

    // PBKDF: 1000 итераций Streebog(salt + key)
    key = password.toUtf8();
    for (int i = 0; i < 1000; ++i) {
        Streebog streebog(256);
        QByteArray input = salt + key;
        unsigned char* hash = streebog.hash((unsigned char*)input.data(), input.size());
        key = QByteArray((char*)hash, 32);
    }

    // Теперь key готов — можно шифровать/расшифровывать
    if (cleanAlg == "Кузнечик") {
        Kuznechik kuz;
        if (!kuz.setKey(key)) {
            inFile.close();
            outFile.close();
            return QString();
        }

        if (encrypt) {
            QByteArray plaintext = inFile.readAll();
            int padLen = 16 - (plaintext.size() % 16);
            if (padLen == 0) padLen = 16;
            plaintext.append(QByteArray(padLen, static_cast<char>(padLen)));

            QByteArray ciphertext = encryptCTR(plaintext, kuz, iv);
            if (ciphertext.isEmpty()) {
                inFile.close();
                outFile.close();
                return QString();
            }

            // Записываем: salt + iv + ciphertext + hmac
            outFile.write(ciphertext);
            QByteArray hmac = hmacStreebog(ciphertext, key);
            outFile.write(hmac);
        } else {
            // Читаем: salt + iv + ciphertext + hmac (32 байта)
            QByteArray ciphertext = inFile.readAll();
            if (ciphertext.size() <= 32) {
                inFile.close();
                outFile.close();
                return QString();
            }

            QByteArray hmac = ciphertext.right(32);
            ciphertext.chop(32);

            // Проверяем HMAC
            QByteArray expectedHmac = hmacStreebog(ciphertext, key);
            if (expectedHmac != hmac) {
                qDebug() << "HMAC не совпадает Файл подделан или повреждён.";
                inFile.close();
                outFile.close();
                return QString();
            }

            QByteArray decrypted = decryptCTR(ciphertext, kuz, iv);
            if (decrypted.isEmpty()) {
                inFile.close();
                outFile.close();
                return QString();
            }

            quint8 pad = decrypted[decrypted.size() - 1];
            if (pad > 0 && pad <= 16) {
                bool valid = true;
                for (int i = decrypted.size() - pad; i < decrypted.size(); ++i) {
                    if (decrypted[i] != pad) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    decrypted.chop(pad);
                }
            }
            outFile.write(decrypted);
        }


    } else if (cleanAlg == "Магма") {
        Magma magma;
        if (!magma.setKey(key)) {
            inFile.close();
            outFile.close();
            return QString();
        }

        if (encrypt) {
            QByteArray plaintext = inFile.readAll();
            int padLen = 8 - (plaintext.size() % 8);
            if (padLen == 0) padLen = 8;
            plaintext.append(QByteArray(padLen, static_cast<char>(padLen)));

            QByteArray ciphertext = encryptCTR(plaintext, magma, iv);
            if (ciphertext.isEmpty()) {
                inFile.close();
                outFile.close();
                return QString();
            }

            outFile.write(ciphertext);
            QByteArray hmac = hmacStreebog(ciphertext, key);
            outFile.write(hmac);
        } else {
            QByteArray ciphertext = inFile.readAll();
            if (ciphertext.size() <= 32) {
                inFile.close();
                outFile.close();
                return QString();
            }

            QByteArray hmac = ciphertext.right(32);
            ciphertext.chop(32);

            QByteArray expectedHmac = hmacStreebog(ciphertext, key);
            if (expectedHmac != hmac) {
                qDebug() << "HMAC не совпадает Файл подделан или повреждён.";
                inFile.close();
                outFile.close();
                return QString();
            }

            QByteArray decrypted = decryptCTR(ciphertext, magma, iv);
            if (decrypted.isEmpty()) {
                inFile.close();
                outFile.close();
                return QString();
            }

            quint8 pad = decrypted[decrypted.size() - 1];
            if (pad > 0 && pad <= 8) {
                bool valid = true;
                for (int i = decrypted.size() - pad; i < decrypted.size(); ++i) {
                    if (decrypted[i] != pad) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    decrypted.chop(pad);
                }
            }
            outFile.write(decrypted);
        }
    }
    inFile.close();
    outFile.close();
    QFile::remove(filePath);
    return outPath;
}


void Diplom::processFiles(const QList<QString> &files, bool encrypt, const QString &algorithm)
{
    int total = files.size();
    ui->progressBar_rabota->setRange(0, total);
    ui->progressBar_rabota->setValue(0);

    for (int i = 0; i < total; ++i) {
        QString filePath = files[i];
        QString result = processFile(filePath, encrypt, algorithm);

        if (result.isEmpty()) {
            QMessageBox msgBox(this);
            msgBox.setWindowTitle("Ошибка");
            msgBox.setText(QString("Ошибка при обработке:\n%1").arg(filePath));
            msgBox.setIcon(QMessageBox::Critical);
            setupMessageBoxStyle(msgBox);
            msgBox.exec();
            break;
        }

        ui->progressBar_rabota->setValue(i + 1);
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Готово");
    msgBox.setText("Операция завершена!");
    msgBox.setIcon(QMessageBox::Information);
    setupMessageBoxStyle(msgBox);
    msgBox.exec();
    ui->progressBar_rabota->reset();
}

void Diplom::startProcedure()
{
    QList<QString> files;
    QStringList errors;

    if (!ui->shifr->isChecked() && !ui->rashifr->isChecked()) {
        errors << "• Выберите режим: шифрование или расшифрование";
    }
    bool isEncrypt = ui->shifr->isChecked();

    if (!ui->one_file->isChecked() && !ui->all_file->isChecked()) {
        errors << "• Выберите: обработать один файл или все";
    }
    bool processOne = ui->one_file->isChecked();

    QString algorithm = ui->comboBox_algoritm->currentText().trimmed();
    if (algorithm.isEmpty()) {
        errors << "• Выберите алгоритм шифрования из списка";
    }

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model || model->rowCount() == 0) {
        errors << "• Таблица пуста — добавьте файлы";
        goto showErrors;
    }

    if (processOne) {
        QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
        if (selected.isEmpty()) {
            errors << "• Выберите файл в таблице";
            goto showErrors;
        }
    }

    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem *item = model->item(i, 1);
        if (!item) continue;

        QString path = item->toolTip();
        QFileInfo info(path);

        if (info.isDir()) {
            QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                files.append(it.next());
            }
        } else if (info.isFile()) {
            files.append(path);
        }
    }


    if (processOne) {
        files.clear();
        QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
        for (const QModelIndex &index : std::as_const(selected)) {
            QStandardItem *item = model->item(index.row(), 1);
            if (item) {
                QString path = item->toolTip();
                if (!path.isEmpty()) {
                    files.append(path);
                }
            }
        }
    }



    for (const QString &file : files) {
        if (!QFile::exists(file)) {
            errors << QString("• Файл не найден:\n%1").arg(file);
        }
    }

showErrors:
    if (!errors.isEmpty()) {
        QDialog dlg(this);
        dlg.setWindowTitle("Ошибка");
        dlg.setModal(true);
        dlg.resize(500, 300);
        dlg.setStyleSheet(
            "QDialog {"
            "   background-color: #2b2b2b;"
            "   font-family: 'Times New Roman';"
            "   font-size: 20px;"
            "}"
            "QLabel {"
            "   color: white;"
            "   font-family: 'Times New Roman';"
            "   font-size: 20px;"
            "}"
            "QTextEdit {"
            "   background-color: #333;"
            "   color: white;"
            "   font-family: 'Times New Roman';"
            "   font-size: 18px;"
            "   border: 1px solid #555;"
            "   padding: 10px;"
            "   border-radius: 4px;"
            "}"
            "QPushButton {"
            "   background-color: #4a4a4a;"
            "   color: white;"
            "   border: 1px solid #555;"
            "   padding: 8px 16px;"
            "   font-family: 'Times New Roman';"
            "   font-size: 20px;"
            "   min-width: 100px;"
            "   min-height: 30px;"
            "   border-radius: 6px;"
            "}"
            "QPushButton:hover {"
            "   background-color: #5a5a5a;"
            "}"
            "QPushButton:pressed {"
            "   background-color: #3a3a3a;"
            "}"
            );

        QVBoxLayout *layout = new QVBoxLayout(&dlg);
        QLabel *title = new QLabel("Исправьте следующие проблемы:");
        title->setStyleSheet("font-weight: bold; margin: 15px; color: white;");
        layout->addWidget(title);

        QTextEdit *details = new QTextEdit();
        details->setPlainText(errors.join("\n\n"));
        details->setReadOnly(true);
        details->setFrameStyle(QFrame::NoFrame);
        layout->addWidget(details, 1);

        QPushButton *btn = new QPushButton("ОК");
        btn->setFixedSize(100, 40);
        connect(btn, &QPushButton::clicked, &dlg, &QDialog::accept);
        layout->addWidget(btn, 0, Qt::AlignCenter);

        dlg.exec();
        return;
    }

    processFiles(files, isEncrypt, algorithm);
}

void Diplom::updateLineEditStyle(bool hasError)
{
    QString baseStyle =
        "QLineEdit {"
        "   background: white;"
        "   color: white;"
        "   padding: 2px;"
        "   border-radius: 4px;"
        "}";

    if (hasError) {
        ui->lineEdit_vod->setStyleSheet(
            baseStyle +
            "QLineEdit { border: 1px solid red; }"
            "QLineEdit::placeholder { color: #aaa; }"
            );
    } else {
        ui->lineEdit_vod->setStyleSheet(
            baseStyle +
            "QLineEdit { border: 1px solid white; }"
            "QLineEdit::placeholder { color: #aaa; }"
            );
    }
}

void Diplom::validatePassword()
{
    QString password = ui->lineEdit_vod->text();

    if (password.contains(' ')) {
        QString cleaned = password;
        cleaned.remove(' ');
        ui->lineEdit_vod->blockSignals(true);
        ui->lineEdit_vod->setText(cleaned);
        ui->lineEdit_vod->blockSignals(false);

        updateLineEditStyle(true);

        QTimer::singleShot(1500, this, [this]() {
            if (!ui->lineEdit_vod->text().contains(' ')) {
                updateLineEditStyle(false);
            }
        });
    } else {
        updateLineEditStyle(false);
    }
}

void Diplom::on_pushButton_passw_clicked()
{
    PasswordDialog dlg(this);
    dlg.setWindowTitle("Введите пароль");

    if (dlg.exec() == QDialog::Accepted) {
        QByteArray hash = dlg.getPasswordHash();  // 32 байта (Streebog-256)

        // Преобразуем хэш в hex-строку (например, "a3e7d...b2c8f")
        QString hashHex = QString::fromLatin1(hash.toHex());

        // Устанавливаем hex-представление хэша
        ui->lineEdit_vod->setText(hashHex);

        // Блокируем редактирование
        ui->lineEdit_vod->setReadOnly(true);
        ui->lineEdit_vod->setFocusPolicy(Qt::NoFocus);
        ui->lineEdit_vod->setStyleSheet("QLineEdit { background: lightgray; }");
    }
}
