/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/ast.hpp>


using namespace std;

kh::AstModule::AstModule(const vector<kh::AstImport>& _imports,
                         const vector<kh::AstFunction>& _functions,
                         const vector<kh::AstUserType>& _user_types,
                         const vector<kh::AstEnumType>& _enums,
                         const vector<kh::AstDeclaration>& _variables)
    : variables(_variables), imports(_imports), functions(_functions), user_types(_user_types),
      enums(_enums) {}

kh::AstImport::AstImport(size_t _index, const vector<string>& _path, bool _is_include,
                         bool _is_relative, const string& _identifier)
    : index(_index), path(_path), is_include(_is_include), is_relative(_is_relative),
      identifier(_identifier) {}

kh::AstUserType::AstUserType(size_t _index, const vector<string>& _identifiers,
                             const shared_ptr<kh::AstIdentifiers>& _base,
                             const vector<string>& _generic_args,
                             const vector<kh::AstDeclaration>& _members,
                             const vector<kh::AstFunction>& _methods, bool _is_class)
    : index(_index), identifiers(_identifiers), base(_base), generic_args(_generic_args),
      members(_members), methods(_methods), is_class(_is_class) {}

kh::AstEnumType::AstEnumType(size_t _index, const vector<string>& _identifiers,
                             const vector<string>& _members, const vector<uint64_t>& _values)
    : index(_index), identifiers(_identifiers), members(_members), values(_values) {}

kh::AstIdentifiers::AstIdentifiers(size_t _index, const vector<string>& _identifiers,
                                   const vector<kh::AstIdentifiers>& _generics,
                                   const vector<size_t>& _generics_refs,
                                   const vector<vector<uint64_t>>& _generics_array)
    : identifiers(_identifiers), generics(_generics), generics_refs(_generics_refs),
      generics_array(_generics_array) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::IDENTIFIER;
}

kh::AstDeclaration::AstDeclaration(size_t _index, const kh::AstIdentifiers& _var_type,
                                   const vector<uint64_t>& _var_array, const string& _var_name,
                                   shared_ptr<kh::AstExpression>& _expression, size_t _refs)
    : var_type(_var_type), var_array(_var_array), var_name(_var_name), expression(_expression),
      refs(_refs) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::DECLARE;
}

kh::AstFunction::AstFunction(size_t _index, const vector<string>& _identifiers,
                             const vector<string>& _generic_args, const vector<uint64_t>& _id_array,
                             const vector<uint64_t>& _return_array,
                             const kh::AstIdentifiers& _return_type, size_t _return_refs,
                             const vector<kh::AstDeclaration>& _arguments,
                             const vector<shared_ptr<kh::AstBody>>& _body, bool _is_conditional)
    : identifiers(_identifiers), generic_args(_generic_args), id_array(_id_array),
      return_array(_return_array), return_type(_return_type), return_refs(_return_refs),
      arguments(_arguments), body(_body), is_conditional(_is_conditional) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::FUNCTION;
}

kh::AstUnaryOperation::AstUnaryOperation(size_t _index, kh::Operator _operation,
                                         shared_ptr<kh::AstExpression>& _rvalue)
    : operation(_operation), rvalue(_rvalue) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::UNARY;
}

kh::AstRevUnaryOperation::AstRevUnaryOperation(size_t _index, kh::Operator _operation,
                                               shared_ptr<kh::AstExpression>& _rvalue)
    : operation(_operation), rvalue(_rvalue) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::REV_UNARY;
}

kh::AstBinaryOperation::AstBinaryOperation(size_t _index, kh::Operator _operation,
                                           shared_ptr<kh::AstExpression>& _lvalue,
                                           shared_ptr<kh::AstExpression>& _rvalue)
    : operation(_operation), lvalue(_lvalue), rvalue(_rvalue) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::BINARY;
}

kh::AstTernaryOperation::AstTernaryOperation(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                                             shared_ptr<kh::AstExpression>& _value,
                                             shared_ptr<kh::AstExpression>& _otherwise)
    : condition(_condition), value(_value), otherwise(_otherwise) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::TERNARY;
}

kh::AstComparisonExpression::AstComparisonExpression(
    size_t _index, const vector<kh::Operator>& _operations,
    const vector<shared_ptr<kh::AstExpression>>& _values)
    : operations(_operations), values(_values) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::COMPARISON;
}

kh::AstSubscriptExpression::AstSubscriptExpression(
    size_t _index, shared_ptr<kh::AstExpression>& _expression,
    const vector<shared_ptr<kh::AstExpression>>& _arguments)
    : expression(_expression), arguments(_arguments) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::SUBSCRIPT;
}

kh::AstCallExpression::AstCallExpression(size_t _index, shared_ptr<kh::AstExpression>& _expression,
                                         const vector<shared_ptr<kh::AstExpression>>& _arguments)
    : expression(_expression), arguments(_arguments) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CALL;
}

kh::AstScoping::AstScoping(size_t _index, shared_ptr<kh::AstExpression>& _expression,
                           const vector<string>& _identifiers)
    : expression(_expression), identifiers(_identifiers) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::SCOPE;
}

kh::AstValue::AstValue(size_t _index, char32_t _character, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->character = _character;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, uint64_t _uinteger, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->uinteger = _uinteger;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, int64_t _integer, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->integer = _integer;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, double _floating, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->floating = _floating;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, const string& _buffer, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->buffer = _buffer;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, const u32string& _ustring, kh::AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->ustring = _ustring;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::CONSTANT;
}

kh::AstTuple::AstTuple(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _elements)
    : elements(_elements) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::TUPLE;
}

kh::AstList::AstList(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _elements)
    : elements(_elements) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::LIST;
}

kh::AstDict::AstDict(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _keys,
                     const vector<shared_ptr<kh::AstExpression>>& _items)
    : keys(_keys), items(_items) {
    this->index = _index;
    this->type = kh::AstBody::EXPRESSION;
    this->expression_type = kh::AstExpression::DICT;
}

kh::AstIf::AstIf(size_t _index, const vector<shared_ptr<kh::AstExpression>>& _conditions,
                 const vector<vector<shared_ptr<kh::AstBody>>>& _bodies,
                 const vector<shared_ptr<kh::AstBody>>& _else_body)
    : conditions(_conditions), bodies(_bodies), else_body(_else_body) {
    this->index = _index;
    this->type = kh::AstBody::IF;
}

kh::AstWhile::AstWhile(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                       const vector<shared_ptr<kh::AstBody>>& _body)
    : condition(_condition), body(_body) {
    this->index = _index;
    this->type = kh::AstBody::WHILE;
}

kh::AstDoWhile::AstDoWhile(size_t _index, shared_ptr<kh::AstExpression>& _condition,
                           const vector<shared_ptr<kh::AstBody>>& _body)
    : condition(_condition), body(_body) {
    this->index = _index;
    this->type = kh::AstBody::DO_WHILE;
}

kh::AstFor::AstFor(size_t _index, shared_ptr<kh::AstExpression>& _initialize,
                   shared_ptr<kh::AstExpression>& _condition, shared_ptr<kh::AstExpression>& _step,
                   const vector<shared_ptr<kh::AstBody>>& _body)
    : initialize(_initialize), condition(_condition), step(_step), body(_body) {
    this->index = _index;
    this->type = kh::AstBody::FOR;
}

kh::AstForEach::AstForEach(size_t _index, shared_ptr<kh::AstExpression>& _target,
                           shared_ptr<kh::AstExpression>& _iterator,
                           const vector<shared_ptr<kh::AstBody>>& _body)
    : target(_target), iterator(_iterator), body(_body) {
    this->index = _index;
    this->type = kh::AstBody::FOREACH;
}

kh::AstStatement::AstStatement(size_t _index, kh::AstStatement::Type _statement_type,
                               shared_ptr<kh::AstExpression>& _expression)
    : statement_type((Type)((size_t)_statement_type)), expression(_expression) {
    this->index = _index;
    this->type = kh::AstBody::STATEMENT;
}

kh::AstStatement::AstStatement(size_t _index, kh::AstStatement::Type _statement_type,
                               size_t _loop_count)
    : statement_type((Type)((size_t)_statement_type)), loop_count(_loop_count) {
    this->index = _index;
    this->type = kh::AstBody::STATEMENT;
}
