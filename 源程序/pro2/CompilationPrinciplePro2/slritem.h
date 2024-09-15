/*
 * Copyright 2024 Li WenJie. All Rights Reserved.
 * File: slritem.h
 * Version: 1.1
 * Modifier: Li WenJie
 * Author: Huang Shuojia
 * Date: May 20, 2024
 * Description: Definition of syntax analyzer class slrItem, structure item of analysis items, and syntax tree node are included.
 * Version History:
 * 1.0 - 2020-12-24 - Huang Shuojia: Initial version.
 * 1.1 - 2024-05-20 - Li WenJie: Modify some code structure,
 *                               fix some program bugs, update UI interface,
 *                               and add functionality to read encoding and build syntax tree.
*/

#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include <stack>
#include <QDebug>
using namespace std;

#define mssi map<string,string>::iterator

struct item//
{
    string key;
    int value; //对应的编号
    int index; //位置
    int type; //1移进/2规约项标识
    item(string k="", int v=0, int i=0, int t=1):key(k),value(v),index(i),type(t){}
    bool operator==(const item &it){return key==it.key&&value==it.value&&index==it.index&&type==it.type;}
};

struct node//语法树节点
{
    string content;//节点内容
    vector<node> child;//节点孩子
    node(){content="";}
    node(string con) {content=con;}
};

class slrItem
{

public:
    slrItem(string input="");

    void init(string input=""); //初始化

    bool isSLR1();//是否SLR文法

    bool rdc_sftConflict(); //是否移进-规约冲突

    bool sft_sftConflict(); //是否规约-规约冲突

    bool permitAly();//是否允许开始分析

    void getCodeList(string s);//获取编码表

    string getReport();//获取分析报告

    string getAccept(); //获取接受符号

    set<string> getNonTerSet();  //获取非终结符集

    set<string> getNterFirst(string key); //获取非终结符的first集

    set<string> getFollow(string key);//获取非终结符的follow集

    vector<vector<item>> getDFAItem();//获取DFA项目信息

    map<string, vector<vector<string>>> getGrammer();//获取文法

    map<int, map<string, int>> getShift();//获取移进关系

    map<int, map<string, int>> getReduce();//获取规约关系

    node analysis(string sentence,vector<string>* res); //分析，需要一个大小至少为4的vector<string>变量，并且返回语法树根

private:

    bool rdc_sft; //移进-规约冲突

    bool sft_sft; //规约-规约冲突

    string accept;  //接受符

    set<string> NonTerSet; //非终结符集合

    map<string, vector<vector<string>>> m; //文法及产生式

    map<string, set<string>> nTerfirst; //非终结符号first集

    map<string, set<string>> nTerfollow; //非终结符follow集

    map<string,string> codeList;//词法的编码表

    map<string,string> expl_codeList;//需要进行解释的词法元素的编码表

    vector<vector<item>> DFAItem; //记录DFA状态

    map<int, map<string, int>> shift; //移进项

    map<int, map<string, int>> reduce; //记录归约项

    string alyReport; //分析报告

    void extend(int k); //扩展DFA项目集

    void getFirstAndFollow(); //求所有非终结符的first和follow集

    set<string> getFirst(vector<string> value); //获取first集

    void getDFA(); //求DFA

    void conflict();//分析是否存在冲突

};

#endif
