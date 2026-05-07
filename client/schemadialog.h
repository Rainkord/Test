/**
 * @file schemadialog.h
 * @brief Диалоговое окно со схемой архитектуры приложения.
 */

#ifndef SCHEMADIALOG_H
#define SCHEMADIALOG_H

#include <QDialog>
#include <QLabel>

/**
 * @class SchemaDialog
 * @brief Модальный диалог, отображающий структурную схему клиент-серверного приложения.
 *
 * Показывает диаграмму взаимодействия компонентов:
 * клиентских виджетов, сетевого слоя и серверной части.
 */
class SchemaDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор. Создаёт и компонует содержимое диалога.
     * @param parent Родительский виджет.
     */
    explicit SchemaDialog(QWidget *parent = nullptr);

private:
    QLabel *schemaLabel; ///< Метка для отображения изображения схемы.
};

#endif // SCHEMADIALOG_H
