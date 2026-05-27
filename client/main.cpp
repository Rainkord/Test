#include <QApplication>
#include "mainwindow.h"

/**
 * @brief Точка входа в приложение
 * 
 * Создаёт экземпляр QApplication, устанавливает имя и версию
 * приложения, создаёт и отображает главное окно.
 * 
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код завершения приложения
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("ТМП — Подгруппа 5");
    a.setApplicationVersion("1.0");

    MainWindow w;
    w.show();

    return a.exec();
}
