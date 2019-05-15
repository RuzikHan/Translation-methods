#include "parser.h"

class lexer {
public:
    ifstream lfin;
    ofstream lfout;
    vector<Token> token_types;
    vector<string> includes;
    string define_type = "string";

    explicit lexer(const string &file, vector<Token> types) {
        this->lfin = ifstream(file);
        this->lfout = ofstream("lexer.out.h");
        token_types = move(types);
    }

    ~lexer() = default;

    void tokenize() {
        string work_line;
        getline(lfin, work_line);
        while (true) {
            if (work_line.substr(0, 15) == "#define YYSTYPE") {
                define_type = work_line.substr(16);
            } else if (work_line.substr(0, 8) == "#include") {
                includes.push_back(work_line);
            } else if (work_line == "%%") {
                getline(lfin, work_line);
                while (work_line != "%%") {
                    int left_pos = work_line.length();
                    while (left_pos >= 0 && work_line[left_pos] != '{') left_pos--;
                    if (left_pos == -1) {
                        work_line.erase(work_line.length() - 1);
                        left_pos = work_line.length() - 1;
                        while (work_line[left_pos] == ' ' || work_line[left_pos] == '\r' ||
                               work_line[left_pos] == '\t')
                            left_pos--;
                        work_line.erase(left_pos + 1);
                        Token tmp;
                        tmp.regex = work_line;
                        tmp.token_name = "";
                        token_types.push_back(tmp);
                    } else {
                        string copy_line = work_line.substr(left_pos);
                        copy_line.erase(copy_line.length() - 1);
                        int pos = 0;
                        while (work_line.substr(pos, 6) != "return") pos++;
                        string token_name = work_line.substr(pos + 7);
                        token_name.erase(token_name.length() - 3);
                        pos = 0;
                        while (work_line[pos] != ' ' && work_line[pos] != '\t' && work_line[pos] != '\r') pos++;
                        for (auto &token_type : token_types)
                            if (token_name == token_type.token_name) {
                                token_type.regex = work_line.substr(0, pos);
                                token_type.function = copy_line;
                            }
                    }
                    getline(lfin, work_line);
                }
                break;
            } else {
                throw runtime_error{"Expected tokens description"};
            }
            getline(lfin, work_line);
        }
    }

    void make_file() {
        lfout << "#include <regex>\n"
                 "#include <stdexcept>\n"
                 "#include <iostream>\n"
                 "#include <vector>\n"
                 "#include <functional>\n";
        for (const auto &i: includes) {
            lfout << i << "\n";
        }
        lfout << "\n"
                 "using namespace std;\n"
                 "\n";
        lfout << "#define YYSTYPE " << define_type << "\n"
                                                      "\n";
        create_token_h();
        lfout << "\n";
        lfout << "vector<Token> tokens;\n"
                 "vector<Token> ans_array;\n"
                 "\n"
                 "void create_tokens() {\n"
                 "      Token tmp;\n";
        for (auto &token_type : token_types) {
            lfout << "      tmp.token_name = \"" << token_type.token_name << "\";\n";
            if (token_type.regex == "<<EOF>>") {
                lfout << "      tmp.regex = \"\";\n";
                lfout << "      tmp.regex += EOF;\n";
            } else {
                string regex_line;
                for (char i : token_type.regex) {
                    if (i == '\\') {
                        regex_line += "\\\\";
                    } else {
                        regex_line += i;
                    }
                }
                lfout << "      tmp.regex = \"" << regex_line << "\";\n";
            }
            lfout << "      tmp.func = [](YYSTYPE* yylval, string yytext)->int{" << token_type.function
                  << "return 0;};\n";
            lfout << "      tokens.push_back(tmp);\n";
        }
        lfout << "}\n"
                 "\n"
                 "string s;\n"
                 "\n"
                 "void read_all() {\n"
                 "    char c;\n"
                 "    c = getchar();\n"
                 "    while (c != EOF) {\n"
                 "        s += c;\n"
                 "        c = getchar();\n"
                 "    }\n"
                 "    s += c;\n"
                 "}\n"
                 "\n"
                 "void processing() {\n"
                 "    create_tokens();\n"
                 "    read_all();\n"
                 "    while (s.length() > 0) {\n"
                 "        unsigned long len = s.length();\n"
                 "        for (auto j: tokens) {\n"
                 "            string temp = s;\n"
                 "            regex regex1(j.regex);\n"
                 "            bool f = false;\n"
                 "            while (temp.length() > 0) {\n"
                 "                if (regex_match(temp, regex1)) {\n"
                 "                    Token tmp;\n"
                 "                    tmp.token_name = j.token_name;\n"
                 "                    tmp.regex = temp;\n"
                 "                    int val = j.func(&tmp.yylval, tmp.regex);\n"
                 "                    tmp.func_value = val;\n"
                 "                    if (val != 0) ans_array.push_back(tmp);\n"
                 "                    s.erase(0, temp.length());\n"
                 "                    f = true;\n"
                 "                    break;\n"
                 "                } else {\n"
                 "                    temp.erase(temp.size() - 1, 1);\n"
                 "                }\n"
                 "            }\n"
                 "            if (f) break;\n"
                 "        }\n"
                 "        if (len == s.length()) {\n"
                 "            string message = \"Unexpected symbol \";\n"
                 "            message += s[0];\n"
                 "            throw runtime_error{message};\n"
                 "        }\n"
                 "    }\n"
                 "}\n"
                 "\n";
    }

private:
    void create_token_h() {
        lfout << "enum tokens {\n"
                 "    ";
        int id = 239;
        for (int i = 0; i < token_types.size(); i++) {
            if (token_types[i].token_name != "") {
                if (i == 0) lfout << token_types[i].token_name << " = " << id;
                else lfout << ", " << token_types[i].token_name << " = " << id;
                id++;
            }
        }
        lfout << "\n"
                 "}; \n"
                 "\n"
                 "class Token {\n"
                 "public:\n"
                 "    string token_name;\n"
                 "    string regex;\n"
                 "    function<int(YYSTYPE*, string)> func;\n"
                 "    YYSTYPE yylval;\n"
                 "    int func_value;\n"
                 "};\n";
    }
};