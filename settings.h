/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include "ui_Settings.h"

class Settings : public QWidget
{
    Q_OBJECT
public:
    explicit Settings(QWidget *parent = nullptr);  // parent = nullptr — независимое окно
    ~Settings();

    void loadStyle();
    void applyStyle(const QString &colorName);

public slots:
    void on_pushButton_set_clicked();
    void showWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::Settings *ui;
};

#endif // SETTINGS_H

