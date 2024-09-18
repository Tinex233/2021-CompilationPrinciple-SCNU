/*
 * Copyright 2024 Tinex. All Rights Reserved.
 * File: mainwindow.h
 * Version: 1.0
 * Author: Tinex
 * Date: May 15, 2024
 * Description: Contains the MainWindow class of QT, which includes the definitions of methods for various window controls.
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QTextEdit>
#include <QDebug>
#include <QTextBlock>
#include <QStringList>
#include <QList>
#include <QTextCodec>
#include <QTableWidget>
#include <QIcon>

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
    void NFA_clicked();

    void DFA_clicked();

    void MinDFA_clicked();

    void PG_clicked();

    void on_CREATEbtn_clicked();

    void on_SAVEbtn_clicked();

    void on_savePGbtn_clicked();

    void on_OPENbtn_clicked();

    void on_LEFTbtn_clicked();

    void on_RIGHTbtn_clicked();

    void on_tabWidget_tabBarClicked(int index);

    void on_IMPbtn_clicked();

    void on_SAVE2btn_clicked();

    void on_alyPGbtn_clicked();

    void on_savCODEbtn_clicked();

    void on_saveListbtn_clicked();

private:
    Ui::MainWindow *ui;

    void SaveFile(QTextEdit *txted);

    void showRes();
};
#endif // MAINWINDOW_H
