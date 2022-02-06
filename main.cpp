#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <functional>
#include <map>
#include <variant>
#include <typeinfo>
#include <cmath>
#include <tuple>
#include <list>

using stackType = std::variant<double, std::string, bool>;

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::vector<std::string> tokStream;
std::stack<stackType> dataStack;

bool compileMode;
bool compileImmediate;
std::string funcName;
std::list<std::variant<double, std::string, bool, std::function<void()> *>> funcAccumulator;

std::string get_next() {
    std::string t = tokStream[0];
    tokStream.erase(tokStream.begin());
    return t;
}

std::map<std::string, std::tuple<std::function<void()>, bool>> dict = {
        {"DUP", {[]() -> void {
            dataStack.push(dataStack.top());
        }, false}},
        {"DROP", {[]() -> void {
            dataStack.pop();
        }, false}},
        {"SWAP", {[] {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(a);
            dataStack.push(b);
        }, false}},
        {"+", {[]() -> void {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(std::visit(overloaded {
                    [](double x, double y) -> stackType {
                        return x + y;
                    },
                    [](std::string x, std::string y) -> stackType {
                        return x + y;
                    },
                    [](auto, auto) -> stackType {
                        throw std::exception();
                    }
            }, a, b));
        }, false}},
        {"-", {[]() -> void {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(std::visit(overloaded {
                    [](double x, double y) -> stackType {
                        return x - y;
                    },
                    [](auto, auto) -> stackType {
                        throw std::exception();
                    }
            }, a, b));
        }, false}},
        {"*", {[]() -> void {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(std::visit(overloaded {
                    [](double x, double y) -> stackType {
                        return x * y;
                    },
                    [](auto, auto) -> stackType {
                        throw std::exception();
                    }
            }, a, b));
        }, false}},
        {"/", {[]() -> void {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(std::visit(overloaded {
                    [](double x, double y) -> stackType {
                        return x / y;
                    },
                    [](auto, auto) -> stackType {
                        throw std::exception();
                    }
            }, a, b));
        }, false}},
        {"%", {[]() -> void {
            auto a = dataStack.top();
            dataStack.pop();
            auto b = dataStack.top();
            dataStack.pop();
            dataStack.push(std::visit(overloaded {
                    [](double x, double y) -> stackType {
                        return fmod(x, y);
                    },
                    [](auto, auto) -> stackType {
                        throw std::exception();
                    }
            }, a, b));
        }, false}},
        {":", {[]() -> void {
            funcName = get_next();
            compileMode = true;
            compileImmediate = false;
        }, true}},
        {":i", {[]() -> void {
            funcName = get_next();
            compileMode = true;
            compileImmediate = true;
        }, true}},
        {";", {[]() -> void {
            compileMode = false;
            compileImmediate = false;
            auto fbody = funcAccumulator;
            dict[funcName] = {[=]() -> void {
                for(std::variant<double, std::string, bool, std::function<void()> *> x : fbody) {
                    std::visit(overloaded {
                        [](double z) {
                            dataStack.push(z);
                        },
                        [](std::string z) {
                            dataStack.push(z);
                        },
                        [](bool z) {
                            dataStack.push(z);
                        },
                        [](std::function<void()>* z) {
                            (*z)();
                        }
                    }, x);
                }
            }, compileImmediate};
            funcAccumulator.erase(funcAccumulator.begin(), funcAccumulator.end());
        }, true}},
        {"IF", {[]() -> void {

        }, false}}
};

void exec(std::string fragment) {
    try {
        if(!compileMode)
            dataStack.push(std::stod(fragment));
        else
            funcAccumulator.insert(funcAccumulator.end(), std::stod(fragment));
    } catch(std::exception &e) {
        if(fragment == "true")
            if(!compileMode)
                dataStack.push(true);
            else
                funcAccumulator.insert(funcAccumulator.end(), true);
        else if(fragment == "false")
            if(!compileMode)
                dataStack.push(false);
            else
                funcAccumulator.insert(funcAccumulator.end(), false);
        else
            if(std::get<1>(dict[fragment]))
                std::get<0>(dict[fragment])();
            else
                if(compileMode)
                    funcAccumulator.insert(funcAccumulator.end(), &std::get<0>(dict[fragment]));
                else
                    std::get<0>(dict[fragment])();
    }
}

int execFile(std::string filename) {

}


int main(int argc, char** argv) {
    if(argc > 2) {
        execFile(argv[1]);
    }
    return 0;
}
