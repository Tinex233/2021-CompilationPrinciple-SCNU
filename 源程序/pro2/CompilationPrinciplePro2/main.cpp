/*
 * Copyright 2024 Li WenJie. All Rights Reserved.
 * File: main.cpp
 * Version: 1.1
 * Modifier: Li WenJie
 * Author: Huang Shuojia
 * Date: May 20, 2024
 * Description: Contains the main function of the QT program, where a MainWindow window object is created.
 * Version History:
 * 1.0 - 2020-12-24 - Huang Shuojia: Initial version.
 * 1.1 - 2024-05-20 - Li WenJie: Modify some code structure,
 *                               fix some program bugs, update UI interface,
 *                               and add functionality to read encoding and build syntax tree.
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
