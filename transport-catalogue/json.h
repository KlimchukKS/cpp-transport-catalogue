#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    inline const std::string kNodeValueNull{"null"};
    
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        using variant::variant;

        const Value& GetValue() const;

        bool IsInt() const;
        bool IsDouble() const;          // Возвращает true, если в Node хранится int либо double.
        bool IsPureDouble() const;      // Возвращает true, если в Node хранится double.
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        Value value_;
    };

    bool operator==(const Node& lhs, const Node& rhs);

    bool operator!=(const Node& lhs, const Node& rhs);

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        out << value;
    }
    void PrintValue(const std::string& value, std::ostream& out);
    void PrintValue(std::nullptr_t, std::ostream& out);
    void PrintValue(bool value, std::ostream& out);
    void PrintValue(const Array& values, std::ostream& out);
    void PrintValue(const Dict&, std::ostream& out);

    void PrintNode(const Node& node, std::ostream& out);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
