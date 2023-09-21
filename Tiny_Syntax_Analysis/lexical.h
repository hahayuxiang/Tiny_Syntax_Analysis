#pragma once
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>

#define MAXRESERVED 50
#define TRUE 1
#define FALSE 0

enum TokenType
{
	ENDFILE, ERROR, COMMENT,
	// reserved words
	IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE, WHILE, ENDWHILE,
	DO, ENDDO, FOR, TO, DOWNTO,
	// multicharacter tokens
	ID, NUM,
	// special symbols
	ASSIGN, EQ, LT, LTEQ, RT, RTEQ, LTRT, PLUS, MINUS, TIMES, OVER, POWER, REM,
	LPAREN, RPAREN, SEMI
};

enum StateType { START, INASSIGN, INPLUS, INLT, INCOMMENT, INNUM, INID, DONE, INRT };
enum NumType { NUMTYPE0, NUMTYPE1, NUMTYPE2, NUMTYPE3, NUMTYPE4, NUMTYPE5, NUMTYPE6, NUMTYPEDONE };

struct Token
{
	TokenType type;
	std::string str;
	Token(TokenType t = END, std::string s = "")
	{
		type = t;
		str = s;
	}
	Token(const Token& t)
	{
		type = t.type;
		str = t.str;
	}
};

struct {
	std::string tokenString;
	TokenType tokenType;
}reservedWords[MAXRESERVED]
= { {"if",IF},{"then",THEN},{"else",ELSE},{"end",END},
	{"repeat",REPEAT},{"until",UNTIL},{"read",READ},
	{"write",WRITE},{"while", WHILE},{"endwhile", ENDWHILE},
	{"do", DO}, {"for", FOR}, {"to", TO}, {"downto", DOWNTO},
	{"enddo", ENDDO} };

class Lexical
{
public:
	std::string fileString = "";
	int strIndex = 0;
	Token token = Token();
	Token tokenStream[10000];
	int tokenCnt = 0;
public:
	void run(const std::string& fs);
public:
	Token getToken();
	Token getNum();
	char getNextToken();
	std::string tokenTypeStr(TokenType t);
	
};

char Lexical::getNextToken() {
	return fileString[strIndex++];
}

TokenType reservedLookup(const std::string& s)
{
	for (int i = 0; i < MAXRESERVED; i++)
	{
		if (s == reservedWords[i].tokenString)
			return reservedWords[i].tokenType;
	}
	return ID;
}

Token Lexical::getNum() {
	TokenType tokenType;
	std::string tokenString = "";
	NumType state = NUMTYPE0;
	while (state != NUMTYPEDONE) {
		char c = getNextToken();
		switch (state)
		{
		case NUMTYPE0:
			if (isdigit(c)) {
				state = NUMTYPE1;
				tokenString += c;
			}
			else if (c == '.') {
				state = NUMTYPE3;
				tokenString += c;
			}
			break;
		case NUMTYPE1:
			if (isdigit(c)) {
				state = NUMTYPE1;
				tokenString += c;
			}
			else if (c == '.') {
				state = NUMTYPE2;
				tokenString += c;
			}
			else if (c == 'E') {
				state = NUMTYPE4;
				tokenString += c;
			}
			else {
				state = NUMTYPEDONE;
				strIndex--;
			}
			break;
		case NUMTYPE2:
			if (isdigit(c)) {
				state = NUMTYPE2;
				tokenString += c;
			}
			else if (c == 'E') {
				state = NUMTYPE4;
				tokenString += c;
			}
			else {
				state = NUMTYPEDONE;
				strIndex--;
			}
			break;
		case NUMTYPE3:
			if (isdigit(c)) {
				state = NUMTYPE2;
				tokenString += c;
			}
			break;
		case NUMTYPE4:
			if (isdigit(c)) {
				state = NUMTYPE6;
				tokenString += c;
			}
			else if (c == '+' || c == '-') {
				state = NUMTYPE5;
				tokenString += c;
			}
			break;
		case NUMTYPE5:
			if (isdigit(c)) {
				state = NUMTYPE6;
				tokenString += c;
			}
			break;
		case NUMTYPE6:
			if (isdigit(c)) {
				state = NUMTYPE6;
				tokenString += c;
			}
			else {
				state = NUMTYPEDONE;
				strIndex--;
			}
			break;
		default:
			break;
		}
	}
	return Token(NUM, tokenString);
}

Token Lexical::getToken()
{
	TokenType tokenType;
	std::string tokenString = "";
	int save;
	StateType state = START;
	while (state != DONE) {
		char c = getNextToken();
		save = TRUE;
		switch (state) {
		case START:
			if (isdigit(c)) {
				--strIndex;
				Token numToken = getNum();
				tokenType = numToken.type;
				tokenString = numToken.str;
				save = FALSE;
				state = DONE;
			}
			else if (isalpha(c)) {
				state = INID;
			}
			else if (c == ':') {
				state = INASSIGN;
			}
			else if (c == '+') {
				state = INPLUS;
			}
			else if (c == '<') {
				state = INLT;
			}
			else if (c == '>') {
				state = INRT;
			}
			else if (c == ' ' || c == '\n' || c == '\t') {
				save = FALSE;
			}
			else if (c == '{') {
				save = FALSE;
				state = INCOMMENT;
			}
			else {
				state = DONE;
				switch (c) {
				case '\0':
					save = FALSE;
					tokenType = ENDFILE;
					break;
				case '=':
					tokenType = EQ;
					break;
				case '-':
					tokenType = MINUS;
					break;
				case '*':
					tokenType = TIMES;
					break;
				case '/':
					tokenType = OVER;
					break;
				case '^':
					tokenType = POWER;
					break;
				case '%':
					tokenType = REM;
					break;
				case '(':
					tokenType = LPAREN;
					break;
				case ')':
					tokenType = RPAREN;
					break;
				case ';':
					tokenType = SEMI;
					break;
				default:
					tokenType = ERROR;
					break;
				}
			}
			break;
		case INCOMMENT:
			if (c == '\0' || c == '}') {
				state = START;
				tokenType = COMMENT;
			}
			save = FALSE;
			break;
		case INASSIGN:
			state = DONE;
			if (c == '=') {
				tokenType = ASSIGN;
			}
			else {
				strIndex--;
				save = FALSE;
				tokenType = ERROR;
			}
			break;
		case INPLUS:
			state = DONE;
			if (c == '=') {
				tokenType = ASSIGN;
			}
			else {
				strIndex--;
				save = FALSE;
				tokenType = PLUS;
			}
			break;
		case INLT:
			state = DONE;
			if (c == '=') {
				tokenType = LTEQ;
			}
			else if (c == '>') {
				tokenType = LTRT;
			}
			else {
				strIndex--;
				save = FALSE;
				tokenType = LT;
			}
			break;
		case INRT:
			state = DONE;
			if (c == '=') {
				tokenType = RTEQ;
			}
			else {
				strIndex--;
				save = FALSE;
				tokenType = RT;
			}
			break;
		case INID:
			if (!isalpha(c)) {
				strIndex--;
				save = FALSE;
				state = DONE;
				tokenType = ID;
			}
			break;
		default: /* should never happen */
			state = DONE;
			tokenType = ERROR;
			break;
		}
		if (save) {
			tokenString += c;
		}
		if (state == DONE) {
			if (tokenType == ID) {
				tokenType = reservedLookup(tokenString);
			}
			//std::string tokenTypeString = tokenTypeStr(tokenType);
			//std::cout << tokenString << " 类型：  " << tokenTypeString << "\n";
			tokenStream[tokenCnt++] = Token(tokenType, tokenString);
		}
	}
	return Token(tokenType, tokenString);
}


void Lexical::run(const std::string& fs)
{
	strIndex = 0;
	fileString = fs;
	Token token;
	token = getToken();
	while (token.type != ENDFILE) {
		token = getToken();
	}
}
//std::string Lexical::tokenTypeStr(TokenType token) {
//	switch (token) {
//	case 0:
//		return "ENDFILE(文件结束)";
//	case 1:
//		return "ERROR(错误)";
//	case 2:
//		return "CONMMENT(注释)";
//	case 3:
//		return "IF(关键字if)";
//	case 4:
//		return "THEN(关键字then)";
//	case 5:
//		return "ELSE(关键字else)";
//	case 6:
//		return "END(关键字end)";
//	case 7:
//		return "REPEAT(关键字repeat)";
//	case 8:
//		return "UNTIL(关键字until)";
//	case 9:
//		return "READ(关键字read)";
//	case 10:
//		return "WRITE(关键字write)";
//	case 11:
//		return "WHILE(关键字while)";
//	case 12:
//		return "ENDWHILE(关键字endwhile)";
//	case 13:
//		return "DO(关键字do)";
//	case 14:
//		return "ENDDO(关键字enddo)";
//	case 15:
//		return "FOR(关键字for)";
//	case 16:
//		return "TO(关键字to)";
//	case 17:
//		return "DOWNTO(关键字downto)";
//	case 18:
//		return "ID(标识符)";
//	case 19:
//		return "NUM(数字)";
//	case 20:
//		return "ASSIGN(赋值符号)";
//	case 21:
//		return "EQ(等于符号)";
//	case 22:
//		return "LT(小于符号)";
//	case 23:
//		return "LTEQ(小于等于符号)";
//	case 24:
//		return "RT(大于符号)";
//	case 25:
//		return "RTEQ(大于等于符号)";
//	case 26:
//		return "LTRT(不等于符号)";
//	case 27:
//		return "PLUS(加号)";
//	case 28:
//		return "MINUS(减号)";
//	case 29:
//		return "TIMES(乘号)";
//	case 30:
//		return "OVER(除号)";
//	case 31:
//		return "POWER(乘方符号)";
//	case 32:
//		return "REM(取余符号)";
//	case 33:
//		return "LPAREN(左括号)";
//	case 34:
//		return "RPAREN(右括号)";
//	case 35:
//		return "SEMI(分号)";
//	}
//}
std::string Lexical::tokenTypeStr(TokenType token) {
	switch (token) {
	case 0:
		return "ENDFILE(文件结束)";
	case 1:
		return "error";
	case 2:
		return "Comment";
	case 3:
		return "if";
	case 4:
		return "then";
	case 5:
		return "else";
	case 6:
		return "end";
	case 7:
		return "repeat";
	case 8:
		return "until";
	case 9:
		return "read";
	case 10:
		return "write";
	case 11:
		return "while";
	case 12:
		return "endwhile";
	case 13:
		return "do";
	case 14:
		return "enddo";
	case 15:
		return "for";
	case 16:
		return "to";
	case 17:
		return "downto";
	case 18:
		return "id";
	case 19:
		return "digit";
	case 20:
		return "assign";
	case 21:
		return "eq";
	case 22:
		return "lt";
	case 23:
		return "lteq";
	case 24:
		return "rt";
	case 25:
		return "rteq";
	case 26:
		return "ltrt";
	case 27:
		return "+";
	case 28:
		return "-";
	case 29:
		return "*";
	case 30:
		return "/";
	case 31:
		return "^";
	case 32:
		return "%";
	case 33:
		return "(";
	case 34:
		return ")";
	case 35:
		return ";";
	}
}