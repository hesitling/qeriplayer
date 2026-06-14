#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace NeriPlayerQt {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
};

} // namespace NeriPlayerQt

#endif // MAINWINDOW_H