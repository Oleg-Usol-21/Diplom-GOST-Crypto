/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

// Подключаем сгенерированный UI
#include "ui_passworddialog.h"

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    ~PasswordDialog();

    QString getPassword() const;
    bool wasAccepted() const;

private slots:
    void on_generateButton_clicked();
    void on_toggleVisible_toggled(bool checked);
    void on_passwordEdit_textChanged(const QString &text);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::PasswordDialog *ui;  // ← теперь это правильно
    bool accepted;
    QByteArray m_passwordHash;  // ← добавь это
    QString generatePassword();
    int checkPasswordStrength(const QString &pass);
    void updateStrengthIndicator(int strength);


public:
        QByteArray getPasswordHash() const;  // ← добавь

};

#endif // PASSWORDDIALOG_H
