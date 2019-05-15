#include <utility>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include "token.h"

class parser {
public:
    ifstream pfin;
    ofstream pfout;
    vector<Token> token_types;
    vector<string> add_code;
    string start_term;
    string define_type;
    bool is_ad = false;

    struct code_save {
        vector<string> head;
        vector<string> code;
    };

    struct rule {
        string from;
        vector<code_save> members;
    };

    vector<rule> rules;
    map<string, set<string>> first, follow;

    explicit parser(const string &file) {
        this->pfin = ifstream(file);
        this->pfout = ofstream("parser.out.h");
    }

    ~parser() = default;

    void create() {
        string work_line;
        getline(pfin, work_line);
        if (work_line.substr(0, 6) != "%token") {
            throw runtime_error{"Expected token description"};
        }
        work_line.erase(0, 7);
        token_types = tokenize(work_line);
        getline(pfin, work_line);
        if (work_line.substr(0, 6) != "%start") {
            throw runtime_error{"Expected start token description"};
        }
        work_line.erase(0, 7);
        start_term = work_line;
        getline(pfin, work_line);
        while (work_line != "%%") {
            while (work_line[0] == ' ' || work_line[0] == '\r' || work_line[0] == '\t')
                work_line.erase(0, 1);
            add_code.push_back(work_line);
            getline(pfin, work_line);
        }
        parse_rules();
        create_first_follow();
    }

    void make_file() {
        pfout << "#include \"lexer.out.h\"\n";
        if (define_type[define_type.length() - 1] == '*') is_ad = true;
        define_type = "YYSTYPE";
        pfout << "\n"
                 "int curToken = 0;\n";
        for (const auto &i: add_code) {
            pfout << i << "\n";
        }
        pfout << "\n"
                 "map<string, int> id;\n"
                 "\n"
                 "void init_id() {\n";
        for (const auto &i: token_types) {
            pfout << "    id[\"" << i.token_name << "\"] = " << i.token_name << ";\n";
        }
        pfout << "}\n"
                 "\n"
                 "Token getToken() {\n"
                 "    return ans_array[curToken];\n"
                 "}\n"
                 "\n";
        for (const auto &rule : rules) {
            pfout << define_type << " " << rule.from << "();\n";
        }
        pfout << "\n";
        for (const auto &rule : rules) {
            pfout << define_type << " " << rule.from << "() {\n";
            pfout << "    vector<" << define_type << "> tmp;\n";
            pfout << "    " << define_type << " ans;\n";
            pfout << "    Token tmp_token = getToken();\n";
            pfout << "    switch(id[tmp_token.token_name]) {\n";
            int count = 0;
            string expected;
            bool eps = false;
            for (const auto &i: rule.members) {
                for (const auto &j: i.head) {
                    if (j == "|") {
                        eps = true;
                        break;
                    }
                }
            }
            for (auto i: rule.members) {
                string s = i.head[0];
                bool need_first_token = false;
                if (s != "|") {
                    if (first.count(s) > 0) {
                        int val = 0;
                        for (const auto &q : first[s]) {
                            if (q != "|") {
                                if (val != 0) pfout << "\n";
                                if (count == 0) {
                                    expected += q;
                                } else {
                                    expected += " or " + q;
                                }
                                count++;
                                pfout << "        case " << q << ": ";
                                val++;
                            } else {
                                for (auto qq: follow[s]) {
                                    if (val != 0) pfout << "\n";
                                    if (count == 0) {
                                        expected += qq;
                                    } else {
                                        expected += " or " + qq;
                                    }
                                    count++;
                                    pfout << "        case " << qq << ": ";
                                    val++;
                                }
                            }
                        }
                        pfout << "{ \n";
                    } else {
                        need_first_token = true;
                        if (count == 0) {
                            expected += s;
                        } else {
                            expected += " or " + s;
                        }
                        count++;
                        pfout << "        case " << s << ": {\n";
                    }
                    int start = 0;
                    if (need_first_token) {
                        pfout << "            curToken++;\n";
                        pfout << "            tmp.push_back(tmp_token.yylval);\n";
                        start++;
                    }
                    bool token_tmp = false;
                    for (int j = start; j < i.head.size(); j++) {
                        if (first.count(i.head[j])) {
                            pfout << "            tmp.push_back(" << i.head[j] << "());\n";
                        } else {
                            if (!token_tmp) {
                                pfout << "            Token val = getToken();\n";
                                token_tmp = true;
                            } else {
                                pfout << "            val = getToken();\n";
                            }
                            pfout << "            if (val.token_name == \"" << i.head[j] << "\") {\n";
                            pfout << "                curToken++;\n";
                            pfout << "                tmp.push_back(val.yylval);\n";
                            pfout << "            } else { \n";
                            pfout << "                throw runtime_error{\"Unexpected token, expected " << i.head[j]
                                  << "\"};\n";
                            pfout << "            }\n";
                        }
                    }
                    code_out(i);
                    pfout << "            break;\n"
                             "        }\n";
                }
            }
            if (eps) {
                bool need_code_out = false;
                for (const auto &i: follow[rule.from]) {
                    if (i != "$") {
                        need_code_out = true;
                        expected += " or " + i;
                        pfout << "        case " << i << ": \n";
                    }
                }
                if (need_code_out) {
                    for (auto i: rule.members) {
                        for (int j = 0; j < i.head.size(); j++) {
                            if (i.head[j] == "|") {
                                code_out(i);
                                break;
                            }
                        }
                    }
                    pfout << "            break;\n";
                }
            }
            pfout << "        default: {\n"
                     "            throw runtime_error{\"Unexpected token, expected " << expected << "\"};\n"
                                                                                                    "        }\n"
                                                                                                    "    }\n"
                                                                                                    "    return ans;\n";
            pfout << "}\n";
            pfout << "\n";
        }
    }

private:
    void code_out(code_save i) {
        for (const auto &j: i.code) {
            string line = j;
            int pos = 0;
            pfout << "            ";
            string out_line;
            for (int k = 0; k < line.length(); k++) {
                if (line[k] == '$') {
                    if (line[k + 1] == '$') {
                        if (is_ad) out_line += "*";
                        out_line += "ans";
                        k++;
                    } else {
                        k++;
                        char num[line.length()];
                        pos = 0;
                        while (isNum(line[k])) {
                            num[pos] = line[k];
                            pos++;
                            k++;
                        }
                        k--;
                        num[pos] = 'S';
                        int number = atoi(num);
                        number--;
                        if (is_ad) out_line += "*";
                        out_line += "tmp[";
                        out_line += to_string(number);
                        out_line += "]";
                    }
                } else {
                    out_line += line[k];
                }
            }
            pfout << out_line << "\n";
        }
    }

    bool isNum(char c) {
        return c >= '0' && c <= '9';
    }

    bool line_with_2(string s, char c1, char c2) {
        if (s.length() < 2) return false;
        if (s[s.length() - 1] != c2) return false;
        if (s[s.length() - 2] != c1) return false;
        for (int i = 0; i < s.length() - 2; i++)
            if (s[i] == ' ' || s[i] == '\r' || s[i] == '\t') continue; else return false;
        return true;
    }

    bool line_with(string s, char c) {
        if (s[s.length() - 1] != c) return false;
        for (int i = 0; i < s.length() - 1; i++)
            if (s[i] == ' ' || s[i] == '\r' || s[i] == '\t') continue; else return false;
        return true;
    }

    void create_first_follow() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto &i : rules) {
                set<string> save = first[i.from];
                for (auto j: i.members) {
                    string s = j.head[0];
                    if (first.count(s) > 0) {
                        for (const auto &q: first[s]) {
                            first[i.from].insert(q);
                        }
                    } else {
                        first[i.from].insert(s);
                    }
                }
                if (save != first[i.from]) changed = true;
            }
        }
        follow[start_term].insert("$");
        changed = true;
        while (changed) {
            changed = false;
            for (const auto &i : rules) {
                for (auto j: i.members) {
                    bool eps_first = true;
                    string s = j.head[j.head.size() - 1];
                    if (follow.count(s) > 0) {
                        set<string> save = follow[s];
                        for (const auto &q: follow[i.from]) {
                            follow[s].insert(q);
                        }
                        if (save != follow[s]) changed = true;
                        if (first[s].count("|") == 0) eps_first = false;
                    }
                    for (int k = j.head.size() - 2; k >= 0; k--) {
                        bool ff = false;
                        if (follow.count(j.head[k])) {
                            set<string> save = follow[j.head[k]];
                            if (follow.count(j.head[k + 1])) {
                                for (auto q: first[j.head[k + 1]]) {
                                    if (q != "|") follow[j.head[k]].insert(q);
                                    else {
                                        ff = true;
                                        if (eps_first) {
                                            for (const auto &qq : follow[i.from]) {
                                                follow[j.head[k]].insert(qq);
                                            }
                                        }
                                    }
                                }
                            } else {
                                follow[j.head[k]].insert(j.head[k + 1]);
                            }
                            if (save != follow[j.head[k]]) changed = true;
                        }
                        if (!ff) eps_first = false;
                    }
                }
            }
        }
    }

    bool checkline(const string &s) {
        for (auto c :s) {
            if (c == ' ' || c == '\t' || c == '\r') continue; else return false;
        }
        return true;
    }

    void parse_rules() {
        string work_line;
        getline(pfin, work_line);
        while (work_line != "%%") {
            if (work_line[work_line.length() - 1] == ':') {
                rule tmp;
                work_line.erase(work_line.length() - 1);
                tmp.from = work_line;
                set<string> empty;
                first[work_line] = empty;
                follow[work_line] = empty;
                getline(pfin, work_line);
                while (true) {
                    if (work_line[work_line.length() - 1] == '{') {
                        work_line.erase(work_line.length() - 2);
                        int pos = 0;
                        while ((work_line[pos] == ' ' || work_line[pos] == '\t' || work_line[pos] == '\r') &&
                               pos < work_line.length())
                            pos++;
                        work_line = work_line.substr(pos);
                        pos = 0;
                        code_save val;
                        bool if_push = false;
                        while (pos < work_line.length()) {
                            while (work_line[pos] != ' ' && pos < work_line.length()) pos++;
                            val.head.push_back(work_line.substr(0, pos));
                            if_push = true;
                            work_line.erase(0, pos + 1);
                            pos = 0;
                        }
                        if (!if_push) val.head.push_back("|");
                        getline(pfin, work_line);
                        while (!line_with_2(work_line, '}', ';')) {
                            if (!checkline(work_line)) {
                                while (work_line[0] == ' ' || work_line[0] == '\r' || work_line[0] == '\t')
                                    work_line.erase(0, 1);
                                val.code.push_back(work_line);
                            }
                            getline(pfin, work_line);
                        }
                        tmp.members.push_back(val);
                        getline(pfin, work_line);
                        if (line_with(work_line, '|')) {
                            getline(pfin, work_line);
                            continue;
                        } else if (line_with(work_line, ';')) break;
                    }
                }
                rules.push_back(tmp);
            } else {
                throw runtime_error{"Expected state description"};
            }
            getline(pfin, work_line);
        }
    }

    vector<Token> tokenize(string line) {
        vector<Token> ans;
        Token tmp;
        int pos = 0;
        while (pos < line.length()) {
            if (line[pos] == ' ') {
                tmp.token_name = line.substr(0, pos);
                ans.push_back(tmp);
                line.erase(0, pos + 1);
                pos = -1;
            }
            pos++;
        }
        tmp.token_name = line;
        ans.push_back(tmp);
        return ans;
    }
};