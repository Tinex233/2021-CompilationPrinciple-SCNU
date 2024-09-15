/*
 * Copyright 2024 Li WenJie. All Rights Reserved.
 * File: main.cpp
 * Version: 1.0
 * Author: Li WenJie
 * Date: May 15, 2024
 * Description: Contains the main function of the QT program, where a MainWindow window object is created.
*/

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
