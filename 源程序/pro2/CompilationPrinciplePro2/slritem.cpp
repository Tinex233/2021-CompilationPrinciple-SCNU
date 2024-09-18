/*
 * Copyright 2024 Tinex. All Rights Reserved.
 * File: slritem.cpp
 * Version: 1.1
 * Modifier: Tinex
 * Author: Huang Shuojia
 * Date: May 20, 2024
 * Description: This file contains the implementation of the methods for the slrItem class.
 * Version History:
 * 1.0 - 2020-12-24 - Huang Shuojia: Initial version.
 * 1.1 - 2024-05-20 - Tinex: Modify some code structure,
 *                               fix some program bugs, update UI interface,
 *                               and add functionality to read encoding and build syntax tree.
*/

#include "slritem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

slrItem::slrItem(string input)
{
    init(input);
}

bool slrItem::isSLR1()
{
    return !rdc_sft&&!sft_sft;
}

string slrItem::getReport()
{
    return alyReport;
}

set<string> slrItem::getNonTerSet()
{
    return NonTerSet;
}

set<string> slrItem::getNterFirst(string key)
{
    if(NonTerSet.find(key) == NonTerSet.end())
        return set<string>{key};
    return nTerfirst[key];
}

set<string> slrItem::getFollow(string key)
{
    return nTerfollow[key];
}

string slrItem::getAccept()
{
    return accept;
}

set<string> slrItem::getFirst(vector<string> value)
{
    set<string> first_set;
    int k;
    for(k=0; k<value.size(); ++k)
    {
        set<string> first_set_k = getNterFirst(value[k]);
        for(auto &s: first_set_k)
        {
            first_set.insert(s);
        }
        if(first_set_k.find("empty") == first_set_k.end())
        {
            break;
        }
        first_set.erase("empty");
    }
    if(k == value.size())
    {
        first_set.insert("empty");
    }
    return first_set;
}

map<int, map<string, int>> slrItem::getShift()
{
    return shift;
}

map<int, map<string, int>> slrItem::getReduce()
{
    return reduce;
}

map<string, vector<vector<string>>> slrItem::getGrammer()
{
    return m;
}

vector<vector<item>> slrItem::getDFAItem()
{
    return DFAItem;
}

bool slrItem::rdc_sftConflict()
{
    return rdc_sft;
}

bool slrItem::sft_sftConflict()
{
    return sft_sft;
}

void slrItem::getFirstAndFollow()
{
    //求非终结符的first集
    for(auto &nt: this->NonTerSet)
        nTerfollow[nt] = set<string>();
    bool isChange = true;
    while(isChange)
    {
        isChange = false;
        for(auto &p: this->m)
        {
            string key = p.first;
            vector<vector<string>> values = p.second;
            for(auto &value: values)
            {
                int k;
                for(k=0; k<value.size(); ++k)
                {
                    auto && first_set_k = this->getNterFirst(value[k]);
                    for(auto &s: first_set_k)
                    {
                        if((s!="empty") && (this->nTerfirst[key].find(s) == this->nTerfirst[key].end()))
                        {
                            nTerfirst[key].insert(s);
                            isChange = true;
                        }
                    }
                    if(first_set_k.find("empty") == first_set_k.end())
                    {
                        break;
                    }
                }
                if(k == value.size() and (this->nTerfirst[key].find("empty") == this->nTerfirst[key].end()))
                {
                    nTerfirst[key].insert("empty");
                    isChange = true;
                }
            }
        }
    }
    //求非终结符的follow集
    for(auto &nt: this->NonTerSet)
        nTerfollow[nt] = set<string>();
    nTerfollow[this->accept].insert("$");
    isChange = true;
    while(isChange)
    {
        isChange = false;
        for(auto &p: this->m)
        { //遍历每一个产生式p
            string key = p.first;
            vector<vector<string>> values = p.second;
            for(auto &value: values)
            {
                //遍历value中每一个非终结符
                for(auto it=value.begin(); it!=value.end(); ++it)
                {
                    if(this->NonTerSet.find(*it) != this->NonTerSet.end())
                    {
                        //特殊情况
                        if(it+1 == value.end())
                        {
                            for(auto &x: this->getFollow(key))
                            {
                                if(this->nTerfollow[*it].find(x) == this->nTerfollow[*it].end())
                                {
                                    this->nTerfollow[*it].insert(x);
                                    isChange = true;
                                }
                            }
                        }
                        else
                        {
                            set<string> first_set = getFirst(vector<string>(it+1, value.end()));
                            for(auto &x: first_set)
                            {
                                if(this->nTerfollow[*it].find(x) == this->nTerfollow[*it].end() and x != "empty")
                                {
                                    this->nTerfollow[*it].insert(x);
                                    isChange = true;
                                }
                            }
                            if(first_set.find("empty") != first_set.end())//如果有@符号
                            {
                                this->nTerfollow[*it].erase("empty");
                                for(auto &x: this->getFollow(key))
                                {
                                    if(this->nTerfollow[*it].find(x) == this->nTerfollow[*it].end())
                                    {
                                        this->nTerfollow[*it].insert(x);
                                        isChange = true;
                                    }
                                }
                            }
                        }

                    }
                }
            }
        }
    }
}

void slrItem::init(string input)
{
    codeList.clear();
    rdc_sft=false,sft_sft=false;
    vector<string> v;
    int from = 0,len = 0;
    for(int i=0; i<input.length();i++)
    {
        if(input[i] == '\n')
        {
            len = i - from;
            v.push_back(input.substr(from, len));
            from = i + 1;
        }
    }
    v.push_back(input.substr(from));

    //文法扩充
    stringstream ss(v[0]);
    string s;
    ss >> s;

    if(v[0].find('|') != string::npos)
    {
        v.insert(v.begin(), string(s+"'"+" -> "+s));
        s += "'";
    }
    else//再筛查，是否有多条同样起始符开头的规则
    {
        for(int i=1;i<v.size();i++)
        {
            stringstream s1(v[i]);
            string header;

            s1>>header;
            if(header==s)
            {
                v.insert(v.begin(), string(s+"'"+" -> "+s));
                s += "'";
                break;
            }
        }
    }

    accept = s;
    set<string> syms;//符号集合
    //规则对构建
    for(auto &x: v)
    {
        string key;
        vector<vector<string>> values;
        string a;
        vector<string> value;

        stringstream ss(x+" $");
        ss >> key;
        ss >> a;
        while(1)
        {
            ss >> a;
            if(a == "$")
            {
                break;
            }
            if(a == "|")
            {
                values.push_back(value);
                value.clear();
                continue;
            }
            int from = 0;
            int len = 0;
            int i;
            for(i=0; i<a.length(); ++i)
            {
                if(a[i] == '(' || a[i] == ')')
                {
                    len = i-from;
                    if(len != 0)
                    {
                        value.push_back(a.substr(from, len));
                    }
                    from = i + 1;
                    value.push_back(string(1, a[i]));
                }
            }
            if(from != i)
            {
                value.push_back(a.substr(from));
            }
        }
        values.push_back(value);
        if(syms.find(key)==syms.end())//第一次出现符号key
        {
            m[key] = values;
            syms.insert(key);
        }
        else
        {
            for(auto v:values)
                m[key].push_back(v);
        }
    }

    //求非终结符集
    for(auto it = m.begin(); it != m.end(); ++it)
    {
        NonTerSet.insert(it->first);
    }

    //求非终结符的follow集和follow集
    getFirstAndFollow();
    //cout<<"first follow over"<<endl;

    //求LR(0)DF
    getDFA();
    //cout<<"LR over"<<endl;
}

bool slrItem::permitAly()
{
    if(codeList.size()==0&&expl_codeList.size()==0)
        return false;
    return true;
}

void slrItem::getCodeList(string s)
{
    //获取编码表
    int l=0,r=0,len=s.length();
    while(s[r]!='\n'&&r<len) r++;

    while(s[r+1]!=']'&&l<len)
    {
        l=r+3;
        string lex;
        r=l+1;
        while(s[r]!=' ') r++;
        lex=s.substr(l,r-l);
        l=r+1,r+=2;
        while(s[r]!='\n') r++;
        if(s[r-1]!='*')
            codeList[s.substr(l,r-l)]=lex;
        else
            expl_codeList[s.substr(l,r-l-1)]=lex;
    }
}

node slrItem::analysis(string sentence,vector<string>* res)
{
    node root;

    queue<node> q1;//树节点队列

    //将编码转换为词法元素，并替换
    string sstr="";
    int l=0,r,len=sentence.length();
    while(l<len)
    {
        r=l+1;
        while(sentence[r]!=' ') r++;
        string sss=sentence.substr(l,r-l);
        mssi it=codeList.find(sss);
        if(it!=codeList.end())
        {
            sstr+=it->second;
            sstr+=" ";
            q1.push(node(it->second));
        }
        else
        {
            it=expl_codeList.find(sss);
            if(it!=expl_codeList.end())
            {
                sstr+=it->second;
                sstr+=" ";
                node n1(it->second);

                //获取后方的解释
                l=r+1,r+=2;
                while(sentence[r]!=' ') r++;

                node n2(node(sentence.substr(l,r-l)));
                n1.child.push_back(n2);
                q1.push(n1);
            }
        }
        l=r+1;
    }
    sentence=sstr;

    stack<node> alystack;//分析栈
    vector<string> s,q;
    stringstream ss(sentence + " $");
    string a;
    while(true)
    {
        ss >> a;
        if(a == "$")
            break;

        //处理括号
        int from = 0;
        int len = 0;
        int i;
        for(i=0; i <a.length(); ++i)
        {
            if(a[i] == '(' || a[i] == ')')
            {
                len = i - from;
                if(len > 0)
                {
                    q.push_back(a.substr(from, len));
                }
                q.push_back(string(1, a[i]));
                from = i + 1;
            }
        }
        if(from != i)
        {
            q.push_back(a.substr(from));
        }
    }
    q.push_back("$");
    s.push_back(to_string(0));
    s.push_back("$");

    //开始分析
    ss.clear();
    ss.setf(ios::left);
    for(int i = 0; ;++i)
    {
        res[0].push_back(to_string(i+1));

        //分析栈
        a = "";
        for(int j=s.size()-1; j>=0; --j)
        {
            a += s[j];
            a += " ";
        }
        res[1].push_back(a);

        //输入栈
        a = "";
        for(auto &x: q)
        {
            a += x;
            a += " ";
        }
        res[2].push_back(a);
        //状态转换
        int f = stoi(s.front());
        string ts = q.front();
        int t;
        if(shift[f].find(ts) != shift[f].end())
        {
            q.erase(q.begin());
            t = shift[f][ts];
            s.insert(s.begin(), ts);
            s.insert(s.begin(), to_string(t));
            res[3].push_back("移进" + to_string(t));
            //qDebug()<<"移进"<<QString::number(t);
            node pn=q1.front();
            q1.pop();
            alystack.push(pn);
        }
        else if(reduce[f].find(ts) != reduce[f].end())
        {
            t = reduce[f][ts];
            item &p = DFAItem[f][t];
            string &key = p.key;
            if(accept == key)
            {
                root=node(accept);
                int ind=alystack.size();
                root.child.resize(alystack.size());
                while (!alystack.empty())
                {
                    root.child[ind-1]=alystack.top();
                    ind--;
                    alystack.pop();
                }

                res[3].push_back("接受" + to_string(t));
                break;
            }
            vector<string> value = m[key][p.value];
            if(value[0] != "empty")
            {
                for(int i=0; i<value.size()*2; ++i)
                    s.erase(s.begin());
            }

            f = stoi(s.front());
            t = shift[f][key];
            s.insert(s.begin(), key);
            s.insert(s.begin(), to_string(t));
            a = "";
            a += "用";
            a += key;
            a += " -> ";

            node fa(key);
            int ind=value.size();
            fa.child.resize(ind);

            for(auto &x: value)
            {
                a += x;
                a += " ";

                if(x=="empty")
                {
                    fa.child[ind-1]=node("empty");
                }
                else
                {
                    node ch=alystack.top();
                    alystack.pop();
                    fa.child[ind-1]=ch;

                }
                ind--;
            }
            a += "归约";
            res[3].push_back(a);

            alystack.push(fa);
        }
        else
        {
            res[3].push_back("出错");
            break;
        }
    }

    return root;
}

void slrItem::conflict()
{
    if(!isSLR1())
        alyReport+="\n";
    for(int i=0; i<DFAItem.size(); ++i)
    {
        set<string> set1, set2, result;
        for(auto &x: shift[i])
            set1.insert(x.first);
        for(auto &x: reduce[i])
            set2.insert(x.first);
        set_intersection(begin(set1), end(set1), begin(set2), end(set2),
          inserter(result, begin(result)));
        if(result.size() != 0)
        {
            rdc_sft = true;
            stringstream ss;
            ss<<"移进-规约冲突：DFA的状态"<<i<<"中移进项的first集和归约项的follow集的交集不为空\n";
            alyReport+= ss.str();
        }
    }
}

void slrItem::getDFA()
{
    alyReport = "";
    vector<item> node{item{accept, 0, 0, 1}};
    DFAItem.push_back(node);
    sft_sft = false;
    //循环处理每一个结点i
    for(int i=0; i<DFAItem.size(); ++i)
    {
        shift[i] = map<string, int>(),reduce[i] = map<string, int>();

        extend(i);

        //遍历该状态的项目
        for(int j=0; j<DFAItem[i].size(); ++j)
        {
            //处理归约项
            item &p = DFAItem[i][j];
            if(p.type == 2){
                set<string> follow_set = this->getFollow(p.key);
                for(auto &s: follow_set)
                {
                    if(reduce[i].find(s) != reduce[i].end())//找到
                    {
                        sft_sft = true;
                        stringstream ss;
                        ss <<"规约-规约冲突：DFA的状态"<<i<<"中归约项目的follow集的交集不为空\n";
                        alyReport += ss.str();
                    }
                    this->reduce[i][s] = j;
                }
            }
            //处理移进项
            else
            {
                string t = m[p.key][p.value][p.index];
                item np(p.key, p.value, p.index+1, 1);
                if(np.index >= m[p.key][p.value].size())
                    np.type = 2;
                if(this->shift[i].find(t)==shift[i].end())
                {
                    //转换t未存在
                    int k=0;
                    for(k=0; k<DFAItem.size(); ++k)
                    {
                        vector<item> & n = DFAItem[k];
                        if(find(n.begin(), n.end(), np) != n.end())
                            break;
                    }
                    if(k < DFAItem.size())//项目已经存在
                    {
                        shift[i][t] = k;
                    }
                    else//项目未存在
                    {
                        //新建节点
                        DFAItem.push_back(vector<item>{np});
                        shift[i][t] = this->DFAItem.size()-1;
                    }
                }
                else//转换t存在
                {
                    int k = shift[i][t];
                    vector<item> & n = DFAItem[k];
                    if(find(n.begin(), n.end(), np) == n.end())//项目不存在
                    {
                        DFAItem[k].push_back(np);
                    }
                }
            }
        }
    }

    conflict();
}



void slrItem::extend(int k)
{
    vector<item> &node = DFAItem[k];
    for(int i=0; i<node.size(); ++i)
    {
        item &p = node[i];
        if(p.type == 1)
        {
            string extend_s = m[p.key][p.value][p.index];
            if(NonTerSet.find(extend_s) != NonTerSet.end())//extend_s是非终结符，需要扩充项目
            {
                vector<vector<string>> &values = m[extend_s];
                //找到非终结符extend对应的所有产生式
                for(int j=0; j<values.size(); ++j)
                {
                    item addP(extend_s, j, 0, 1);
                    if(values[j][0] == "empty")
                    {
                        addP.index = 1;
                        addP.type = 2;
                    }
                    //判断待扩充项目是否已存在
                    if(find(node.begin(), node.end(), addP) == node.end())
                        node.push_back(addP);
                }
            }
        }
    }
}
