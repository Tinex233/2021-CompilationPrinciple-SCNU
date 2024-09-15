/*
 * Copyright 2024 Li WenJie. All Rights Reserved.
 * File: mainwindow.h
 * Version: 1.1
 * Modifier: Li WenJie
 * Author: Huang Shuojia
 * Date: May 20, 2024
 * Description: Contains the MainWindow class of QT, which includes the definitions of methods for various window controls.
 * Version History:
 * 1.0 - 2020-12-24 - Huang Shuojia: Initial version.
 * 1.1 - 2024-05-20 - Li WenJie: Modify some code structure,
 *                               fix some program bugs, update UI interface,
 *                               and add functionality to read encoding and build syntax tree.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCodec>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QStyleFactory>
#include <QTextEdit>
#include "slritem.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_importBTN_clicked();

    void on_saveBTN_clicked();

    void on_slrBTN_clicked();

    void on_alyBTN_clicked();

    void on_importSenBTN_clicked();

    void on_saveSenBTN_clicked();

    void on_impListBTN_clicked();

    void on_saveListBTN_clicked();

    void on_setCodeBTN_clicked();

private:
    Ui::MainWindow *ui;

    void createTree(node tree,QTreeWidgetItem *ti);

    void saveFile(QTextEdit *te);//保存文件

    void impFile(QTextEdit *te);//导入文件
};
#endif // MAINWINDOW_H
