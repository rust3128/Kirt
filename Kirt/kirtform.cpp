#include "kirtform.h"
#include "ui_kirtform.h"

#include "operatorlogparser.h"
#include "optimumlogparser.h"
#include "bposlogparser.h"
#include "ecrcommxlogparser.h"
#include "cardpaylogparser.h"
#include "smartcardslogparser.h"
#include "talonshttplogparser.h"
#include "mposloyaltylogparser.h"
#include "loyaltylogparser.h"
#include "dlottologparser.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QRandomGenerator>
#include <QPixmap>
#include <QIcon>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QHeaderView>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QApplication>
#include <QSharedPointer>

KirtForm::KirtForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KirtForm)
{
    ui->setupUi(this);
    createUI();
    createConnections();
}

KirtForm::~KirtForm()
{
    delete ui;
}

void KirtForm::createUI()
{
    // 1. Налаштування головного горизонтального спліттера (ліва зона / права панель файлів)
    // Задаємо співвідношення приблизно 80% на 20%
    QList<int> mainSplitterSizes;
    mainSplitterSizes << 800 << 200;
    ui->splitter_2->setSizes(mainSplitterSizes);

    // 2. Налаштування внутрішнього вертикального спліттера (таблиця / деталі)
    // Задаємо співвідношення приблизно 70% на 30%
    QList<int> innerSplitterSizes;
    innerSplitterSizes << 700 << 300;
    ui->splitter->setSizes(innerSplitterSizes);

    // Опціонально: можна додати красивий початковий час для фільтрів,
    // щоб вони не стояли по нулях
    ui->timeEditStart->setTime(QTime(0, 0, 0));
    ui->timeEditEnd->setTime(QTime(23, 59, 59));

    // 1. Створюємо модель
    m_logModel = new LogModel(this);

    // === НОВИЙ КОД ДЛЯ СОРТУВАННЯ ===
    m_proxyModel = new LogFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_logModel); // Підключаємо нашу модель до проксі
    m_proxyModel->setSortRole(Qt::UserRole);  // Кажемо сортувати за нашими правилами (об'єктами QDateTime)

    // Тепер таблиця дивиться не на m_logModel, а на m_proxyModel!
    ui->tableViewLog->setModel(m_proxyModel);

    // === НОВИЙ КОД: Підключаємо підсвічувач до нижнього вікна ===
    m_highlighter = new LogSyntaxHighlighter(ui->textEditDetal->document());

    // Зробимо шрифт у нижньому вікні моноширинним (як у редакторах коду), щоб XML і JSON виглядали ідеально рівно
    QFont font = ui->textEditDetal->font();
    font.setFamily("Consolas"); // Або "Courier New"
    font.setPointSize(10);
    ui->textEditDetal->setFont(font);
    // =============================================================

    // Дозволяємо сортування по кліку на заголовок колонки
    ui->tableViewLog->setSortingEnabled(true);

    // Сортуємо за замовчуванням по першій колонці (Час) за зростанням
    ui->tableViewLog->sortByColumn(0, Qt::AscendingOrder);
    // ================================

    // 3. Робимо таблицю красивою та зручною
    // Виділяти весь рядок, а не одну клітинку
    ui->tableViewLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Виділяти лише один рядок за раз
    ui->tableViewLog->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // === НОВЕ: Налаштування контекстного меню ===
    ui->tableViewLog->setContextMenuPolicy(Qt::CustomContextMenu);


    // === НОВЕ: Гаряча клавіша Ctrl+C для таблиці ===
    QShortcut *copyShortcut = new QShortcut(QKeySequence::Copy, ui->tableViewLog);
    copyShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(copyShortcut, &QShortcut::activated, this, &KirtForm::copySelectedLogs);
    // ==========================================================


    // Ховаємо нумерацію рядків зліва (вона нам не потрібна)
    ui->tableViewLog->verticalHeader()->setVisible(false);
    // Остання колонка (Повідомлення) розтягується на весь вільний простір
    ui->tableViewLog->horizontalHeader()->setStretchLastSection(true);

    this->setAcceptDrops(true);
}

void KirtForm::createConnections()
{
    // Підключаємо обробник навігації по таблиці ===
    connect(ui->tableViewLog->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &KirtForm::onTableRowChanged);

    connect(ui->listWidgetFile, &QListWidget::itemChanged,
            this, &KirtForm::onFileItemChanged);

    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            this, &KirtForm::onSearchTextChanged);

    connect(ui->timeEditStart, &QTimeEdit::timeChanged, this, &KirtForm::onTimeRangeChanged);
    connect(ui->timeEditEnd, &QTimeEdit::timeChanged, this, &KirtForm::onTimeRangeChanged);
    connect(ui->pushButtonResetFilter, &QPushButton::clicked, this, &KirtForm::on_pushButtonResetFilter_clicked);

    connect(ui->tableViewLog, &QWidget::customContextMenuRequested,
            this, &KirtForm::onTableViewContextMenu);
}

void KirtForm::on_pushButtonAddFiles_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this, "Виберіть лог-файли", "", "Log Files (*.log *.txt);;All Files (*.*)", nullptr, QFileDialog::DontUseNativeDialog
        );
    // Просто передаємо весь список у наш новий метод!
    processFilesWithProgress(files);
}



// Генератор світлих кольорів (щоб чорний текст добре читався поверх них)
QColor KirtForm::generatePastelColor()
{
    // Використовуємо HSV:
    // H (Hue/Відтінок): від 0 до 359. Крок 137 градусів для кожного нового файлу.
    // S (Saturation/Насиченість): 70 (досить блідо, щоб чорний текст читався).
    // V (Value/Яскравість): 255 (максимально світлий фон).

    int h = (m_loadedFiles.size() * 137) % 360;

    return QColor::fromHsv(h, 70, 255);
}

// "Охоронець": перевіряє, чи ми тягнемо файли
void KirtForm::dragEnterEvent(QDragEnterEvent *event)
{
    // Якщо те, що ми тягнемо, містить URL (тобто це файли з провідника)
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction(); // Дозволяємо курсору змінити вигляд на "Копіювання"
    } else {
        event->ignore(); // Інакше ігноруємо (наприклад, якщо тягнуть просто текст)
    }
}

// "Приймальник": спрацьовує, коли користувач відпускає кнопку миші
void KirtForm::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList files;
        for (const QUrl &url : mimeData->urls()) {
            if (url.isLocalFile()) {
                files.append(url.toLocalFile());
            }
        }
        processFilesWithProgress(files); // Віддаємо список сюди!
        event->acceptProposedAction();
    }
}

void KirtForm::on_tableViewLog_clicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    // Переводимо відсортований індекс таблиці у справжній індекс нашої моделі даних
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

    // Витягуємо запис використовуючи справжній індекс
    LogEntry entry = m_logModel->getEntry(sourceIndex.row());

    ui->textEditDetal->setPlainText(entry.fullText);
}

void KirtForm::processFilesWithProgress(const QStringList& filePaths)
{
    if (filePaths.isEmpty()) return;

    // Створюємо список парсерів ОДИН РАЗ перед циклом
    QList<QSharedPointer<ILogParser>> parsers;
    parsers.append(QSharedPointer<ILogParser>(new OperatorLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new BPosLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new OptimumLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new EcrCommXLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new CardPayLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new SmartCardsLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new TalonsHttpLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new MposLoyaltyLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new LoyaltyLogParser()));
    parsers.append(QSharedPointer<ILogParser>(new DLottoLogParser()));

    QDialog progressDialog(this);
    progressDialog.setWindowTitle("Завантаження файлів...");
    progressDialog.setMinimumWidth(450);
    progressDialog.setWindowModality(Qt::WindowModal);

    QVBoxLayout *layout = new QVBoxLayout(&progressDialog);
    QLabel *overallLabel = new QLabel("Загальний прогрес:", &progressDialog);
    QProgressBar *overallBar = new QProgressBar(&progressDialog);
    overallBar->setRange(0, filePaths.count());
    overallBar->setValue(0);

    QLabel *fileLabel = new QLabel("Читання файлу...", &progressDialog);
    QProgressBar *fileBar = new QProgressBar(&progressDialog);
    fileBar->setRange(0, 100);
    fileBar->setValue(0);

    layout->addWidget(overallLabel);
    layout->addWidget(overallBar);
    layout->addWidget(fileLabel);
    layout->addWidget(fileBar);

    progressDialog.show();
    int currentFileIndex = 0;

    for (const QString& filePath : filePaths) {
        fileLabel->setText(QString("Перевірка: %1").arg(QFileInfo(filePath).fileName()));
        fileBar->setValue(0);
        QApplication::processEvents();

        // 1. Захист від дублікатів
        bool exists = false;
        for (const auto& info : m_loadedFiles) {
            if (info.filePath == filePath) { exists = true; break; }
        }
        if (exists) {
            currentFileIndex++;
            overallBar->setValue(currentFileIndex);
            continue;
        }

        // 2. === НОВИЙ ПІДХІД: ШУКАЄМО ПАРСЕР ДО ДОДАВАННЯ В ІНТЕРФЕЙС ===
        QSharedPointer<ILogParser> matchedParser;
        for (auto& parser : parsers) {
            if (parser->canParse(filePath)) {
                matchedParser = parser;
                break;
            }
        }

        // Якщо парсер не знайдено - цей файл нам не підходить. Пропускаємо його повністю!
        if (!matchedParser) {
            qDebug() << "Файл проігноровано (немає парсера або невірний формат):" << filePath;
            currentFileIndex++;
            overallBar->setValue(currentFileIndex);
            continue;
        }

        // 3. Парсер є! Тепер можна безпечно додавати файл у візуальний список
        LogFileInfo info;
        info.filePath = filePath;
        info.fileName = QFileInfo(filePath).fileName();
        info.color = generatePastelColor();
        info.isEnabled = true;
        m_loadedFiles.append(info);

        auto *item = new QListWidgetItem(info.fileName, ui->listWidgetFile);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        QPixmap pixmap(16, 16);
        pixmap.fill(info.color);
        item->setIcon(QIcon(pixmap));
        item->setData(Qt::UserRole, filePath);

        // 4. Читаємо файл знайденим парсером
        fileLabel->setText(QString("Читання: %1").arg(info.fileName));
        QList<LogEntry> entries = matchedParser->parseFile(filePath, [&fileBar](int percent) {
            fileBar->setValue(percent);
            QApplication::processEvents();
        });

        m_logModel->addEntries(entries, info.color);

        currentFileIndex++;
        overallBar->setValue(currentFileIndex);
        QApplication::processEvents();
    }
    updateFilter(); // Оновлюємо фільтр після завантаження нових файлів
    ui->tableViewLog->resizeColumnsToContents();
}

// Обробник виділення рядка (Мишка + Клавіатура)
void KirtForm::onTableRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous); // Попередній індекс нам не потрібен, тому кажемо компілятору ігнорувати його

    // Якщо нічого не виділено - виходимо
    if (!current.isValid()) return;

    // Переводимо відсортований індекс таблиці у справжній індекс нашої моделі даних
    QModelIndex sourceIndex = m_proxyModel->mapToSource(current);

    // Витягуємо запис
    LogEntry entry = m_logModel->getEntry(sourceIndex.row());

    // Виводимо повний текст у нижнє вікно
    ui->textEditDetal->setPlainText(entry.fullText);
}

// Викликається щоразу, коли користувач ставить або знімає галочку
void KirtForm::onFileItemChanged(QListWidgetItem *item)
{
    // Отримуємо шлях до файлу з елемента списку
    QString filePath = item->data(Qt::UserRole).toString();
    bool isChecked = (item->checkState() == Qt::Checked);

    // Оновлюємо статус (увімкнено/вимкнено) у нашій базі файлів
    for (auto& info : m_loadedFiles) {
        if (info.filePath == filePath) {
            info.isEnabled = isChecked;
            break;
        }
    }

    // Запускаємо перерахунок фільтра
    updateFilter();
}

// Збирає всі активні файли і віддає їх фільтру
void KirtForm::updateFilter()
{
    QSet<QString> enabledFiles;
    for (const auto& info : m_loadedFiles) {
        if (info.isEnabled) {
            enabledFiles.insert(info.filePath);
        }
    }
    // Віддаємо набір у ProxyModel. Вона сама миттєво сховає або покаже рядки!
    m_proxyModel->setEnabledFiles(enabledFiles);
}

void KirtForm::onSearchTextChanged(const QString &text)
{
    // Просто передаємо введений текст у наш розумний фільтр
    m_proxyModel->setSearchText(text);
}


// Обробник зміни часу в будь-якому з двох полів
void KirtForm::onTimeRangeChanged()
{
    QTime start = ui->timeEditStart->time();
    QTime end = ui->timeEditEnd->time();

    // === КОНТРОЛЬ ДІАПАЗОНУ ===
    // Тимчасово блокуємо сигнали, щоб не викликати нескінченний цикл змін
    ui->timeEditStart->blockSignals(true);
    ui->timeEditEnd->blockSignals(true);

    if (start > end) {
        // Якщо користувач крутить "Початок" вперед - суваємо і "Кінець"
        if (sender() == ui->timeEditStart) {
            ui->timeEditEnd->setTime(start);
            end = start;
        }
        // Якщо крутить "Кінець" назад - суваємо і "Початок"
        else if (sender() == ui->timeEditEnd) {
            ui->timeEditStart->setTime(end);
            start = end;
        }
    }

    ui->timeEditStart->blockSignals(false);
    ui->timeEditEnd->blockSignals(false);
    // ========================

    // Передаємо правильний діапазон у фільтр
    m_proxyModel->setTimeRange(start, end);
}

// Обробник кнопки "Скинути"
void KirtForm::on_pushButtonResetFilter_clicked()
{
    // 1. Очищаємо текст
    ui->lineEditSearch->clear(); // Це автоматично викличе onSearchTextChanged і скине текстовий фільтр

    // 2. Скидаємо час (блокуємо сигнали, щоб фільтр не перемальовувався двічі)
    ui->timeEditStart->blockSignals(true);
    ui->timeEditEnd->blockSignals(true);

    ui->timeEditStart->setTime(QTime(0, 0, 0));
    ui->timeEditEnd->setTime(QTime(23, 59, 59));

    ui->timeEditStart->blockSignals(false);
    ui->timeEditEnd->blockSignals(false);

    // Передаємо скинутий час у фільтр
    m_proxyModel->setTimeRange(QTime(0, 0, 0), QTime(23, 59, 59, 999));

    // 3. Вмикаємо всі файли (ставимо всі галочки)
    for (int i = 0; i < ui->listWidgetFile->count(); ++i) {
        QListWidgetItem *item = ui->listWidgetFile->item(i);
        item->setCheckState(Qt::Checked); // Це автоматично викличе onFileItemChanged і оновить фільтр
    }
}


// Відмальовування меню при кліку правою кнопкою миші
void KirtForm::onTableViewContextMenu(const QPoint &pos)
{
    // Перевіряємо, чи клікнули по реальному рядку
    QModelIndex index = ui->tableViewLog->indexAt(pos);
    if (!index.isValid()) return;

    QMenu menu(this);

    // Створюємо дію (кнопку) в меню
    QAction *copyAction = menu.addAction("Копіювати виділені рядки (Ctrl+C)");

    // Підключаємо клік по цій дії до нашого методу копіювання
    connect(copyAction, &QAction::triggered, this, &KirtForm::copySelectedLogs);

    // Показуємо меню там, де знаходиться курсор миші
    menu.exec(ui->tableViewLog->viewport()->mapToGlobal(pos));
}

// Головна логіка копіювання у буфер обміну
// Головна логіка копіювання у буфер обміну
void KirtForm::copySelectedLogs()
{
    // Отримуємо список усіх виділених рядків
    QModelIndexList selectedRows = ui->tableViewLog->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) return;

    // Сортуємо індекси, щоб вони копіювалися зверху вниз (як на екрані)
    std::sort(selectedRows.begin(), selectedRows.end());

    QStringList copiedData;

    // Проходимось по кожному виділеному рядку
    for (const QModelIndex &idx : selectedRows) {
        // 1. Беремо ПОВНИЙ багаторядковий текст логу
        QString fullText = m_proxyModel->data(idx, Qt::UserRole + 1).toString();

        // 2. Беремо коротку назву джерела (1 - це індекс колонки Source)
        QModelIndex sourceIdx = m_proxyModel->index(idx.row(), 1);
        QString sourceName = m_proxyModel->data(sourceIdx, Qt::DisplayRole).toString();

        // 3. Формуємо фінальний рядок у форматі: [Джерело] Текст
        QString entryText = QString("[%1] %2").arg(sourceName, fullText);

        copiedData.append(entryText);
    }

    // З'єднуємо всі записи докупи. Додаємо гарний розділювач між логами!
    QString clipboardText = copiedData.join("\n--------------------------------------------------\n");

    // Відправляємо текст у системний буфер обміну
    QApplication::clipboard()->setText(clipboardText);
}
