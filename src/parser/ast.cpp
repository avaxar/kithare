/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/ast.hpp>


using namespace kh;

kh::AstModule::AstModule(const std::vector<AstImport>& _imports,
                         const std::vector<AstFunction>& _functions,
                         const std::vector<AstUserType>& _user_types,
                         const std::vector<AstEnumType>& _enums,
                         const std::vector<AstDeclaration>& _variables)
    : imports(_imports), functions(_functions), user_types(_user_types), enums(_enums),
      variables(_variables) {}

kh::AstImport::AstImport(size_t _index, const std::vector<std::string>& _path, bool _is_include,
                         bool _is_relative, const std::string& _identifier)
    : index(_index), path(_path), is_include(_is_include), is_relative(_is_relative),
      identifier(_identifier) {}

kh::AstUserType::AstUserType(size_t _index, const std::vector<std::string>& _identifiers,
                             const std::shared_ptr<AstIdentifiers>& _base,
                             const std::vector<std::string>& _generic_args,
                             const std::vector<AstDeclaration>& _members,
                             const std::vector<AstFunction>& _methods, bool _is_class)
    : index(_index), identifiers(_identifiers), base(_base), generic_args(_generic_args),
      members(_members), methods(_methods), is_class(_is_class) {}

kh::AstEnumType::AstEnumType(size_t _index, const std::vector<std::string>& _identifiers,
                             const std::vector<std::string>& _members,
                             const std::vector<uint64_t>& _values)
    : index(_index), identifiers(_identifiers), members(_members), values(_values) {}

kh::AstIdentifiers::AstIdentifiers(size_t _index, const std::vector<std::string>& _identifiers,
                                   const std::vector<AstIdentifiers>& _generics,
                                   const std::vector<size_t>& _generics_refs,
                                   const std::vector<std::vector<uint64_t>>& _generics_array)
    : identifiers(_identifiers), generics(_generics), generics_refs(_generics_refs),
      generics_array(_generics_array) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::IDENTIFIER;
}

kh::AstDeclaration::AstDeclaration(size_t _index, const AstIdentifiers& _var_type,
                                   const std::vector<uint64_t>& _var_array,
                                   const std::string& _var_name,
                                   std::shared_ptr<AstExpression>& _expression, size_t _refs)
    : var_type(_var_type), var_array(_var_array), var_name(_var_name), expression(_expression),
      refs(_refs) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::DECLARE;
}

kh::AstFunction::AstFunction(size_t _index, const std::vector<std::string>& _identifiers,
                             const std::vector<std::string>& _generic_args,
                             const std::vector<uint64_t>& _id_array, const AstIdentifiers& _return_type,
                             const std::vector<uint64_t>& _return_array, size_t _return_refs,
                             const std::vector<AstDeclaration>& _arguments,
                             const std::vector<std::shared_ptr<AstBody>>& _body, bool _is_conditional)
    : identifiers(_identifiers), generic_args(_generic_args), id_array(_id_array),
      return_type(_return_type), return_array(_return_array), return_refs(_return_refs),
      arguments(_arguments), body(_body), is_conditional(_is_conditional) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::FUNCTION;
}

kh::AstUnaryOperation::AstUnaryOperation(size_t _index, Operator _operation,
                                         std::shared_ptr<AstExpression>& _rvalue)
    : operation(_operation), rvalue(_rvalue) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::UNARY;
}

kh::AstRevUnaryOperation::AstRevUnaryOperation(size_t _index, Operator _operation,
                                               std::shared_ptr<AstExpression>& _rvalue)
    : operation(_operation), rvalue(_rvalue) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::REV_UNARY;
}

kh::AstBinaryOperation::AstBinaryOperation(size_t _index, Operator _operation,
                                           std::shared_ptr<AstExpression>& _lvalue,
                                           std::shared_ptr<AstExpression>& _rvalue)
    : operation(_operation), lvalue(_lvalue), rvalue(_rvalue) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::BINARY;
}

kh::AstTernaryOperation::AstTernaryOperation(size_t _index, std::shared_ptr<AstExpression>& _condition,
                                             std::shared_ptr<AstExpression>& _value,
                                             std::shared_ptr<AstExpression>& _otherwise)
    : condition(_condition), value(_value), otherwise(_otherwise) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::TERNARY;
}

kh::AstComparisonExpression::AstComparisonExpression(
    size_t _index, const std::vector<Operator>& _operations,
    const std::vector<std::shared_ptr<AstExpression>>& _values)
    : operations(_operations), values(_values) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::COMPARISON;
}

kh::AstSubscriptExpression::AstSubscriptExpression(
    size_t _index, std::shared_ptr<AstExpression>& _expression,
    const std::vector<std::shared_ptr<AstExpression>>& _arguments)
    : expression(_expression), arguments(_arguments) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::SUBSCRIPT;
}

kh::AstCallExpression::AstCallExpression(size_t _index, std::shared_ptr<AstExpression>& _expression,
                                         const std::vector<std::shared_ptr<AstExpression>>& _arguments)
    : expression(_expression), arguments(_arguments) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CALL;
}

kh::AstScoping::AstScoping(size_t _index, std::shared_ptr<AstExpression>& _expression,
                           const std::vector<std::string>& _identifiers)
    : expression(_expression), identifiers(_identifiers) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::SCOPE;
}

kh::AstValue::AstValue(size_t _index, char32_t _character, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->character = _character;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, uint64_t _uinteger, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->uinteger = _uinteger;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, int64_t _integer, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->integer = _integer;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, double _floating, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->floating = _floating;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, const std::string& _buffer, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->buffer = _buffer;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstValue::AstValue(size_t _index, const std::u32string& _string, AstValue::ValueType _value_type)
    : value_type((ValueType)((size_t)_value_type)) {
    this->index = _index;
    this->string = _string;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::CONSTANT;
}

kh::AstTuple::AstTuple(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _elements)
    : elements(_elements) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::TUPLE;
}

kh::AstList::AstList(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _elements)
    : elements(_elements) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::LIST;
}

kh::AstDict::AstDict(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _keys,
                     const std::vector<std::shared_ptr<AstExpression>>& _items)
    : keys(_keys), items(_items) {
    this->index = _index;
    this->type = AstBody::EXPRESSION;
    this->expression_type = AstExpression::DICT;
}

kh::AstIf::AstIf(size_t _index, const std::vector<std::shared_ptr<AstExpression>>& _conditions,
                 const std::vector<std::vector<std::shared_ptr<AstBody>>>& _bodies,
                 const std::vector<std::shared_ptr<AstBody>>& _else_body)
    : conditions(_conditions), bodies(_bodies), else_body(_else_body) {
    this->index = _index;
    this->type = AstBody::IF;
}

kh::AstWhile::AstWhile(size_t _index, std::shared_ptr<AstExpression>& _condition,
                       const std::vector<std::shared_ptr<AstBody>>& _body)
    : condition(_condition), body(_body) {
    this->index = _index;
    this->type = AstBody::WHILE;
}

kh::AstDoWhile::AstDoWhile(size_t _index, std::shared_ptr<AstExpression>& _condition,
                           const std::vector<std::shared_ptr<AstBody>>& _body)
    : condition(_condition), body(_body) {
    this->index = _index;
    this->type = AstBody::DO_WHILE;
}

kh::AstFor::AstFor(size_t _index, std::shared_ptr<AstExpression>& _initialize,
                   std::shared_ptr<AstExpression>& _condition, std::shared_ptr<AstExpression>& _step,
                   const std::vector<std::shared_ptr<AstBody>>& _body)
    : initialize(_initialize), condition(_condition), step(_step), body(_body) {
    this->index = _index;
    this->type = AstBody::FOR;
}

kh::AstForEach::AstForEach(size_t _index, std::shared_ptr<AstExpression>& _target,
                           std::shared_ptr<AstExpression>& _iterator,
                           const std::vector<std::shared_ptr<AstBody>>& _body)
    : target(_target), iterator(_iterator), body(_body) {
    this->index = _index;
    this->type = AstBody::FOREACH;
}

kh::AstStatement::AstStatement(size_t _index, AstStatement::Type _statement_type,
                               std::shared_ptr<AstExpression>& _expression)
    : statement_type((Type)((size_t)_statement_type)), expression(_expression) {
    this->index = _index;
    this->type = AstBody::STATEMENT;
}

kh::AstStatement::AstStatement(size_t _index, AstStatement::Type _statement_type, size_t _loop_count)
    : statement_type((Type)((size_t)_statement_type)), loop_count(_loop_count) {
    this->index = _index;
    this->type = AstBody::STATEMENT;
}
