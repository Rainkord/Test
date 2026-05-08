/**
 * @file taskdialog.h
 * @brief Диалоговое окно с описанием задания.
 */

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>

/**
 * @class TaskDialog
 * @brief Модальный диалог, отображающий условие задания.
 */
class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = nullptr);
    ~TaskDialog();

private:
    QLabel      *titleLabel;
    QLabel      *workTitleLabel;
    QLabel      *groupLabel;
    QLabel      *membersLabel;
    QLabel      *formulaLabel;
    QPushButton *closeBtn;

    void setupUI();
};

#endif // TASKDIALOG_H
