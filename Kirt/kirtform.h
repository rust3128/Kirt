#ifndef KIRTFORM_H
#define KIRTFORM_H

#include "logmodel.h"
#include "logsyntaxhighlighter.h"
#include "logfilterproxymodel.h"

#include <QWidget>
#include <QColor>
#include <QList>
#include <QSortFilterProxyModel>
#include <QListWidgetItem>
#include <QMenu>
#include <QClipboard>
#include <QShortcut>



// Структура для зберігання інформації про завантажений файл
struct LogFileInfo {
    QString filePath;   // Повний шлях (D:/logs/OperatorLog.txt)
    QString fileName;   // Тільки ім'я (OperatorLog.txt)
    QColor color;       // Колір для підсвітки рядків
    bool isEnabled;     // Чи стоїть галочка в списку
};

namespace Ui {
class KirtForm;
}

class KirtForm : public QWidget
{
    Q_OBJECT

public:
    explicit KirtForm(QWidget *parent = nullptr);
    ~KirtForm();

protected:
    // Додаємо обробники подій Drag & Drop
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private slots:
    // Слот для обробки натискання на кнопку "+ Додати файли"
    void on_pushButtonAddFiles_clicked();
    // ДОДАЄМО: слот для кліку по таблиці
    void on_tableViewLog_clicked(const QModelIndex &index);
    // ДОДАЄМО: Новий обробник зміни виділеного рядка (працює і для миші, і для клавіатури)
    void onTableRowChanged(const QModelIndex &current, const QModelIndex &previous);

    void onFileItemChanged(QListWidgetItem *item);

    void onSearchTextChanged(const QString &text);

    void onTimeRangeChanged();
    void on_pushButtonResetFilter_clicked();

    void onTableViewContextMenu(const QPoint &pos);
    void copySelectedLogs();

private:
    void createUI();   // Початкове налаштування інтефесу.
    void createConnections();
    // Замість старого addFileToList тепер приймаємо список файлів
    void processFilesWithProgress(const QStringList& filePaths);
    QColor generatePastelColor(); // Генерує світлий колір для фону
    void updateFilter();
private:
    Ui::KirtForm *ui;
    QList<LogFileInfo> m_loadedFiles;

    LogModel *m_logModel;
    LogFilterProxyModel *m_proxyModel;
    LogSyntaxHighlighter *m_highlighter;
};

#endif // KIRTFORM_H
