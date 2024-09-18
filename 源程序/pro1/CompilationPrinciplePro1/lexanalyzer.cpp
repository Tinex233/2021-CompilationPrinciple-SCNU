/*
 * Copyright 2024 Tinex. All Rights Reserved.
 * File: lexanalyzer.cpp
 * Version: 1.0
 * Author: Tinex
 * Date: May 15, 2024
 * Description: The file contains the implementation of methods for the NFAGraph, DFAGraph, and LexAnalyzer classes.
*/

#include "lexanalyzer.h"

//nfa
unordered_map<char,int> NFAGraph::isp,NFAGraph::icp;

NFAGraph::NFAGraph(){}

void NFAGraph::initPriNum()
{
    //优先数表初始化，isp:栈内，icp:栈外
    isp['(']=0;
    isp[')']=7;
    isp['*']=6;
    isp['?']=6;
    isp['&']=4;
    isp['|']=2;

    icp['(']=7;
    icp[')']=0;
    icp['*']=5;
    icp['?']=5;
    icp['&']=3;
    icp['|']=1;
}

bool NFAGraph::explain()
{
    return epl;
}

bool NFAGraph::isTrans(const char c)
{
    return !isOperator(c);
}

string NFAGraph::getContent()
{
    return content;
}

bool NFAGraph::isOperator(const char c)
{
    return c=='('||c==')'||c=='?'||c=='|'||c=='*'||c=='&';
}

bool NFAGraph::isLegal(const string regex)//字符是否合法、括号是否匹配
{
    int len=regex.length();
    int num=0;//跟踪括号数量
    for(int i=0;i<len;i++)
    {
        char c=regex[i];
        if(c=='(')
            num++;
        else if(c==')')
        {
            if(num<=0)//先出现右括号
                return false;
            num--;
        }
    }
    return num==0;
}

string NFAGraph::delEmpBrk(string regex)
{
    string res=regex;
    bool f=false;
    while(!f)
    {
        f=true;
        for(int i=0;i<res.length()-1;i++)
        {
            if(res[i]=='('&&res[i+1]==')')
            {
                f=false;
                if(i<res.length()-1)
                    res=res.substr(0,i)+res.substr(i+2);
                else
                    res=res.substr(0,i);
            }
        }
    }
    return res;
}

string NFAGraph::addOP(string regex)
{
    string res=delEmpBrk(regex);
    int len=res.length();
    bool f=0;//f=true时表明发生了转义
    for(int i=0;i<len-1;)//主要是情况枚举
    {
        char c1=res[i],c2=res[i+1];
        if(res[i]=='\\'&&f==0)
        {
            f=true;
            i++;
            continue;
        }

        if((isTrans(c1)||f)||c1==')'||c1=='?'||c1=='*')
        {
            f=false;
            if(isTrans(c2)||c2=='(')
            {
                res=(res.substr(0,i+1)+'&')+res.substr(i+1);
                len=res.length();
                i+=2;
            }
            else
                i++;
        }
        else
            i++;
    }
    escF.resize(res.length());
    content=res;
    for(int i=0;i<res.length();i++)
    {
        if(res[i]=='\\'&&i<res.length()-1)
            escF[++i]=true;
    }
    for(int i=0,j=0;i<res.length()&&j<content.length();i++,j++)
    {
        if(escF[i])
        {
            content=content.substr(0,j-1)+content.substr(j,content.length()-j);
            j--;
        }

        else if(content[j]=='&'&&!escF[i])
        {
            content=content.substr(0,j)+content.substr(j+1,content.length()-j-1);
            j--;
        }
    }

    return res;
}

string NFAGraph::postfix(string regex)
{
//    if(!isLegal(regex))
//    {
//        cout<<"正则表达式格式存在错误"<<endl;
//        return "";
//    }

    regex=addOP(regex);

    //开始转换
    vector<bool> esc(escF.size());
    stack<char> s;
    string str="";
    int len=regex.length();
    for(int i=0;i<len;)
    {
        char c=regex[i];
        if(c=='\\'&&escF[i]==false)
            i++;
        else if(isTrans(c)||escF[i]==true)
        {
            esc[str.length()]=true;
            str+=c;
            i++;
        }
        else
        {
            if(s.empty())
            {
                s.push(c);
                i++;
            }
            else
            {
                char ch=s.top();
                //①当前操作符栈外优先数大于栈顶操作符栈内优先数，当前操作符入栈；
                if(icp[c]>isp[ch])
                {
                    s.push(c);
                    i++;
                }
                //②若小于，栈顶操作符退栈并输出（或并入储存后缀表达式的字符串）；
                else if(icp[c]<isp[ch])
                {
                    while(!s.empty()&&icp[c]<isp[s.top()])
                    {
                        str+=s.top();
                        s.pop();
                    }
                }
                //③若等于，栈顶操作符退栈但不输出，如果退‘(’，读入下一字符。
                else
                {
                    if(s.top()=='(')
                        i++;
                    s.pop();
                }
            }

        }
    }
    while(!s.empty())
    {
        str+=s.top();
        s.pop();
    }
    escF=esc;

    return str;
}

NFAGraph::NFAGraph(string regex)
{
    init(regex);
}

void NFAGraph::init(string regex)
{
    regex=postfix(regex);
    NumOfStatus=0;
    epl=false;
    stack<pii> ss;//后缀计算栈，存储状态
    graph.push_back(set<edge>());
    int len=regex.length();

    for(int i=0;i<len;i++)
    {
        char cur=regex[i];
        if(isTrans(cur)||escF[i]==true)//创建基本正则表达式单字符
        {
            cs.insert(cur);
            int start=++NumOfStatus,end=++NumOfStatus;//创建两个新状态
            graph.push_back(set<edge>()),graph.push_back(set<edge>());
            graph[start].insert(edge(cur,end));
            ss.push({start,end});
        }
        else if(cur=='&')//连接
        {
            pii nt=ss.top();
            ss.pop();
            pii ns=ss.top();
            ss.pop();

            int nsleft=ns.first,nsright=ns.second;
            int ntleft=nt.first,ntright=nt.second;

            graph[nsright].insert(edge('#',ntleft));
            ss.push({nsleft,ntright});
            cs.insert('#');
        }
        else if(cur=='|')//选择
        {
            epl=true;
            pii nt=ss.top();
            ss.pop();
            pii ns=ss.top();
            ss.pop();

            int nsleft=ns.first,nsright=ns.second;
            int ntleft=nt.first,ntright=nt.second;

            int start=++NumOfStatus,end=++NumOfStatus;
            graph.push_back(set<edge>()),graph.push_back(set<edge>());
            graph[start].insert(edge('#',nsleft));
            graph[start].insert(edge('#',ntleft));
            graph[nsright].insert(edge('#',end));
            graph[ntright].insert(edge('#',end));
            ss.push({start,end});
            cs.insert('#');
        }
        else if(cur=='*')//闭包
        {
            epl=true;
            pii ns=ss.top();
            ss.pop();
            int nsleft=ns.first,nsright=ns.second;

            int start=++NumOfStatus,end=++NumOfStatus;
            graph.push_back(set<edge>()),graph.push_back(set<edge>());

            graph[start].insert(edge('#',end));
            graph[start].insert(edge('#',nsleft));
            graph[nsright].insert(edge('#',end));
            graph[nsright].insert(edge('#',nsleft));
            ss.push({start,end});
            cs.insert('#');
        }
        else if(cur=='?')//可选
        {
            epl=true;
            pii ns=ss.top();
            ss.pop();
            int nsleft=ns.first,nsright=ns.second;

            int start=++NumOfStatus,end=++NumOfStatus;
            graph.push_back(set<edge>()),graph.push_back(set<edge>());

            graph[start].insert(edge('#',end));
            graph[start].insert(edge('#',nsleft));
            graph[nsright].insert(edge('#',end));
            ss.push({start,end});
            cs.insert('#');
        }
    }
    if(!ss.empty())
    {
        InitState=ss.top().first;
        AcceptState=ss.top().second;
        //qDebug()<<InitState<<"t"<<AcceptState<<endl;
    }
    escF=vector<bool>();
}

int NFAGraph::getNumOfStatus()
{
    return NumOfStatus;
}

int NFAGraph::getNumOfchar()
{
    return cs.size();
}

set<char> NFAGraph::getCharSet()
{
    return cs;
}

string NFAGraph::getStateTrans(int row, char col)
{
    if(row>NumOfStatus)
        return "";
    set<int> s;
    for(auto i:graph[row])
    {
        if(i.CvType==col)
            s.insert(i.vj);
    }
    string res="";

    for(sti it=s.begin();it!=s.end();)
    {
        res+=to_string(*it);
        it++;
        if(it!=s.end())
            res+=",";
    }
    return res;
}

int NFAGraph::getInitState()
{
    return InitState;
}

int NFAGraph::getAcceptState()
{
    return AcceptState;
}

void NFAGraph::DFS(set<int> &s,int fa)
{
    s.insert(fa);
    for(auto node:graph[fa])
    {
        if(node.CvType=='#')
            DFS(s,node.vj);
    }
}

vector<set<int>> NFAGraph::getClosure()
{
    vector<set<int>> v;
    if(NumOfStatus<=0)
        return v;
    v.push_back(set<int>());

    for(int i=1;i<=NumOfStatus;i++)
    {
        set<int> s;
        DFS(s,i);
        v.push_back(s);
    }

    return v;
}

map<int,int> NFAGraph::getCharTrans(char ch)
{
    map<int,int> m;
    for(int i=1;i<=NumOfStatus;i++)
    {
        for(auto j:graph[i])
        {
            if(j.CvType==ch)
                m[i]=j.vj;
        }
    }
    return m;
}

//dfa
bool setEqual(const set<int> &s1,const set<int> &s2)
{
    if(s1.size()!=s2.size())
        return false;
    sti it1=s1.begin(),it2=s2.begin();
    for(;it1!=s1.end()&&it2!=s2.end();it1++,it2++)
    {
        if(*it1!=*it2)
            return false;
    }
    return true;
}

bool transEqual(const set<edge> &s1,const set<edge> &s2)
{
    if(s1.size()!=s2.size())
        return false;
    stsi it1=s1.begin();
    for(;it1!=s1.end();it1++)
    {
        if(s2.count(*it1)==0)
            return false;
    }
    return true;
}

DFAGraph::DFAGraph(NFAGraph &nfa)
{
    init(nfa);
}

void DFAGraph::init(NFAGraph &nfa)
{
    epl=nfa.explain();
    isMinimized=false;
    vector<set<int>> closure=nfa.getClosure();//ε-闭包
    map<char,map<int,int>> chtrans;//字符转换表
    set<char> chs=nfa.getCharSet();//获取字符集
    for(auto c:chs)
    {
        if(c!='#')
        {
            cs.insert(c);
            chtrans[c]=nfa.getCharTrans(c);
        }
    }

    if(closure.size()<=1)
        return;

    states.push_back(set<int>());
    isAccepted.push_back(false);

    //加入初始状态
    int initst=nfa.getInitState();
    states.push_back(closure[initst]);
    graph.push_back(set<edge>());//下标0不用
    graph.push_back(set<edge>());//为初始状态创建邻接表
    InitState=states.size()-1;
    if(closure[initst].find(nfa.getAcceptState())!=closure[initst].end())
        isAccepted.push_back(true);
    else
    {
        isAccepted.push_back(false);
        isAccepted[0]=1;
    }
    //开始转换
    for(int i=1;i<states.size();i++)
    {
        for(mcmi mi=chtrans.begin();mi!=chtrans.end();mi++)
        {
            char ch=mi->first;//当前转换字符
            set<int> res;
            for(auto j:states[i])//遍历i的状态集合
            {
                map<int,int>::iterator it=chtrans[ch].find(j);
                if(it!=chtrans[ch].end())//当前字符ch，当前状态j，若找到转换的状态，进行集合并
                    set_union(res.begin(),res.end(),closure[it->second].begin(),closure[it->second].end(),inserter(res,res.begin()));
            }
            if(res.empty())
                continue;

            int sz=states.size();
            int w=addState(nfa,res);
            if(sz<states.size())
                graph.push_back(set<edge> ());
            graph[i].insert(edge(ch,w));
        }
    }
    NumOfStates=states.size()-1;
}

int DFAGraph::addState(NFAGraph &nfa,set<int> &s)
{
    for(int i=1;i<states.size();i++)
    {
        if(setEqual(s,states[i]))
            return i;
    }
    states.push_back(s);
    if(s.find(nfa.getAcceptState())!=s.end())
        isAccepted.push_back(true);
    else
    {
        isAccepted.push_back(false);
        isAccepted[0]=1;
    }
    return states.size()-1;
}

int DFAGraph::getNumOfStates()
{
    return NumOfStates;
}

vector<set<edge>> DFAGraph::getGraph()
{
    return graph;
}

bool DFAGraph::acceptState(int i)
{
    if(i>NumOfStates)
        return false;
    return isAccepted[i];
}

bool DFAGraph::explain()
{
    return epl;
}

string DFAGraph::getState(int i)
{
    if(i>NumOfStates)
        return "error";
    string res="";
    for(sti j=states[i].begin();j!=states[i].end();)
    {
        res+=to_string(*j);
        j++;
        if(j!=states[i].end())
            res+=",";
    }
    return res;
}

bool DFAGraph::isMin()
{
    return isMinimized;
}

DFAGraph DFAGraph::toMinDFA()
{
    if(isMinimized)
        return *this;

    DFAGraph mdfa;
    if(NumOfStates<=0)
        return mdfa;

    vector<int> setNum;//集合号表，指示DFA集合被划分到minDFA的哪个集合中
    setNum.resize(NumOfStates+1);
    set<int> k1,k2;//划分为非终止状态集k1和终止状态集k2
    int mNumStates;//MDFA状态数

    if(isAccepted[0]==false)//是否全为接受状态
        mNumStates=1;
    else
        mNumStates=2;

    for(int i=1;i<isAccepted.size();i++)
    {
        if(mNumStates==2)
        {
            if(isAccepted[i])
            {
                setNum[i]=2;
                k2.insert(i);
            }
            else
            {
                setNum[i]=1;
                k1.insert(i);
            }
        }
        else
        {
            setNum[i]=1;
            k2.insert(i);
        }
    }

    deque<set<int>> dq;//状态集合
    if(k1.size()>1)
        dq.push_back(k1);
    if(k2.size()>1)
        dq.push_back(k2);

    bool part=true;//当上一次队列中集合发生过划分时，需要重新检查一遍是否可划分
    while(part)
    {
        if(mNumStates>2)//统计各集合划分情况，对多于一个元素的集合检查是否能划分
        {
            for(int ii=1;ii<=mNumStates;ii++)
            {
                set<int> sss;
                for(int ij=1;ij<=isAccepted.size();ij++)
                    if(setNum[ij]==ii)
                        sss.insert(ij);
                if(sss.size()>1)
                    dq.push_back(sss);
            }
        }
        part=false;
        while(!dq.empty())//开始最小化
        {
            set<int> s=dq.front();
            dq.pop_front();
            vector<set<edge>> ve;//转移集合表
            vector<set<int>> vs;//划分的集合
            for(auto i:s)//对集合s进行分割
            {
                //获取i的能转移到的集合号的集合
                set<edge> ste;
                for(int vi=1;vi<graph.size();vi++)
                {
                    for(auto cm:graph[i])//遍历i的邻接表
                        ste.insert(edge(cm.CvType,setNum[cm.vj]));//在集合号表中找到转换后状态所在的集合
                }

                if(vs.size()==0)
                {
                    ve.push_back(ste);
                    vs.push_back(set<int>());
                    vs[0].insert(i);
                }
                else
                {
                    bool fequal=false;//转换相同标志
                    for(int vi=0;vi<ve.size();vi++)
                    {
                        if(transEqual(ve[vi],ste))
                        {
                            vs[vi].insert(i);//转换相同
                            fequal=true;
                            break;
                        }
                    }
                    if(!fequal)//不同，划分
                    {
                        ve.push_back(ste);
                        vs.push_back(set<int>());
                        vs[vs.size()-1].insert(i);
                        part=true;
                    }
                }
            }
//            for(int i=0;i<vs.size();i++)
//            {
//                qDebug()<<"set"<<i;
//                for(auto vv:vs[i])
//                    qDebug()<<vv;
//            }
            //清算划分集合vs
            for(int j=1;j<vs.size();j++)//为划分出去的集合编号
            {
                if(vs[j].size()>1)
                    dq.push_back(vs[j]);
                mNumStates++;
                for(auto vk:vs[j])//遍历集合内状态，为其赋予新的状态号
                {
                    setNum[vk]=mNumStates;
                }
            }
        }
    }

    mdfa.InitState=setNum[InitState];//mdfa的初始状态是dfa初始状态被划分到的集合
    mdfa.epl=epl;
    mdfa.NumOfStates=mNumStates;
    mdfa.states.resize(mNumStates+1);
    mdfa.isAccepted.resize(mNumStates+1);
    mdfa.isMinimized=true;
    mdfa.cs=set<char> (cs);
    mdfa.graph.resize(mNumStates+1);
    for(int t=1;t<mdfa.states.size();t++)
        mdfa.states[t].insert(t);

    //观察各状态编号
//    for(int ii=1;ii<setNum.size();ii++)
//        qDebug()<<"stateNum"<<ii<<":"<<setNum[ii];

    //合并状态集合
    //接受状态判断
    for(int i=1;i<setNum.size();i++)
    {
        if(isAccepted[i])
            mdfa.isAccepted[setNum[i]]=true;
        else
            mdfa.isAccepted[0]=true;
    }
//    for(int i=1;i<mdfa.isAccepted.size();i++)
//        qDebug()<<"accept"<<mdfa.isAccepted[i];

    //构建最小化DFA的邻接表
    for(int vi=1;vi<graph.size();vi++)
    {
        set<edge> ts;//存储状态合并后的集合转换
        for(auto ls:graph[vi])
        {
            int w=ls.vj;
            char ttype=ls.CvType;
            //映射到合并后集合
            w=setNum[w];
            ts.insert({ttype,w});
        }

        //处理mdfa的状态转换表
        for(auto ti:ts)
            mdfa.graph[setNum[vi]].insert(ti);
    }

    return mdfa;
}

int DFAGraph::getInitState()
{
    return InitState;
}

set<char> DFAGraph::getCharSet()
{
    return cs;
}


//LexAnalyzer
LexAnalyzer::LexAnalyzer()
{
    NumOfRegex=0;
}

void LexAnalyzer::init(vector<string> v)
{
    CaseSensitive=true;
    nlFlag=-1;
    NumOfRegex=0;
    int code=ENCODING_START;
    for(int i=0;i<v.size();i++)
    {
        int j=0;
        while(v[i][j]!=' ') j++;

        string name=v[i].substr(0,j);

        if(v[i].length()-name.length()<=3)
        {
            cout<<"正则表达式格式有误"<<endl;
            return;
        }

        string con=v[i].substr(j+3,v[i].size()-j-3);//表达式内容

        //对关键字、注释与一般正则表达式的处理
        if(name=="KEYWORD")
        {
            int l=0,r=0;
            while(l<con.length())
            {
                while(r<con.length()&&con[r]!='|') r++;
                KWCodeTable[con.substr(l,r-l)]=code++;
                l=r+1,r=l;
            }
        }
        else if(name=="KEYWORD*")
        {
            CaseSensitive=false;
            string str;
            int l=0,r=0;
            while(l<con.length())
            {
                while(r<con.length()&&con[r]!='|') r++;
                str=con.substr(l,r-l);
                transform(str.begin(),str.end(),str.begin(),::toupper);
                KWCodeTable[str]=code;
                transform(str.begin(),str.end(),str.begin(),::tolower);
                KWCodeTable[str]=code++;
                l=r+1,r=l;
            }
        }
        else if(name=="L_COMMENT")
        {
            NFAGraph n=NFAGraph(con);
            LComment.push_back(DFAGraph(n).toMinDFA());
        }
        else if(name=="R_COMMENT")
        {
            NFAGraph n=NFAGraph(con);
            RComment.push_back(DFAGraph(n).toMinDFA());
            if(con=="\\n")
                nlFlag=RComment.size()-1;
        }
        else//一般正则表达式
        {
            REXcontents.push_back(con);
            REXCodeTable.push_back(code++);
            NumOfRegex++;
            regexName.push_back(name);
        }
    }

//    for(auto s:KWCodeTable)
//        cout<<s.first<<" "<<s.second<<endl;

//    for(auto t:REXCodeTable)
//        cout<<t<<endl;

}

void LexAnalyzer::getKMPNext(vector<int>& ne,int x)
{
    int i=0,j=-1;
    ne[0]=-1;
    while(i<regexName[x].length())
    {
        if(j==-1||regexName[x][i]==regexName[x][j])
        {
            i++;
            j++;

            if(regexName[x][i]!=regexName[x][j])
                ne[i]=j;
            else
                ne[i]=ne[j];
        }
        else
            j=ne[j];
    }
}

const string LexAnalyzer::getCodeList()
{
    string res="Encoding Table[\n";

    if(KWCodeTable.size()>0)
    {
        for(auto p:KWCodeTable)
        {
            if(!CaseSensitive&&(p.first[0]>='A'&&p.first[0]<='Z'))//大小写不敏感时仅输出小写编码
                continue;
            res+="  ";
            res+=p.first;
            res+=" ";
            res+=to_string(p.second);
            res+="\n";
        }
    }
    for(int i=0;i<NumOfRegex;i++)
    {
        res+="  ";
        if(DFAgroup[i].explain())
        {
            res+=regexName[i];
            res+=" ";
            res+=to_string(REXCodeTable[i]);
            res+="*";
        }
        else
        {
            res+=string(REXcontents[i]);
            res+=" ";
            res+=to_string(REXCodeTable[i]);
        }
        res+="\n";
    }
    res+="]\n";
    return res;
}

int LexAnalyzer::getNumOfRegex()
{
    return NumOfRegex;
}

void LexAnalyzer::anlyRegex()
{
    //执行KMP算法，匹配并替换正则表达式
    vector<int> next[NumOfRegex];
    for(int k=0;k<NumOfRegex;k++)
    {
        next[k].resize(regexName[k].length());
        getKMPNext(next[k],k);
    }

    //执行KMP匹配并替换表达式
    for(int i=0;i<NumOfRegex;i++)
    {
        for(int j=0;j<NumOfRegex;j++)
        {
            if(i==j)
                continue;
            string s=REXcontents[i],t=regexName[j];
            int sl=s.length(),tl=t.length();
            if(sl<tl)
                continue;
            int ii=0,ij=0;
            while(ii<sl&&ij<tl)
            {
                if(ij==-1||s[ii]==t[ij])
                    ii++,ij++;
                else
                    ij=next[j][ij];
                if(ij>=tl)
                {
                    string ss="("+REXcontents[j];
                    ss+=")";
                    s=s.substr(0,ii-tl)+ss+s.substr(ii,sl-ii);
                    ij=0;
                    ii=ii-tl+ss.length();
                    sl=s.length();
                }
            }
            REXcontents[i]=s;
        }
        NFAgroup.push_back(NFAGraph(REXcontents[i]));
        REXcontents[i]=NFAgroup[NFAgroup.size()-1].content;
        DFAgroup.push_back(DFAGraph(NFAgroup[i]));
    }

}

void LexAnalyzer::minimizeDFA()
{
    for(int i=0;i<NumOfRegex;i++)
        DFAgroup[i]=DFAgroup[i].toMinDFA();
}

const vector<NFAGraph> LexAnalyzer::getNFAgroup()
{
    return NFAgroup;
}

const vector<DFAGraph> LexAnalyzer::getDFAgroup()
{
    return DFAgroup;
}

const vector<string> LexAnalyzer::getNameOfRex()
{
    return regexName;
}

const string LexAnalyzer::GenProgram()
{
    string head="#include<iostream>\n"
                "#include<map>\n"
                "#include<string>\n"
                "#define NumOfRegex ";

    head+=to_string(NumOfRegex);
    head+="\n#define NumOfComment ";
    head+=to_string(LComment.size());
    head+="\nusing namespace std;\n";

    head+="bool CaseSensitive=true;\n"
          "int (*regex[NumOfRegex])(string &,int,int);\n"
          "int (*lcomment[NumOfComment])(string &,int,int);\n"
          "int (*rcomment[NumOfComment])(string &,int,int);\n"
          "string regexName[NumOfRegex];\n"
          "int REXCodeTable[NumOfRegex];\n"
          "map<string,int> KWCodeTable;\n"
          "bool epl[NumOfRegex];\n";

    //生成注释的正则表达式
    for(int i=0;i<LComment.size();i++)
    {
        head+=Gen1Function(LComment[i],i,2);
        head+=Gen1Function(RComment[i],i,3);
    }

    //生成各正则表达式的分析程序
    for(int i=0;i<NumOfRegex;i++)
        head+=Gen1Function(DFAgroup[i],i,1);

    head+="void init()\n"
                  "{\n";

    if(CaseSensitive)
        head+="\tCaseSensitive=true;\n";
    else
        head+="\tCaseSensitive=false;\n";

    if(KWCodeTable.size()>0)
    {
        for(auto p:KWCodeTable)
        {
            head+="\tKWCodeTable[\"";
            head+=p.first;
            head+="\"]=";
            head+=to_string(p.second);
            head+=";\n";
        }
    }
    for(int i=0;i<NumOfRegex;i++)
    {
        string num=to_string(i);
        head+="\tregexName[";
        head+=num;
        head+="]=\"";
        if(DFAgroup[i].explain())
            head+=regexName[i];
        else
            head+=REXcontents[i];
        head+="\";\n"
              "\tREXCodeTable[";
        head+=to_string(i);
        head+="]=";
        head+=to_string(REXCodeTable[i]);
        head+=";\n"
              "\tregex[";
        head+=num;
        head+="]=F";
        head+=num;
        head+=";\n";
        if(DFAgroup[i].explain())
        {
            head+="\tepl[";
            head+=num;
            head+="]=true;\n";
        }
    }
    for(int i=0;i<LComment.size();i++)
    {
        string num=to_string(i);
        head+="\tlcomment[";
        head+=num;
        head+="]=LC";
        head+=num;
        head+=";\n";
        head+="\trcomment[";
        head+=num;
        head+="]=RC";
        head+=num;
        head+=";\n";
    }

    head+="}\n"
          "void print()\n"
          "{\n\tcout<<\"";

    head+="Encoding Table[\\n\";\n"
          "\tfor(auto i:KWCodeTable)\n\t{\n"
          "\t\tif(!CaseSensitive&&(i.first[0]>='A'&&i.first[0]<='Z'))\n"
          "\t\t\tcontinue;\n"
          "\t\tcout<<\"    \"<<i.first<<\" \"<<i.second<<\"\\n\";\n\t}\n"
          "\tfor(int j=0;j<NumOfRegex;j++)\n\t{\n"
          "\t\tcout<<\"    \"<<regexName[j]<<\" \"<<REXCodeTable[j];\n"
          "\t\tif(epl[j])\n"
          "\t\t\tcout<<\"*\";\n"
          "\t\tcout<<\"\\n\";\n\t}\n"
          "\tcout<<\"]\\n\";\n}\n";

    string inMain="int main()\n{\n"
                  "\tinit();\n"
                  "\tprint();\n"
                  "\tstring pro=\"\",s;\n"
                  "\tcout<<\"Enter your program:\\n\";\n"
                  "\twhile(getline(cin,s))\n\t{\n"
                  "\t\tif(s==\"EOF\")\n"
                  "\t\t\tbreak;\n"
                  "\t\tpro+=s+'\\n';\n\t}\n"
                  "\tint l=0,r=0,len=pro.length();\n"
                  "\twhile(l<len)\n"
                  "\t{\n"
                  "\t\twhile(pro[l]==' '||pro[l]=='\\n'||pro[l]=='\\t') l++;\n"
                  "\t\tif(l>=len) break;\n"
                  "\t\tr=l+1;\n"
                  "\t\twhile(r<len&&pro[r]!=' ') r++;\n"
                  "\t\tint i,maxl=0,pos=-1;\n"
                  "\t\tfor(int ii=0;ii<NumOfComment;ii++)\n\t\t{\n"
                  "\t\t\ti=lcomment[ii](pro,l,r);\n"
                  "\t\t\tif(i>=l)\n\t\t\t{\n"
                  "\t\t\t\tpos=ii;\n"
                  "\t\t\t\tl=i+1;\n"
                  "\t\t\t\tbreak;\n\t\t\t}\n"
                  "\t\t}\n"
                  "\t\tif(pos!=-1)\n\t\t{\n"
                  "\t\t\tint po=-1;\n"
                  "\t\t\twhile(po<l)\n"
                  "\t\t\t\tpo=rcomment[pos](pro,l++,len);\n"
                  "\t\t\tif(po==-1)\n\t\t\t{\n"
                  "\t\t\t\tcout<<\"ERROR:COMMENT\\n\";\n"
                  "\t\t\t\tl=len;\n\t\t\t}\n"
                  "\t\t\telse\n"
                  "\t\t\t\tl=po;\n"
                  "\t\t\tcontinue;\n"
                  "\t\t}\n"
                  "\t\tfor(int j=0;j<NumOfRegex;j++)\n\t\t{\n"
                  "\t\t\ti=regex[j](pro,l,r);\n"
                  "\t\t\tif(i>=l&&i-l+1>maxl)\n\t\t\t{\n"
                  "\t\t\t\tmaxl=i-l+1;\n"
                  "\t\t\t\tpos=j;\n"
                  "\t\t\t}\n"
                  "\t\t}\n"
                  "\t\tif(pos!=-1)\n\t\t{\n"
                  "\t\t\tstring s1=pro.substr(l,maxl);\n"
                  "\t\t\tif(KWCodeTable.find(s1)!=KWCodeTable.end())\n\t\t\t{\n"
                  "\t\t\t\tcout<<to_string(KWCodeTable[s1])<<\" \";\n"
                  "\t\t\t\tl+=maxl;\n\t\t\t}\n"
                  "\t\t\telse\n\t\t\t{\n"
                  "\t\t\t\tcout<<to_string(REXCodeTable[pos])<<\" \";\n"
                  "\t\t\t\tif(epl[pos])\n"
                  "\t\t\t\t\tcout<<s1<<\" \";\n"
                  "\t\t\t\tl+=maxl;\n\t\t\t}\n\t\t}\n"
                  "\t\telse\n\t\t{\n"
                  "\t\t\tif(pos=-1)\n\t\t\t{\n"
                  "\t\t\t\tstring s1=pro.substr(l,r-l);\n"
                  "\t\t\t\tif(KWCodeTable.find(s1)!=KWCodeTable.end())\n\t\t\t\t{\n"
                  "\t\t\t\t\tcout<<to_string(KWCodeTable[s1])<<\" \";\n"
                  "\t\t\t\t\tl=r;\n\t\t\t\t}\n"
                  "\t\t\t\telse\n\t\t\t\t{\n"
                  "\t\t\t\t\tcout<<\"ERROR:\"<<pro.substr(l,r-l)<<\" \";\n"
                  "\t\t\t\t\tl=r;\n\t\t\t\t}\n"
                  "\t\t\t}\n\t\t}\n\t}\n";

    inMain+="\treturn 0;\n"
            "}\n";
    return head+inMain;
}

const string LexAnalyzer::outputCode(string prog)
{
    string res="";

    int l=0,r=0,len=prog.length();
    while(l<len)
    {
        //跳过空格、tab和\n
        while(prog[l]==' '||prog[l]=='\n'||prog[l]=='\t') l++;
        if(l>=len) break;
        r=l+1;
        while(r<len&&prog[r]!=' ') r++;

        //[l,r)范围是一个token
        int i,maxl=0,pos=-1;
        //注释匹配
        for(int ii=0;ii<LComment.size();ii++)
        {
            i=matchRegex(prog,l,r,ii,LComment);
            if(i>=l)
            {
                pos=ii;
                l=i+1;
                break;
            }
        }
        if(pos!=-1)//匹配成功
        {
            int po=matchComment(prog,l,len,pos);
            if(po==-1)
            {
                res+="ERROR:COMMENT\n";
                l=len;
            }
            else
            {
                l=po;
            }
            continue;
        }


        //正则表达式的判断
        for(int j=0;j<this->NumOfRegex;j++)
        {
            i=matchRegex(prog,l,r,j,DFAgroup);
            if(i>=l&&i-l+1>maxl)//匹配成功且最长
            {
                maxl=i-l+1;
                pos=j;
            }
        }

        if(pos!=-1)//成功匹配某条正则表达式
        {
            string s1=prog.substr(l,maxl);
            if(KWCodeTable.find(s1)!=KWCodeTable.end())//是关键字
            {
                res+=to_string(KWCodeTable[s1]);
                res+=" ";
                l+=maxl;
            }
            else
            {
                res+=to_string(REXCodeTable[pos]);
                res+=" ";
                if(DFAgroup[pos].explain())
                {
                    res+=s1;
                    res+=" ";
                }
                l+=maxl;
            }  
        }
        else//匹配失败
        {
            if(pos==-1)
            {
                string s1=prog.substr(l,r-l);
                if(KWCodeTable.find(s1)!=KWCodeTable.end())//是关键字
                {
                    res+=to_string(KWCodeTable[s1]);
                    res+=" ";
                    l=r;
                }
                else
                {
                    res+="ERROR:";
                    res+=prog.substr(l,r-l);
                    res+=" ";
                    l=r;
                }
            }
        }
    }

    return res;
}

const int LexAnalyzer::matchRegex(string &p, int l, int r,int k,vector<DFAGraph> &v)
{
    int i=v[k].InitState,accpos=l-1;
    while(l<r&&i!=-1)
    {
        if(v[k].graph[i].size()==0) break;
        for(auto j:v[k].graph[i])
        {
            if(j.CvType==p[l])
            {
                i=j.vj;
                l++;
                if(v[k].acceptState(i))
                    accpos=l;
                break;
            }
            else
                i=-1;
        }
    }
    return accpos-1;
}

const int LexAnalyzer::matchComment(string &p, int l, int len, int k)
{
    bool suc=false;
    if(k==nlFlag)
    {
        while(l<len)
        {
            if(p[l]=='\n'||l==len-1)
            {
                suc=true;
                l++;
                return l;
            }
            l++;
        }
    }
    else
    {
        while(l<len)
        {
            int ll=matchRegex(p,l,len,k,RComment);
            if(ll<l)//匹配失败
                l++;
            else
            {
                suc=true;
                l=ll+1;
                break;
            }
        }
    }

    if(!suc)
        return -1;
    return l;
}

const string LexAnalyzer::Gen1Function(DFAGraph &dfa,int k,int mode)
{
    if(!dfa.isMin())
        return "This DFA isn't the minimized DFA!";

    string res;
    if(mode==1)
        res="int F";
    else if(mode==2)
        res+="int LC";
    else if(mode==3)
    {
        res+="int RC";
        if(k==nlFlag)//右注释为换行符
        {
            res+=to_string(k);
            res+="(string &pro,int l,int r)\n{\n";
            res+="\twhile(l<r)\n\t{\n"
                 "\t\tif(pro[l]=='\\n'||l==r-1)\n\t\t{\n"
                 "\t\t\tl++;\n"
                 "\t\t\treturn l;\n\t\t}\n"
                 "\t\tl++;\n\t}\n}\n";
            return res;
        }
        else
        {
            res+=to_string(k);
            res+="(string &pro,int l,int r)\n{\n";
            res+="\twhile(l<r)\n\t{\n\t\tint state=";
            int initState=dfa.getInitState();
            res+=to_string(initState);
            res+=";\n\t\tint acpos=l;\n";
            string cyCondition="\t\twhile((";//循环条件
            string cyBody="\t\t{\n\t\tchar trans=pro[l++];\n\t\t\tswitch(state)\n\t\t\t{\n";//循环体内容
            vector<set<edge>> g=dfa.getGraph();
            for(int i=1;i<g.size();i++)
            {
                if(g[i].size()>0)//状态i有转换
                    cyCondition+="state=="+to_string(i)+"||";
                if(g[i].size()>0)
                {
                    cyBody+="\t\t\tcase "+to_string(i)+":\n";
                    cyBody+="\t\t\t\tswitch(trans)\n\t\t\t\t{\n";
                    for(auto j:g[i])
                    {
                        cyBody+="\t\t\t\tcase \'";
                        cyBody+=j.CvType;
                        cyBody+="\':\n";
                        cyBody+="\t\t\t\t\tstate="+to_string(j.vj);
                        cyBody+=";\n\t\t\t\t\tbreak;\n";
                    }
                    cyBody+="\t\t\t\tdefault:\n\t\t\t\t\tstate=-1;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n";
                    cyBody+="\t\t\t\tbreak;\n";
                }
            }
            cyCondition=cyCondition.substr(0,cyCondition.length()-2)+"))\n";
            cyBody+="\t\t\tdefault:\n\t\t\t\tstate=-1;\n\t\t\t\tbreak;\n\t\t\t}\n";
            string pgend="\n\t\t\tif(";
            for(int i=1;i<g.size();i++)
            {
                if(dfa.acceptState(i))
                    pgend+="state=="+to_string(i)+"||";
            }
            pgend=pgend.substr(0,pgend.size()-2)+")\n\t\t\t\treturn l;\n\t\t}\n";
            pgend+="\t\tl++;\n\t}\n\treturn -1;\n}\n";
            res+=cyCondition;
            res+=cyBody;
            res+=pgend;
            return res;
        }
    }

    res+=to_string(k);
    res+="(string &pro,int l,int r)\n{\n"
         "\tint state=";
    int initState=dfa.getInitState();
    res+=to_string(initState);
    res+=";\n\tint acpos=l;\n";
    string cyCondition="\twhile((";//循环条件
    string cyBody="\t{\n\t\tchar trans=pro[l++];\n\t\tswitch(state)\n\t\t{\n";//循环体内容
    vector<set<edge>> g=dfa.getGraph();
    for(int i=1;i<g.size();i++)
    {
        if(g[i].size()>0)//状态i有转换
            cyCondition+="state=="+to_string(i)+"||";
        if(g[i].size()>0)
        {
            cyBody+="\t\tcase "+to_string(i)+":\n";
            cyBody+="\t\t\tswitch(trans)\n\t\t\t{\n";
            for(auto j:g[i])
            {
                cyBody+="\t\t\tcase \'";
                cyBody+=j.CvType;
                cyBody+="\':\n";
                cyBody+="\t\t\t\tstate="+to_string(j.vj);
                cyBody+=";\n\t\t\t\tbreak;\n";
            }
            cyBody+="\t\t\tdefault:\n\t\t\t\tstate=-1;\n\t\t\t\tbreak;\n\t\t\t}\n";
            cyBody+="\t\t\tbreak;\n";
        }
    }
    cyCondition=cyCondition.substr(0,cyCondition.length()-2)+")&&l<r)\n";
    cyBody+="\t\tdefault:\n\t\t\tstate=-1;\n\t\t\tbreak;\n\t\t}\n";
    string pgend="\n\t\tif(";
    for(int i=1;i<g.size();i++)
    {
        if(dfa.acceptState(i))
            pgend+="state=="+to_string(i)+"||";
    }
    pgend=pgend.substr(0,pgend.size()-2)+")\n\t\t\tacpos=l;\n\t}\n";
    pgend+="\treturn acpos-1;\n}\n";
    res+=cyCondition;
    res+=cyBody;
    res+=pgend;

    return res;
}
