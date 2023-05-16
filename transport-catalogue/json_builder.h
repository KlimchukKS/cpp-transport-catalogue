#pragma once

#include "json.h"

namespace json {

    class Builder {
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::string key_;
        bool is_complete = false;

        class BaseContext;
        class DictValueContext;
        class DictItemContext;
        class ArrayItemContext;
        class ValueItemContext;

        class BaseContext {
        public:
            BaseContext(Builder& builder);

            Node Build();

            DictValueContext Key(const std::string& key);

            BaseContext Value(const Node::Value& value);

            DictItemContext StartDict();

            ArrayItemContext StartArray();

            BaseContext EndDict();

            BaseContext EndArray();

        private:
            Builder& builder_;
        };

        class DictValueContext : public BaseContext {
        public:
            DictValueContext(BaseContext base);
            DictItemContext Value(const Node::Value& value);
            Node Build() = delete;
            DictValueContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            BaseContext EndArray() = delete;
        };

        class DictItemContext : public BaseContext {
        public:
            DictItemContext(BaseContext bc);

            ArrayItemContext StartArray() = delete;
            DictItemContext StartDict() = delete;
            BaseContext EndArray() = delete;
            BaseContext Value(Node::Value value) = delete;
            Node& Build() = delete;
        };

        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(BaseContext bc);
            ArrayItemContext Value(const Node::Value& value);
            Node Build() = delete;
            DictValueContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
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
        ValueItemContext Value(const Node::Value& value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        BaseContext EndArray();

        BaseContext EndDict();

        DictValueContext Key(const std::string& str);

        Node& Build();
    };
}
