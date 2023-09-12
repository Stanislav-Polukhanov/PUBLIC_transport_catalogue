#pragma once

#include "json.h"

namespace json {
	class ArrayContext;
	class DictValueContext;
	class DictKeyContext;
	class BaseContext;


	class Builder {

	public:
		Builder& Key(std::string key);

		template <typename Value_type>
		Builder& Value(Value_type value) {
			if (IsFinished()) {
				throw std::logic_error("Can not modify finished object");
			}
			if (IsEmpty()) {
				root_ = value;
				is_empty = false;
				return *this;
			}
			else if (BuildingArray()) {
				nodes_stack_.back()->AsArray().push_back(value);
				return *this;
			}
			else if (LastElementIsKey()) {
				nodes_stack_.back()->AsDict().emplace(last_key_, value);
				last_key_ = "this is not a key lol";;
				return *this;
			}

			throw std::logic_error("Value write error: not empty, not in an array, not a val for a key");
		}

		DictKeyContext StartDict();

		ArrayContext StartArray();

		Builder& EndDict();

		Builder& EndArray();

		Node Build();


		bool IsEmpty() const;

		bool IsFinished() const;

		bool BuildingArray() const;

		bool BuildingDict() const;

		bool LastElementIsKey() const;

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;

		std::string last_key_ = "this is not a key lol";
		bool is_empty = true;
	};


	/*
	логика - методы билдера выдают один из субклассов контекста, которые имеют строго определенный набор методов,
	уместных в данной ситуации.
	*/

	class BaseContext {
	public:
		BaseContext(Builder& builder)
			: builder(builder)
		{}

		DictKeyContext StartDict();

		ArrayContext StartArray();

		Builder& builder;
	};

	//За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray
	class ArrayContext : public BaseContext {
	public:
		ArrayContext(Builder& builder)
			: BaseContext(builder)
		{}

		template <typename Value_type>
		ArrayContext Value(Value_type value);

		DictKeyContext StartDict();

		ArrayContext StartArray();

		Builder& EndArray();
	};

	//Непосредственно после Key вызван не Value, не StartDict и не StartArray
	class DictValueContext : public BaseContext {
	public:
		DictValueContext(Builder& builder)
			: BaseContext(builder)
		{}

		template <typename Value_type>
		DictKeyContext Value(Value_type value);

		DictKeyContext StartDict();

		ArrayContext StartArray();
	};

	//За вызовом StartDict следует не Key и не EndDict
	class DictKeyContext : public BaseContext{
	public:
		DictKeyContext(Builder& builder)
			: BaseContext(builder)
		{}

		DictValueContext Key(std::string key);

		Builder& EndDict();

		DictKeyContext StartDict() = delete;

		ArrayContext StartArray() = delete;
	};

	template <typename Value_type>
	ArrayContext ArrayContext::Value(Value_type value) {
		builder.Value(value);
		return ArrayContext(builder);
	}

	template <typename Value_type>
	DictKeyContext DictValueContext::Value(Value_type value) {
		builder.Value(value);
		return DictKeyContext(builder);
	}


} //namespace json