#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTableWidget>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <cstring>

extern "C" {
#include "borrow.h"
#include "item.h"
#include "report.h"
#include "storage.h"
#include "sync.h"
#include "user.h"
}

namespace {
QLabel *createMetricLabel(const QString &title)
{
    auto *label = new QLabel(title + "\n0");
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumHeight(88);
    label->setStyleSheet("QLabel { background: #f5f7fb; border: 1px solid #d8dee9; border-radius: 6px; font-size: 15px; }"
                         "QLabel::first-line { color: #5b6472; }");
    return label;
}

void setHeaders(QTableWidget *table, const QStringList &headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
}

void setCell(QTableWidget *table, int row, int column, const QString &value)
{
    table->setItem(row, column, new QTableWidgetItem(value));
}

QString borrowStatusText(BorrowStatus status)
{
    return status == BORROW_RETURNED ? "已归还" : "借用中";
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buildUi();
    loadData();
    if (!showLoginDialog())
    {
        close();
        return;
    }
    refreshAll();
    updateUserState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::buildUi()
{
    setWindowTitle("实验室物品出入账管理系统");
    resize(1180, 760);

    auto *root = new QWidget(this);
    auto *rootLayout = new QHBoxLayout(root);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(14);

    m_navList = new QListWidget(root);
    m_navList->addItems({"总览", "物品管理", "借还管理", "报表查询"});
    m_navList->setFixedWidth(150);
    m_navList->setCurrentRow(0);
    m_navList->setStyleSheet("QListWidget { border: 1px solid #d8dee9; border-radius: 6px; font-size: 15px; }"
                             "QListWidget::item { height: 42px; padding-left: 12px; }"
                             "QListWidget::item:selected { background: #2f6fed; color: white; }");

    m_pages = new QStackedWidget(root);
    buildOverviewPage();
    buildItemsPage();
    buildBorrowPage();
    buildReportsPage();

    rootLayout->addWidget(m_navList);
    rootLayout->addWidget(m_pages, 1);
    setCentralWidget(root);

    connect(m_navList, &QListWidget::currentRowChanged, this, &MainWindow::onPageChanged);
    statusBar()->showMessage("就绪");
}

void MainWindow::buildOverviewPage()
{
    auto *page = new QWidget(m_pages);
    auto *layout = new QVBoxLayout(page);
    layout->setSpacing(14);

    auto *title = new QLabel("业务总览");
    title->setStyleSheet("font-size: 22px; font-weight: 600;");
    m_currentUserLabel = new QLabel("当前用户：未登录");
    m_currentUserLabel->setStyleSheet("color: #5b6472;");

    auto *metrics = new QHBoxLayout;
    m_itemCountLabel = createMetricLabel("物品总数");
    m_borrowCountLabel = createMetricLabel("借用记录");
    m_activeBorrowLabel = createMetricLabel("当前借用中");
    m_returnCountLabel = createMetricLabel("归还记录");
    metrics->addWidget(m_itemCountLabel);
    metrics->addWidget(m_borrowCountLabel);
    metrics->addWidget(m_activeBorrowLabel);
    metrics->addWidget(m_returnCountLabel);

    auto *actions = new QHBoxLayout;
    auto *reloadButton = new QPushButton("重新加载数据");
    auto *syncButton = new QPushButton("保存全部数据");
    auto *switchUserButton = new QPushButton("切换用户");
    actions->addWidget(reloadButton);
    actions->addWidget(syncButton);
    actions->addWidget(switchUserButton);
    actions->addStretch();

    auto *hint = new QLabel("左侧切换业务模块；新增、借用、归还等操作会更新内存数据，点击保存全部数据写入 data 目录。");
    hint->setWordWrap(true);
    hint->setStyleSheet("color: #5b6472;");

    layout->addWidget(title);
    layout->addWidget(m_currentUserLabel);
    layout->addLayout(metrics);
    layout->addLayout(actions);
    layout->addWidget(hint);
    layout->addStretch();
    m_pages->addWidget(page);

    connect(reloadButton, &QPushButton::clicked, this, &MainWindow::onReloadData);
    connect(syncButton, &QPushButton::clicked, this, &MainWindow::onSyncData);
    connect(switchUserButton, &QPushButton::clicked, this, &MainWindow::onSwitchUser);
}

void MainWindow::buildItemsPage()
{
    auto *page = new QWidget(m_pages);
    auto *layout = new QHBoxLayout(page);
    layout->setSpacing(14);

    m_itemsTable = new QTableWidget(page);
    setHeaders(m_itemsTable, {"ID", "编号", "名称", "型号", "库存", "描述"});

    auto *formBox = new QGroupBox("物品信息", page);
    formBox->setFixedWidth(320);
    auto *formLayout = new QVBoxLayout(formBox);
    auto *form = new QFormLayout;

    m_itemCodeEdit = new QLineEdit;
    m_itemNameEdit = new QLineEdit;
    m_itemModelEdit = new QLineEdit;
    m_itemQuantitySpin = new QSpinBox;
    m_itemQuantitySpin->setRange(0, 1000000);
    m_itemDescriptionEdit = new QTextEdit;
    m_itemDescriptionEdit->setFixedHeight(92);

    form->addRow("编号", m_itemCodeEdit);
    form->addRow("名称", m_itemNameEdit);
    form->addRow("型号", m_itemModelEdit);
    form->addRow("库存", m_itemQuantitySpin);
    form->addRow("描述", m_itemDescriptionEdit);

    auto *addButton = new QPushButton("新增物品");
    auto *updateButton = new QPushButton("修改选中");
    auto *deleteButton = new QPushButton("删除选中");
    auto *clearButton = new QPushButton("清空表单");

    formLayout->addLayout(form);
    formLayout->addWidget(addButton);
    formLayout->addWidget(updateButton);
    formLayout->addWidget(deleteButton);
    formLayout->addWidget(clearButton);
    formLayout->addStretch();

    layout->addWidget(m_itemsTable, 1);
    layout->addWidget(formBox);
    m_pages->addWidget(page);

    connect(m_itemsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onItemSelectionChanged);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddItem);
    connect(updateButton, &QPushButton::clicked, this, &MainWindow::onUpdateItem);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteItem);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::clearItemForm);
}

void MainWindow::buildBorrowPage()
{
    auto *page = new QWidget(m_pages);
    auto *layout = new QHBoxLayout(page);
    layout->setSpacing(14);

    m_borrowTable = new QTableWidget(page);
    setHeaders(m_borrowTable, {"ID", "借用人", "物品编号", "数量", "借用日期", "预计归还", "状态"});

    auto *side = new QWidget(page);
    side->setFixedWidth(320);
    auto *sideLayout = new QVBoxLayout(side);

    auto *borrowBox = new QGroupBox("借用登记");
    auto *borrowLayout = new QVBoxLayout(borrowBox);
    auto *borrowForm = new QFormLayout;
    m_borrowUserEdit = new QLineEdit;
    m_borrowItemCombo = new QComboBox;
    m_borrowItemCombo->setMinimumWidth(220);
    m_borrowQuantitySpin = new QSpinBox;
    m_borrowQuantitySpin->setRange(1, 1000000);
    m_borrowDueDateEdit = new QDateEdit(QDate::currentDate().addDays(7));
    m_borrowDueDateEdit->setCalendarPopup(true);
    m_borrowDueDateEdit->setDisplayFormat("yyyy-MM-dd");
    borrowForm->addRow("借用人", m_borrowUserEdit);
    borrowForm->addRow("物品", m_borrowItemCombo);
    borrowForm->addRow("数量", m_borrowQuantitySpin);
    borrowForm->addRow("预计归还", m_borrowDueDateEdit);
    auto *borrowButton = new QPushButton("登记借用");
    borrowLayout->addLayout(borrowForm);
    borrowLayout->addWidget(borrowButton);

    auto *returnBox = new QGroupBox("归还登记");
    auto *returnLayout = new QVBoxLayout(returnBox);
    auto *returnForm = new QFormLayout;
    m_returnBorrowCombo = new QComboBox;
    m_returnBorrowCombo->setMinimumWidth(220);
    m_returnQuantitySpin = new QSpinBox;
    m_returnQuantitySpin->setRange(1, 1000000);
    returnForm->addRow("借用条目", m_returnBorrowCombo);
    returnForm->addRow("归还数量", m_returnQuantitySpin);
    auto *returnButton = new QPushButton("登记归还");
    returnLayout->addLayout(returnForm);
    returnLayout->addWidget(returnButton);

    sideLayout->addWidget(borrowBox);
    sideLayout->addWidget(returnBox);
    sideLayout->addStretch();

    layout->addWidget(m_borrowTable, 1);
    layout->addWidget(side);
    m_pages->addWidget(page);

    connect(borrowButton, &QPushButton::clicked, this, &MainWindow::onBorrowItem);
    connect(returnButton, &QPushButton::clicked, this, &MainWindow::onReturnItem);
    connect(m_returnBorrowCombo, &QComboBox::currentIndexChanged, this, [this]() {
        onReturnBorrowSelectionChanged();
    });
}

void MainWindow::buildReportsPage()
{
    auto *page = new QWidget(m_pages);
    auto *layout = new QVBoxLayout(page);
    auto *tabs = new QTabWidget(page);

    m_detailReportTable = new QTableWidget;
    setHeaders(m_detailReportTable, {"借用ID", "借用人", "物品", "借出量", "借出日期", "预计归还", "归还日期", "归还量", "状态"});

    m_statReportTable = new QTableWidget;
    setHeaders(m_statReportTable, {"物品编号", "物品名称", "总借出次数", "总借出数量", "总归还数量", "当前在外"});

    tabs->addTab(m_detailReportTable, "出入账明细");
    tabs->addTab(m_statReportTable, "物品借用统计");
    layout->addWidget(tabs);
    m_pages->addWidget(page);
}

bool MainWindow::showLoginDialog()
{
    while (true)
    {
        QDialog dialog(this);
        dialog.setWindowTitle("用户登录");
        dialog.setModal(true);

        auto *layout = new QVBoxLayout(&dialog);
        auto *title = new QLabel("实验室物品出入账管理系统");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 18px; font-weight: 600;");

        auto *form = new QFormLayout;
        auto *usernameEdit = new QLineEdit;
        auto *passwordEdit = new QLineEdit;
        passwordEdit->setEchoMode(QLineEdit::Password);
        form->addRow("用户名", usernameEdit);
        form->addRow("密码", passwordEdit);

        auto *buttons = new QDialogButtonBox;
        auto *loginButton = buttons->addButton("登录", QDialogButtonBox::AcceptRole);
        auto *registerButton = buttons->addButton("注册", QDialogButtonBox::ActionRole);
        buttons->addButton("退出", QDialogButtonBox::RejectRole);

        layout->addWidget(title);
        layout->addLayout(form);
        layout->addWidget(buttons);

        connect(loginButton, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        connect(registerButton, &QPushButton::clicked, [&]() {
            const QString username = usernameEdit->text().trimmed();
            const QString password = passwordEdit->text().trimmed();
            if (username.isEmpty() || password.isEmpty())
            {
                QMessageBox::warning(&dialog, "注册失败", "用户名和密码不能为空");
                return;
            }
            if (!registerGuiUser(username, password))
            {
                QMessageBox::warning(&dialog, "注册失败", "用户名已存在或用户数量已达上限");
                return;
            }
            QMessageBox::information(&dialog, "注册成功", "注册成功，请使用该账号登录");
        });

        if (dialog.exec() != QDialog::Accepted)
            return false;

        QByteArray username = textBytes(usernameEdit->text());
        QByteArray password = textBytes(passwordEdit->text());
        if (username.isEmpty() || password.isEmpty())
        {
            QMessageBox::warning(this, "登录失败", "用户名和密码不能为空");
            continue;
        }

        User user{};
        if (verify_login(username.constData(), password.constData(), &user))
        {
            m_currentUser = user;
            m_hasCurrentUser = true;
            return true;
        }

        QMessageBox::warning(this, "登录失败", "用户名或密码错误");
    }
}

bool MainWindow::registerGuiUser(const QString &username, const QString &password)
{
    User users[MAX_USER];
    int userCount = 0;
    load_users(users, &userCount);

    QByteArray usernameBytes = textBytes(username);
    QByteArray passwordBytes = textBytes(password);
    for (int i = 0; i < userCount; ++i)
    {
        if (std::strcmp(users[i].username, usernameBytes.constData()) == 0)
            return false;
    }

    if (userCount >= MAX_USER)
        return false;

    std::strncpy(users[userCount].username, usernameBytes.constData(), MAX_USERNAME_LENGTH - 1);
    users[userCount].username[MAX_USERNAME_LENGTH - 1] = '\0';
    std::strncpy(users[userCount].password, passwordBytes.constData(), MAX_PASSWORD_LENGTH - 1);
    users[userCount].password[MAX_PASSWORD_LENGTH - 1] = '\0';
    std::strncpy(users[userCount].level, "guest", MAX_LEVEL_LENGTH - 1);
    users[userCount].level[MAX_LEVEL_LENGTH - 1] = '\0';
    userCount++;

    return save_users(users, userCount);
}

void MainWindow::updateUserState()
{
    const bool isAdmin = currentUserIsAdmin();
    m_currentUserLabel->setText(QString("当前用户：%1（%2）")
                                    .arg(QString::fromUtf8(m_currentUser.username), QString::fromUtf8(m_currentUser.level)));

    m_navList->item(1)->setHidden(!isAdmin);
    m_navList->item(3)->setHidden(!isAdmin);
    if (!isAdmin && (m_navList->currentRow() == 1 || m_navList->currentRow() == 3))
        m_navList->setCurrentRow(2);

    m_borrowUserEdit->setText(QString::fromUtf8(m_currentUser.username));
    m_borrowUserEdit->setReadOnly(!isAdmin);
}

bool MainWindow::currentUserIsAdmin() const
{
    return m_hasCurrentUser && is_admin_user(&m_currentUser);
}

void MainWindow::loadData()
{
    m_itemCount = 0;
    m_borrowCount = 0;
    m_returnCount = 0;

    if (!ensure_data_directory())
        showMessage("无法创建或访问 data 目录", false);
    if (!load_items(m_items, &m_itemCount))
        showMessage("读取物品数据失败", false);
    if (!load_borrow_records(m_borrows, &m_borrowCount))
        showMessage("读取借用数据失败", false);
    if (!load_return_records(m_returns, &m_returnCount))
        showMessage("读取归还数据失败", false);
}

void MainWindow::refreshAll()
{
    refreshOverview();
    refreshItemsTable();
    refreshBorrowTable();
    refreshBorrowItemCombo();
    refreshReturnBorrowCombo();
    refreshDetailReport();
    refreshStatReport();
}

void MainWindow::refreshOverview()
{
    int activeCount = 0;
    for (int i = 0; i < m_borrowCount; ++i)
    {
        if (m_borrows[i].status == BORROW_ACTIVE)
            activeCount++;
    }

    m_itemCountLabel->setText(QString("物品总数\n%1").arg(m_itemCount));
    m_borrowCountLabel->setText(QString("借用记录\n%1").arg(m_borrowCount));
    m_activeBorrowLabel->setText(QString("当前借用中\n%1").arg(activeCount));
    m_returnCountLabel->setText(QString("归还记录\n%1").arg(m_returnCount));
}

void MainWindow::refreshItemsTable()
{
    m_itemsTable->setRowCount(m_itemCount);
    for (int i = 0; i < m_itemCount; ++i)
    {
        const Item &item = m_items[i];
        setCell(m_itemsTable, i, 0, QString::number(item.id));
        setCell(m_itemsTable, i, 1, QString::fromUtf8(item.code));
        setCell(m_itemsTable, i, 2, QString::fromUtf8(item.name));
        setCell(m_itemsTable, i, 3, QString::fromUtf8(item.model));
        setCell(m_itemsTable, i, 4, QString::number(item.quantity));
        setCell(m_itemsTable, i, 5, QString::fromUtf8(item.description));
    }
}

void MainWindow::refreshBorrowTable()
{
    m_borrowTable->setRowCount(m_borrowCount);
    for (int i = 0; i < m_borrowCount; ++i)
    {
        const BorrowRecord &borrow = m_borrows[i];
        setCell(m_borrowTable, i, 0, QString::number(borrow.id));
        setCell(m_borrowTable, i, 1, QString::fromUtf8(borrow.user));
        setCell(m_borrowTable, i, 2, QString::fromUtf8(borrow.item_code));
        setCell(m_borrowTable, i, 3, QString::number(borrow.quantity));
        setCell(m_borrowTable, i, 4, QString::fromUtf8(borrow.borrow_date));
        setCell(m_borrowTable, i, 5, QString::fromUtf8(borrow.due_date));
        setCell(m_borrowTable, i, 6, borrowStatusText(borrow.status));
    }
}

void MainWindow::refreshBorrowItemCombo()
{
    const QString selectedCode = m_borrowItemCombo->currentData().toString();
    m_borrowItemCombo->clear();

    for (int i = 0; i < m_itemCount; ++i)
    {
        const Item &item = m_items[i];
        QString label = QString("%1 - %2（剩余：%3）")
                            .arg(QString::fromUtf8(item.code), QString::fromUtf8(item.name))
                            .arg(item.quantity);
        m_borrowItemCombo->addItem(label, QString::fromUtf8(item.code));
    }

    int index = m_borrowItemCombo->findData(selectedCode);
    if (index >= 0)
        m_borrowItemCombo->setCurrentIndex(index);
}

void MainWindow::refreshReturnBorrowCombo()
{
    const int selectedBorrowId = m_returnBorrowCombo->currentData().toInt();
    m_returnBorrowCombo->clear();

    QString currentUser = QString::fromUtf8(m_currentUser.username);
    for (int i = 0; i < m_borrowCount; ++i)
    {
        const BorrowRecord &borrow = m_borrows[i];
        if (borrow.status != BORROW_ACTIVE)
            continue;
        if (!currentUserIsAdmin() && QString::fromUtf8(borrow.user) != currentUser)
            continue;

        QString itemName = "未知物品";
        int itemIndex = item_find_by_code(m_items, m_itemCount, borrow.item_code);
        if (itemIndex >= 0)
            itemName = QString::fromUtf8(m_items[itemIndex].name);

        int returnedQuantity = borrow_returned_quantity(m_returns, m_returnCount, borrow.id);
        int remainingQuantity = borrow.quantity - returnedQuantity;
        if (remainingQuantity <= 0)
            continue;

        QString label = QString("ID %1 - %2 / %3（待还：%4）")
                            .arg(borrow.id)
                            .arg(QString::fromUtf8(borrow.user), itemName)
                            .arg(remainingQuantity);
        m_returnBorrowCombo->addItem(label, borrow.id);
    }

    int index = m_returnBorrowCombo->findData(selectedBorrowId);
    if (index >= 0)
        m_returnBorrowCombo->setCurrentIndex(index);
    onReturnBorrowSelectionChanged();
}

void MainWindow::refreshDetailReport()
{
    DetailReportRow rows[MAX_BORROW_RECORDS];
    int rowCount = build_detail_report(m_items, m_itemCount, m_borrows, m_borrowCount, m_returns, m_returnCount,
                                       rows, MAX_BORROW_RECORDS);
    m_detailReportTable->setRowCount(rowCount);
    for (int i = 0; i < rowCount; ++i)
    {
        const DetailReportRow &row = rows[i];
        setCell(m_detailReportTable, i, 0, QString::number(row.borrow_id));
        setCell(m_detailReportTable, i, 1, QString::fromUtf8(row.user));
        setCell(m_detailReportTable, i, 2, QString::fromUtf8(row.item_name));
        setCell(m_detailReportTable, i, 3, QString::number(row.borrow_quantity));
        setCell(m_detailReportTable, i, 4, QString::fromUtf8(row.borrow_date));
        setCell(m_detailReportTable, i, 5, QString::fromUtf8(row.due_date));
        setCell(m_detailReportTable, i, 6, QString::fromUtf8(row.return_date));
        setCell(m_detailReportTable, i, 7, QString::number(row.return_quantity));
        setCell(m_detailReportTable, i, 8, borrowStatusText(row.status));
    }
}

void MainWindow::refreshStatReport()
{
    StatReportRow rows[MAX_ITEMS];
    int rowCount = build_stat_report(m_items, m_itemCount, m_borrows, m_borrowCount, m_returns, m_returnCount,
                                     rows, MAX_ITEMS);
    m_statReportTable->setRowCount(rowCount);
    for (int i = 0; i < rowCount; ++i)
    {
        const StatReportRow &row = rows[i];
        setCell(m_statReportTable, i, 0, QString::fromUtf8(row.item_code));
        setCell(m_statReportTable, i, 1, QString::fromUtf8(row.item_name));
        setCell(m_statReportTable, i, 2, QString::number(row.total_borrow_times));
        setCell(m_statReportTable, i, 3, QString::number(row.total_borrow_quantity));
        setCell(m_statReportTable, i, 4, QString::number(row.total_return_quantity));
        setCell(m_statReportTable, i, 5, QString::number(row.out_quantity));
    }
}

void MainWindow::clearItemForm()
{
    m_itemCodeEdit->clear();
    m_itemNameEdit->clear();
    m_itemModelEdit->clear();
    m_itemQuantitySpin->setValue(0);
    m_itemDescriptionEdit->clear();
    m_itemsTable->clearSelection();
}

QByteArray MainWindow::textBytes(const QString &text) const
{
    return text.trimmed().toUtf8();
}

QString MainWindow::todayString() const
{
    return QDate::currentDate().toString("yyyy-MM-dd");
}

void MainWindow::showMessage(const QString &message, bool success)
{
    statusBar()->showMessage(message, 3500);
    if (!success)
        QMessageBox::warning(this, "提示", message);
}

void MainWindow::onPageChanged(int row)
{
    if (row < 0)
        return;

    if ((row == 1 || row == 3) && !currentUserIsAdmin())
    {
        showMessage("当前用户无权访问该模块", false);
        m_navList->setCurrentRow(2);
        return;
    }

    m_pages->setCurrentIndex(row);
}

void MainWindow::onItemSelectionChanged()
{
    const int row = m_itemsTable->currentRow();
    if (row < 0 || row >= m_itemCount)
        return;

    const Item &item = m_items[row];
    m_itemCodeEdit->setText(QString::fromUtf8(item.code));
    m_itemNameEdit->setText(QString::fromUtf8(item.name));
    m_itemModelEdit->setText(QString::fromUtf8(item.model));
    m_itemQuantitySpin->setValue(item.quantity);
    m_itemDescriptionEdit->setPlainText(QString::fromUtf8(item.description));
}

void MainWindow::onAddItem()
{
    if (!currentUserIsAdmin())
    {
        showMessage("当前用户无权新增物品", false);
        return;
    }

    QByteArray code = textBytes(m_itemCodeEdit->text());
    QByteArray name = textBytes(m_itemNameEdit->text());
    QByteArray model = textBytes(m_itemModelEdit->text());
    QByteArray desc = textBytes(m_itemDescriptionEdit->toPlainText());

    if (name.isEmpty() || model.isEmpty())
    {
        showMessage("物品名称和型号不能为空", false);
        return;
    }

    if (!item_add(m_items, &m_itemCount, code.constData(), name.constData(), model.constData(),
                  m_itemQuantitySpin->value(), desc.constData()))
    {
        showMessage("新增物品失败，请检查编号是否重复", false);
        return;
    }

    refreshAll();
    clearItemForm();
    showMessage("物品已新增");
}

void MainWindow::onUpdateItem()
{
    if (!currentUserIsAdmin())
    {
        showMessage("当前用户无权修改物品", false);
        return;
    }

    QByteArray code = textBytes(m_itemCodeEdit->text());
    QByteArray name = textBytes(m_itemNameEdit->text());
    QByteArray model = textBytes(m_itemModelEdit->text());
    QByteArray desc = textBytes(m_itemDescriptionEdit->toPlainText());

    if (code.isEmpty() || name.isEmpty() || model.isEmpty())
    {
        showMessage("编号、名称和型号不能为空", false);
        return;
    }

    if (!item_update(m_items, m_itemCount, code.constData(), name.constData(), model.constData(),
                     m_itemQuantitySpin->value(), desc.constData()))
    {
        showMessage("修改物品失败，请确认编号存在", false);
        return;
    }

    refreshAll();
    showMessage("物品已修改");
}

void MainWindow::onDeleteItem()
{
    if (!currentUserIsAdmin())
    {
        showMessage("当前用户无权删除物品", false);
        return;
    }

    QByteArray code = textBytes(m_itemCodeEdit->text());
    if (code.isEmpty())
    {
        showMessage("请选择或输入要删除的物品编号", false);
        return;
    }

    if (QMessageBox::question(this, "确认删除", "确定删除该物品吗？") != QMessageBox::Yes)
        return;

    if (!item_remove(m_items, &m_itemCount, m_borrows, m_borrowCount, code.constData()))
    {
        showMessage("删除失败，物品不存在或仍有活动借用", false);
        return;
    }

    refreshAll();
    clearItemForm();
    showMessage("物品已删除");
}

void MainWindow::onBorrowItem()
{
    QByteArray user = textBytes(m_borrowUserEdit->text());
    QByteArray itemCode = m_borrowItemCombo->currentData().toString().toUtf8();
    QByteArray borrowDate = todayString().toUtf8();
    QByteArray dueDate = m_borrowDueDateEdit->date().toString("yyyy-MM-dd").toUtf8();

    if (user.isEmpty())
    {
        showMessage("借用人不能为空", false);
        return;
    }
    if (itemCode.isEmpty())
    {
        showMessage("请选择要借用的物品", false);
        return;
    }

    if (!borrow_register(m_borrows, &m_borrowCount, m_items, m_itemCount, user.constData(), itemCode.constData(),
                         m_borrowQuantitySpin->value(), borrowDate.constData(), dueDate.constData()))
    {
        showMessage("借用登记失败，请检查库存、编号和日期", false);
        return;
    }

    sync_data(m_items, m_itemCount, m_borrows, m_borrowCount, m_returns, m_returnCount);
    refreshAll();
    showMessage("借用登记成功");
}

void MainWindow::onReturnItem()
{
    int borrowId = m_returnBorrowCombo->currentData().toInt();
    if (borrowId <= 0)
    {
        showMessage("请选择要归还的借用条目", false);
        return;
    }

    QByteArray returnDate = todayString().toUtf8();
    if (!borrow_return(m_borrows, m_borrowCount, m_returns, &m_returnCount, m_items, m_itemCount,
                       borrowId, m_returnQuantitySpin->value(), returnDate.constData()))
    {
        showMessage("归还登记失败，请检查借用条目和归还数量", false);
        return;
    }

    sync_data(m_items, m_itemCount, m_borrows, m_borrowCount, m_returns, m_returnCount);
    refreshAll();
    showMessage("归还登记成功");
}

void MainWindow::onReturnBorrowSelectionChanged()
{
    int borrowId = m_returnBorrowCombo->currentData().toInt();
    int borrowIndex = borrow_find_by_id(m_borrows, m_borrowCount, borrowId);
    if (borrowIndex < 0)
    {
        m_returnQuantitySpin->setRange(1, 1);
        m_returnQuantitySpin->setValue(1);
        return;
    }

    const BorrowRecord &borrow = m_borrows[borrowIndex];
    int returnedQuantity = borrow_returned_quantity(m_returns, m_returnCount, borrow.id);
    int remainingQuantity = borrow.quantity - returnedQuantity;
    if (remainingQuantity < 1)
        remainingQuantity = 1;

    m_returnQuantitySpin->setRange(1, remainingQuantity);
    m_returnQuantitySpin->setValue(remainingQuantity);
}

void MainWindow::onSyncData()
{
    if (!currentUserIsAdmin())
    {
        showMessage("当前用户无权保存全部数据", false);
        return;
    }

    if (!sync_data(m_items, m_itemCount, m_borrows, m_borrowCount, m_returns, m_returnCount))
    {
        showMessage("保存失败，请检查 data 目录权限", false);
        return;
    }

    showMessage("数据已保存");
}

void MainWindow::onReloadData()
{
    loadData();
    refreshAll();
    updateUserState();
    showMessage("数据已重新加载");
}

void MainWindow::onSwitchUser()
{
    User previousUser = m_currentUser;
    bool hadUser = m_hasCurrentUser;
    m_hasCurrentUser = false;

    if (!showLoginDialog())
    {
        m_currentUser = previousUser;
        m_hasCurrentUser = hadUser;
    }

    refreshAll();
    updateUserState();
    showMessage("当前用户已更新");
}
