/*
 * Diplom — шифрование файлов по ГОСТ с использованием Кузнечика и Магмы
 * Copyright (C) 2025 Олег Усольцев <jeep2036@mail.ru>
 *
 * Этот программный обеспечением распространяется на условиях
 * GNU General Public License версии 3 или более поздней.
 * Подробнее: https://www.gnu.org/licenses/gpl-3.0
 */
#include "settings.h"
#include <QCloseEvent>
#include <QSettings>
#include <QApplication>

static QMap<QString, QString> colorGradients() {
    return {
        {"Тёмно-серый", "background-color: #2b2b2b;"},
        {"Красный", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(135, 0, 0, 255), stop:0.5 rgba(80, 0, 0, 255), stop:1 rgba(0, 0, 0, 255));"},
        {"Синий", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(0, 0, 135, 255), stop:0.5 rgba(0, 0, 80, 255), stop:1 rgba(0, 0, 0, 255));"},
        {"Голубой", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(0, 135, 135, 255), stop:0.5 rgba(0, 80, 80, 255), stop:1 rgba(0, 0, 0, 255));"},
        {"Жёлтый", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(135, 135, 0, 255), stop:0.5 rgba(80, 80, 0, 255), stop:1 rgba(0, 0, 0, 255));"},
        {"Фиолетовый", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(135, 0, 135, 255), stop:0.5 rgba(80, 0, 80, 255), stop:1 rgba(0, 0, 0, 255));"},
        {"Чёрный", "background-color: qlineargradient(spread:pad, x1:1, y1:1, x2:0, y2:0, stop:0 rgba(30, 30, 30, 255), stop:1 rgba(0, 0, 0, 255));"}
    };
}

Settings::Settings(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Settings)
{
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    setWindowTitle("Настройки цвета");

    // Получаем размер экрана
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = static_cast<int>(screenGeometry.width() * 0.7);
    int height = static_cast<int>(screenGeometry.height() * 0.7);

    // Устанавливаем размер и делаем его фиксированным
    resize(width, height);
    setFixedSize(size());  // 🔒 Нельзя изменять размер

    ui->setupUi(this);
    // Стиль для comboBox_set — чтобы текст был белым и читаемым
    ui->comboBox_set->setStyleSheet(
        "QComboBox {"
        "   background-color: #3a3a3a;"
        "   border: 1px solid #555;"
        "   color: white;"
        "   padding: 8px;"
        "   min-height: 20px;"
        "   font-size: 16px;"
        "}"
        "QComboBox::drop-down {"
        "   border: 0;"
        "}"
        "QComboBox::down-arrow {"
        "   image: none;"
        "   width: 12px;"
        "   height: 12px;"
        "   right: 10px;"
        "}"
        // 🔥 Вот ключ — стиль выпадающего списка
        "QComboBox QAbstractItemView {"
        "   background-color: #333;"
        "   color: white;"
        "   selection-background-color: #5a5a5a;"
        "   selection-color: white;"
        "   outline: none;"
        "   padding: 5px;"
        "   border: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "   min-height: 25px;"
        "   padding: 5px;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "   background-color: #5a5a5a;"
        "   color: white;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "   background-color: #4a4a4a;"
        "}"
        );
    // Добавляем цвета в комбобокс
    ui->comboBox_set->addItems({"Тёмно-серый", "Красный", "Синий", "Голубой", "Жёлтый", "Фиолетовый", "Чёрный"});

    loadStyle();
}


Settings::~Settings()
{
    delete ui;
}

void Settings::loadStyle()
{
    QSettings settings("MyCompany", "DiplomApp");
    QString savedColor = settings.value("BackgroundColor", "Красный").toString();

    applyStyle(savedColor);

    int index = ui->comboBox_set->findText(savedColor);
    if (index != -1) {
        ui->comboBox_set->setCurrentIndex(index);
    }
}


void Settings::applyStyle(const QString &colorName)
{
    const QMap<QString, QString> gradients = colorGradients();  // Сохраняем один раз
    QString style = gradients.value(colorName, gradients.value("Красный"));

    setStyleSheet(style);

    for (QWidget *widget : QApplication::topLevelWidgets()) {
        if (widget->objectName() == "Diplom") {
            widget->setStyleSheet(style);
        }
    }
}

void Settings::on_pushButton_set_clicked()
{
    QString color = ui->comboBox_set->currentText();
    applyStyle(color);

    QSettings settings("MyCompany", "DiplomApp");
    settings.setValue("BackgroundColor", color);

    // Закрываем окно
    close();  // close() → вызовет closeEvent → hide()
}


void Settings::showWindow()
{
    // Обновляем позицию: на случай, если экран изменился
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.center() - rect().center());

    show();
    raise();
    activateWindow();
}

void Settings::closeEvent(QCloseEvent *event)
{
    hide();           // Скрываем
    event->ignore();  // Не уничтожаем
}
