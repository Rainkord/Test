#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

/**
 * @brief Диалоговое окно с информацией о задании
 *
 * Отображает описание задания, название работы, номер подгруппы,
 * список участников и формулу функции. Используется как справочный
 * экран для пользователя.
 */
class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор диалога задания
     * @param parent родительский виджет
     */
    explicit TaskDialog(QWidget *parent = nullptr);

    /// Деструктор диалога
    ~TaskDialog();

private:
    QLabel      *titleLabel;      ///< Метка заголовка «Задание»
    QLabel      *workTitleLabel;  ///< Метка с названием работы
    QLabel      *groupLabel;      ///< Метка с номером подгруппы
    QLabel      *membersLabel;    ///< Метка со списком участников
    QLabel      *formulaLabel;    ///< Метка с изображением формулы
    QPushButton *closeBtn;        ///< Кнопка «Закрыть»

    /// Инициализация пользовательского интерфейса диалога
    void setupUI();
};

#endif // TASKDIALOG_H
