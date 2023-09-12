#include "json_builder.h"

namespace json {
	class ArrayContext;
	class DictValueContext;
	class DictKeyContext;
	class BaseContext;


	Builder& Builder::Key(std::string key) {
		if (BuildingDict()) {
			last_key_ = key;
			return *this;
		}
		else if (LastElementIsKey()) {
			throw std::logic_error("Can not insert a key after a key");
		}
		else {
			throw std::logic_error("Can not insert key outside of a dict");
		}
	}

	DictKeyContext Builder::StartDict() {
		if (IsEmpty()) {
			root_ = Dict{};
			nodes_stack_.push_back(&root_);
			is_empty = false;
		}
		else if (BuildingArray()) {
			nodes_stack_.back()->AsArray().push_back(Dict{});
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if (LastElementIsKey()) {
			nodes_stack_.back()->AsDict().emplace(last_key_, Dict{});
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[last_key_]);
			last_key_ = "this is not a key lol";;
		}
		else {
			throw std::logic_error("Can not build a dict in non-empty builder or not in an array or not as a dict value");
		}
		return DictKeyContext(*this);
	}

	ArrayContext Builder::StartArray() {
		if (IsEmpty()) {
			root_ = Array{};
			nodes_stack_.push_back(&root_);
			is_empty = false;
		}
		else if (BuildingArray()) {
			nodes_stack_.back()->AsArray().push_back(Array{});
			nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
		}
		else if (LastElementIsKey()) {
			nodes_stack_.back()->AsDict().emplace(last_key_, Array{});
			nodes_stack_.push_back(&nodes_stack_.back()->AsDict()[last_key_]);
			last_key_ = "this is not a key lol";;
		}
		else {
			throw std::logic_error("Can not build an array in non-empty builder or not in an array or not as a dict value");
		}
		return ArrayContext(*this);
	}

	Builder& Builder::EndDict() {
		if (LastElementIsKey()) {
			throw std::logic_error("Dict end error: there is a key wiwthout a value");
		}
		if (BuildingDict()) {
			nodes_stack_.pop_back();
			return *this;
		}
		else {
			throw std::logic_error("Can not end a dict while not in a dict");
		}
	}

	Builder& Builder::EndArray() {
		if (BuildingArray()) {
			nodes_stack_.pop_back();
			return *this;
		}
		else {
			throw std::logic_error("Can not end an array while not in an array");
		}
	}

	Node Builder::Build() {
		if (nodes_stack_.empty() && !is_empty) {
			return root_;
		}
		if (IsEmpty()) {
			throw std::logic_error("Can not build an empty object");
		}
		throw std::logic_error("Can not build an unfinished object");
	}


	bool Builder::IsEmpty() const {
		return nodes_stack_.empty() && root_.IsNull();
	}

	bool Builder::IsFinished() const {
		return !root_.IsNull() && nodes_stack_.empty();
	}

	bool Builder::BuildingArray() const {
		if (!nodes_stack_.empty()) {
			return !root_.IsNull() && nodes_stack_.back()->IsArray();
		}
		return false;
	}

	bool Builder::BuildingDict() const {
		if (!nodes_stack_.empty()) {
			return !root_.IsNull() && nodes_stack_.back()->IsDict();
		}
		return false;
	}

	bool Builder::LastElementIsKey() const {
		return BuildingDict() && (last_key_ != "this is not a key lol");
	}


	/*
	логика - методы билдера выдают один из субклассов контекста, которые имеют строго определенный набор методов,
	уместных в данной ситуации.

	А если несколько вложенных структур - как вернуться в предыдущий контекст при завершении записи? Вернуть BuildContext?
	*/

	DictKeyContext BaseContext::StartDict() {
		builder.StartDict();
		return DictKeyContext(builder);
	}

	ArrayContext BaseContext::StartArray() {
		builder.StartArray();
		return ArrayContext(builder);
	}


	DictKeyContext ArrayContext::StartDict() {
		builder.StartDict();
		return DictKeyContext(builder);
	}

	ArrayContext ArrayContext::StartArray() {
		builder.StartArray();
		return ArrayContext(builder);
	}

	Builder& ArrayContext::EndArray() {
		return builder.EndArray();
	}


	DictKeyContext DictValueContext::StartDict() {
		builder.StartDict();
		return DictKeyContext(builder);
	}

	ArrayContext DictValueContext::StartArray() {
		builder.StartArray();
		return ArrayContext(builder);
	}

	DictValueContext DictKeyContext::Key(std::string key) {
		builder.Key(key);
		return DictValueContext(builder);
	}

	Builder& DictKeyContext::EndDict() {
		return builder.EndDict();
	}

} //namespace json