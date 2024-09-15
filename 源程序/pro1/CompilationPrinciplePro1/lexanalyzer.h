/*
 * Copyright 2024 Li WenJie. All Rights Reserved.
 * File: lexanayzer.h
 * Version: 1.0
 * Author: Li WenJie
 * Date: May 15, 2024
 * Description: The file contains the definitions of the NFAGraph, DFAGraph, and LexAnalyzer classes,
 *              with deliberate effort to avoid coupling with QT methods and structures in the implementation,
 *              making it a fully standalone C++ program.
*/

#ifndef ANALYZER_H
#define ANALYZER_H

#include <iostream>
#include <algorithm>
#include <string>
#include <string.h>
#include <unordered_map>
#include <stack>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <deque>

using namespace std;

//以下宏定义仅用于简化变量类型名，无其他含义
#define pii pair<int,int>
#define sti set<int>::iterator
#define vsi vector<set<edge>>::iterator
#define mcmi map<char,map<int,int>>::iterator
#define mcl map<char,list<pii>>::iterator
#define stsi set<edge>::iterator

//设置编码起始码
#define ENCODING_START 100

struct edge//转换边
{
    char CvType;//转换类型
    int vj;//转换的下一状态
    edge(){CvType='0',vj=0;}
    edge(char c,int v):CvType(c),vj(v){}
    bool operator<(const edge &t)const {if(CvType==t.CvType)return vj<t.vj;  else return CvType<t.CvType;}
    bool operator==(const edge &t)const {return vj==t.vj&&CvType==t.CvType;}
    bool operator!=(const edge &t)const {return vj!=t.vj||CvType!=t.CvType;}
};

//NFA类
class NFAGraph
{
public:
    NFAGraph();

    NFAGraph(string regex);

    void init(string regex);//初始化

    int getNumOfStatus();//获取状态数

    int getNumOfchar();//获取字符表字符数量

    int getInitState();//获取初始状态号

    int getAcceptState();//获取接收状态号

    bool explain();//是否解释单词内容

    string getContent();//获取表达式内容

    vector<set<int>> getClosure();//获取ε-闭包

    map<int,int> getCharTrans(char ch);//获取字符转换，以<转换前状态，转换后状态>形式返回

    string getStateTrans(int row,char col);//获取某行列的状态转换

    set<char> getCharSet();//获取字符集

    static bool isTrans(const char c);//是否为转换

    static bool isOperator(const char c);//是否是合法运算符

    static void initPriNum();//初始化优先数

private:

    friend class LexAnalyzer;

    bool epl;//为true时，在词法分析时需要将单词解释为具体内容

    int NumOfStatus;//NFA结构的状态数

    int InitState;//初始状态

    int AcceptState;//接收状态

    string content;//表达式内容

    vector<set<edge>> graph;//邻接表，从1开始

    set<char> cs;//字符表

    vector<bool> escF;//标记发生转义位置的辅助容器

    static unordered_map<char,int> isp,icp;//后缀表达式优先数

    static string delEmpBrk(string regex);//删除空括号

    void DFS(set<int> &s,int fa);//深搜ε-闭包

    bool isLegal(const string regex);//简单检测正则表达式是否合法，主要关注字符上的错误，暂时忽略部分语法出现的错误

    string addOP(string regex);//添加'&'连接符号

    string postfix(string regex);//将中缀正则表达式转后缀，res传参接收结果，返回表达式是否合法的布尔值

};

//DFA类
class DFAGraph
{
public:
    DFAGraph(){isMinimized=false;};

    DFAGraph(NFAGraph &nfa);

    void init(NFAGraph &nfa);

    int getNumOfStates();//获取状态数

    bool isMin();//询问是否最小化

    bool acceptState(int i);//询问是否接受状态

    bool explain();//是否解释单词内容

    vector<set<edge>> getGraph();//获取邻接表

    string getState(int i);//获取状态

    DFAGraph toMinDFA();//转换至最小化

    int getInitState();//获取初始状态编号

    set<char> getCharSet();//获取字符表

private:

    friend class LexAnalyzer;

    int InitState;//初始状态

    bool epl;//为true时，在词法分析时需要将单词解释为具体内容

    bool isMinimized;//最小化

    int NumOfStates;//状态数

    vector<set<edge>> graph;//状态转换表，<转换名,<转换前状态,转换后状态>>，其中状态使用状态集中的索引

    vector<set<int>> states; //状态集

    vector<bool> isAccepted;//接收状态，isAccepted[0]为false时表示全部状态都是接受状态

    set<char> cs;//字符表

    int addState(NFAGraph &nfa,set<int> &s);//寻找状态是否已存在，不存在则加入状态，最终返回状态参数索引
};


string GenProgram(DFAGraph &dfa);

//词法分析器类
class LexAnalyzer
{
public:
    LexAnalyzer();

    void init(vector<string> v);

    int getNumOfRegex();//正则表达式数量，不包括关键词和注释类型

    void anlyRegex();//分析正则表达式

    void minimizeDFA();//DFA最小化

    const string getCodeList();//获取单词编码表

    const string GenProgram();//生成词法分析程序

    const string outputCode(string prog);//输出编码文件

    const vector<NFAGraph> getNFAgroup();//获取NFA组

    const vector<DFAGraph> getDFAgroup();//获取DFA组

    const vector<string> getNameOfRex();//获取正则表达式名

private:

    bool CaseSensitive;//是否大小写敏感

    int NumOfRegex;//正则表达式数量

    int nlFlag;//若有注释方法以'\n'结尾，标记之

    vector<NFAGraph> NFAgroup;//NFA组

    vector<DFAGraph> DFAgroup;//DFA组

    vector<DFAGraph> MDFAgroup;//最小化DFA组

    vector<string> regexName;//正则表达式名

    vector<string> REXcontents;//正则表达式内容

    vector<int> REXCodeTable;//正则表达式编码表

    map<string,int> KWCodeTable;//关键词编码表

    vector<DFAGraph> LComment;//注释特殊处理

    vector<DFAGraph> RComment;

    void getKMPNext(vector<int>& ne,int x);//计算KMP的next

    const int matchRegex(string &p,int l,int r,int k,vector<DFAGraph> &v);//匹配正则表达式，返回匹配串的最后一个位置

    const int matchComment(string &p,int l,int len,int k);//匹配注释，返回匹配串的最后一个位置

    const string Gen1Function(DFAGraph &dfa,int k,int mode);//生成一个表达式分析的函数，三种模式：1-一般正则式 2-左注释 3-右注释
};

#endif // ANALYZER_H
