#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <thread>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <array>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__ANDROID__)
#include <sys/system_properties.h>
#endif

#define VERSION "0.32_beta"

class BinaryScriptEngine {
private:
    std::map<std::string, std::string> variables;
    const int step_delay_ms = 10;

    void reportError(const std::string& type, const std::string& msg, const std::string& ctx = "") {
        std::cerr << "\033[1;31m[" << type << "]\033[0m " << msg;
        if (!ctx.empty()) std::cerr << " -> \"" << ctx << "\"";
        std::cerr << std::endl;
    }

    std::string stripComments(std::string code) {
        std::string clean_code = "";
        size_t i = 0;
        while (i < code.length()) {
            if (code[i] == '#') {
                size_t next_hash = code.find('#', i + 1);
                if (next_hash != std::string::npos) {
                    i = next_hash + 1;
                } else {
                    reportError("СИНТАКСИС", "Обнаружен незакрытый комментарий '#'", code.substr(i, 10));
                    break;
                }
            } else {
                clean_code += code[i];
                i++;
            }
        }
        return clean_code;
    }

    std::string getCurrentTimeStr() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* local_tm = std::localtime(&now_time);
        char buf[100];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", local_tm);
        return std::string(buf);
    }

    std::string execCommand(const char* cmd) {
        #if defined(_WIN32)
        FILE* pipe = _popen(cmd, "r");
        if (!pipe) return "";
        std::unique_ptr<FILE, int(*)(FILE*)> pipe_ptr(pipe, _pclose);
        #elif defined(__APPLE__) || defined(__linux__)
        FILE* pipe = popen(cmd, "r");
        if (!pipe) return "";
        std::unique_ptr<FILE, int(*)(FILE*)> pipe_ptr(pipe, pclose);
        #else
        return "";
        #endif
        std::array<char, 128> buffer;
        std::string result;
        while (fgets(buffer.data(), buffer.size(), pipe_ptr.get()) != nullptr) {
            result += buffer.data();
        }
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        result.erase(std::remove(result.begin(), result.end(), '\r'), result.end());
        return result;
    }

    std::string getSystemSpecs() {
        std::string os_details = "Unknown OS";

        #ifdef _WIN32
        os_details = "Windows";
        typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);
        HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
        if (hMod) {
            RtlGetVersionPtr pRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
            if (pRtlGetVersion) {
                RTL_OSVERSIONINFOEXW osInfo = { 0 };
                osInfo.dwOSVersionInfoSize = sizeof(osInfo);
                if (pRtlGetVersion(&osInfo) == 0) {
                    if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0) {
                        if (osInfo.dwBuildNumber >= 22000) os_details = "Windows 11";
                        else os_details = "Windows 10";
                    } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 3) {
                        os_details = "Windows 8.1";
                    } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 2) {
                        os_details = "Windows 8";
                    } else if (osInfo.dwMajorVersion == 6 && osInfo.dwMinorVersion == 1) {
                        os_details = "Windows 7";
                    } else {
                        os_details = "Windows NT " + std::to_string(osInfo.dwMajorVersion) + "." + std::to_string(osInfo.dwMinorVersion);
                    }
                }
            }
        }
        #elif defined(__ANDROID__)
        char rel[PROP_VALUE_MAX] = {0};
        char sdk[PROP_VALUE_MAX] = {0};
        __system_property_get("ro.build.version.release", rel);
        __system_property_get("ro.build.version.sdk", sdk);

        std::string rel_str = rel;
        std::string sdk_str = sdk;

        if (!rel_str.empty()) {
            os_details = "Android " + rel_str;
            if (!sdk_str.empty()) {
                os_details += " (API " + sdk_str + ")";
            }
        } else {
            os_details = "Android";
        }
        #elif __APPLE__
        std::string version = execCommand("sw_vers -productVersion");
        std::string name = execCommand("sw_vers -productName");
        if (name.empty()) name = "macOS";
        os_details = name + " " + (version.empty() ? "" : version);
        #elif __linux__
        std::ifstream os_release("/etc/os-release");
        if (os_release.is_open()) {
            std::string line;
            while (std::getline(os_release, line)) {
                if (line.substr(0, 12) == "PRETTY_NAME=") {
                    os_details = line.substr(12);
                    os_details.erase(std::remove(os_details.begin(), os_details.end(), '"'), os_details.end());
                    break;
                }
            }
            os_release.close();
        } else {
            os_details = "Linux Generic";
        }
        #endif

        unsigned int cores = std::thread::hardware_concurrency();
        std::string cores_str = (cores > 0) ? std::to_string(cores) + " Cores" : "Unknown Cores";

        return "OS: " + os_details + " | Engine: BSE " VERSION " | CPU: " + cores_str;
    }

    void replaceSystemPlaceholders(std::string& str) {
        size_t pos;
        while ((pos = str.find("{[time]}")) != std::string::npos) {
            str.replace(pos, 8, "{" + getCurrentTimeStr() + "}");
        }
        while ((pos = str.find("{[sys]}")) != std::string::npos) {
            str.replace(pos, 7, "{" + getSystemSpecs() + "}");
        }
    }

    size_t findTerminator(const std::string& code, size_t start_pos) {
        int depth = 0;
        int p_depth = 0;
        for (size_t i = start_pos; i < code.length(); ++i) {
            if (code[i] == '(') p_depth++;
            if (code[i] == ')') p_depth--;

            if (code[i] == ':' && p_depth == 0) depth++;

            if (code[i] == ';' && p_depth == 0) {
                if (depth == 0) return i;
                depth--;
            }
        }
        return std::string::npos;
    }

    std::string decodeBinaryString(std::string content) {
        if (!content.empty() && content.front() == '[') content.erase(0, 1);
        if (!content.empty() && content.back() == ']') content.pop_back();

        std::string result;
        std::stringstream ss(content);
        std::string word;
        while (std::getline(ss, word, '}')) {
            if (word.empty()) continue;
            if (word[0] == '{') word.erase(0, 1);
            std::stringstream bytes(word);
            std::string byteStr;
            while (std::getline(bytes, byteStr, '_')) {
                if (!byteStr.empty()) {
                    try {
                        int character = std::stoi(byteStr, nullptr, 2);
                        result += (char)character;
                    } catch (...) {}
                }
            }
            result += " ";
        }
        if (!result.empty() && result.back() == ' ') result.pop_back();
        return result;
    }

    bool trySolveMath(std::string s, long long& out_result) {
        s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
        if (s.empty()) return false;

        for (int i = (int)s.length() - 1; i >= 0; --i) {
            if (s[i] == '+' || s[i] == '-') {
                if (i == 0) continue;
                if (s[i-1] == '+' || s[i-1] == '-' || s[i-1] == '*' || s[i-1] == '/') continue;

                long long left = 0, right = 0;
                if (trySolveMath(s.substr(0, i), left) && trySolveMath(s.substr(i + 1), right)) {
                    out_result = (s[i] == '+') ? (left + right) : (left - right);
                    return true;
                }
                return false;
            }
        }

        for (int i = (int)s.length() - 1; i >= 0; --i) {
            if (s[i] == '*' || s[i] == '/') {
                if (i == 0 || i == (int)s.length() - 1) return false;
                long long left = 0, right = 0;
                if (trySolveMath(s.substr(0, i), left) && trySolveMath(s.substr(i + 1), right)) {
                    if (s[i] == '*') {
                        out_result = left * right;
                    } else {
                        if (right == 0) return false;
                        out_result = left / right;
                    }
                    return true;
                }
                return false;
            }
        }

        try {
            size_t idx = 0;
            long long val = std::stoll(s, &idx);
            if (idx == s.length()) {
                out_result = val;
                return true;
            }
        } catch (...) {}

        return false;
    }

    std::string evaluate(std::string expr) {
        replaceSystemPlaceholders(expr);

        expr.erase(0, expr.find_first_not_of(" \t"));
        size_t last = expr.find_last_not_of(" \t");
        if (last != std::string::npos) expr = expr.substr(0, last + 1);
        if (expr.empty()) return "";

        // Строковые литералы в фигурных скобках {...}
        if (expr.front() == '{' && expr.back() == '}') {
            return expr.substr(1, expr.size() - 2);
        }

        // Если выражением является двоичный BS-блок с подчеркиваниями
        if (expr.find('_') != std::string::npos) {
            return decodeBinaryString(expr);
        }

        // Преобразование двоичных токенов (последовательностей из '0' и '1') в десятичные числа
        // Выполняется ДО заменяемых переменных, чтобы уже вычисленные значения из переменных не конвертировались повторно
        std::string processed;
        size_t idx = 0;
        while (idx < expr.length()) {
            if (isalnum((unsigned char)expr[idx])) {
                size_t start = idx;
                while (idx < expr.length() && isalnum((unsigned char)expr[idx])) {
                    idx++;
                }
                std::string token = expr.substr(start, idx - start);
                bool isBinaryNumber = !token.empty() && std::all_of(token.begin(), token.end(), [](char c) {
                    return c == '0' || c == '1';
                });
                if (isBinaryNumber) {
                    try {
                        processed += std::to_string(std::stoll(token, nullptr, 2));
                    } catch (...) {
                        processed += token;
                    }
                } else {
                    processed += token;
                }
            } else {
                processed += expr[idx];
                idx++;
            }
        }
        expr = processed;

        // Замена переменных пользователя
        std::vector<std::pair<std::string, std::string>> v(variables.begin(), variables.end());
        std::sort(v.begin(), v.end(), [](const auto& a, const auto& b){ return a.first.length() > b.first.length(); });
        for (auto const& [name, val] : v) {
            size_t pos = 0;
            while ((pos = expr.find(name, pos)) != std::string::npos) {
                bool left = (pos == 0 || !isalnum(expr[pos-1]));
                bool right = (pos + name.length() == expr.length() || !isalnum(expr[pos+name.length()]));
                if (left && right) {
                    expr.replace(pos, name.length(), val);
                    pos += val.length();
                } else pos += 1;
            }
        }

        // Попытка вычислить математику
        long long mathResult = 0;
        if (trySolveMath(expr, mathResult)) {
            return std::to_string(mathResult);
        }

        return expr;
    }

    bool checkCondition(std::string cond) {
        replaceSystemPlaceholders(cond);

        size_t eq = cond.find("==");
        if (eq == std::string::npos) return false;
        return evaluate(cond.substr(0, eq)) == evaluate(cond.substr(eq + 2));
    }

    void runSimpleCmd(std::string cmd) {
        cmd.erase(0, cmd.find_first_not_of(" \t"));
        while(!cmd.empty() && isspace(cmd.back())) cmd.pop_back();
        if (cmd.empty()) return;

        std::this_thread::sleep_for(std::chrono::milliseconds(step_delay_ms));

        if (cmd == "1[time]") {
            std::cout << getCurrentTimeStr() << std::endl << std::flush;
        }
        else if (cmd == "1[sys]") {
            std::cout << getSystemSpecs() << std::endl << std::flush;
        }
        else if (cmd.size() > 3 && cmd.substr(0, 2) == "1[") {
            std::string filePath = "";
            std::string mode = "run";

            if (cmd.back() == ']') {
                filePath = cmd.substr(2, cmd.size() - 3);
            } else if (cmd.back() == ')') {
                size_t sep = cmd.rfind("](");
                if (sep != std::string::npos) {
                    filePath = cmd.substr(2, sep - 2);
                    mode = cmd.substr(sep + 2, cmd.size() - (sep + 2) - 1);
                }
            }

            if (!filePath.empty()) {
                filePath.erase(0, filePath.find_first_not_of(" \t"));
                while(!filePath.empty() && isspace(filePath.back())) filePath.pop_back();
                mode.erase(0, mode.find_first_not_of(" \t"));
                while(!mode.empty() && isspace(mode.back())) mode.pop_back();

                if (filePath.rfind("http://", 0) == 0 || filePath.rfind("https://", 0) == 0) {
                    std::string systemCmd = "";
                    #if defined(_WIN32)
                    systemCmd = "start \"\" \"" + filePath + "\"";
                    #elif defined(__APPLE__)
                    systemCmd = "open \"" + filePath + "\"";
                    #elif defined(__linux__)
                    systemCmd = "xdg-open \"" + filePath + "\" &";
                    #endif

                    if (!systemCmd.empty()) {
                        std::system(systemCmd.c_str());
                    } else {
                        reportError("СИСТЕМА", "Открытие ссылок не поддерживается на вашей ОС");
                    }
                }
                else {
                    if (mode == "run") {
                        std::ifstream scriptFile(filePath);
                        if (scriptFile.is_open()) {
                            std::string fileContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
                            scriptFile.close();

                            fileContent = stripComments(fileContent);

                            size_t pos = 0;
                            while ((pos = fileContent.find("\r\n", pos)) != std::string::npos) {
                                fileContent.replace(pos, 2, "\n");
                            }
                            pos = 0;
                            while ((pos = fileContent.find('\n', pos)) != std::string::npos) {
                                fileContent.replace(pos, 1, " & ");
                                pos += 3;
                            }
                            this->execute(fileContent);
                        } else {
                            reportError("ФАЙЛ", "Не удалось прочитать или найти скрипт", filePath);
                        }
                    }
                    else if (mode == "read") {
                        std::ifstream scriptFile(filePath);
                        if (scriptFile.is_open()) {
                            std::string fileContent((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
                            scriptFile.close();
                            std::cout << fileContent << std::endl << std::flush;
                        } else {
                            reportError("ФАЙЛ", "Не удалось прочитать или найти файл", filePath);
                        }
                    }
                    else if (mode == "edit") {
                        std::ifstream scriptFile(filePath);
                        std::string fileContent = "";
                        if (scriptFile.is_open()) {
                            fileContent.assign((std::istreambuf_iterator<char>(scriptFile)), std::istreambuf_iterator<char>());
                            scriptFile.close();
                            std::cout << "Текущий код файла:\n" << fileContent << "\n" << std::endl;
                        } else {
                            std::cout << "Файл не найден. Будет создан новый файл." << std::endl;
                        }

                        std::cout << "Напишите измененный код:" << std::endl << "> " << std::flush;
                        std::string newContent;
                        std::getline(std::cin, newContent);

                        std::ofstream outFile(filePath);
                        if (outFile.is_open()) {
                            outFile << newContent;
                            outFile.close();
                            std::cout << "Файл успешно сохранен." << std::endl;
                        } else {
                            reportError("ФАЙЛ", "Не удалось сохранить изменения в файл", filePath);
                        }
                    }
                    else if (mode == "write") {
                        std::cout << "Напишите код:" << std::endl << "> " << std::flush;
                        std::string newContent;
                        std::getline(std::cin, newContent);

                        std::ofstream outFile(filePath);
                        if (outFile.is_open()) {
                            outFile << newContent;
                            outFile.close();
                            std::cout << "Файл успешно создан и записан." << std::endl;
                        } else {
                            reportError("ФАЙЛ", "Не удалось создать и записать файл", filePath);
                        }
                    }
                    else {
                        reportError("СИНТАКСИС", "Неизвестный режим работы с файлом (run, read, edit, write)", mode);
                    }
                }
            }
        }
        // Вывод 2([x]) - ТЕКСТ/СТРОКИ/СИМВОЛЫ
        else if (cmd.size() > 3 && cmd.substr(0, 3) == "2([" && cmd.back() == ')') {
            std::string inner = cmd.substr(3, cmd.length() - 4);
            if (!inner.empty() && inner.back() == ']') {
                inner.pop_back();
            }
            std::string val = evaluate(inner);

            bool isNumber = !val.empty() && std::all_of(val.begin(), val.end(), [](char c) {
                return std::isdigit((unsigned char)c) || c == '-';
            });

            if (isNumber) {
                reportError("ОШИБКА ТИПА", "2([x]) ожидает текст, но получил число", val);
            } else {
                std::cout << val << std::endl << std::flush;
            }
        }
        // Вывод 2(x) - ЧИСЛА
        else if (cmd.size() > 3 && cmd.substr(0, 2) == "2(" && cmd.back() == ')') {
            std::string inner = cmd.substr(2, cmd.length() - 3);
            std::string val = evaluate(inner);

            bool isNumber = !val.empty() && std::all_of(val.begin(), val.end(), [](char c) {
                return std::isdigit((unsigned char)c) || c == '-';
            });

            if (!isNumber) {
                reportError("ОШИБКА ТИПА", "2(x) ожидает число, но получил текст", val);
            } else {
                std::cout << val << std::endl << std::flush;
            }
        }
        // Присваивание переменной 0 ...
        else if (cmd.substr(0, 2) == "0 ") {
            std::string body = cmd.substr(2);
            size_t eq = body.find('=');
            if (eq != std::string::npos) {
                std::string name = body.substr(0, eq);
                name.erase(std::remove(name.begin(), name.end(), ' '), name.end());

                if (!name.empty() && name.front() == '!') {
                    std::string var_name = name.substr(1);
                    std::cout << "Введите значение для " << var_name << ": " << std::flush;
                    std::string input_val;
                    std::getline(std::cin, input_val);
                    // Сохраняем ввод пользователя 1 к 1 без обработки
                    variables[var_name] = input_val;
                } else {
                    variables[name] = evaluate(body.substr(eq + 1));
                }
            } else {
                size_t opPos = body.find_first_of("+-*/");
                if (opPos != std::string::npos) {
                    std::string name = body.substr(0, opPos);
                    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
                    if (variables.count(name)) {
                        variables[name] = evaluate(variables[name] + body[opPos] + body.substr(opPos + 1));
                    }
                }
            }
        }
    }

public:
    void execute(std::string code) {
        code = stripComments(code);

        size_t i = 0;
        bool branch_taken = false;

        while (i < code.length()) {
            while (i < code.length() && (isspace(code[i]) || code[i] == '&')) i++;
            if (i >= code.length()) break;

            if (code.substr(i, 2) == "//") {
                while (i < code.length() && code[i] != '\n') i++;
                continue;
            }

            if (code[i] == '6' || code[i] == '8' || code[i] == '3' || code[i] == '4') {
                size_t colon = code.find(':', i);
                if (colon == std::string::npos) { i++; continue; }

                size_t semi = findTerminator(code, colon + 1);
                if (semi == std::string::npos) {
                    reportError("СИНТАКСИС", "Пропущен ';' для блока", code.substr(i, 10));
                    break;
                }

                std::string header = code.substr(i, colon - i);
                std::string body = code.substr(colon + 1, semi - colon - 1);

                if (header[0] == '6') {
                    std::string cond = header.substr(1);
                    cond.erase(std::remove(cond.begin(), cond.end(), '('), cond.end());
                    cond.erase(std::remove(cond.begin(), cond.end(), ')'), cond.end());
                    while (!checkCondition(cond)) execute(body);
                }
                else if (header[0] == '8') {
                    while (true) execute(body);
                }
                else if (header[0] == '3' || header[0] == '4') {
                    bool cond_res = false;
                    if (header.substr(0, 2) == "34") {
                        if (!branch_taken) cond_res = checkCondition(header.substr(2));
                    } else if (header[0] == '3') {
                        cond_res = checkCondition(header.substr(1));
                        branch_taken = cond_res;
                    } else if (header[0] == '4') {
                        if (!branch_taken) cond_res = true;
                    }

                    if (cond_res) {
                        execute(body);
                        branch_taken = true;
                    }
                }

                i = semi + 1;
            }
            else {
                size_t next_struct = code.find_first_of("6834", i);
                size_t next_sep = code.find_first_of(";&", i);
                size_t limit = std::min({next_struct, next_sep, code.length()});

                runSimpleCmd(code.substr(i, limit - i));
                i = limit;
                if (i < code.length() && (code[i] == ';' || code[i] == '&')) {
                    i++;
                }
                branch_taken = false;
            }
        }
    }
};

int main() {
    #ifdef _WIN32
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= 0x0004;
    SetConsoleMode(hOut, dwMode);
    #endif

    BinaryScriptEngine engine;
    std::cout << "BinaryScript Engine v" VERSION " [RU_ver]" << std::endl;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") break;
        engine.execute(line);
    }
    return 0;
}