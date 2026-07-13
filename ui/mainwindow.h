#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

extern "C" {
#include "model.h"
}

class QComboBox;
class QDateEdit;
class QFormLayout;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QTableWidget;
class QTextEdit;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void buildUi();
    void buildOverviewPage();
    void buildItemsPage();
    void buildBorrowPage();
    void buildReportsPage();
    bool showLoginDialog();
    bool registerGuiUser(const QString &username, const QString &password);
    void updateUserState();
    bool currentUserIsAdmin() const;
    void loadData();
    void refreshAll();
    void refreshOverview();
    void refreshItemsTable();
    void refreshBorrowTable();
    void refreshBorrowItemCombo();
    void refreshReturnBorrowCombo();
    void refreshDetailReport();
    void refreshStatReport();
    void clearItemForm();
    QByteArray textBytes(const QString &text) const;
    QString todayString() const;
    void showMessage(const QString &message, bool success = true);

private slots:
    void onPageChanged(int row);
    void onItemSelectionChanged();
    void onAddItem();
    void onUpdateItem();
    void onDeleteItem();
    void onBorrowItem();
    void onReturnItem();
    void onSyncData();
    void onReloadData();
    void onSwitchUser();
    void onReturnBorrowSelectionChanged();

private:
    Ui::MainWindow *ui;

    Item m_items[MAX_ITEMS];
    BorrowRecord m_borrows[MAX_BORROW_RECORDS];
    ReturnRecord m_returns[MAX_RETURN_RECORDS];
    int m_itemCount = 0;
    int m_borrowCount = 0;
    int m_returnCount = 0;
    User m_currentUser{};
    bool m_hasCurrentUser = false;

    QListWidget *m_navList = nullptr;
    QStackedWidget *m_pages = nullptr;

    QLabel *m_currentUserLabel = nullptr;
    QLabel *m_itemCountLabel = nullptr;
    QLabel *m_borrowCountLabel = nullptr;
    QLabel *m_activeBorrowLabel = nullptr;
    QLabel *m_returnCountLabel = nullptr;

    QTableWidget *m_itemsTable = nullptr;
    QLineEdit *m_itemCodeEdit = nullptr;
    QLineEdit *m_itemNameEdit = nullptr;
    QLineEdit *m_itemModelEdit = nullptr;
    QSpinBox *m_itemQuantitySpin = nullptr;
    QTextEdit *m_itemDescriptionEdit = nullptr;

    QTableWidget *m_borrowTable = nullptr;
    QLineEdit *m_borrowUserEdit = nullptr;
    QComboBox *m_borrowItemCombo = nullptr;
    QSpinBox *m_borrowQuantitySpin = nullptr;
    QDateEdit *m_borrowDueDateEdit = nullptr;
    QComboBox *m_returnBorrowCombo = nullptr;
    QSpinBox *m_returnQuantitySpin = nullptr;

    QTableWidget *m_detailReportTable = nullptr;
    QTableWidget *m_statReportTable = nullptr;
};

#endif // MAINWINDOW_H
