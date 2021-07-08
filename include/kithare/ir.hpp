/*
 * This file is a part of the Kithare programming language source code.
 * The source code for Kithare programming language is distributed under the MIT license.
 * Copyright (C) 2021 Kithare Organization
 */

#pragma once

#include <vector>


namespace kh {
    class IrModule;
    class IrScope;
    class IrVariable;

    class IrFunction;
    class IrIntrinsic;
    class IrUserFunction;

    class IrClassGeneric;
    class IrStructGeneric;
    class IrFunctionGeneric;

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
    class IrTernaryExpression;
    class IrAssignExpression;
    class IrDerefExpression;
    class IrScopeExpression;
    class IrCallExpression;

    class IrConst;
    class IrIntegerConst;
    class IrUnsignedIntegerConst;
    class IrFloatConst;
    class IrDoubleConst;
    class IrComplexConst;
    class IrComplexFConst;
    class IrStringConst;
    class IrBufferConst;
    class IrEnumerationConst;

    class IrModule {};
}
