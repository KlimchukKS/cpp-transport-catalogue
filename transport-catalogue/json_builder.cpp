#include "json_builder.h"

namespace json {

    Builder::BaseContext::BaseContext(Builder& b)
            : builder_(b) {}

    Builder::KeyItemContext Builder::BaseContext::Key(const std::string& key) {
        return BaseContext{builder_.Key(key)};
    }

    Builder::ArrayItemContext Builder::BaseContext::StartArray() {
        return BaseContext{builder_.StartArray()};
    }

    Builder::DictItemContext Builder::BaseContext::StartDict() {
        return BaseContext{builder_.StartDict()};
    }

    Builder::BaseContext Builder::BaseContext::EndDict() {
        return BaseContext{builder_.EndDict()};
    }

    Builder::BaseContext Builder::BaseContext::EndArray() {
        return BaseContext{builder_.EndArray()};
    }

    Builder::BaseContext Builder::BaseContext::Value(const Node::Value& value) {
        return BaseContext{builder_.Value(value)};
    }

    Node& Builder::BaseContext::Build() {
        return builder_.Build();
    }

    Builder::DictItemContext::DictItemContext(Builder::BaseContext bc)
            : BaseContext(bc) {}

    Builder::ArrayItemContext::ArrayItemContext(Builder::BaseContext bc)
            : BaseContext(bc) {}

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(const Node::Value& value) {
        return Builder::BaseContext::Value(value);
    }

    Builder::KeyItemContext::KeyItemContext(Builder::BaseContext bc)
            : BaseContext(bc) {}

    Builder::DictValueItemContext Builder::KeyItemContext::Value(const Node::Value& value) {
        return BaseContext::Value(value);
    }

    Builder::DictValueItemContext::DictValueItemContext(Builder::BaseContext bc)
            : BaseContext(bc) {}

    Builder::ValueItemContext::ValueItemContext(Builder::BaseContext bc)
            : BaseContext(bc) {}

    Builder::BaseContext Builder::Value(const Node::Value& value) {
        if (is_complite) {
            throw std::logic_error("");
        }

        if (nodes_stack_.empty()) {
            Node::Value& ref_root = const_cast<Node::Value&>(root_.GetValue());
            ref_root = value;
            is_complite = true;
            return BaseContext(*this);
        }
        if (nodes_stack_.back()->IsArray()) {
            Node tmp;
            tmp.GetValue() = value;
            Array& ref_arr = const_cast<Array&>(nodes_stack_.back()->AsArray());
            ref_arr.push_back(std::move(tmp));
            return BaseContext(*this);
        }
        if ((nodes_stack_.size() > 1) && (nodes_stack_[nodes_stack_.size() - 2]->IsDict())) {
            Node::Value& ref_root = const_cast<Node::Value&>(nodes_stack_.back()->GetValue());
            ref_root = value;
            nodes_stack_.pop_back();
            return BaseContext(*this);
        }

        throw std::logic_error("");
    }

    Builder::DictItemContext Builder::StartDict() {
        if (nodes_stack_.empty()) {
            Node::Value& ref_root = const_cast<Node::Value&>(root_.GetValue());
            ref_root = std::move(Dict{});
            nodes_stack_.push_back(&root_);
            return DictItemContext(*this);
        }

        if ((nodes_stack_.size() > 1) && ((nodes_stack_[nodes_stack_.size() - 2]->IsDict()))
            && !(nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsDict())) {
            Node::Value& ref_root = const_cast<Node::Value&>(nodes_stack_.back()->GetValue());
            ref_root = Dict{};

            return DictItemContext(*this);
        }

        if (nodes_stack_.back()->IsArray()) {
            Node* tmp = new Node;
            tmp->GetValue() = Dict{};
            nodes_stack_.push_back(tmp);
            return DictItemContext(*this);
        }

        throw std::logic_error("");
    }

    Builder::ArrayItemContext Builder::StartArray() {
        if (nodes_stack_.empty()) {
            Node::Value& ref_root = const_cast<Node::Value&>(root_.GetValue());
            ref_root = std::move(Array{});
            nodes_stack_.push_back(&root_);
            return ArrayItemContext(*this);
        }
        if (nodes_stack_.size() > 1 && (nodes_stack_[nodes_stack_.size() - 2]->IsDict())) {
            Node::Value& ref_root = const_cast<Node::Value&>(nodes_stack_.back()->GetValue());
            ref_root = Array{};
            return ArrayItemContext(*this);
        }

        if (nodes_stack_.back()->IsArray()) {
            Node* tmp = new Node;
            tmp->GetValue() = Array{};
            nodes_stack_.push_back(tmp);
            return ArrayItemContext(*this);
        }

        throw std::logic_error("");
    }

    Builder& Builder::EndArray() {
        if (is_complite) {
            throw std::logic_error("");
        }

        if (!nodes_stack_.back()->IsArray()) {
            throw std::logic_error("");
        }
        if ((nodes_stack_.size() > 1) && (nodes_stack_[nodes_stack_.size() - 2]->IsArray())) {
            Array& ref_arr = const_cast<Array&>(nodes_stack_[nodes_stack_.size() - 2]->AsArray());
            Node* tmp = nodes_stack_.back();
            ref_arr.push_back(std::move(*tmp));
        }

        nodes_stack_.pop_back();
        if (nodes_stack_.empty()) {
            is_complite = true;
        }

        return *this;
    }

    Builder& Builder::EndDict() {
        if (is_complite) {
            throw std::logic_error("");
        }

        if (!nodes_stack_.back()->IsDict()) {
            throw std::logic_error("");
        }
        // Если перед таблицей на стеке есть массив, то таблицу добавляем в массив
        if ((nodes_stack_.size() > 1) && (nodes_stack_[nodes_stack_.size() - 2]->IsArray())) {
            Array& ref = const_cast<Array&>(nodes_stack_[nodes_stack_.size() - 2]->AsArray());
            Node* tmp = nodes_stack_.back();
            ref.push_back(std::move(*tmp));
        }

        nodes_stack_.pop_back();
        if (nodes_stack_.empty()) {
            is_complite = true;
        }

        return *this;
    }

    Builder::KeyItemContext Builder::Key(const std::string& str) {
        if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || !key_.empty()){
            throw std::logic_error("");
        }

        key_ = str;

        Dict& ref_arr = const_cast<Dict&>(nodes_stack_.back()->AsDict());
        nodes_stack_.push_back(&ref_arr[std::move(key_)]);

        return KeyItemContext(*this);
    }

    Node& Builder::Build() {
        if (!nodes_stack_.empty() || !is_complite) {
            throw std::logic_error("");
        }

        return root_;
    }
}
