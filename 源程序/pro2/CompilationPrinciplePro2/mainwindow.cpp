/*
 * Copyright 2024 Tinex. All Rights Reserved.
 * File: mainwindow.cpp
 * Version: 1.1
 * Modifier: Tinex
 * Author: Huang Shuojia
 * Date: May 20, 2024
 * Description: The functionality of obtaining input information,
 *              analyzing it through slrItem,
 *              and displaying the analyzed information on the controls of the MainWindow window object has been implemented.
 * Version History:
 * 1.0 - 2020-12-24 - Huang Shuojia: Initial version.
 * 1.1 - 2024-05-20 - Tinex: Modify some code structure,
 *                               fix some program bugs, update UI interface,
 *                               and add functionality to read encoding and build syntax tree.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "slritem.h"

slrItem *slr=NULL;
node tree;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("SLR Analyzer");
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    setWindowIcon(QIcon(":/new/prefix1/ic.ico"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_importBTN_clicked()
{
    //打开并读取文件
    QFileDialog* f = new QFileDialog(this);
    f->setWindowTitle("选择文件*.*");
    f->setNameFilter("*.*");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if(f->exec() == QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream readStream(&file);
        readStream.setCodec("UTF-8");//设置文件流编码方式

        ui->RuleEdit->clear();

        while(!readStream.atEnd())
        {
            ui->RuleEdit->append(readStream.readLine());
        }
    }
    else
    {
        QMessageBox::warning(this, tr("提示"), tr("未打开文件或文件打开失败！"));
        return;
    }
}

void MainWindow::on_saveBTN_clicked()
{
    if(ui->RuleEdit->document()->isEmpty())
    {
        QMessageBox::information(this,"提示","内容为空！",QMessageBox::Ok);
        return;
    }

    //以文本格式保存正则表达式内容
    QFileDialog dlg(this);

    //获取内容的保存路径
    QString fileName = dlg.getSaveFileName(this, tr("保存为"), "./", tr("Text File(*.txt)"));

    if( fileName == "" )
        return;

    //内容保存到路径文件
    QFile file(fileName);

    //以文本方式打开
    if( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream out(&file); //IO设备对象的地址对其进行初始化

        out << ui->RuleEdit->toPlainText(); //输出

        QMessageBox::warning(this, tr("提示"), tr("保存文件成功！"));

        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败！"));
    }
}

void MainWindow::on_slrBTN_clicked()
{
    if(ui->RuleEdit->document()->isEmpty())
    {
        QMessageBox::information(this,"提示","内容为空！",QMessageBox::Ok);
        return;
    }

    if(slr!=NULL)
    {
        delete slr;
        slr=NULL;
    }
    slr=new slrItem(ui->RuleEdit->toPlainText().toStdString());
    ui->RepEdit->clear();
    if(slr->isSLR1())
        ui->IsslrBTN->setText("是");
    else
    {
        ui->IsslrBTN->setText("否");
        ui->RepEdit->setText(QString::fromStdString(slr->getReport()));
    }

    ui->FirWidget->clear();
    ui->FolWidget->clear();
    ui->DFAProWidget->clear();
    ui->DFAAdjWidget->clear();
    ui->SLRWidget->clear();
    ui->ALYWidget->clear();

    //显示First集和Follow集
    set<string> nts=slr->getNonTerSet();

    ui->FirWidget->setRowCount(nts.size());
    ui->FirWidget->setColumnCount(2);
    ui->FolWidget->setRowCount(nts.size());
    ui->FolWidget->setColumnCount(2);

    int r=0;
    for(auto &nt: nts)
    {
        ui->FirWidget->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(nt)));
        string s="";
        for(auto &x: slr->getNterFirst(nt))
        {
            s+=x;
            s+=" ";
        }

        ui->FirWidget->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(s)));
        r++;
    }
    r=0;
    for(auto &nt: nts)
    {
        ui->FolWidget->setItem(r, 0, new QTableWidgetItem(QString::fromStdString(nt)));
        string s="";
        for(auto &x: slr->getFollow(nt))
        {
            s+=x;
            s+=" ";
        }

        ui->FolWidget->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(s)));
        r++;
    }

    ui->FirWidget->verticalHeader()->setVisible(0);
    ui->FirWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->FirWidget->resizeColumnsToContents();
    ui->FolWidget->verticalHeader()->setVisible(0);
    ui->FolWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->FolWidget->resizeColumnsToContents();

    //显示DFA信息
    QStringList header={"状态","项目集"};
    vector<vector<item>> dfa=slr->getDFAItem();
    map<string, vector<vector<string>>> m=slr->getGrammer();
    ui->DFAProWidget->setRowCount(dfa.size());
    ui->DFAProWidget->setColumnCount(2);
    ui->DFAProWidget->setHorizontalHeaderLabels(header);
    ui->DFAProWidget->setWordWrap(1);

    r=0;
    for(int i=0;i<dfa.size();i++)
    {
        ui->DFAProWidget->setItem(r, 0, new QTableWidgetItem(QString::number(i)));
        string s="";
        for(auto &p:dfa[i])
        {
            s+=p.key;
            s+="->";
            for(int j=0; j<m[p.key][p.value].size(); ++j)
            {
                if(j==p.index)
                    s+= ".";
                 s+=m[p.key][p.value][j];
                 s+=" ";
            }
            if(p.index == m[p.key][p.value].size()){
                s+= ".";
            }
            s+="\n";
        }
        ui->DFAProWidget->setItem(r, 1, new QTableWidgetItem(QString::fromStdString(s)));
        r++;
    }

    map<int, map<string, int>> shift=slr->getShift(),reduce=slr->getReduce();
    QStringList hheader,vheader;
    vector<string> hh;
    for(auto c:nts)
    {
        hheader.append(QString::fromStdString(c));
        hh.push_back(c);
    }
    for(int i=0;i<dfa.size();i++)
        vheader.append(QString::number(i));
    ui->DFAAdjWidget->setRowCount(dfa.size());
    ui->DFAAdjWidget->setColumnCount(nts.size());
    //ui->DFAAdjWidget->setHorizontalHeaderLabels(hheader);
    ui->DFAAdjWidget->setVerticalHeaderLabels(vheader);
    for(auto &x: shift)
    {
        int xrow = x.first;
        for(auto &y: x.second)
        {
            int xcol=-1;
            for(int i=0;i<hh.size();i++)
            {
                if(hh[i]==y.first)
                {
                    xcol=i;
                    break;
                }
            }
            if(xcol==-1)
            {
                hh.push_back(y.first);
                hheader.append(QString::fromStdString(y.first));
                xcol=hh.size()-1;
            }

            ui->DFAAdjWidget->setColumnCount(hh.size());
            ui->DFAAdjWidget->setItem(xrow, xcol, new QTableWidgetItem(QString::number(y.second)));
        }
    }
    ui->DFAAdjWidget->setHorizontalHeaderLabels(hheader);
    ui->DFAProWidget->verticalHeader()->setVisible(0);
    ui->DFAProWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DFAProWidget->resizeColumnsToContents();
    ui->DFAProWidget->resizeRowsToContents();
    ui->DFAAdjWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->DFAAdjWidget->resizeColumnsToContents();

    //显示SLR(1)分析表
    ui->SLRWidget->setRowCount(0);
    ui->SLRWidget->setColumnCount(0);
    if(slr->sft_sftConflict())
        return;
    QStringList slrHeader;
    slrHeader.append(" ");
    set<string> sSet;
    for(auto &x: shift)
    {
        for(auto &y: x.second)
        {
            sSet.insert(y.first);
        }
    }

    for(auto &x: reduce)
    {
        for(auto &y: x.second)
        {
            sSet.insert(y.first);
        }
    }

    for(auto &x:sSet)
        slrHeader.append(QString::fromStdString(x));

    ui->SLRWidget->setColumnCount(sSet.size()+1);
    ui->SLRWidget->setRowCount(dfa.size());
    ui->SLRWidget->setHorizontalHeaderLabels(slrHeader);

    for(int i=0; i<dfa.size(); ++i)
    {
        ui->SLRWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        int col=1;
        for(auto &s: sSet)
        {
            if(shift[i].find(s) != shift[i].end())//移进项
            {
                if(nts.find(s) != nts.end())
                    ui->SLRWidget->setItem(i, col, new QTableWidgetItem(QString::number(shift[i][s])));
                else
                {
                    string s1="s";
                    s1+=to_string(shift[i][s]);
                    ui->SLRWidget->setItem(i, col, new QTableWidgetItem(QString::fromStdString(s1)));
                }
            }
            else if(reduce[i].find(s) != reduce[i].end())//归约项
            {
                item & p = dfa[i][reduce[i][s]];
                if(p.key == slr->getAccept())
                    ui->SLRWidget->setItem(i, col, new QTableWidgetItem("接受"));
                else
                {
                    string a = "r(";
                    a += p.key;
                    a += " -> ";
                    vector<string> & value = m[p.key][p.value];
                    for(auto &b: value)
                    {
                        a += b;
                        a += " ";
                    }
                    a[a.length()-1] = ')';
                    ui->SLRWidget->setItem(i, col, new QTableWidgetItem(QString::fromStdString(a)));
                }
            }
            col++;
        }

    }
    ui->SLRWidget->verticalHeader()->setVisible(0);
    ui->SLRWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->SLRWidget->resizeColumnsToContents();

}

void MainWindow::createTree(node tree,QTreeWidgetItem *ti)
{
    if(tree.content=="")
        return;
    QList<QString> child{QString::fromStdString(tree.content)};
    QTreeWidgetItem* ch = new QTreeWidgetItem(child);
    ti->addChild(ch);
    for (int i=0;i<tree.child.size();i++)
         createTree(tree.child[i],ch);
}

void MainWindow::on_alyBTN_clicked()
{
    if(slr==NULL)
    {
        QMessageBox::information(this,"提示","未设置SLR(1)文法规则",QMessageBox::Ok);
        return;
    }

    if(!slr->permitAly())
    {
        QMessageBox::information(this,"提示","未设置编码表",QMessageBox::Ok);
        return;
    }

    string sen=ui->programEdit->document()->toPlainText().toStdString();
    if(sen.length()==0)
    {
        QMessageBox::information(this,"提示","未输入编码内容",QMessageBox::Ok);
        return;
    }

    vector<string> v[4];
    tree=slr->analysis(sen,v);

    ui->SyntaxTree->clear();
    QList<QString> title{ "[Syntax Tree]" };
    QTreeWidgetItem *trItem = new QTreeWidgetItem(title);
    ui->SyntaxTree->setStyle(QStyleFactory::create("windows"));
    createTree(tree, trItem);
    ui->SyntaxTree->addTopLevelItem(trItem);
    ui->SyntaxTree->expandAll();

    QStringList header={"步骤","状态栈","输入栈","行为"};

    ui->ALYWidget->setRowCount(v[0].size());
    ui->ALYWidget->setColumnCount(4);
    ui->ALYWidget->setHorizontalHeaderLabels(header);
    for(int i=0;i<4;i++)
        for(int j=0;j<v[i].size();j++)
            ui->ALYWidget->setItem(j, i, new QTableWidgetItem(QString::fromStdString(v[i][j])));

    //隐藏列表头
    ui->ALYWidget->verticalHeader()->setVisible(0);

    //设置不可编辑
    ui->ALYWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //完整显示表格内容
    ui->ALYWidget->resizeColumnsToContents();
}

void MainWindow::on_importSenBTN_clicked()
{
    impFile(ui->programEdit);
}

void MainWindow::impFile(QTextEdit *te)
{
    //打开并读取文件
    QFileDialog* f = new QFileDialog(this);
    f->setWindowTitle("选择文件*.*");
    f->setNameFilter("*.*");
    f->setViewMode(QFileDialog::Detail);

    QString filePath;
    if(f->exec() == QDialog::Accepted)
        filePath = f->selectedFiles()[0];

    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream readStream(&file);
        readStream.setCodec("UTF-8");//设置文件流编码方式

        te->clear();

        while(!readStream.atEnd())
        {
            te->append(readStream.readLine());
        }
    }
    else
    {
        QMessageBox::warning(this, tr("提示"), tr("未打开文件或文件打开失败！"));
        return;
    }
}

void MainWindow::saveFile(QTextEdit *te)
{
    //以文本格式保存正则表达式内容
    QFileDialog dlg(this);

    //获取内容的保存路径
    QString fileName = dlg.getSaveFileName(this, tr("保存为"), "./", tr("Text File(*.txt)"));

    if( fileName == "" )
        return;

    //内容保存到路径文件
    QFile file(fileName);

    //以文本方式打开
    if( file.open(QIODevice::WriteOnly | QIODevice::Text) )
    {
        QTextStream out(&file); //IO设备对象的地址对其进行初始化

        out << te->document(); //输出

        QMessageBox::warning(this, tr("提示"), tr("保存文件成功！"));

        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("错误"), tr("保存文件失败！"));
    }
}

void MainWindow::on_saveSenBTN_clicked()
{
    saveFile(ui->programEdit);
}

void MainWindow::on_impListBTN_clicked()
{
    impFile(ui->codeListEdit);
}

void MainWindow::on_saveListBTN_clicked()
{
    saveFile(ui->codeListEdit);
}

void MainWindow::on_setCodeBTN_clicked()
{
    string s=ui->codeListEdit->document()->toPlainText().toStdString();

    if(s.length()==0)
    {
        QMessageBox::information(this,"提示","未输入编码表",QMessageBox::Ok);
        return;
    }

    slr->getCodeList(s);

    QMessageBox::information(this,"提示","已读入编码表",QMessageBox::Ok);
}
