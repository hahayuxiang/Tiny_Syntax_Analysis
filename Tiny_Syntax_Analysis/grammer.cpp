#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>

#include "lexical.h"

typedef std::pair<std::string, std::string> PSS;
const int N = 10010;
struct Node {
	std::string name;
	std::vector<Node*> child;
	int NO;
	Node(std::string name) : name(name) {}
};

struct Tree
{
	Node* root;
};
class Grammer
{
private:
	std::string filename; // 文件名
	std::string testcode; // 测试代码
	Lexical lexical; // 词法分析对象
	std::vector<std::string> T; //终结符
	std::vector<std::string> NT; //非终结符
	std::map<std::string, int> NT2cnt; //非终结符出现的次数，方便打印
	std::map<std::string, std::vector<std::string>> P; //产生式
	std::map<std::string, std::set<std::string>> FIRST; //FIRST集
	std::map<std::string, std::set<std::string>> FOLLOW; //FOLLOW集
	std::map<PSS, std::string> TABLE; // 预测分析表
	int h[N], e[N], ne[N], idx; // 链式前向星
	Tree tree; // 语法树

public:
	Grammer(std::string fileName, std::string testCode)
	{
		memset(h, -1, sizeof h);
		filename = fileName;
		testcode = testCode;
	}
	void run(); //运行整个程序
	void outputTest(); // 对过程性的代码进行测试
	void readGrammer(std::string fileName); 
	void addP(std::string left, std::string right); //添加产生式
	void addT(); // 添加终结符
	void eliminateLeftRecursion(); // 消除左递归
	void eliminateDirectLeftRecursion(std::string left); // 消除直接左递归
	std::string maxPrefix(std::string left); // 求最大前缀
	void leftFactoring(); // 提取左公因子
	void getFirst(); // 求FIRST集
	void dfsFirst(std::string si, std::string res);	// 求FIRST集的递归函数
	void getFollow(); // 求FOLLOW集
	void getTable(); // 求预测分析表
	void procedure(); // 总控分析程序
	std::set<std::string> spilt(std::string s); // 求字符串的集合
	std::vector<std::string> spiltToVector(std::string s);
	void ERROR(std::string X, std::string a); // 错误处理
	void preorder(Node *root, int depth);
	void drawPictures();
	void add(int a, int b); // 添加边

	
};
void Grammer::ERROR(std::string X, std::string a) 
{
	std::cout << "ERROR" << std::endl;
	std::cout << "未找到M["+X+","+ a+"]对应的产生式" << std::endl;
	exit(0);
}
void Grammer::add(int a, int b) {
	e[idx] = b, ne[idx] = h[a], h[a] = idx++;
}
std::set<std::string> Grammer::spilt(std::string s) {
	s += '#';
	std::set<std::string> res;
	std::string tmp = "";
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == ' ' || s[i] == '#') {
			res.insert(tmp);
			tmp = "";
		}
		else tmp += s[i];
	}
	return res;
}
std::vector<std::string> Grammer::spiltToVector(std::string s) {
	s += '#';
	std::vector<std::string> res;
	std::string tmp = "";
	for (int i = 0; i < s.size(); i++) {
		if (s[i] == ' ' || s[i] == '#') {
			res.push_back(tmp);
			tmp = "";
		}
		else tmp += s[i];
	}
	return res;
}

void Grammer::addP(std::string left, std::string right) {
	right += '#';
	std::string sRight = "";
	for (int i = 0; i < right.size(); i++) {
		if (right[i] == '|' || right[i] == '#') {
			P[left].push_back(sRight);
			sRight = "";
		}
		else {
			sRight += right[i];
		}
	}	
}
void Grammer::addT() {
	std::string t = "";
	for (auto left : NT) {
		for (auto right : P[left]) {
			right += '#';
			for (int i = 0; i < right.size(); i++) {
				if (right[i] == ' ' || right[i] == '#') { //需要right[i] == '|'吗？
					if (t != "@" && std::find(NT.begin(), NT.end(), t) == NT.end()) {
						T.push_back(t);						
					}
					t = "";
				}				
				else t += right[i];
			}
			
		}
	}
	std::sort(T.begin(), T.end());
	T.erase(std::unique(T.begin(), T.end()), T.end());
}
void Grammer::readGrammer(std::string fileName) 
{
	std::ifstream fin(fileName);
	if (!fin.is_open())
	{
		std::cout << "文件打开失败\n";
		return;
	}
	std::string line;
	while (std::getline(fin, line)) {
		std::string Left = "";
		std::string Right = "";
		int i = 0;
		while (line[i] != '-' && line[i + 1] != '>') {
			Left += line[i];
			i++;
		}
		NT.push_back(Left);
		i += 2;
		Right = line.substr(i);
		addP(Left, Right);
	}
	addT();
	fin.close();	
}

void Grammer::eliminateDirectLeftRecursion(std::string left) {
	// 按照定义产生新规则
	std::string newNT = left + "'";
	NT.insert(std::find(NT.begin(), NT.end(), left) + 1, newNT);
	std::vector<std::string> newFirstP;
	std::vector<std::string> newSecondP;
	for (auto right : P[left]) {
		std::string tmp = "";
		int pos = right.find(" ");
		tmp = right.substr(0, pos);
		if (tmp == left) {
			newSecondP.push_back(right.substr(pos + 1) + " " + newNT);
		}
		else {
			newFirstP.push_back(right + " " + newNT);
		}
	}
	newSecondP.push_back("@");
	P[newNT] = newSecondP;
	P[left] = newFirstP;	
}
void Grammer::eliminateLeftRecursion()
{
	// 通过替换将所有的规则按一定顺序排列，就是将后面的规则
	// 使用前面已经存在的规则进行替换
	for (int i = 0; i < NT.size(); i++) {
		std::string Ai = NT[i];
		std::vector<std::string> newRight;
		std::vector<std::string> oldRight = P[Ai];
		for (auto right : oldRight) {
			int flag = 0;
			
			for (int j = 0; j < i; j++) {
				std::string Aj = NT[j];
				std::string tmp = "";
				int pos = right.find(" ");
				if (pos == -1) tmp = right;
				else tmp = right.substr(0, pos);
				
				if (tmp == Aj) {
					flag = 1;					
					for (auto jt : P[Aj]) {
						if (pos != -1) {
							jt = jt + " " + right.substr(pos + 1);
						}
						newRight.push_back(jt);
					}
				}
			}
			if (flag == 0) {
				newRight.push_back(right);
			}
		}
		
		if (!i) P[Ai] = newRight;

	// 消除所有规则的直接左递归
		for (int j = 0; j < P[Ai].size(); j++) {
			std::string  right = P[Ai][j];
			std::string tmp = "";
			int pos = right.find(" ");
			if (pos == -1) tmp = right;
			else tmp = right.substr(0, pos);
			
			if (tmp == Ai) {
				eliminateDirectLeftRecursion(Ai);
			}
		}
	}
}

std::string Grammer::maxPrefix(std::string left) {
	std::vector<std::string> tmpP = P[left];
	std::string prefix = "";
	std::string firstP = tmpP[0];
	firstP += " ";
	while (firstP.find(' ') != firstP.size() - 1) {
		std::string temp = firstP.substr(0, firstP.find(' ')); // 第一个产生式的第一个字符
		
		for (int i = 1; i < tmpP.size(); i++) {
			std::string right = tmpP[i];
			right += " ";
			
			std::string cmp = right.substr(0, right.find(' '));// 后面产生式的第一个字符
			if (cmp != temp) {
				break;
			}
			else {
				prefix += temp + " ";
			}
			tmpP[i] = right.substr(right.find(' ') + 1);
		}
		firstP = firstP.substr(firstP.find(' ') + 1);
	}
	prefix.erase(prefix.find_last_not_of(" ") + 1);
	return prefix;
}
void Grammer::leftFactoring()
{

	for (int i = 0; i < NT.size(); i++) {
		std::string left = NT[i];
		std::string prefix = maxPrefix(left);
		// 如果公共前缀不为空，提取公因式并生成新的规则
		if (prefix != "") {
			std::string newNT = left + "'";
			NT.insert(find(NT.begin(), NT.end(), left), newNT);
			
			std::vector<std::string> newFirstRight;
			std::vector<std::string> newSecondRight;
			for (auto right : P[left]) {
				std::string newRight;
				if (right.find(prefix) == -1) {
					newFirstRight.push_back(right);
				}
				else {
					if (right.find(prefix) + prefix.size() != right.size()) {
						newRight = right.substr(right.find(prefix) + prefix.size() + 1);
					}
					else newRight = "@";
					newSecondRight.push_back(newRight);
				}
			}
			newFirstRight.push_back(prefix + " " + newNT);
			P[left] = newFirstRight;
			P[newNT] = newSecondRight;
		}
	}
}

void Grammer::dfsFirst(std::string si, std::string res) {
	// 如果当前分析串为空，直接加入
	if (res == "@") {
		FIRST[si].insert("@");
		return;
	}
	else {
		// 取出第一个token
		std::string tmp = "";
		int pos = res.find(" ");
		if (pos != -1) tmp = res.substr(0, pos);
		else tmp = res;
		// 如果为非空的终结符，直接加入并返回
		if (std::count(T.begin(), T.end(), tmp) && tmp != "@") {
			FIRST[si].insert(tmp);
			return;
		}
		// 非空的话，直接继续递归分析
		if (tmp == "@") {
			std::string next = res.substr(pos + 1);
			dfsFirst(si, next);
		}
		// 不为空的话，将非终结符替换后递归分析
		else {
			for (auto newp : P[tmp]) {
				std::string next = "";
				if (pos != -1) next = newp + " " + res.substr(pos + 1);
				else next = newp;
				dfsFirst(si, next);
			}
		}
	}
}

void Grammer::getFirst() {

	//求规则右部字符串的FIRST集合
	for (auto nt : NT) {
		
		for (int i = 0; i < P[nt].size(); i++) {
			std::string right = P[nt][i];
			dfsFirst(right, right);
		}
	}


	//根据右部求规则左部字符的FIRST集合
	for (auto nt : NT) {
		std::set<std::string> tmp;
		for (auto p : P[nt]) {
			tmp.insert(FIRST[p].begin(), FIRST[p].end());
		}
		FIRST[nt].insert(tmp.begin(), tmp.end());
	}
}

void Grammer::getFollow()
{
	// 枚举所有非终结符，为当前分析对象
	std::string ST = NT[0];
	for (int i = 0; i < NT.size(); i ++ ) {
		std::string nt = NT[i];
		// 文法开始符号添加“@”
		if (!i) FOLLOW[nt].insert("@");
		// 枚举所有规则
		for (int j = 0; j < NT.size(); j++) {
			std::string newnt = NT[j];
			for (auto right : P[newnt]) {
				// 将当前规则分离开，便于查找
				std::set<std::string> findNT = spilt(right);
				// 如果当前规则包含当前的分析对象
				if (findNT.count(nt)) {
					int pos = right.find(nt);
					// 不在最后一个
					if (pos != right.size() - nt.size()) {
						// 取出下一个字符
						std::string behind = "";
						int idxx = pos + nt.size() + 1;
						while (right[idxx] != ' ' && idxx < right.size()) behind += right[idxx], idxx ++;
					    //如果是终结符，则直接添加
						if (std::count(T.begin(), T.end(), behind)) FOLLOW[nt].insert(behind);
						//如果FIRST集合中由空，则有依赖的FOLLOW集合，添加依赖边，并且删除集合中的空符号
						else {
							FOLLOW[nt].insert(FIRST[behind].begin(), FIRST[behind].end());
							if (FIRST[behind].count("@")) {
								if(nt != ST) FOLLOW[nt].erase("@");
								add(j, i);
							}
						}
					}
					// 在最后一个，存在依赖的FOLLOW集合，添加边
					else {
						add(j, i);
					}
				}
			}
		}
	}
	//通过bfs求依赖图的最终收敛结果
	std::queue<int> q;
	//把所有的点添加进队列
	for (int i = 0; i < NT.size(); i++) {
		q.push(i);
	}

	while (q.size()) {
		int t = q.front();
		std::string T = NT[t];
		q.pop();
		//遍历当前点依赖的点，将所依赖的集合并入自己的集合
		for (int i = h[t]; i != -1; i = ne[i]) {
			int j = e[i];
			std::string J = NT[j];
			std::set<std::string> checkBackUp = FOLLOW[J];
			FOLLOW[J].insert(FOLLOW[T].begin(), FOLLOW[T].end());
			//如果并入后与之前没改变，表示当前点已经收敛，不需要添加进队列；反之加进队列
			if (FOLLOW[J] != checkBackUp) {
				q.push(j);
			}
		}
	}
}
void Grammer::getTable() {
	for (auto nt : NT) {
		for (auto p : P[nt]) {
			for (auto t : FIRST[p]) {
				if (t != "@") {
					TABLE[{nt, t}] = p;
				}
				else {
					for (auto q : FOLLOW[nt]) {
						if (q != "@") {
							TABLE[{nt, q}] = p;
						}
						else {
							TABLE[{nt, "@"}] = p;
						}
					}
				}
			}
		}
	}
 }

void Grammer::procedure() {
	
	std::ifstream in(testcode, std::ios::in);
	std::istreambuf_iterator<char> beg(in), end;
	std::string str(beg, end);
	in.close();

	lexical.run(str);
	int cnt = 0; // 树的结点编号
	std::stack<std::string> analysisStack;
	analysisStack.push("#");
	analysisStack.push(NT[0]);
	tree.root = new Node(NT[0]);
	tree.root->NO = cnt ++;
	std::stack<Node*> q;
	q.push(tree.root);
	lexical.tokenStream[lexical.tokenCnt - 1] = Token(ENDFILE, "@");
	for (int i = 0; i < lexical.tokenCnt; i++) {
		Token token = lexical.tokenStream[i];
		std::string typetoken = lexical.tokenTypeStr(token.type);
		std::string value = token.str;
		std::string top = analysisStack.top();
		while (top == "@") {
			analysisStack.pop();
			top = analysisStack.top();
		}
		if (std::count(NT.begin(), NT.end(), top)) {
			std::string replace;
			if (TABLE.count({ top, value })) {
				replace = TABLE[{top, value}];
			}
			else if (TABLE.count({ top, typetoken })) {
				replace = TABLE[{top, typetoken}];
			}
			else {
				ERROR(top, typetoken);
				break;
			}
			Node* node = q.top();
			while (std::count(T.begin(), T.end(), node->name) || node->name == "@") {
				q.pop();
				node = q.top();
			}
			q.pop();
			std::vector<std::string> replaceSet = spiltToVector(replace);
			analysisStack.pop();
			for (int j = replaceSet.size() - 1; j >= 0; j--) {
				Node* newnode = new Node(replaceSet[j]);
				newnode->NO = cnt++;
				node->child.push_back(newnode);
				q.push(newnode);
				analysisStack.push(replaceSet[j]);
			}
			reverse(node->child.begin(), node->child.end());
			i--;
		}
		else {
			if (top == "#") {
				std::cout << "success\n";
				
			}
			else if (top == value || top == typetoken) {
				analysisStack.pop();
			}
			else {
				ERROR(top, value);
				break;
			}
		}
	}
}

void Grammer::preorder(Node* root, int depth) {
	if (root == NULL) {
		return;
	}
	std::ofstream out("tree.dot", std::ios::app);
	/*for (int i = 0; i < depth; i++) {
		out << "  ";
	}*/
	//out << root->NO << ":" << root->name << "\n";
	for (auto child : root->child) {
		std::string A = root->name, B = child->name;
		out << "\"" << root->NO << ":" << "" << A << "\"->\"" << child->NO << ":" << B << "\"\n";
		preorder(child, depth + 1);
	}	
}

void Grammer::drawPictures() {
	std::ofstream out("tree.dot", std::ios::out);
	out << "digraph binaryTree {\n";
	out << "node[shape=circle,color=red,fontcolor=blue,fontsize=10];\n";	
	out.close();
	preorder(tree.root, 0);
	std::ofstream outt("tree.dot", std::ios::app);
	


	// 添加子节点的顺序限制
	std::queue<Node*> q;
	q.push(tree.root);
	while(q.size()) {
		Node* node = q.front();
		q.pop();
		if (node->child.size() > 1) {
			outt << "{\nrank=same;\n";
			int t = node->NO;
			std::string rank = "rank" + std::to_string(t);
			outt << "\"" << rank << "\"" << "[style=invis];\n";
			outt << "\"" << rank << "\"";
			for (auto child : node->child) {
				outt << "->" << "\"" << child->NO << ":" << child->name << "\"";				
			}					
			outt << "[style=invis];\n";
			outt << "rankdir=LR;\n}\n";
		}		
		for (auto child : node->child) {
			q.push(child);
		}
	}
	
	outt << "}\n";
	outt.close();	
	system("dot -Tpng tree.dot -o tree.png");

	
}
void Grammer::outputTest() {
	std::cout << "终结符：\n";
	for (auto t : T) {
		std::cout << t << " ";
	}
	std::cout << "\n非终结符：\n";
	for (auto nt : NT) {
		std::cout << nt << " ";
	}
	std::cout << "\n产生式：\n";
	for (auto p : P) {
		std::cout << p.first << "->";
		for (auto right : p.second) {
			std::cout << right << "|";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	for (auto nt : NT) {
		std::cout << nt << " 的FIRST集合：  ";
		for (auto hh : FIRST[nt]) {
			std::cout << hh << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	for (auto nt : NT) {
		std::cout << nt << " 的FOLLOW集合：  ";
		for (auto hh : FOLLOW[nt]) {
			std::cout << hh << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	
	T.push_back("@");
	for (auto t : T) std::cout << "\t" << t;
	
	std::cout << "\n";
	for (auto nt : NT) {
		std::cout << nt << "\t";
		for (auto t : T) {
			if (TABLE.count({ nt, t })) {
				std::cout << TABLE[{nt, t}] << "\t";
			}
			else {
				std::cout << "\t";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	for (int i = 0; i < lexical.tokenCnt; i++) {
		Token tokenTemp = lexical.tokenStream[i];
		std::string tokenTypeString = lexical.tokenTypeStr(tokenTemp.type);
		std::cout << tokenTemp.str << " 类型：  " << tokenTypeString << "\n";
		
	}
	
	preorder(tree.root, 0);
	std::cout << "\n";
	drawPictures();
}


void Grammer::run()
{
	readGrammer(filename);
	eliminateLeftRecursion();
	leftFactoring();
	getFirst();
	getFollow();
	getTable();
	procedure();
	outputTest();
}
int main()
{
	Grammer grammer("grammer.txt", "tinytest.txt");
	grammer.run();
	return 0;
}