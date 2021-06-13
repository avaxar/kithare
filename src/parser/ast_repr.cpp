/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/ast.hpp>


#define REPR_ALL_IN(var)  \
    for (auto _var : var) \
        if (_var)         \
    str += U"\n\t" + ind + kh::repr(*_var, indent + 1)


std::u32string kh::repr(const kh::Ast& module_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"ast:";

    REPR_ALL_IN(module_ast.imports);
    REPR_ALL_IN(module_ast.functions);
    REPR_ALL_IN(module_ast.user_types);
    REPR_ALL_IN(module_ast.enums);
    REPR_ALL_IN(module_ast.variables);
    return str;
}

std::u32string kh::repr(const kh::AstImport& import_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = import_ast.is_include ? U"include:" : U"import:";
    str += U"\n\t" + ind + U"type: " + (import_ast.is_relative ? U"relative" : U"absolute");

    str += U"\n\t" + ind + U"path: ";
    for (const std::u32string& dir : import_ast.path)
        str += dir + (&dir == &import_ast.path.back() ? U"" : U".");

    if (!import_ast.is_include)
        str += U"\n\t" + ind + U"identifier: " + import_ast.identifier;

    return str;
}

std::u32string kh::repr(const kh::AstUserType& type_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = (type_ast.is_class ? U"class:\n\t" : U"struct:\n\t") + ind + U"name: ";
    for (const std::u32string& identifier : type_ast.identifiers)
        str += identifier + (&identifier == &type_ast.identifiers.back() ? U"" : U".");

    if (!type_ast.bases.empty()) {
        str += U"\n\t" + ind + U"base(s):";
        for (const std::shared_ptr<kh::AstIdentifierExpression>& base : type_ast.bases)
            if (base)
                str += U"\n\t\t" + ind + kh::repr(*base, indent + 3);
    }

    if (!type_ast.generic_args.empty()) {
        str += U"\n\t" + ind + U"generic argument(s): ";
        for (const std::u32string& generic_ : type_ast.generic_args)
            str += generic_ + (&generic_ == &type_ast.generic_args.back() ? U"" : U", ");
    }

    str += U"\n\t" + ind + U"member(s):";
    for (auto member : type_ast.members)
        if (member)
            str += U"\n\t\t" + ind + kh::repr(*member, indent + 2);

    if (!type_ast.methods.empty()) {
        str += U"\n\t" + ind + U"method(s):";
        for (auto method : type_ast.methods)
            if (method)
                str += U"\n\t\t" + ind + kh::repr(*method, indent + 2);
    }

    return str;
}

std::u32string kh::repr(const kh::AstEnumType& enum_ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str = U"enum:\n\t" + ind + U"name: ";
    for (const std::u32string& identifier : enum_ast.identifiers)
        str += identifier + (&identifier == &enum_ast.identifiers.back() ? U"" : U".");

    str += U"\n\t" + ind + U"member(s):";
    for (size_t member = 0; member < enum_ast.members.size(); member++)
        str += U"\n\t\t" + ind + enum_ast.members[member] + U": " + kh::repr(enum_ast.values[member]);

    return str;
}

std::u32string kh::repr(const kh::AstBody& ast, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str;

    switch (ast.type) {
        case kh::AstBody::Type::EXPRESSION: {
            str += kh::repr(*(kh::AstExpression*)&ast, indent);
            break;
        }

        case kh::AstBody::Type::IF: {
            const kh::AstIf& ast_if = *(kh::AstIf*)&ast;
            str += U"if:";

            for (size_t clause = 0; clause < ast_if.conditions.size(); clause++) {
                str += U"\n\t" + ind + U"if clause:";

                if (ast_if.conditions[clause])
                    str += U"\n\t\t" + ind + U"condition:\n\t\t\t" + ind +
                           kh::repr(*ast_if.conditions[clause], indent + 3);

                if (!ast_if.bodies[clause].empty()) {
                    str += U"\n\t\t" + ind + U"body:";
                    for (auto part : ast_if.bodies[clause])
                        if (part)
                            str += U"\n\t\t\t" + ind + kh::repr(*part, indent + 3);
                }
            }

            if (!ast_if.else_body.empty()) {
                str += U"\n\t" + ind + U"else body:";
                for (auto part : ast_if.else_body)
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
            }

            break;
        }

        case kh::AstBody::Type::WHILE: {
            const kh::AstWhile& ast_while = *(kh::AstWhile*)&ast;
            str += U"while:";

            if (ast_while.condition) {
                str += U"\n\t" + ind + U"condition:\n\t\t" + ind +
                       kh::repr(*ast_while.condition, indent + 2);
            }

            if (!ast_while.body.empty()) {
                str += U"\n\t" + ind + U"body:";
                for (auto part : ast_while.body)
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
            }

            break;
        }

        case kh::AstBody::Type::DO_WHILE: {
            const kh::AstDoWhile& ast_do_while = *(kh::AstDoWhile*)&ast;
            str += U"do while:";

            if (ast_do_while.condition)
                str += U"\n\t" + ind + U"condition:\n\t\t" + ind +
                       kh::repr(*ast_do_while.condition, indent + 2);

            if (!ast_do_while.body.empty()) {
                str += U"\n\t" + ind + U"body:";
                for (auto part : ast_do_while.body)
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
            }

            break;
        }

        case kh::AstBody::Type::FOR: {
            const kh::AstFor& ast_for = *(kh::AstFor*)&ast;
            str += U"for:";

            if (ast_for.initialize)
                str += U"\n\t" + ind + U"initializer:\n\t\t" + ind +
                       kh::repr(*ast_for.initialize, indent + 2);
            if (ast_for.condition)
                str += U"\n\t" + ind + U"condition:\n\t\t" + ind +
                       kh::repr(*ast_for.condition, indent + 2);
            if (ast_for.step)
                str += U"\n\t" + ind + U"step:\n\t\t" + ind + kh::repr(*ast_for.step, indent + 2);

            if (!ast_for.body.empty()) {
                str += U"\n\t" + ind + U"body:";
                for (auto part : ast_for.body)
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
            }

            break;
        }

        case kh::AstBody::Type::FOREACH: {
            const kh::AstForEach& ast_foreach = *(kh::AstForEach*)&ast;
            str += U"foreach:";

            if (ast_foreach.target)
                str +=
                    U"\n\t" + ind + U"target:\n\t\t" + ind + kh::repr(*ast_foreach.target, indent + 2);
            if (ast_foreach.iterator)
                str += U"\n\t" + ind + U"iterator:\n\t\t" + ind +
                       kh::repr(*ast_foreach.iterator, indent + 2);

            if (!ast_foreach.body.empty()) {
                str += U"\n\t" + ind + U"body:";
                for (auto part : ast_foreach.body)
                    if (part)
                        str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);
            }

            break;
        }

        case kh::AstBody::Type::STATEMENT: {
            const kh::AstStatement& ast_statement = *(kh::AstStatement*)&ast;
            str += U"statement: ";

            switch (ast_statement.statement_type) {
                case kh::AstStatement::Type::CONTINUE:
                    str += U"continue";
                    break;
                case kh::AstStatement::Type::BREAK:
                    str += U"break";
                    break;
                case kh::AstStatement::Type::RETURN:
                    str += U"return";
                    break;
                default:
                    str += U"unknown";
                    break;
            }

            if (ast_statement.statement_type == kh::AstStatement::Type::RETURN) {
                if (ast_statement.expression)
                    str += U"\n\t" + ind + kh::repr(*ast_statement.expression, indent + 1);
            }
            else
                str += U" " + kh::repr((uint64_t)ast_statement.loop_count);

            break;
        }

        default:
            str += U"[unknown body]";
    }

    return str;
}

std::u32string kh::repr(const kh::AstExpression& expr, const size_t indent) {
    std::u32string ind;
    ind.reserve(indent);
    for (size_t i = 0; i < indent; i++)
        ind += '\t';

    std::u32string str;

    switch (expr.expression_type) {
        case kh::AstExpression::ExType::IDENTIFIER: {
            const kh::AstIdentifierExpression& expr_id = *(kh::AstIdentifierExpression*)&expr;

            for (const std::u32string& identifier : expr_id.identifiers)
                str += identifier + (&identifier == &expr_id.identifiers.back() ? U"" : U".");

            bool is_function = false;
            if (expr_id.identifiers.size() == 1 && expr_id.identifiers[0] == U"func")
                is_function = true;

            if (!expr_id.generics.empty()) {
                str += U"!(";
                for (size_t i = 0; i < expr_id.generics.size(); i++) {
                    if (!expr_id.generics[i])
                        continue;

                    for (size_t refs = 0; refs < expr_id.generics_refs[i]; refs++)
                        str += U"ref ";
                    str += kh::repr(*(expr_id.generics[i]), indent);

                    if (expr_id.generics_array[i].size() && expr_id.generics_array[i][0] != 0) {
                        for (size_t d = 0; d < expr_id.generics_array[i].size(); d++)
                            str += U"[" + kh::repr(expr_id.generics_array[i][d]) + U"]";
                    }

                    if (is_function && i == 0)
                        str += U"(";
                    else if (i != expr_id.generics.size() - 1)
                        str += U", ";
                }
                str += is_function ? U"))" : U")";
            }

            break;
        }

        case kh::AstExpression::ExType::UNARY: {
            const kh::AstUnaryExpression& expr_unary = *(kh::AstUnaryExpression*)&expr;
            str += U"unary expression:";

            str += U"\n\t" + ind + U"operator: " + kh::repr(expr_unary.operation);

            if (expr_unary.rvalue)
                str +=
                    U"\n\t" + ind + U"rvalue:\n\t\t" + ind + kh::repr(*expr_unary.rvalue, indent + 2);

            break;
        }

        case kh::AstExpression::ExType::BINARY: {
            const kh::AstBinaryExpression& expr_binary = *(kh::AstBinaryExpression*)&expr;
            str += U"binary expression:";

            str += U"\n\t" + ind + U"operator: " + kh::repr(expr_binary.operation);

            if (expr_binary.lvalue)
                str +=
                    U"\n\t" + ind + U"lvalue:\n\t\t" + ind + kh::repr(*expr_binary.lvalue, indent + 2);
            if (expr_binary.rvalue)
                str +=
                    U"\n\t" + ind + U"rvalue:\n\t\t" + ind + kh::repr(*expr_binary.rvalue, indent + 2);

            break;
        }

        case kh::AstExpression::ExType::TERNARY: {
            const kh::AstTernaryExpression& expr_ternary = *(kh::AstTernaryExpression*)&expr;
            str += U"ternary expression:";

            if (expr_ternary.condition)
                str += U"\n\t" + ind + U"condition:\n\t\t" + ind +
                       kh::repr(*expr_ternary.condition, indent + 2);
            if (expr_ternary.value)
                str +=
                    U"\n\t" + ind + U"value:\n\t\t" + ind + kh::repr(*expr_ternary.value, indent + 2);
            if (expr_ternary.otherwise)
                str += U"\n\t" + ind + U"otherwise:\n\t\t" + ind +
                       kh::repr(*expr_ternary.otherwise, indent + 2);

            break;
        }

        case kh::AstExpression::ExType::COMPARISON: {
            const kh::AstComparisonExpression& expr_comparison = *(kh::AstComparisonExpression*)&expr;
            str += U"comparison expression:\n\t" + ind + U"operation(s): ";

            for (const kh::Operator& operation : expr_comparison.operations)
                str += kh::repr(operation) +
                       (&operation == &expr_comparison.operations.back() ? U"" : U",");

            str += U"\n\t" + ind + U"value(s):";
            for (std::shared_ptr<kh::AstExpression> value : expr_comparison.values)
                if (value)
                    str += U"\n\t\t" + ind + kh::repr(*value);

            break;
        }

        case kh::AstExpression::ExType::SUBSCRIPT: {
            const kh::AstSubscriptExpression& expr_subscript = *(kh::AstSubscriptExpression*)&expr;
            str += U"subscript:";

            if (expr_subscript.expression)
                str += U"\n\t" + ind + U"expression:\n\t\t" + ind +
                       kh::repr(*expr_subscript.expression, indent + 2);

            if (!expr_subscript.arguments.empty()) {
                str += U"\n\t" + ind + U"argument(s):";
                for (auto argument : expr_subscript.arguments)
                    if (argument)
                        str += U"\n\t\t" + ind + kh::repr(*argument, indent + 2);
            }

            break;
        }

        case kh::AstExpression::ExType::CALL: {
            const kh::AstCallExpression& expr_call = *(kh::AstCallExpression*)&expr;
            str += U"call:";

            if (expr_call.expression)
                str += U"\n\t" + ind + U"expression:\n\t\t" + ind +
                       kh::repr(*expr_call.expression, indent + 2);

            if (!expr_call.arguments.empty()) {
                str += U"\n\t" + ind + U"argument(s):";
                for (auto argument : expr_call.arguments)
                    if (argument)
                        str += U"\n\t\t" + ind + kh::repr(*argument, indent + 2);
            }

            break;
        }

        case kh::AstExpression::ExType::DECLARE: {
            const kh::AstDeclarationExpression& expr_declare = *(kh::AstDeclarationExpression*)&expr;
            str += U"declare:";

            if (expr_declare.var_type) {
                str += U"\n\t" + ind + U"type: ";

                if (expr_declare.is_static)
                    str += U"static ";
                if (!expr_declare.is_public)
                    str += U"private ";

                for (size_t refs = 0; refs < expr_declare.refs; refs++)
                    str += U"ref ";
                str += kh::repr(*expr_declare.var_type, indent + 1);

                if (expr_declare.var_array.size() && expr_declare.var_array[0] != 0) {
                    for (const uint64_t dimension : expr_declare.var_array)
                        str += U'[' + kh::repr(dimension) + U']';
                }
            }

            str += U"\n\t" + ind + U"name: " + expr_declare.var_name;

            if (expr_declare.expression)
                str += U"\n\t" + ind + U"initializer expression:\n\t\t" + ind +
                       kh::repr(*expr_declare.expression, indent + 2);

            break;
        }

        case kh::AstExpression::ExType::FUNCTION: {
            const kh::AstFunctionExpression& expr_func = *(kh::AstFunctionExpression*)&expr;

            str += expr_func.is_conditional ? U"conditional function:" : U"function:";

            str += U"\n\t" + ind + U"static: " + (expr_func.is_static ? U"true" : U"false");
            str += U"\n\t" + ind + U"access: " + (expr_func.is_public ? U"public" : U"private");

            if (expr_func.identifiers.empty())
                str += U"\n\t" + ind + U"name: (lambda)";
            else {
                str += U"\n\t" + ind + U"name: ";
                for (const std::u32string& identifier : expr_func.identifiers)
                    str += identifier + (&identifier == &expr_func.identifiers.back() ? U"" : U".");

                if (!expr_func.generic_args.empty()) {
                    str += U"\n\t" + ind + U"generic argument(s): ";
                    for (const std::u32string& generic_ : expr_func.generic_args)
                        str += generic_ + (&generic_ == &expr_func.generic_args.back() ? U"" : U", ");
                }
            }

            if (expr_func.return_type) {
                str += U"\n\t" + ind + U"return type: ";

                for (size_t refs = 0; refs < expr_func.return_refs; refs++)
                    str += U"ref ";
                str += kh::repr(*expr_func.return_type, indent + 1);

                if (expr_func.return_array.size() && expr_func.return_array[0] != 0) {
                    for (const uint64_t dimension : expr_func.return_array)
                        str += U'[' + kh::repr(dimension) + U']';
                }
            }

            str += U"\n\t" + ind + U"argument(s):";
            if (expr_func.arguments.empty())
                str += U" [none]";
            for (auto arg : expr_func.arguments)
                if (arg)
                    str += U"\n\t\t" + ind + kh::repr(*arg, indent + 2);

            str += U"\n\t" + ind + U"body:";
            for (auto part : expr_func.body)
                if (part)
                    str += U"\n\t\t" + ind + kh::repr(*part, indent + 2);

            break;
        }

        case kh::AstExpression::ExType::SCOPE: {
            const kh::AstScopeExpression& expr_scope = *(kh::AstScopeExpression*)&expr;
            str += U"scoping:";

            if (expr_scope.expression)
                str += U"\n\t" + ind + U"expression:\n\t\t" + ind +
                       kh::repr(*expr_scope.expression, indent + 2);

            if (!expr_scope.identifiers.empty()) {
                str += U"\n\t" + ind + U"scoping: [expr]";
                for (const std::u32string& identifier : expr_scope.identifiers)
                    str += U'.' + identifier;
            }

            break;
        }

        case kh::AstExpression::ExType::CONSTANT: {
            const kh::AstConstValue& expr_const = *(kh::AstConstValue*)&expr;

            switch (expr_const.value_type) {
                case kh::AstConstValue::ValueType::CHARACTER:
                    str += U"character: " + expr_const.character;
                    break;

                case kh::AstConstValue::ValueType::UINTEGER:
                    str += U"unsigned integer: " + kh::repr(expr_const.uinteger);
                    break;

                case kh::AstConstValue::ValueType::INTEGER:
                    str += U"integer: " + kh::repr(expr_const.integer);
                    break;

                case kh::AstConstValue::ValueType::FLOATING:
                    str += U"floating: " + kh::repr(expr_const.floating);
                    break;

                case kh::AstConstValue::ValueType::IMAGINARY:
                    str += U"imaginary: " + kh::repr(expr_const.imaginary) + U"i";
                    break;

                case kh::AstConstValue::ValueType::BUFFER:
                    str += U"buffer: " + kh::quote(expr_const.buffer);
                    break;

                case kh::AstConstValue::ValueType::STRING:
                    str += U"string: " + kh::quote(expr_const.string);
                    break;

                default:
                    str += U"[unknown constant]";
            }

            break;
        }

        case kh::AstExpression::ExType::TUPLE: {
            const kh::AstTupleExpression& expr_const = *(kh::AstTupleExpression*)&expr;
            str += U"tuple:";

            if (expr_const.elements.empty()) {
                str += U" [no elements]";
                break;
            }

            for (auto element : expr_const.elements)
                if (element)
                    str += U"\n\t" + ind + kh::repr(*element, indent + 1);

            break;
        }

        case kh::AstExpression::ExType::LIST: {
            const kh::AstListExpression& expr_const = *(kh::AstListExpression*)&expr;
            str += U"list:";

            for (auto element : expr_const.elements)
                if (element)
                    str += U"\n\t" + ind + kh::repr(*element, indent + 1);

            break;
        }

        case kh::AstExpression::ExType::DICT: {
            const kh::AstDictExpression& expr_const = *(kh::AstDictExpression*)&expr;
            str += U"dict:";

            for (size_t i = 0; i < expr_const.keys.size(); i++) {
                str += U"\n\t" + ind + U"pair:";
                if (expr_const.keys[i])
                    str += U"\n\t\t" + ind + kh::repr(*expr_const.keys[i], indent + 2);
                if (expr_const.items[i])
                    str += U"\n\t\t" + ind + kh::repr(*expr_const.items[i], indent + 2);
            }

            break;
        }

        default:
            str += U"[unknown expression]";
    }

    return str;
}
