#include "json.h"

#include <sstream>

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;

            for (char c; input >> c;) {
                if (c == ']') {
                    return Node(move(result));
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            throw ParsingError("String parsing error");
        }

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            } else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return Node(std::stoi(parsed_num));
                    } catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return Node(std::stod(parsed_num));
            } catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        Node LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                } else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                        case 'n':
                            s.push_back('\n');
                            break;
                        case 't':
                            s.push_back('\t');
                            break;
                        case 'r':
                            s.push_back('\r');
                            break;
                        case '"':
                            s.push_back('"');
                            break;
                        case '\\':
                            s.push_back('\\');
                            break;
                        default:
                            // Встретили неизвестную escape-последовательность
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                } else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                } else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return Node(move(s));
        }

        Node LoadDict(istream& input) {
            Dict result;

            for (char c; input >> c;) {
                if (c == '}') {
                    return Node(move(result));
                }
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({move(key), LoadNode(input)});
            }
            throw ParsingError("String parsing error");
        }

        Node LoadNULL(istream& input) {
            std::string result;

            for (char c; input >> c;) {
                result.push_back(c);
                if (c == 'l' && result.size() == 4 && result == kNodeValueNull) {
                    return Node(nullptr);
                }
            }
            throw ParsingError("String parsing error");
        }

        Node LoadBool(istream& input) {
            std::string result;

            for (char c; input >> c;) {
                result.push_back(c);
                if (c == 'e') {
                    if (result == "true"s) {
                        return Node(true);
                    } else if (result == "false"s) {
                        return Node(false);
                    }
                    throw ParsingError("String parsing error");
                }
            }
            throw ParsingError("String parsing error");
        }

        Node LoadNode(istream& input) {
            char c;

            input >> c;

            switch (c) {
                case '[':
                    return LoadArray(input);
                case '{':
                    return LoadDict(input);
                case '"':
                    return LoadString(input);
                case 'n':
                    input.putback(c);
                    return LoadNULL(input);
                case 't':
                    input.putback(c);
                    return LoadBool(input);
                case 'f':
                    input.putback(c);
                    return LoadBool(input);
                default:
                    if (isdigit(c) || c == '-') {
                        input.putback(c);
                        return LoadNumber(input);
                    }
                    throw ParsingError("Syntax error"s);
            }
        }

    }  // namespace

// ------------------ Node ------------------
    const Node::Value& Node::GetValue() const { return *this; }

    bool Node::IsInt() const        { return std::holds_alternative<int>(*this); }
    bool Node::IsPureDouble() const { return holds_alternative<double>(*this); }
    bool Node::IsDouble() const     { return IsInt() || IsPureDouble(); }
    bool Node::IsBool() const       { return holds_alternative<bool>(*this); }
    bool Node::IsString() const     { return holds_alternative<std::string>(*this); }
    bool Node::IsNull() const       { return holds_alternative<std::nullptr_t>(*this); }
    bool Node::IsArray() const      { return holds_alternative<Array>(*this); }
    bool Node::IsMap() const        { return holds_alternative<Dict>(*this); }

    int Node::AsInt() const {
        if (!IsInt()) {
            throw std::logic_error("value_ is not int");
        }
        return std::get<int>(*this);
    }
    bool Node::AsBool() const {
        if (!IsBool()) {
            throw std::logic_error("value_ is not bool");
        }
        return std::get<bool>(*this);
    }
    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw std::logic_error("value_ is not array");
        }
        return std::get<Array>(*this);
    }
    const Dict& Node::AsMap() const {
        if (!IsMap()) {
            throw std::logic_error("value_ is not bool");
        }
        return std::get<Dict>(*this);
    }
    const std::string& Node::AsString() const {
        if (!IsString()) {
            throw std::logic_error("value_ is not string");
        }
        return std::get<std::string>(*this);
    }
    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw std::logic_error("value_ is not double");
        }
        return (IsPureDouble()) ? std::get<double>(*this) : static_cast<double>(std::get<int>(*this));
    }

// ---- операторы сравнения для Node --------

    bool operator==(const Node& lhs, const Node& rhs) {
        bool tmp = (lhs.IsPureDouble() && rhs.IsPureDouble());
        return (lhs.IsInt() && rhs.IsInt() && (lhs.AsInt() == rhs.AsInt()))
               || (tmp && (lhs.AsDouble() == rhs.AsDouble()))
               || (lhs.IsBool() && rhs.IsBool() && (lhs.AsBool() == rhs.AsBool()))
               || (lhs.IsString() && rhs.IsString() && (lhs.AsString() == rhs.AsString()))
               || (lhs.IsArray() && rhs.IsArray() && (lhs.AsArray() == rhs.AsArray()))
               || (lhs.IsMap() && rhs.IsMap() && (lhs.AsMap() == rhs.AsMap()))
               || (lhs.IsNull() && rhs.IsNull());
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }

// ------------------------------------------

    bool operator==(const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document::Document(Node root)
            : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{LoadNode(input)};
    }
    // Вывод строки
    void PrintValue(const std::string& value, std::ostream& out) {
        out << "\""s;
        // "\r\n\t\"\\"
        for(char ch : value) {
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (ch) {
                case '\n':
                    out << "\\n"s;
                    break;
                case '\t':
                    out << "\t"s;
                    break;
                case '\r':
                    out << "\\r"s;
                    break;
                case '"':
                    out << "\\"s << ch;
                    break;
                case '\\':
                    out << "\\\\"s;
                    break;
                default:
                    out << ch;
            }
        }

        out << "\""s;
    }
    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out) {
        out << "null"sv;
    }
    // Вывод булевое значение
    void PrintValue(bool value, std::ostream& out) {
        out << ((value) ? "true"s : "false"s);
    }
    void PrintValue(const Array& values, std::ostream& out) {
        out << "["s;

        if (values.size()) {
            auto it_begin = values.cbegin();

            while (it_begin != values.end() - 1) {
                PrintNode(*it_begin, out);
                out << ","s;
                ++it_begin;
            }
            PrintNode(*it_begin, out);
        }

        out << "]";
    }
    void PrintValue(const Dict& values, std::ostream& out) {
        out << "{"s;
        auto it_begin = values.cbegin();

        for (size_t i = 0; i < values.size() - 1; ++i, ++it_begin) {
            PrintNode(it_begin->first, out);
            out << ":"s;
            PrintNode(it_begin->second, out);
            out << ","s;
        }
        PrintNode(it_begin->first, out);
        out << ":"s;
        PrintNode(it_begin->second, out);

        out << "}";
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
                [&out](const auto& value){ PrintValue(value, out); },
                node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }

}  // namespace json
