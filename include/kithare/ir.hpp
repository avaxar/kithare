/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once


namespace kh {
    class Ir;
    class IrModule;
    class IrScope;
    class IrVariable;

    class IrBody;
    class IrExpressionBody;
    class IrIf;
    class IrWhile;
    class IrDoWhile;
    class IrFor;
    class IrContinueLoop;
    class IrBreakLoop;
    class IrReturn;

    class IrExpression;
    class IrVariableExpression;
    class IrReturnRefExpression;
    class IrFunctionExpression;
    class IrAssignExpression;
    class IrDerefExpression;
    class IrScopeExpression;
    class IrCallExpression;

    class IrType;
    class IrRefType;
    class IrArrayType;
    class IrListType;
    class IrTupleType;
    class IrDictType;
    class IrFuncType;
    class IrClassType;
    class IrStructType;
    class IrEnumType;

    class IrFunction;
    class IrIntrinsic;
    class IrUserFunction;

    class IrClassGeneric;
    class IrStructGeneric;
    class IrFunctionGeneric;
}
