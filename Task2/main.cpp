#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

using namespace std;

ofstream fout("output.txt");

enum class Token {
    LPAREN, RPAREN, END, SEMICOLON, ASTERISK, NAME, COMMA, AMPERSAND
};

class tree {
public:
    string node;
    vector<tree> children;

public:
    tree(string node, vector<tree> children) {
        this->node = move(node);
        this->children = move(children);
    }
};

vector<Token> tokens;
int curToken = 0;

Token getToken() {
    return tokens[curToken];
}

tree M() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::NAME: {
            curToken++;
            vector<tree> empty;
            tmp.emplace_back("n", empty);
            break;
        }
        case Token::LPAREN:
        case Token::RPAREN:
        case Token::COMMA: {
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected NAME"};
        }
    }
    return tree("M", tmp);
}

tree Z() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::ASTERISK: {
            curToken++;
            tmp.emplace_back(Z());
            break;

        }
        case Token::AMPERSAND: {
            curToken++;
            tmp.emplace_back(M());
            break;
        }
        case Token::NAME: {
            curToken++;
            vector<tree> empty;
            tmp.emplace_back("n", empty);
            break;
        }
        case Token::LPAREN:
        case Token::RPAREN:
        case Token::COMMA: {
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected NAME"};
        }
    }
    return tree("Z", tmp);
}

tree N() {
    vector<tree> tmp;
    switch (getToken()) {
        /*case Token::AMPERSAND: {
            curToken++;
            tmp.push_back(M());
            break;
        }
        case Token::ASTERISK: {
            curToken++;
            tmp.push_back(Z());
            break;
        }
        case Token::NAME: {
            curToken++;
            vector<tree> empty;
            tmp.emplace_back("n", empty);
            break;
        }*/
        case Token::AMPERSAND:
        case Token::ASTERISK:
        case Token::NAME: {
            tmp.push_back(Z());
            break;
        }
        case Token::LPAREN:
        case Token::RPAREN:
        case Token::COMMA: {
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected AMPERSAND, ASTERISK or NAME"};
        }
    }
    return tree("N", tmp);
}

tree T() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::NAME: {
            curToken++;
            vector<tree> empty;
            tmp.emplace_back("n", empty);
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected TYPE"};
        }
    }
    return tree("T", tmp);
}

tree B() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::COMMA: {
            curToken++;
            tmp.push_back(T());
            tmp.push_back(N());
            tmp.push_back(B());
            break;
        }
        case Token::RPAREN: {
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected COMMA or RPAREN"};
        }
    }
    return tree("B", tmp);
}

tree A() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::NAME: {
            tmp.push_back(T());
            tmp.push_back(N());
            tmp.push_back(B());
        }
        case Token::RPAREN: {
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected TYPE or RPAREN"};
        }
    }
    return tree("A", tmp);
}

tree S() {
    vector<tree> tmp;
    switch (getToken()) {
        case Token::NAME: {
            tmp.push_back(T());
            tmp.push_back(N());
            if (getToken() == Token::LPAREN) {
                curToken++;
            } else {
                throw runtime_error{"Unexpected token, expected LPAREN"};
            }
            tmp.push_back(A());
            if (getToken() == Token::RPAREN) {
                curToken++;
            } else {
                throw runtime_error{"Unexpected token, expected RPAREN"};
            }
            if (getToken() == Token::SEMICOLON) {
                curToken++;
            } else {
                throw runtime_error{"Unexpected token, expected SEMICOLON"};
            }
            break;
        }
        default: {
            throw runtime_error{"Unexpected token, expected TYPE"};
        }
    }
    return tree("S", tmp);
}

bool isNum(char c) {
    return c >= '0' && c <= '9';
}

bool isValid(char c) {
    return isNum(c) || c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

vector<Token> create_tokens(string s) {
    vector<Token> ans;
    int cur = 0;
    while (cur < s.length()) {
        switch (s[cur]) {
            case '(': {
                ans.push_back(Token::LPAREN);
                cur++;
                break;
            }
            case ')': {
                ans.push_back(Token::RPAREN);
                cur++;
                break;
            }
            case ';': {
                ans.push_back(Token::SEMICOLON);
                cur++;
                break;
            }
            case '*': {
                ans.push_back(Token::ASTERISK);
                cur++;
                break;
            }
            case '&': {
                ans.push_back(Token::AMPERSAND);
                cur++;
                break;
            }
            case ',': {
                ans.push_back(Token::COMMA);
                cur++;
                break;
            }
            case ' ':
            case '\r':
            case '\t':
            case '\n': {
                cur++;
                break;
            }
            default: {
                if (isNum(s[cur])) {
                    string message = "Name can't begin with number";
                    throw runtime_error{message};
                } else if (isValid(s[cur])) {
                    cur++;
                    bool f = true;
                    while (f && cur < s.length()) {
                        switch (s[cur]) {
                            case ' ':
                            case '\r':
                            case '\n':
                            case '\t':
                            case ',' :
                            case '*' :
                            case '&' :
                            case ')' :
                            case '(' : {
                                ans.push_back(Token::NAME);
                                f = false;
                                break;
                            }
                            default: {
                                if (!isValid(s[cur])) {
                                    string message = "Unexpected symbol ";
                                    message += s[cur];
                                    message += " in lexer";
                                    throw runtime_error{message};
                                }
                                cur++;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    ans.push_back(Token::END);
    return ans;
}

void visualize(tree Tree, int depth) {
    for (int i = 0; i < depth; i++)
        fout << "   ";
    fout << Tree.node << endl;
    for (const auto &i: Tree.children) {
        visualize(i, depth + 1);
    }
}

bool do_test(string s, int i) {
    try {
        tokens = create_tokens(s);
        curToken = 0;
        tree newTree = S();
        fout << "Test " << i << ", line = " << s << endl;
        visualize(newTree, 0);
        fout << endl;
        return true;
    } catch (exception &e) {
        cout << e.what() << endl;
        return false;
    }
}

void test1() {
    string s = "int func(int **n, ff &b, string param1, _lon *&x, ttt *****&z);";
    if (do_test(s, 1)) cout << "Test 1 - OK" << endl;
    else {
        cout << "An error occurred during test 1" << endl;
        exit(1);
    }
}

void test2() {
    string s = "void f();";
    if (do_test(s, 2)) cout << "Test 2 - OK" << endl;
    else {
        cout << "An error occurred during test 2" << endl;
        exit(1);
    }
}

void test3() {
    string s = "bool many(long a1, int a2, bool a3, int a4, int a5, long a6, long a7, int a8, int a9, int a10, bool a11, long a12, int a13, long a14);";
    if (do_test(s, 3)) cout << "Test 3 - OK" << endl;
    else {
        cout << "An error occurred during test 3" << endl;
        exit(1);
    }
}

void test4() {
    string s = "long rand(int *a, string s, long ***b, bool f, int e, int **q);";
    if (do_test(s, 4)) cout << "Test 4 - OK" << endl;
    else {
        cout << "An error occurred during test 4" << endl;
        exit(1);
    }
}

void test5() {
    string s = "int func(s*,type1);";
    if (do_test(s, 5)) cout << "Test 5 - OK" << endl;
    else {
        cout << "An error occurred during test 5" << endl;
        exit(1);
    }
}

void test6() {
    string s = "_ _ty3    (     st **tt     )   ;";
    if (do_test(s, 6)) cout << "Test 6 - OK" << endl;
    else {
        cout << "An error occurred during test 6" << endl;
        exit(1);
    }
}

void test7() {
    string s = "   ll    f    (   a*b,cs *ss,     rt    bam,    pp ******qqq   )  ;";
    if (do_test(s, 7)) cout << "Test 7 - OK" << endl;
    else {
        cout << "An error occurred during test 7" << endl;
        exit(1);
    }
}

void run_tests() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    cout << "All tests passed" << endl;
}

void get_test() {
    string s;
    getline(cin, s);
    do_test(s, 0);
}

int main() {
    run_tests();
    //get_test();
    return 0;
}