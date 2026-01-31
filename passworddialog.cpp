/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#include "passworddialog.h"
#include "crypto/striborg.h"
#include <QRandomGenerator>
#include <QMessageBox>
#include <QRegularExpression>

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog),
    accepted(false)
{
    ui->setupUi(this);
    setWindowTitle("Пароль");

    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->spinBox->setMinimum(8);
    ui->spinBox->setMaximum(128);
    ui->spinBox->setValue(16);

    ui->lowercaseCheckBox->setChecked(true);
    ui->uppercaseCheckBox->setChecked(true);
    ui->digitsCheckBox->setChecked(true);
    ui->specialCheckBox->setChecked(true);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Принять");
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");

    connect(ui->passwordEdit, &QLineEdit::textChanged,
            this, &PasswordDialog::on_passwordEdit_textChanged);

    updateStrengthIndicator(0);
}


PasswordDialog::~PasswordDialog()
{
    delete ui;
}

QString PasswordDialog::getPassword() const
{
    return ui->passwordEdit->text().trimmed();
}

bool PasswordDialog::wasAccepted() const
{
    return accepted;
}

QString PasswordDialog::generatePassword()
{
    int length = ui->spinBox->value();
    QString chars;

    if (ui->lowercaseCheckBox->isChecked())
        chars += "abcdefghijklmnopqrstuvwxyz";
    if (ui->uppercaseCheckBox->isChecked())
        chars += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (ui->digitsCheckBox->isChecked())
        chars += "0123456789";
    if (ui->specialCheckBox->isChecked())
        chars += "!#$%&()*+-.:/;<=>?@[]^_`{|}~";

    if (chars.isEmpty())
        return "";

    QString pass;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.size());
        pass.append(chars[index]);
    }
    return pass;
}

void PasswordDialog::on_generateButton_clicked()
{
    QString pass = generatePassword();
    if (pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы один набор символов.");
        return;
    }
    ui->passwordEdit->setText(pass);
    ui->passwordEdit->setFocus();
}


void PasswordDialog::on_toggleVisible_toggled(bool checked)
{
    ui->passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}


int PasswordDialog::checkPasswordStrength(const QString &pass)
{
    if (pass.length() < 8) return 0;

    int score = 0;
    if (pass.length() >= 12) score++;
    if (pass.contains(QRegularExpression("[a-z]"))) score++;
    if (pass.contains(QRegularExpression("[A-Z]"))) score++;
    if (pass.contains(QRegularExpression("[0-9]"))) score++;
    if (pass.contains(QRegularExpression("[#$%&()*+\\-.:/;<=>?@\\[\\]^_`{|}~]"))) score++;

    return score;
}



void PasswordDialog::updateStrengthIndicator(int strength)
{
    QLabel *label = ui->strengthLabel_2;  // ← исправлено на strengthLabel_2
    if (!label) return;

    if (strength < 3) {
        label->setText("Слабый");
        label->setStyleSheet("color: red;");
    } else if (strength < 5) {
        label->setText("Средний");
        label->setStyleSheet("color: orange;");
    } else {
        label->setText("Надёжный");
        label->setStyleSheet("color: green;");
    }
}



void PasswordDialog::on_passwordEdit_textChanged(const QString &text)
{
    int strength = checkPasswordStrength(text);
    updateStrengthIndicator(strength);
}
void PasswordDialog::on_buttonBox_accepted()
{
    QString pass = ui->passwordEdit->text().trimmed();
    if (pass.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Пароль не может быть пустым.");
        return;
    }

    int strength = checkPasswordStrength(pass);
    if (strength < 3) {
        int ret = QMessageBox::warning(this, "Слабый пароль",
                                       "Пароль слишком слабый. Всё равно использовать?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::No)
            return;
    }

    // 🔐 Хэшируем пароль через Streebog (256 или 512)
    Streebog streebog(256);  // или 512 — как требуется
    QByteArray passUtf8 = pass.toUtf8();
    unsigned char* hash = streebog.hash((unsigned char*)passUtf8.data(), passUtf8.size());

    // Копируем хэш в QByteArray (32 байта для Streebog-256)
    QByteArray hashBytes;
    for (int i = 0; i < 32; ++i) {
        hashBytes.append(static_cast<char>(hash[i]));
    }

    // Сохраняем хэш (например, в поле класса)
    m_passwordHash = hashBytes;

    accepted = true;
    accept();
}
QByteArray PasswordDialog::getPasswordHash() const
{
    return m_passwordHash;
}


void PasswordDialog::on_buttonBox_rejected()
{
    accepted = false;
    reject();
}
