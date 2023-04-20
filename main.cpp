#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <vector>
#include <stack>
#include <algorithm>
#include <exception>

using namespace std;

// для обработки ошибок
class MyException : public std::exception {
private:
    std::string message_;
public:
    explicit MyException(const string& message) : message_(message) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }
};

// для обработки ошибок
class MyCharException : public std::exception {
private:
    char c_;
public:
    explicit MyCharException(char c) : c_(c) {}

     const char* what() const noexcept override{
        return "Char exception occurred";
    }

    char GetChar() const noexcept {
        return c_;
    }
};


enum type_of_lex { //все лексемы
    LEX_NULL,                                                                                            /*0*/
    LEX_AND, LEX_BOOLEAN, LEX_WHILE, LEX_IF, LEX_ELSE, LEX_FALSE, LEX_INT, LEX_STRING,                   /*8*/
    LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_TRUE, LEX_WRITE, LEX_BREAK,                              /*15*/
    LEX_FIN,                                                                                             /*16*/
    LEX_SEMICOLON, LEX_COMMA, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,                       /*23*/
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_MOD, LEX_LEQ, LEX_NEQ, LEX_GEQ, LEX_LFIG,    /*33*/
    LEX_RFIG,                                                                                            /*34*/
    LEX_NUM,                                                                                             /*35*/
    LEX_STR,                                                                                             /*36*/
    LEX_ID,                                                                                              /*37*/
    POLIZ_LABEL,                                                                                         /*38*/
    POLIZ_ADDRESS,                                                                                       /*39*/
    POLIZ_GO,                                                                                            /*40*/
    POLIZ_FGO                                                                                            /*41*/
};

/*------------------------------------LEX---------------------------------------------*/

class Lex {
    type_of_lex t_lex;
    int v_lex;
    string v_lex_str;
public:
    Lex(type_of_lex t = LEX_NULL, int v = 0, string n = "") : t_lex(t), v_lex(v), v_lex_str(n) {}

    type_of_lex get_type() const {
        return t_lex;
    }

    int get_value() const {
        return v_lex;
    }

    friend ostream &operator<<(ostream &s, Lex l);
};

class LexException : public std::exception {
private:
    const Lex& lex_;
public:

    explicit LexException(const Lex& lex) : lex_(lex) {}

    const char* what() const noexcept override {
        return "Unexpected lexeme";
    }

    const Lex& GetLex() const noexcept {
        return lex_;
    }
};

/*------------------------------------Ident-------------------------------------------*/

class Ident {
    string name;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
public:
    Ident() {
        declare = false;
        assign = false;
        type = LEX_NULL;
        value = 0;
    }

    bool operator==(const string &s) const {
        return name == s;
    }

    explicit Ident(const string& n) {
        name = n;
        declare = false;
        assign = false;
        type = LEX_NULL;
        value = 0;
    }

    string get_name() const {
        return name;
    }

    bool get_declare() const {
        return declare;
    }

    void put_declare() {
        declare = true;
    }

    type_of_lex get_type() const {
        return type;
    }

    void put_type(type_of_lex t) {
        type = t;
    }

    bool get_assign() const {
        return assign;
    }

    void put_assign() {
        assign = true;
    }

    int get_value() const {
        return value;
    }

    void put_value(int v) {
        value = v;
    }
};

/*------------------------------------TID----------------------------------------*/

vector<Ident> TID; //таблица имен

int put(const string &buf) {
    vector<Ident>::iterator k;

    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end())
        return int(k - TID.begin());
    TID.push_back(Ident(buf));
    return int(TID.size() - 1);
}

/*--------------------------------------------------------------------------------------*/

class Scanner {
    FILE *fp;
    char c = ' ';

    static int look(const string &buf, const char **list) {
        int i = 0;
        while (list[i]) {
            if (buf == list[i])
                return i;
            ++i;
        }
        return 0;
    }

    void gc() {
        if (c != EOF) {
            c = char(fgetc(fp));
        }
    }

public:
    static const char *TW[], *TD[];

    explicit Scanner(const char *program) {
        if (!(fp = fopen(program, "r")))
            throw MyException("Can't open file");;
    }

    Lex get_lex();
};

const char *
        Scanner::TW[] = {"", "and", "boolean", "while", "if", "else", "false", "int", "string",
                         "not", "or", "program", "read", "true", "write",
                         "break", nullptr};

const char *
        Scanner::TD[] = {"@", ";", ",", "=", "(", ")", "==", "<", ">",
                         "+", "-", "*", "/", "%", "<=", "!=", ">=", "{", "}",
                         nullptr};

Lex Scanner::get_lex() { //получение лексемы
    enum state {
        H, IDENT, NUMB, ALE, STR, COM
    };
    int d, j;
    string buf;
    string str;
    state CS = H;
    do {
        gc();
        switch (CS) {
            case H:
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                } else if (isalpha(c)) {
                    buf.push_back(c);
                    CS = IDENT;
                } else if (isdigit(c)) {
                    d = c - '0';
                    CS = NUMB;
                } else if (c == '!' || c == '<' || c == '>' || c == '=') {
                    buf.push_back(c);
                    CS = ALE;
                } else if (int(c) == EOF) {
                    return LEX_FIN;
                } else if (c == '"'){
                    CS = STR;
                } else if( c == '/'){
                    gc();
                    if (c == '*')
                        CS = COM;
                    else {
                        ungetc(c, fp);
                        return Lex(LEX_SLASH, 13);
                    }
                } else {
                    buf.push_back(c);
                    if ((j = look(buf, TD))) {
                        return Lex((type_of_lex) (j + (int) LEX_FIN), j);
                    } else {
                        throw MyCharException(c);
                    }
                }
                break;
            case IDENT:
                if (isalpha(c) || isdigit(c)) {
                    buf.push_back(c);
                } else {
                    ungetc(c, fp);
                    if ((j = look(buf, TW))) {
                        return Lex((type_of_lex) j, j);
                    } else {
                        j = put(buf);
                        return Lex(LEX_ID, j);
                    }
                }
                break;
            case NUMB:
                if (isdigit(c)) {
                    d = d * 10 + (c - '0');
                } else {
                    ungetc(c, fp);
                    return Lex(LEX_NUM, d);
                }
                break;
            case ALE:
                if (c == '=') {
                    buf.push_back(c);
                    j = look(buf, TD);
                    return Lex((type_of_lex) (j + (int) LEX_FIN), j);
                } else {
                    ungetc(c, fp);
                    j = look(buf, TD);
                    return Lex((type_of_lex) (j + (int) LEX_FIN), j);
                }
            case STR:
                if (c == EOF){
                    throw MyException("Error brackets");
                }
                if (c == '"'){
                    return Lex(LEX_STR, 0, str);
                } else {
                    str += c;
                }
                break;
            case COM:
                if (c == '*') {
                    gc();
                    if (c == '/')
                        CS = H;
                    else if (int(c) == EOF)
                        throw MyCharException(c);;
                } else if (int(c) == EOF)
                    throw MyCharException(c);
                break;
        }
    } while (true);
}

ostream &operator<<(ostream &s, Lex l) { //вывод на экран
    string t;
    if (l.t_lex <= LEX_BREAK)
        t = Scanner::TW[l.t_lex];
    else if (l.t_lex >= LEX_FIN && l.t_lex <= LEX_GEQ)
        t = Scanner::TD[l.t_lex - LEX_FIN];
    else if (l.t_lex == LEX_NUM)
        t = "NUMB";
    else if (l.t_lex == LEX_STR) {
        t = "STR";
        s << '(' << t << ',' << l.v_lex_str << ");" << endl;
        return s;
    } else if (l.t_lex == LEX_ID)
        t = TID[l.v_lex].get_name();
    else if (l.t_lex == LEX_LFIG)
        t = "{";
    else if (l.t_lex == LEX_RFIG)
        t = "}";
    else if (l.t_lex == POLIZ_LABEL)
        t = "Label";
    else if (l.t_lex == POLIZ_ADDRESS)
        t = "Addr";
    else if (l.t_lex == POLIZ_GO)
        t = "!";
    else if (l.t_lex == POLIZ_FGO)
        t = "!F";
    else
        throw LexException(l);;
    s << '(' << t << ',' << l.v_lex << ");" << endl;
    return s;
}

class Parser { //семантический анализатор
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    bool flag_block = true, flag_while = false, flag_no_eq = false;
    Scanner scan;
    stack<int> st_int;
    stack<type_of_lex> st_lex;
    void P();
    void D1();
    void D();
    void O();
    void O1();
    void Oor();
    void Oequasion();
    void Oand();
    void Oplus_minus();
    void Oper();
    void Operand();
    void OperMod();
    void V();
    void C();
    void E();
    void check_id();
    void eq_bool();
    void check_id_in_read();
    void gl() {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
        c_val = curr_lex.get_value();
    }
public:
    vector<Lex> poliz;
    explicit Parser(const char* program) : scan(program) {
        c_type = LEX_NULL;
        c_val = 0;
    }
    void analyze();
};

void Parser::analyze() { //начало анализа
    gl();
    P();
    if (c_type != LEX_FIN)
        throw LexException(curr_lex);
    cout << endl << "YEEEES!!!!!!!!!!!!" << endl;
}

void Parser::P() { //первая лексема
    if (c_type == LEX_PROGRAM) {
        gl();
    } else throw LexException(curr_lex);
    if (c_type == LEX_LFIG)
        gl();
    else throw LexException(curr_lex);
    D();
    O();
    if (c_type == LEX_RFIG) {
        gl();
    } else throw LexException(curr_lex);
}

void Parser::D() //описания
{
    D1();
    while (c_type == LEX_SEMICOLON) {
        gl();
        D1();
    }
}

void Parser::D1() //описание
{
    if (c_type == LEX_INT || c_type == LEX_STRING || c_type == LEX_BOOLEAN) {
        gl();
        V();
        while (c_type == LEX_COMMA) {
            gl();
            V();
        }
    }
}

void Parser::V() { //переменная
    if (c_type == LEX_ID) {
        gl();
        if (c_type != LEX_ASSIGN && c_type != LEX_SEMICOLON && c_type != LEX_COMMA){
            throw LexException(curr_lex);
        }
        if (c_type == LEX_ASSIGN) {
            gl();
            C();
        }
    } else throw LexException(curr_lex);
}

void Parser::C() { //константы
    if (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        gl();
        if (c_type == LEX_NUM) {
            gl();
        } else throw LexException(curr_lex);
    } else if (c_type == LEX_NUM) {
        gl();
    } else if (c_type == LEX_TRUE || c_type == LEX_FALSE) {
        gl();
    } else if (c_type == LEX_STR) {
        gl();
    } else throw LexException(curr_lex);
}

void Parser::O() { //операторы
    O1();
    while (flag_block) {
        O1();
    }
}

void Parser::O1() { //один оператор
    if (c_type == LEX_ID) {
        gl();
        if (c_type == LEX_ASSIGN) {
            gl();
            check_id();
            Oor();
            while (c_type == LEX_ASSIGN) {
                gl();
                Oequasion();
            }
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else throw LexException(curr_lex);
        } else throw LexException(curr_lex);
    } else if (c_type == LEX_IF) {
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            Oequasion();
            eq_bool();
        } else throw LexException(curr_lex);
        if (c_type == LEX_RPAREN) {
            flag_no_eq = false;
            gl();
            O1();
            if (!flag_block) throw LexException(curr_lex);
            if (c_type != LEX_ELSE) {
                //
            }
        } else throw LexException(curr_lex);
        if (c_type == LEX_ELSE) {
            gl();
            O1();
            if (!flag_block) throw LexException(curr_lex);
        }
    } else if (c_type == LEX_WHILE) {
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            Oequasion();
            eq_bool();
        } else {
            throw LexException(curr_lex);
        }
        if (c_type == LEX_RPAREN){
            flag_while = true;
            gl();
            O1();
            flag_while = false;
            if (flag_block == 0) throw LexException(curr_lex);
        }
    } else if (c_type == LEX_READ){
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN){
            gl();
            if (c_type == LEX_ID){
                check_id_in_read();
                gl();
            } else throw LexException(curr_lex);
            if (c_type == LEX_RPAREN){
                flag_no_eq = false;
                gl();
            } else throw LexException(curr_lex);
            if (c_type == LEX_SEMICOLON){
                gl();
            } else throw LexException(curr_lex);
        } else throw LexException(curr_lex);
    } else if (c_type == LEX_WRITE){
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN){
            gl();
            Oequasion();
            while(c_type == LEX_COMMA){
                gl();
                Oequasion();
            }
            if (c_type == LEX_RPAREN){
                flag_no_eq = false;
                gl();
            } else throw LexException(curr_lex);
            if (c_type == LEX_SEMICOLON){
                gl();
            } else throw LexException(curr_lex);
        } else throw LexException(curr_lex);
    } else if (c_type == LEX_BREAK){
        if (!flag_while){
            throw LexException(curr_lex);
        }
        gl();
        if (c_type != LEX_SEMICOLON){
            throw LexException(curr_lex);
        } else {
            gl();
        }
    } else if (c_type == LEX_LFIG){
        gl();
        O1();
        flag_block = true;
        while (c_type != LEX_RFIG){
            O1();
            if (!flag_block){
                break;
            }
        }
        if (c_type == LEX_RFIG){
            gl();
        } else throw LexException(curr_lex);
    } else {
        flag_block = false;
    }
}

void Parser::Oequasion(){ // выражение
    Oor();
    while (c_type == LEX_ASSIGN){
        if (flag_no_eq){
            throw LexException(curr_lex);
        }
        gl();
        Oor();
    }
}

void Parser::Oor(){ // или
    Oand();
    while(c_type == LEX_OR){
        gl();
        Oand();
    }
}

void Parser::Oand(){ // и
    E();
    while(c_type == LEX_AND){
        gl();
        E();
    }
}

void Parser::E(){ //операции
    Oplus_minus();
    while (c_type == LEX_NEQ || c_type == LEX_LSS || c_type == LEX_GTR
           || c_type == LEX_GEQ || c_type == LEX_LEQ || c_type == LEX_EQ){
        gl();
        Oplus_minus();
    }
}

void Parser::Oplus_minus() { //плюс и минус
    Oper();
    while(c_type == LEX_PLUS || c_type == LEX_MINUS){
        gl();
        Oper();
    }
}

void Parser::Oper(){ //операции умножения и деления
    OperMod();
    while (c_type == LEX_SLASH || c_type == LEX_TIMES){
        gl();
        OperMod();
    }
}

void Parser::OperMod(){ //остаток
    Operand();
    while (c_type == LEX_MOD){
        gl();
        Operand();
    }
}

void Parser::Operand(){ //операнд
    if (c_type == LEX_PLUS || c_type == LEX_MINUS){
        gl();
        if (c_type == LEX_NUM){
            gl();
        } else if (c_type == LEX_TRUE || c_type == LEX_FALSE){
            gl();
        } else {
            throw LexException(curr_lex);
        }
    } else if (c_type == LEX_ID){
        check_id();
        gl();
    } else if(c_type == LEX_STR){
        gl();
    } else if(c_type == LEX_NUM){
        gl();
    } else if(c_type == LEX_NOT){
        gl();
        Operand();
    } else if(c_type == LEX_TRUE || c_type == LEX_FALSE){
        gl();
    }else if(c_type == LEX_LPAREN){
        gl();
        Oequasion();
        if (c_type == LEX_RPAREN){
            gl();
        } else throw LexException(curr_lex);
    } else throw LexException(curr_lex);
}


void Parser::check_id_in_read() {
}

void Parser::eq_bool() {
}

void Parser::check_id() {
    if (c_type != LEX_ID)
        return;
}

int main() {
    try {
        Parser pars("prog.txt");
        pars.analyze();
    }
    catch (const MyCharException& e) {
        cout << e.what() << " symbol: " << e.GetChar() << endl;
        return 1;
    }
    catch (const LexException& e) {
        cout << e.what() << e.GetLex() << endl;
        return 1;
    }
    catch (const MyException& e) {
        cout << e.what() << endl;
        return 1;
    }
}