/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#include <kithare/ast.hpp>


using namespace kh;

#define REPR_ALL_IN(var)   \
    for (auto& _var : var) \
    str += "\n\t" + ind + kh::strfy(_var, indent + 1)

#define STRFY_HEADER()                  \
    std::string ind;                    \
    ind.reserve(indent);                \
    for (size_t i = 0; i < indent; i++) \
        ind += '\t';                    \
                                        \
    std::string str;

std::string kh::strfy(const AstModule& module_ast, size_t indent) {
    STRFY_HEADER();

    str = "ast:";
    REPR_ALL_IN(module_ast.imports);
    REPR_ALL_IN(module_ast.functions);
    REPR_ALL_IN(module_ast.user_types);
    REPR_ALL_IN(module_ast.enums);
    REPR_ALL_IN(module_ast.variables);
    return str;
}

std::string kh::strfy(const AstImport& import_ast, size_t indent) {
    STRFY_HEADER();

    str = import_ast.is_include ? "include:" : "import:";
    str += "\n\t" + ind + "type: " + (import_ast.is_relative ? "relative" : "absolute");
    str += "\n\t" + ind + "access: " + (import_ast.is_public ? "public" : "private");

    str += "\n\t" + ind + "path: ";
    for (const std::string& dir : import_ast.path) {
        str += dir + (&dir == &import_ast.path.back() ? "" : ".");
    }

    if (!import_ast.is_include) {
        str += "\n\t" + ind + "identifier: " + import_ast.identifier;
    }

    return str;
}

std::string kh::strfy(const AstUserType& type_ast, size_t indent) {
    STRFY_HEADER();

    str = (type_ast.is_class ? "class:\n\t" : "struct:\n\t") + ind + "name: ";
    for (const std::string& identifier : type_ast.identifiers) {
        str += identifier + (&identifier == &type_ast.identifiers.back() ? "" : ".");
    }

    str += "\n\t" + ind + "access: " + (type_ast.is_public ? "public" : "private");

    if (type_ast.base) {
        str += "\n\t" + ind + "base " + (type_ast.is_class ? "class:" : "struct:");
        str += "\n\t\t" + ind + kh::strfy(*type_ast.base, indent + 3);
    }

    if (!type_ast.generic_args.empty()) {
        str += "\n\t" + ind + "generic argument(s): ";
        for (const std::string& generic_ : type_ast.generic_args) {
            str += generic_ + (&generic_ == &type_ast.generic_args.back() ? "" : ", ");
        }
    }

    if (!type_ast.members.empty()) {
        str += "\n\t" + ind + "member(s):";
        for (auto& member : type_ast.members) {
            str += "\n\t\t" + ind + kh::strfy(member, indent + 2);
        }
    }

    if (!type_ast.methods.empty()) {
        str += "\n\t" + ind + "method(s):";
        for (auto& method : type_ast.methods) {
            str += "\n\t\t" + ind + kh::strfy(method, indent + 2);
        }
    }

    return str;
}

std::string kh::strfy(const AstEnumType& enum_ast, size_t indent) {
    STRFY_HEADER();

    str = "enum:\n\t" + ind + "name: ";
    for (const std::string& identifier : enum_ast.identifiers) {
        str += identifier + (&identifier == &enum_ast.identifiers.back() ? "" : ".");
    }

    str += "\n\t" + ind + "access: " + (enum_ast.is_public ? "public" : "private");

    str += "\n\t" + ind + "member(s):";
    for (size_t member = 0; member < enum_ast.members.size(); member++) {
        str += "\n\t\t" + ind + enum_ast.members[member] + ": " + kh::strfy(enum_ast.values[member]);
    }

    return str;
}

std::string kh::strfy(const AstBody& body_ast, size_t indent) {
    return body_ast.strfy(indent);
}

std::string kh::AstBody::strfy(size_t indent) const {
    return "[unknown body]";
}

std::string kh::AstExpression::strfy(size_t indent) const {
    return "[unknown expression]";
}

std::string kh::AstIdentifiers::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "identifier(s): ";

    for (const std::string& identifier : this->identifiers) {
        str += identifier + (&identifier == &this->identifiers.back() ? "" : ".");
    }

    bool is_function = this->identifiers.size() == 1 && this->identifiers[0] == "func";

    if (!this->generics.empty()) {
        str += "!(";
        for (size_t i = 0; i < this->generics.size(); i++) {
            for (size_t refs = 0; refs < this->generics_refs[i]; refs++) {
                str += "ref ";
            }

            str += kh::strfy(this->generics[i], indent);

            for (size_t d = 0; d < this->generics_array[i].size(); d++) {
                str += "[" + kh::strfy(this->generics_array[i][d]) + "]";
            }

            if (is_function && i == 0) {
                str += "(";
            }
            else if (i != this->generics.size() - 1) {
                str += ", ";
            }
        }
        str += is_function ? "))" : ")";
    }

    return str;
}

std::string kh::AstUnaryOperation::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "unary expression:";

    str += "\n\t" + ind + "operator: " + kh::strfy(this->operation);

    if (this->rvalue) {
        str += "\n\t" + ind + "rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::string kh::AstRevUnaryOperation::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "reverse unary expression:";

    str += "\n\t" + ind + "operator: " + kh::strfy(this->operation);

    if (this->rvalue) {
        str += "\n\t" + ind + "rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::string kh::AstBinaryOperation::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "binary expression:";

    str += "\n\t" + ind + "operator: " + kh::strfy(this->operation);

    if (this->lvalue) {
        str += "\n\t" + ind + "lvalue:\n\t\t" + ind + kh::strfy(*this->lvalue, indent + 2);
    }

    if (this->rvalue) {
        str += "\n\t" + ind + "rvalue:\n\t\t" + ind + kh::strfy(*this->rvalue, indent + 2);
    }

    return str;
}

std::string kh::AstTernaryOperation::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "ternary expression:";

    if (this->condition) {
        str += "\n\t" + ind + "condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (this->value) {
        str += "\n\t" + ind + "value:\n\t\t" + ind + kh::strfy(*this->value, indent + 2);
    }

    if (this->otherwise) {
        str += "\n\t" + ind + "otherwise:\n\t\t" + ind + kh::strfy(*this->otherwise, indent + 2);
    }

    return str;
}

std::string kh::AstComparisonExpression::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "comparison expression:\n\t" + ind + "operation(s): ";

    for (Operator operation : this->operations) {
        str += kh::strfy(operation) + (&operation == &this->operations.back() ? "" : ",");
    }

    str += "\n\t" + ind + "value(s):";
    for (auto& value : this->values) {
        if (value) {
            str += "\n\t\t" + ind + kh::strfy(*value);
        }
    }

    return str;
}

std::string kh::AstSubscriptExpression::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "subscript:";

    if (this->expression) {
        str += "\n\t" + ind + "expression:\n\t\t" + ind + kh::strfy(*this->expression, indent + 2);
    }

    if (!this->arguments.empty()) {
        str += "\n\t" + ind + "argument(s):";
        for (auto& argument : this->arguments) {
            if (argument) {
                str += "\n\t\t" + ind + kh::strfy(*argument, indent + 2);
            }
        }
    }

    return str;
}

std::string kh::AstCallExpression::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "call:";

    if (this->expression) {
        str += "\n\t" + ind + "expression:\n\t\t" + ind + kh::strfy(*this->expression, indent + 2);
    }

    if (!this->arguments.empty()) {
        str += "\n\t" + ind + "argument(s):";
        for (auto& argument : this->arguments) {
            if (argument) {
                str += "\n\t\t" + ind + kh::strfy(*argument, indent + 2);
            }
        }
    }

    return str;
}

std::string kh::AstDeclaration::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "declare:";

    str += "\n\t" + ind + "access: " + (this->is_static ? "static " : "") +
           (this->is_public ? "public" : "private");

    str += "\n\t" + ind + "type: ";
    for (size_t refs = 0; refs < this->refs; refs++) {
        str += "ref ";
    }
    str += kh::strfy(this->var_type, indent + 1);

    for (uint64_t dimension : this->var_array) {
        str += '[' + kh::strfy(dimension) + ']';
    }

    str += "\n\t" + ind + "name: " + this->var_name;

    if (this->expression)
        str += "\n\t" + ind + "initializer expression:\n\t\t" + ind +
               kh::strfy(*this->expression, indent + 2);

    return str;
}

std::string kh::AstFunction::strfy(size_t indent) const {
    STRFY_HEADER();
    str = this->is_conditional ? "conditional function:" : "function:";

    str += "\n\t" + ind + "access: " + (this->is_static ? "static " : "") +
           (this->is_public ? "public" : "private");

    if (this->identifiers.empty()) {
        str += "\n\t" + ind + "name: (lambda)";
    }
    else {
        str += "\n\t" + ind + "name: ";
        for (const std::string& identifier : this->identifiers) {
            str += identifier + (&identifier == &this->identifiers.back() ? "" : ".");
        }

        if (!this->generic_args.empty()) {
            str += "\n\t" + ind + "generic argument(s): ";
            for (const std::string& generic_ : this->generic_args) {
                str += generic_ + (&generic_ == &this->generic_args.back() ? "" : ", ");
            }
        }

        if (!this->id_array.empty()) {
            str += "\n\t" + ind + "array type dimension: ";
            for (uint64_t size : this->id_array) {
                str += '[' + kh::strfy(size) + ']';
            }
        }
    }

    str += "\n\t" + ind + "return type: ";

    for (size_t refs = 0; refs < this->return_refs; refs++) {
        str += "ref ";
    }
    str += kh::strfy(this->return_type, indent + 1);

    for (uint64_t dimension : this->return_array) {
        str += '[' + kh::strfy(dimension) + ']';
    }

    str += "\n\t" + ind + "argument(s):";
    if (this->arguments.empty()) {
        str += " [none]";
    }
    for (auto& arg : this->arguments) {
        str += "\n\t\t" + ind + kh::strfy(arg, indent + 2);
    }

    str += "\n\t" + ind + "body:";
    for (auto& part : this->body) {
        if (part) {
            str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
        }
    }

    return str;
}

std::string kh::AstScoping::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "scoping (";

    if (!this->identifiers.empty()) {
        for (const std::string& identifier : this->identifiers) {
            str += (&this->identifiers.back() == &identifier ? "" : ".") + identifier;
        }
    }

    str += "):"; /* sad face */

    if (this->expression) {
        str += "\n\t" + ind + kh::strfy(*this->expression, indent + 1);
    }

    return str;
}

std::string kh::AstValue::strfy(size_t indent) const {
    STRFY_HEADER();

    switch (this->value_type) {
        case AstValue::ValueType::CHARACTER:
            str = "character: " + kh::strfy(this->character);
            break;

        case AstValue::ValueType::UINTEGER:
            str = "unsigned integer: " + kh::strfy(this->uinteger);
            break;

        case AstValue::ValueType::INTEGER:
            str = "integer: " + kh::strfy(this->integer);
            break;

        case AstValue::ValueType::FLOATING:
            str = "floating: " + kh::strfy(this->floating);
            break;

        case AstValue::ValueType::IMAGINARY:
            str = "imaginary: " + kh::strfy(this->imaginary) + "i";
            break;

        case AstValue::ValueType::BUFFER:
            str = "buffer: " + quote(this->buffer);
            break;

        case AstValue::ValueType::STRING:
            str = "string: " + quote(this->string);
            break;

        default:
            str = "[unknown constant]";
    }

    return str;
}

std::string kh::AstTuple::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "tuple:";

    if (this->elements.empty()) {
        str += " [no elements]";
    }
    else {
        for (auto& element : this->elements) {
            if (element) {
                str += "\n\t" + ind + kh::strfy(*element, indent + 1);
            }
        }
    }

    return str;
}

std::string kh::AstList::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "list:";

    if (this->elements.empty()) {
        str += " [no elements]";
    }
    else {
        for (auto& element : this->elements) {
            if (element) {
                str += "\n\t" + ind + kh::strfy(*element, indent + 1);
            }
        }
    }

    return str;
}

std::string kh::AstDict::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "dict:";

    if (this->keys.empty()) {
        str += " [no pairs]";
    }
    else {
        for (size_t i = 0; i < this->keys.size(); i++) {
            str += "\n\t" + ind + "pair:";
            if (this->keys[i]) {
                str += "\n\t\t" + ind + kh::strfy(*this->keys[i], indent + 2);
            }
            if (this->items[i]) {
                str += "\n\t\t" + ind + kh::strfy(*this->items[i], indent + 2);
            }
        }
    }

    return str;
}

std::string kh::AstIf::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "if:";

    for (size_t clause = 0; clause < this->conditions.size(); clause++) {
        str += "\n\t" + ind + "if clause:";

        if (this->conditions[clause])
            str += "\n\t\t" + ind + "condition:\n\t\t\t" + ind +
                   kh::strfy(*this->conditions[clause], indent + 3);

        if (!this->bodies[clause].empty()) {
            str += "\n\t\t" + ind + "body:";
            for (auto& part : this->bodies[clause])
                if (part) {
                    str += "\n\t\t\t" + ind + kh::strfy(*part, indent + 3);
                }
        }
    }

    if (!this->else_body.empty()) {
        str += "\n\t" + ind + "else body:";
        for (auto& part : this->else_body)
            if (part) {
                str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::string kh::AstWhile::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "while:";

    if (this->condition) {
        str += "\n\t" + ind + "condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (!this->body.empty()) {
        str += "\n\t" + ind + "body:";
        for (auto& part : this->body)
            if (part) {
                str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::string kh::AstDoWhile::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "do while:";

    if (this->condition) {
        str += "\n\t" + ind + "condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (!this->body.empty()) {
        str += "\n\t" + ind + "body:";
        for (auto& part : this->body)
            if (part) {
                str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::string kh::AstFor::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "for:";

    if (this->initialize) {
        str += "\n\t" + ind + "initializer:\n\t\t" + ind + kh::strfy(*this->initialize, indent + 2);
    }

    if (this->condition) {
        str += "\n\t" + ind + "condition:\n\t\t" + ind + kh::strfy(*this->condition, indent + 2);
    }

    if (this->step) {
        str += "\n\t" + ind + "step:\n\t\t" + ind + kh::strfy(*this->step, indent + 2);
    }

    if (!this->body.empty()) {
        str += "\n\t" + ind + "body:";
        for (auto& part : this->body)
            if (part) {
                str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::string kh::AstForEach::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "foreach:";

    if (this->target) {
        str += "\n\t" + ind + "target:\n\t\t" + ind + kh::strfy(*this->target, indent + 2);
    }

    if (this->iterator) {
        str += "\n\t" + ind + "iterator:\n\t\t" + ind + kh::strfy(*this->iterator, indent + 2);
    }

    if (!this->body.empty()) {
        str += "\n\t" + ind + "body:";
        for (auto& part : this->body)
            if (part) {
                str += "\n\t\t" + ind + kh::strfy(*part, indent + 2);
            }
    }

    return str;
}

std::string kh::AstStatement::strfy(size_t indent) const {
    STRFY_HEADER();
    str = "statement: ";

    switch (this->statement_type) {
        case AstStatement::Type::CONTINUE:
            str += "continue";
            break;
        case AstStatement::Type::BREAK:
            str += "break";
            break;
        case AstStatement::Type::RETURN:
            str += "return";
            break;
        default:
            str += "unknown";
            break;
    }

    if (this->statement_type == AstStatement::Type::RETURN) {
        if (this->expression) {
            str += "\n\t" + ind + kh::strfy(*this->expression, indent + 1);
        }
    }
    else {
        str += " " + kh::strfy((uint64_t)this->loop_count);
    }
    return str;
}
