/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#ifndef DIPLOM_H
#define DIPLOM_H

#include <QMainWindow>
#include "ui_diplom.h"
#include <QThread>
#include <QTimer>
#include <QMessageBox>
#include "settings.h"  // Подключение класса Settings
#include "passworddialog.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <QFileInfo>
class Settings;

QT_BEGIN_NAMESPACE
namespace Ui {
class Diplom;
}
QT_END_NAMESPACE

class Diplom : public QMainWindow
{
    Q_OBJECT

public:
    Diplom(QWidget *parent = nullptr);
    ~Diplom();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_5_clicked();
    void startProcedure();
    void on_pushButton_port1_clicked();
    void on_pushButton_port2_clicked();  // Добавить папку
    void on_pushButton_port3_clicked();  // Удалить выбранный
    void on_pushButton_port4_clicked();  // Удалить все

    void on_pushButton_obn_clicked();
    void validatePassword();
    void on_pushButton_procedure_clicked();   
    void on_pushButton_passw_clicked();

private:
    Ui::Diplom *ui;
    Settings *settingsWindow = nullptr;  // Инициализируем nullptr
    QStandardItemModel *fileModel;  // Модель для tableView
    int fileCounter;  // Счётчик для нумерации
    // Вспомогательные методы
    void addFileToTable(const QString &path, bool isDir = false);
    void renumberRows();
    void processFiles(const QList<QString> &files, bool encrypt, const QString &algorithm);
    QString processFile(const QString &filePath, bool encrypt, const QString &algorithm);
    void updateRowStatus(int row);
    void updateTableRowWithPath(int row, const QString &newPath, const QString &status, const QString &method);
    void on_pushButton_shifr_clicked();
    void setupMessageBoxStyle(QMessageBox &msgBox);
    void updateLineEditStyle(bool hasError = false);
    QByteArray generateRandom(int length);
    QByteArray hmacStreebog(const QByteArray &data, const QByteArray &key);

    QString generatePassword();
    int checkPasswordStrength(const QString &pass);
    void updatePasswordStrengthIndicator(int strength);
    void processDirectory(const QString &dirPath, bool encrypt, const QString &algorithm);

};
#endif // DIPLOM_H
