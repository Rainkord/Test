/**
 * @file taskdialog.h
 * @brief Диалоговое окно с описанием задания.
 */

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>

/**
 * @class TaskDialog
 * @brief Модальный диалог, отображающий условие задания №9.
 *
 * Показывает текст задания, формулу кусочной функции и
 * требования к реализации клиент-серверного приложения.
 */
class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор. Создаёт и компонует содержимое диалога.
     * @param parent Родительский виджет.
     */
    explicit TaskDialog(QWidget *parent = nullptr);
};

#endif // TASKDIALOG_H
