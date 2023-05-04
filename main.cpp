#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <vector>
#include <stack>
#include <algorithm>
#include <exception>

using namespace std;

int str_count = 1; //для отслеживания нумерации строк в программе

// для обработки ошибок
class MyException : public std::exception {
private:
    std::string message_;
public:
    explicit MyException(const string &message) : message_(message) {}

    const char *what() const noexcept override {
        cout << "Error at line: " << str_count << endl;
        return message_.c_str();
    }
};

// для обработки ошибок
class MyCharException : public std::exception {
private:
    char c_;
public:
    explicit MyCharException(char c) : c_(c) {}

    const char *what() const noexcept override {
        cout << "Error at line: " << str_count << endl;
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
    bool str = false;
public:
    explicit Lex(type_of_lex t = LEX_NULL, int v = 0, string n = "") : t_lex(t), v_lex(v), v_lex_str(n) {}

    bool is_str() const {
        return str;
    }

    void make_str() {
        str = true;
    }

    type_of_lex get_type() const {
        return t_lex;
    }

    int get_value() const {
        return v_lex;
    }

    string get_value_str() const {
        return v_lex_str;
    }

    void put_value(int n) {
        v_lex = n;
    }

    friend ostream &operator<<(ostream &s, Lex l);
};

class LexException : public std::exception {
private:
    const Lex &lex_;
public:

    explicit LexException(const Lex &lex) : lex_(lex) {}

    const char *what() const noexcept override {
        cout << "Error at line: " << str_count << endl;
        return "Unexpected lexeme";
    }

    const Lex &GetLex() const noexcept {
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
    string value_str;
    bool str_check = false;
public:
    Ident() {
        declare = false;
        assign = false;
        type = LEX_NULL;
        value = 0;
        value_str = "";
    }

    bool operator==(const string &s) const {
        return name == s;
    }

    explicit Ident(const string &n) {
        name = n;
        declare = false;
        assign = false;
        type = LEX_NULL;
        value = 0;
        str_check = false;
    }

    string get_name() const {
        return name;
    }

    bool is_str() const {
        return str_check;
    }

    void make_string() {
        str_check = true;
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

    string get_value_str() {
        return value_str;
    }

    void put_value_str(string v) {
        value_str = v;
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
        if (!(fp = fopen(program, "r"))) {
            throw MyException("Can't open file");
        }
    }

    ~Scanner(){fclose(fp);}

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
                    if (c == '\n'){
                        str_count++;
                    }
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
                    return Lex(LEX_FIN);
                } else if (c == '"') {
                    CS = STR;
                } else if (c == '/') {
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
                if (c == EOF) {
                    throw MyException("Error brackets");
                }
                if (c == '"') {
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
                        throw MyCharException(c);
                } else if (int(c) == EOF)
                {
                    throw MyCharException(c);
                }
                break;
        }
    } while (true);
}

ostream &operator<<(ostream &s, Lex l) { //вывод лексем на экран
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
    else {
        throw LexException(l);
    }
    s << '(' << t << ',' << l.v_lex << ");" << endl;
    return s;
}

template<class T, class T_EL>
void from_st(T &st, T_EL &i) {
    i = st.top();
    st.pop();
}

class Parser { //семантический анализатор
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    string c_val_str;
    bool flag_block = true, flag_while = false, flag_no_eq = false;
    Scanner scan;
    stack<int> st_int;
    stack<type_of_lex> st_lex;
    stack<int> st_break;
    stack<int> break_flag;

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

    void V(int &flag_dec);

    void C();

    void Ocompr();

    void dec(type_of_lex type);

    void check_operands();

    void check_id();

    void check_not();

    void eq_bool();

    void eq_type();

    void check_id_in_read() const;

    void gl() {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
        c_val = curr_lex.get_value();
        c_val_str = curr_lex.get_value_str();
    }

public:
    vector<Lex> poliz;

    ~Parser(){
        while (!st_int.empty()) st_int.pop();
        while (!st_lex.empty()) st_lex.pop();
        while (!st_break.empty()) st_break.pop();
        while (!break_flag.empty()) break_flag.pop();
    }

    explicit Parser(const char *program) : scan(program) {
        c_type = LEX_NULL;
        c_val = 0;
    }

    void analyze();
};

void Parser::analyze() { //начало анализа
    gl();
    P();
    if (c_type != LEX_FIN)
    {

        throw LexException(curr_lex);
    }
//    for ( Lex l : poliz ) //вывод полиза
//        cout << l;
//    cout << endl << "YES!" << endl;
}

void Parser::P() { //первая лексема
    if (c_type == LEX_PROGRAM) {
        gl();
    } else {

        throw LexException(curr_lex);
    }
    if (c_type == LEX_LFIG)
        gl();
    else {

        throw LexException(curr_lex);
    }
    D();
    O();
    if (c_type == LEX_RFIG) {
        gl();
    } else {
        throw LexException(curr_lex);
    }
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
    int flag = 0;
    if (c_type == LEX_INT || c_type == LEX_STRING || c_type == LEX_BOOLEAN) {
        if (c_type == LEX_INT) {
            flag = 1;
            gl();
        } else if (c_type == LEX_BOOLEAN) {
            flag = 2;
            gl();
        } else {
            flag = 3;
            gl();
        }
        V(flag);
        while (c_type == LEX_COMMA) {
            gl();
            V(flag);
        }
    }
}

void Parser::V(int &flag_dec) { //переменная
    int tmp_value;
    bool flag_str = false;
    if (c_type == LEX_ID) {
        tmp_value = c_val;
        st_int.push(c_val);
        switch (flag_dec) { //объявление переменных
            case 1:
                dec(LEX_INT);
                break;
            case 2:
                dec(LEX_BOOLEAN);
                break;
            case 3:
                flag_str = true;
                curr_lex.make_str();
                dec(LEX_STRING);
                break;
            default:
                break;
        }
        gl();
        if (c_type != LEX_ASSIGN && c_type != LEX_SEMICOLON && c_type != LEX_COMMA) {
            {
                throw LexException(curr_lex);
            }
        }
        if (c_type == LEX_ASSIGN) { //объявление переменной сразу после ее описания
            poliz.push_back(Lex(POLIZ_ADDRESS, tmp_value));
            if (flag_str) {
                poliz[poliz.size() - 1].make_str();
            }
            if (TID[tmp_value].get_declare()) {
                st_lex.push(TID[tmp_value].get_type());
            }
            gl();
            C(); //константа!
            eq_type();
            poliz.push_back(Lex(LEX_ASSIGN));
        }
    } else {

        throw LexException(curr_lex);
    }
}

void Parser::C() { //константы
    int flag = 1;
    if (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        if (c_type == LEX_MINUS) flag = -1; //чтобы сохранить отрицательные числа
        gl();
        if (c_type == LEX_NUM) {
            curr_lex.put_value(flag * curr_lex.get_value());
            st_lex.push(LEX_INT);
            poliz.push_back(curr_lex);
            gl();
        } else {
            throw LexException(curr_lex);
        }
    } else if (c_type == LEX_NUM) {
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        gl();
    } else if (c_type == LEX_TRUE || c_type == LEX_FALSE) {
        if (c_type == LEX_TRUE) {
            st_lex.push(LEX_BOOLEAN);
            poliz.push_back(Lex(LEX_TRUE, 1));
        } else {
            st_lex.push(LEX_BOOLEAN);
            poliz.push_back(Lex(LEX_FALSE, 0));
        }
        gl();
    } else if (c_type == LEX_STR) {
        st_lex.push(LEX_STRING);
        poliz.push_back(curr_lex);
        gl();
    } else {

        throw LexException(curr_lex);
    }
}

void Parser::O() { //операторы
    O1();
    while (flag_block) {
        O1();
    }
}

void Parser::O1() { //один оператор
    int pl0, pl1, pl2, pl3, tmp_br;

    if (c_type == LEX_ID) { // идентификатор
        check_id();
        poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
        gl();
        if (c_type == LEX_ASSIGN) {
            gl();
            Oequasion();
            eq_type();
            poliz.push_back(Lex(LEX_ASSIGN));
            while (c_type == LEX_ASSIGN) {
                gl();
                Oequasion();
                eq_type();
                poliz.push_back(Lex(LEX_ASSIGN));
            }
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                throw LexException(curr_lex);
            }
        } else {

            throw LexException(curr_lex);
        }
    } else if (c_type == LEX_IF) { // лексема if
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            Oequasion();
            eq_bool();
            pl2 = int(poliz.size());
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_FGO));
        } else {
            throw LexException(curr_lex);
        }
        if (c_type == LEX_RPAREN) {
            flag_no_eq = false;
            gl();
            O1();
            pl3 = int(poliz.size());
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl2] = Lex(POLIZ_LABEL, int(poliz.size()));
            if (!flag_block) throw LexException(curr_lex);
            if (c_type != LEX_ELSE) {
                poliz[pl3] = Lex(POLIZ_LABEL, int(poliz.size()));
            }
        } else {
            throw LexException(curr_lex);
        }
        if (c_type == LEX_ELSE) {
            gl();
            O1();
            poliz[pl3] = Lex(POLIZ_LABEL, int(poliz.size()));
            if (!flag_block) {

                throw LexException(curr_lex);
            }
        }

    } else if (c_type == LEX_WHILE) { // лексема while
        pl0 = int(poliz.size());
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            Oequasion();
            eq_bool();
            pl1 = int(poliz.size());
            poliz.push_back(Lex());
            poliz.push_back(Lex(POLIZ_FGO));
        } else {
            throw LexException(curr_lex);
        }
        if (c_type == LEX_RPAREN) {
            flag_no_eq = false;
            flag_while = true;
            gl();
            O1();
            poliz.push_back(Lex(POLIZ_LABEL, pl0));
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl1] = Lex(POLIZ_LABEL, int(poliz.size()));
            if (!break_flag.empty()) {
                tmp_br = break_flag.top();
                break_flag.pop();
                poliz[tmp_br] = Lex(POLIZ_LABEL, int(poliz.size()));
            }
            flag_while = false;
            if (flag_block == 0) {
                throw LexException(curr_lex);
            }
        }

    } else if (c_type == LEX_READ) { // лексема read
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            if (c_type == LEX_ID) { // идентификатор
                check_id_in_read();
                poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
                if (TID[c_val].is_str()) {
                    poliz[poliz.size() - 1].make_str();
                }
                gl();
            } else {
                throw LexException(curr_lex);
            }
            if (c_type == LEX_RPAREN) {
                poliz.push_back(Lex(LEX_READ));
                flag_no_eq = false;
                gl();
            } else {
                throw LexException(curr_lex);
            }
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                throw LexException(curr_lex);
            }
        } else {
            throw LexException(curr_lex);
        }

    } else if (c_type == LEX_WRITE) { //лексема while
        flag_no_eq = true;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            Oequasion();
            while (c_type == LEX_COMMA) {
                gl();
                poliz.push_back(Lex(LEX_WRITE));
                Oequasion();
            }
            if (c_type == LEX_RPAREN) {
                poliz.push_back(Lex(LEX_WRITE));
                flag_no_eq = false;
                gl();
            } else {

                throw LexException(curr_lex);
            }
            if (c_type == LEX_SEMICOLON) {
                gl();
            } else {
                throw LexException(curr_lex);
            }
        } else {
            throw LexException(curr_lex);
        }

    } else if (c_type == LEX_BREAK) { //лексема break
        if (!flag_while) {
            throw LexException(curr_lex);
        }
        break_flag.push(int(poliz.size()));
        poliz.push_back(Lex()); //пустота для будущей метки
        poliz.push_back(Lex(POLIZ_GO));
        gl();
        if (c_type != LEX_SEMICOLON) {
            throw LexException(curr_lex);
        } else {
            gl();
        }

    } else if (c_type == LEX_LFIG) {
        gl();
        O1();
        flag_block = true;
        while (c_type != LEX_RFIG) {
            O1();
            if (!flag_block) {
                break;
            }
        }
        if (c_type == LEX_RFIG) {
            gl();
        } else throw LexException(curr_lex);

    } else {
        flag_block = false;
    }
}

void Parser::Oequasion() { // выражение
    Oor();
    while (c_type == LEX_ASSIGN) {
        if (flag_no_eq) {
            throw LexException(curr_lex);
        }
        gl();
        Oor();
    }
}

void Parser::Oor() { // или
    Oand();
    while (c_type == LEX_OR) {
        st_lex.push(c_type);
        gl();
        Oand();
        check_operands();
    }
}

void Parser::Oand() { // и
    Ocompr();
    while (c_type == LEX_AND) {
        st_lex.push(c_type);
        gl();
        Ocompr();
        check_operands();
    }
}

void Parser::Ocompr() { //операции
    Oplus_minus();
    while (c_type == LEX_NEQ || c_type == LEX_LSS || c_type == LEX_GTR
           || c_type == LEX_GEQ || c_type == LEX_LEQ || c_type == LEX_EQ) {
        st_lex.push(c_type);
        gl();
        Oplus_minus();
        check_operands();
    }
}

void Parser::Oplus_minus() { //плюс и минус
    Oper();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        st_lex.push(c_type);
        gl();
        Oper();
        check_operands();
    }
}

void Parser::Oper() { //операции умножения и деления
    OperMod();
    while (c_type == LEX_SLASH || c_type == LEX_TIMES) {
        st_lex.push(c_type);
        gl();
        OperMod();
        check_operands();
    }
}

void Parser::OperMod() { //остаток
    Operand();
    while (c_type == LEX_MOD) {
        st_lex.push(c_type);
        gl();
        Operand();
        check_operands();
    }
}

void Parser::Operand() { //операнд
    int flag_minus = 1;
    if (c_type == LEX_PLUS || c_type == LEX_MINUS) {
        if (c_type == LEX_MINUS) {
            flag_minus = -1;
        }
        gl();
        if (c_type == LEX_NUM) {
            st_lex.push(LEX_INT);
            poliz.push_back(Lex(LEX_NUM, flag_minus * c_val));
            gl();
        } else if (c_type == LEX_TRUE || c_type == LEX_FALSE) {
            if (c_type == LEX_TRUE) {
                st_lex.push(LEX_BOOLEAN);
                poliz.push_back(Lex(LEX_TRUE, 1));
            } else {
                st_lex.push(LEX_BOOLEAN);
                poliz.push_back(Lex(LEX_FALSE, 0));
            }
            gl();
        } else {
            throw LexException(curr_lex);
        }
    } else if (c_type == LEX_ID) {
        check_id();
        poliz.push_back(Lex(LEX_ID, c_val, c_val_str));
        gl();
    } else if (c_type == LEX_STR) {
        st_lex.push(LEX_STRING);
        poliz.push_back(curr_lex);
        gl();
    } else if (c_type == LEX_NUM) {
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        gl();
    } else if (c_type == LEX_NOT) {
        gl();
        Operand();
        check_not();
    } else if (c_type == LEX_TRUE || c_type == LEX_FALSE) {
        if (c_type == LEX_TRUE) {
            st_lex.push(LEX_BOOLEAN);
            poliz.push_back(Lex(LEX_TRUE, 1));
        } else {
            st_lex.push(LEX_BOOLEAN);
            poliz.push_back(Lex(LEX_FALSE, 0));
        }
        gl();
    } else if (c_type == LEX_LPAREN) {
        gl();
        Oor();
        if (c_type == LEX_RPAREN) {
            gl();
        } else throw LexException(curr_lex);
    } else throw LexException(curr_lex);
}

void Parser::check_operands() { //проверка типов операндов
    type_of_lex t1, t2, op, t = LEX_INT, r = LEX_BOOLEAN, s = LEX_STRING;

    from_st(st_lex, t2);
    from_st(st_lex, op);
    from_st(st_lex, t1);
    if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH || op == LEX_MOD) {
        r = LEX_INT;
        if ((t1 == s || t2 == s) && (op == LEX_PLUS)) {
            r = LEX_STRING;
        } else {
            if (t1 == s || t2 == s) {
                throw MyException("wrong operation");
            }
        }
    }
    if (op == LEX_OR || op == LEX_AND) {
        t = LEX_BOOLEAN;
    }
    if (t1 == t2 && (t1 == t || t1 == s || op == LEX_EQ || op == LEX_NEQ || op == LEX_GEQ || op == LEX_LEQ)) {
        st_lex.push(r);
    } else {
        throw MyException("wrong types are in operation");
    }
    poliz.push_back(Lex(op));
}

void Parser::dec(type_of_lex type) { //объявление переменной
    int i = 0;
    string s;
    while (!st_int.empty()) {
        from_st(st_int, i);
        if (TID[i].get_declare()) {
            s = "error: redeclaration of variable ";
            s += TID[i].get_name();
            throw MyException(s);
        } else {
            if (type == LEX_STRING) {
                TID[i].make_string();
            }
            TID[i].put_declare();
            TID[i].put_type(type);
        }
    }
}

void Parser::eq_type() { // проверка типов в присваивании
    type_of_lex t;
    from_st(st_lex, t);
    if (t != st_lex.top()) {
        throw MyException("error: wrong types are in =");
    }
    st_lex.pop();
}


void Parser::check_not() { //проверка на bool
    if (st_lex.top() != LEX_BOOLEAN) {
        throw MyException("error: wrong type is in not");
    } else {
        poliz.push_back(Lex(LEX_NOT));
    }
}

void Parser::check_id_in_read() const { //проверка на объявление внутри read
    string s;
    if (!TID[c_val].get_declare()) {
        s = "variable not declared: ";
        s += TID[c_val].get_name();
        throw MyException(s);
    }
}

void Parser::eq_bool() { //проверка, является ли выражение типа bool
    if (st_lex.top() != LEX_BOOLEAN) {
        throw MyException("expression is not boolean");
    }
    st_lex.pop();
}

void Parser::check_id() { //проверка имен
    string s;
    if (TID[c_val].get_declare()) {
        st_lex.push(TID[c_val].get_type());
    } else {
        s = "variable not declared: ";
        s += TID[c_val].get_name();
        throw MyException(s);
    }
}

class Executer {
public:
   static void execute(vector<Lex> &poliz);
};

void Executer::execute(vector<Lex> &poliz) { //выполнение!
    Lex pol_elem;
    stack<int> int_args;
    stack<string> str_args;
    string tmp, tmp_twin; //для работы со строками
    int k = 0;
    string tmp_read; //для считывания строки в write
    int i, j, index = 0, size = int(poliz.size());
    bool flag_str = false;
    while (index < size) {
        pol_elem = poliz[index];
        switch (pol_elem.get_type()) {
            case LEX_TRUE: //для переменных, констант и адресов
            case LEX_FALSE:
            case LEX_NUM:
            case POLIZ_ADDRESS:
            case POLIZ_LABEL:
            case LEX_STR:
                if (pol_elem.is_str() || pol_elem.get_type() == LEX_STR) { //для строк
                    flag_str = true;
                    if (pol_elem.get_type() == POLIZ_ADDRESS) {
                        int_args.push(pol_elem.get_value());
                    }
                    str_args.push(pol_elem.get_value_str());
                    break;
                }
                int_args.push(pol_elem.get_value());
                break;

            case LEX_ID: //идентификаторы
                i = pol_elem.get_value();
                if (TID[i].get_type() == LEX_STRING) { //для строк
                    if (TID[i].get_assign()) {
                        flag_str = true;
                        str_args.push(TID[i].get_value_str());
                        break;
                    }
                }
                if (TID[i].get_assign()) {
                    int_args.push(TID[i].get_value());
                    break;
                } else {
                    cout << TID[i].get_name() << endl;
                    throw MyException("POLIZ: indefinite identifier");
                }

            case LEX_NOT: //отрицание
                from_st(int_args, i);
                int_args.push(!i);
                break;

            case LEX_OR: //или
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j || i);
                break;

            case LEX_AND: //и
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j && i);
                break;

            case POLIZ_GO: //безусловный переход
                from_st(int_args, i);
                index = i - 1;
                break;

            case POLIZ_FGO: //переход по лжи
                from_st(int_args, i);
                from_st(int_args, j);
                if (!j) index = i - 1;
                break;

            case LEX_WRITE: //для вывода на экран
                if (flag_str) { //для строк
                    tmp = str_args.top();
                    str_args.pop();
                    cout << tmp << endl;
                    flag_str = false;
                    break;
                }
                from_st(int_args, j);
                cout << j << endl;
                break;

            case LEX_READ: //для считывания переменных
                from_st(int_args, i);
                if (TID[i].get_type() == LEX_INT) {
                    cin >> k;
                } else if (flag_str) { //для строк
                    cin >> tmp_read;
                    TID[i].put_value_str(tmp_read);
                    TID[i].put_assign();
                    flag_str = false;
                    break;
                } else { //true или false
                    string s;
                    cin >> s;
                    if (s != "true" && s != "false") {
                        throw MyException("Error in input for true or false");
                    }
                    k = (s == "true") ? 1 : 0;
                    break;
                }
                TID[i].put_value(k);
                TID[i].put_assign();
                break;

            case LEX_PLUS: //сложение
                if (flag_str) { //для строк
                    tmp = str_args.top();
                    str_args.pop();
                    tmp_twin = str_args.top();
                    str_args.pop();
                    str_args.push(tmp_twin + tmp);
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(i + j);
                break;

            case LEX_MOD: //остаток от деления
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j % i);
                break;

            case LEX_TIMES: //умножение
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(i * j);
                break;

            case LEX_MINUS: //вычитание
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j - i);
                break;

            case LEX_SLASH: //деление
                from_st(int_args, i);
                from_st(int_args, j);
                if (i) {
                    int_args.push(j / i);
                    break;
                } else
                    throw MyException("POLIZ: you can not divide by zero!");

            case LEX_EQ: //сравнения
                if (flag_str) { //для строк
                    tmp = str_args.top();
                    str_args.pop();
                    tmp_twin = str_args.top();
                    str_args.pop();
                    int_args.push(tmp_twin == tmp);
                    flag_str = false;
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(i == j);
                break;

            case LEX_LSS: //меньше
                if (flag_str) {
                    tmp = str_args.top();
                    str_args.pop();
                    tmp_twin = str_args.top();
                    str_args.pop();
                    int_args.push(tmp_twin < tmp);
                    flag_str = false;
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j < i);
                break;

            case LEX_GTR: //больше
                if (flag_str) { //для строк
                    tmp = str_args.top();
                    str_args.pop();
                    tmp_twin = str_args.top();
                    str_args.pop();
                    int_args.push(tmp_twin > tmp);
                    flag_str = false;
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j > i);
                break;

            case LEX_LEQ: // <=
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j <= i);
                break;

            case LEX_GEQ: // >=
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j >= i);
                break;

            case LEX_NEQ: //не равно
                if (flag_str) { //для строк
                    tmp = str_args.top();
                    str_args.pop();
                    tmp_twin = str_args.top();
                    str_args.pop();
                    int_args.push(tmp_twin != tmp);
                    flag_str = false;
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                int_args.push(j != i);
                break;

            case LEX_ASSIGN: //присваивание
                if (flag_str) { //для строк
                    from_st(int_args, j);
                    tmp = str_args.top();
                    str_args.pop();
                    TID[j].put_value_str(tmp);
                    TID[j].put_assign();
                    flag_str = false;
                    break;
                }
                from_st(int_args, i);
                from_st(int_args, j);
                TID[j].put_value(i);
                TID[j].put_assign();
                break;

            default: //ошибка
                cout << pol_elem << endl;
                throw MyException("POLIZ: unexpected element");
        }
        ++index;
    }
    cout << endl;
    cout << "Finished executing!!!" << endl;
    while (!int_args.empty()) int_args.pop();
    while (!str_args.empty()) str_args.pop();
    TID.clear();
    poliz.clear();
}

class Interpretator {
    Parser parsed;
    Executer Ex;
public:
    explicit Interpretator(const char *program) : parsed(program) {}

    void interpretation();
};

void Interpretator::interpretation() {
    parsed.analyze();
    Ex.execute(parsed.poliz);
}

int main() {
    try {
        Interpretator I("prog.txt");
        I.interpretation();
        return 0;
    }
    catch (const MyCharException &e) {
        cout << e.what() << " symbol: " << e.GetChar() << endl;
        return 1;
    }
    catch (const LexException &e) {
        cout << e.what() << e.GetLex() << endl;
        return 1;
    }
    catch (const MyException &e) {
        cout << e.what() << endl;
        return 1;
    }
}