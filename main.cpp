#include <iostream>
#include <string>
#include <cstdio>
#include <cctype>
#include <vector>
#include <algorithm>

using namespace std;

enum type_of_lex {
    LEX_NULL,                                                                                            /*0*/
    LEX_AND, LEX_BOOLEAN, LEX_DO, LEX_IF, LEX_FALSE, LEX_INT, LEX_STRING,                                /*7*/
    LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_TRUE, LEX_WRITE, LEX_BREAK,                              /*14*/
    LEX_FIN,                                                                                             /*15*/
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,            /*23*/
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_MOD, LEX_LEQ, LEX_NEQ, LEX_GEQ, LEX_LFIG,    /*33*/
    LEX_RFIG,                                                                                            /*34*/
    LEX_NUM,                                                                                             /*35*/
    LEX_STR,                                                                                             /*36*/
    LEX_ID,                                                                                              /*37*/
    LEX_COM,                                                                                             /*38*/
    POLIZ_LABEL,                                                                                         /*39*/
    POLIZ_ADDRESS,                                                                                       /*40*/
    POLIZ_GO,                                                                                            /*41*/
    POLIZ_FGO                                                                                            /*42*/
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
    }

    bool operator==(const string &s) const {
        return name == s;
    }

    Ident(const string& n) {
        name = n;
        declare = false;
        assign = false;
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

vector<Ident> TID;

int put(const string &buf) {
    vector<Ident>::iterator k;

    if ((k = find(TID.begin(), TID.end(), buf)) != TID.end())
        return k - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

/*--------------------------------------------------------------------------------------*/

class Scanner {
    FILE *fp;
    char c = ' ';

    int look(const string buf, const char **list) {
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
            c = fgetc(fp);
        }
    }

public:
    static const char *TW[], *TD[];

    Scanner(const char *program) {
        if (!(fp = fopen(program, "r")))
            throw "can’t open file";
    }

    Lex get_lex();
};

const char *
        Scanner::TW[] = {"", "and", "boolean", "do", "if", "false", "int", "string",
                         "not", "or", "program", "read", "true", "write",
                         "break", nullptr};

const char *
        Scanner::TD[] = {"@", ";", ",", ":", "=", "(", ")", "==", "<", ">",
                         "+", "-", "*", "/", "%", "<=", "!=", ">=", "{", "}",
                         nullptr};

Lex Scanner::get_lex() {
    enum state {
        H, IDENT, NUMB, ALE, STR, COM
    };
    int d, j;
    bool flag = false;
    bool flag1 = false;
    bool flag2 = false;
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
                    flag = true;
                    CS = COM;
                } else {
                    buf.push_back(c);
                    if ((j = look(buf, TD))) {
                        return Lex((type_of_lex) (j + (int) LEX_FIN), j);
                    } else {
                        throw c;
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
                    throw "Error brackets";
                }
                if (c == '"'){
                    return Lex(LEX_STR, 0, str);
                } else {
                    str += c;
                }
                break;
            case COM:
                if (c == '/' && flag2){
                    return Lex(LEX_COM, 0, str);
                } else {
                    if (flag2){
                        str += '*';
                        if (c != '*' && c != ' '){
                            str += c;
                        }
                    }
                    if (c == '/'){
                        str += c;
                    }
                    flag2 = false;
                }
                if (flag1 && c != '*' && c != '/'){
                    str += c;
                }
                if (c == '*' && !flag){
                    flag2 = true;
                }
                if (c == '*' && flag){
                    flag1 = true;
                    flag = false;
                } else {
                    if (flag) {
                        buf.push_back('/');
                        if ((j = look(buf, TD))) {
                            cout << j << endl;
                            return Lex((type_of_lex) (j + (int) LEX_FIN), j);
                        } else {
                            throw c;
                        }
                    }
                }
                break;
        }
    } while (true);
}

ostream &operator<<(ostream &s, Lex l) {
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
    }
    else if (l.t_lex == LEX_COM) {
        t = "CMNT";
        s << '(' << t << ',' << l.v_lex_str << ");" << endl;
        return s;
    }
    else if (l.t_lex == LEX_ID)
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
        throw l;
    s << '(' << t << ',' << l.v_lex << ");" << endl;
    return s;
}

int main() {
    try {
        Scanner scan("prog.txt");
        while (true) {
            Lex l = scan.get_lex();
            if (l.get_type() == LEX_FIN) {
                break;
            }
            cout << l;
        }
    }
    catch (char c) {
        cout << "unexpected symbol " << c << endl;
        return 1;
    }
    catch (Lex& l) {
        cout << "unexpected lexeme" << l << endl;
        return 1;
    }
    catch (const char *source) {
        cout << source << endl;
        return 1;
    }
}
