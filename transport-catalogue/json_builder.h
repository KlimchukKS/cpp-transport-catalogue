#pragma once

#include "json.h"

namespace json {

    class Builder {
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::string key_;
        bool is_complite = false;

        class BaseContext;
        class DictItemContext;
        class ArrayItemContext;
        class KeyItemContext;
        class ValueItemContext;
        class DictValueItemContext;

        class BaseContext {
            Builder& builder_;
        public:
            BaseContext(Builder& b);

            KeyItemContext Key(const std::string& key);

            ArrayItemContext StartArray();

            DictItemContext StartDict();

            BaseContext EndDict();

            BaseContext EndArray();

            BaseContext Value(const Node::Value& value);

            Node& Build();
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(BaseContext bc);

            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
            BaseContext EndArray() = delete;
            BaseContext Value(const Node::Value& value) = delete;
            Node& Build() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext bc);

            ArrayItemContext Value(const Node::Value& value);

            Node& Build() = delete;
            DictItemContext Key(const std::string& key) = delete;
            DictItemContext EndDict() = delete;
        };

        class KeyItemContext : public BaseContext {
        public:
            KeyItemContext(BaseContext bc);

            DictValueItemContext Value(const Node::Value& value);

            Node& Build() = delete;
            DictItemContext Key(const std::string& key) = delete;
            DictItemContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };

        class DictValueItemContext : public BaseContext {
        public:
            DictValueItemContext(BaseContext bc);

            BaseContext Value(const Node::Value& value) = delete;
            Node& Build() = delete;
            BaseContext EndArray() = delete;
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
        };

        class ValueItemContext : public BaseContext {
        public:
            ValueItemContext(BaseContext bc);

            BaseContext Value(const Node::Value& value) = delete;
            DictItemContext Key(const std::string& key) = delete;
            DictItemContext EndDict() = delete;
            BaseContext EndArray() = delete;
            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
        };

    public:
        BaseContext Value(const Node::Value& value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        Builder& EndArray();

        Builder& EndDict();

        KeyItemContext Key(const std::string& str);

        Node& Build();
    };
}
