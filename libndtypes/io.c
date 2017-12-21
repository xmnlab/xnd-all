/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2017, plures
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include "ndtypes.h"


/******************************************************************************/
/*                      Various string representations                        */
/******************************************************************************/

/*
 * Return the keyword string if the type is a keyword.  Otherwise, return
 * "not a keyword".
 */
static const char *
ndt_type_keyword(const ndt_t *t)
{
    switch (t->tag) {
    case Void: return "void";

    case AnyKind: return "Any";
    case FixedDim: return "fixed";
    case VarDim: return "var";

    case Ref: return "ref";

    case ScalarKind: return "ScalarKind";
    case Categorical: return "categorical";

    case FixedStringKind: return "FixedStringKind";
    case FixedString: return "FixedString";

    case FixedBytesKind: return "FixedBytesKind";
    case FixedBytes: return "FixedBytes";

    case String: return "string";
    case Bytes: return "bytes";
    case Char: return "char";

    case Bool: return "bool";

    case SignedKind: return "SignedKind";
    case Int8: return "int8";
    case Int16: return "int16";
    case Int32: return "int32";
    case Int64: return "int64";

    case UnsignedKind: return "UnsignedKind";
    case Uint8: return "uint8";
    case Uint16: return "uint16";
    case Uint32: return "uint32";
    case Uint64: return "uint64";

    case FloatKind: return "FloatKind";
    case Float16: return "float16";
    case Float32: return "float32";
    case Float64: return "float64";

    case ComplexKind: return "ComplexKind";
    case Complex32: return "complex32";
    case Complex64: return "complex64";
    case Complex128: return "complex128";

    case Module: case Function:
    case SymbolicDim: case EllipsisDim:
    case Tuple: case Record:
    case Nominal: case Constr:
    case Typevar:
        return "not a keyword";
    }

    /* NOT REACHED: tags should be exhaustive. */
    ndt_internal_error("invalid tag");
}

/* Return the type name in constructor form for the abstract syntax tree. */
static const char *
ndt_type_name(const ndt_t *t)
{
    switch (t->tag) {
    case Module: return "Module";
    case Function: return "Function";
    case Void: return "Void";

    case AnyKind: return "Any";

    case FixedDim: return "FixedDim";
    case VarDim: return "VarDim";
    case SymbolicDim: return "SymbolicDim";
    case EllipsisDim: return "EllipsisDim";

    case Tuple: return "Tuple";
    case Record: return "Record";
    case Ref: return "Ref";
    case Constr: return "Constr";
    case Nominal: return "Nominal";

    case ScalarKind: return "ScalarKind";
    case Categorical: return "Categorical";

    case FixedStringKind: return "FixedStringKind";
    case FixedString: return "FixedString";

    case FixedBytesKind: return "FixedBytesKind";
    case FixedBytes: return "FixedBytes";

    case String: return "String";
    case Bytes: return "Bytes";
    case Char: return "Char";

    case Bool: return "Bool";

    case SignedKind: return "SignedKind";
    case Int8: return "Int8";
    case Int16: return "Int16";
    case Int32: return "Int32";
    case Int64: return "Int64";

    case UnsignedKind: return "UnsignedKind";
    case Uint8: return "Uint8";
    case Uint16: return "Uint16";
    case Uint32: return "Uint32";
    case Uint64: return "Uint64";

    case FloatKind: return "FloatKind";
    case Float16: return "Float16";
    case Float32: return "Float32";
    case Float64: return "Float64";

    case ComplexKind: return "ComplexKind";
    case Complex32: return "Complex32";
    case Complex64: return "Complex64";
    case Complex128: return "Complex128";

    case Typevar: return "Typevar";
    }

    /* NOT REACHED: tags should be exhaustive. */
    ndt_internal_error("invalid encoding");
}


/******************************************************************************/
/*                                String buffer                               */
/******************************************************************************/

#undef buf_t
typedef struct {
    size_t count; /* count the required size */
    size_t size;  /* buffer size (0 for the count phase) */
    char *cur;    /* buffer data (NULL for the count phase) */
} buf_t;

static int indent(ndt_context_t *ctx, buf_t *buf, int n);

static int
_ndt_snprintf(ndt_context_t *ctx, buf_t *buf, const char *fmt, va_list ap)
{
    int n;

    errno = 0;
    n = vsnprintf(buf->cur, buf->size, fmt, ap);
    if (buf->cur && n < 0) {
        if (errno == ENOMEM) {
            ndt_err_format(ctx, NDT_MemoryError, "out of memory");
        }
        else {
            ndt_err_format(ctx, NDT_OSError, "output error");
        }
        return -1;
    }

    if (buf->cur && (size_t)n >= buf->size) {
         ndt_err_format(ctx, NDT_ValueError, "insufficient buffer size");
        return -1;
    }

    buf->count += n;

    if (buf->cur) {
        buf->cur += n;
        buf->size -= n;
    }

    return 0;
}

static int
ndt_snprintf(ndt_context_t *ctx, buf_t *buf, const char *fmt, ...)
{
    va_list ap;
    int n;

    va_start(ap, fmt);
    n = _ndt_snprintf(ctx, buf, fmt, ap);
    va_end(ap);

    return n;
}

static int
ndt_snprintf_d(ndt_context_t *ctx, buf_t *buf, int d, const char *fmt, ...)
{
    va_list ap;
    int n;

    n = indent(ctx, buf, d);
    if (n < 0) return -1;

    va_start(ap, fmt);
    n = _ndt_snprintf(ctx, buf, fmt, ap);
    va_end(ap);

    return n;
}

static int
indent(ndt_context_t *ctx, buf_t *buf, int n)
{
    int i;

    if (n < 0) {
        return 0;
    }

    for (i = 0; i < n; i++) {
        if (ndt_snprintf(ctx, buf, " ") < 0) {
            return -1;
        }
    }

    return 0;
}


/******************************************************************************/
/*                       String representation of a type                      */
/******************************************************************************/

static int datashape(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx);

static int
tuple_fields(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int64_t i;
    int n;

    assert(t->tag == Tuple);

    for (i = 0; i < t->Tuple.shape; i++) {
        if (i >= 1) {
            n = ndt_snprintf(ctx, buf, ", ");
            if (n < 0) return -1;
        }

        n = datashape(buf, t->Tuple.types[i], d, ctx);
        if (n < 0) return -1;
    }

    return 0;
}

static int
record_fields(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int64_t i;
    int n;

    assert(t->tag == Record);

    for (i = 0; i < t->Tuple.shape; i++) {
        if (i >= 1) {
            if (d >= 0) {
                n = ndt_snprintf(ctx, buf, ",\n");
                if (n < 0) return -1;

                n = indent(ctx, buf, d);
                if (n < 0) return -1;
            }
            else {
                n = ndt_snprintf(ctx, buf, ", ");
                if (n < 0) return -1;
            }
        }

        n = ndt_snprintf(ctx, buf, "%s : ", t->Record.names[i]);
        if (n < 0) return -1;

        n = datashape(buf, t->Record.types[i], d, ctx);
        if (n < 0) return -1;
    }

    return 0;
}

static int
variadic_flag(buf_t *buf, enum ndt_variadic flag, ndt_context_t *ctx)
{
    if (flag == Variadic) {
        return ndt_snprintf(ctx, buf, "...");
    }

    return 0;
}

static int
comma_variadic_flag(buf_t *buf, enum ndt_variadic flag, int d, ndt_context_t *ctx)
{
    int n;

    if (flag == Variadic) {
        if (d >= 0) {
            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = indent(ctx, buf, d);
            if (n < 0) return -1;

            return ndt_snprintf(ctx, buf, "...");
        }
        else {
            return ndt_snprintf(ctx, buf, ", ...");
        }
    }

    return 0;
}

static int
value(buf_t *buf, const ndt_value_t *mem, ndt_context_t *ctx)
{
    switch (mem->tag) {
    case ValBool:
        return ndt_snprintf(ctx, buf, mem->ValBool ? "true" : "false");
    case ValInt64:
        return ndt_snprintf(ctx, buf, "%" PRIi64, mem->ValInt64);
    case ValFloat64:
        return ndt_snprintf(ctx, buf, "%g", mem->ValFloat64);
    case ValString:
        return ndt_snprintf(ctx, buf, "'%s'", mem->ValString);
    case ValNA:
        return ndt_snprintf(ctx, buf, "NA");
    }

    /* NOT REACHED: tags should be exhaustive. */
    ndt_err_format(ctx, NDT_RuntimeError, "invalid value tag");
    return 0;
}

static int
categorical(buf_t *buf, ndt_value_t *mem, size_t ntypes, ndt_context_t *ctx)
{
    size_t i;
    int n;

    for (i = 0; i < ntypes; i++) {
        if (i >= 1) {
            n = ndt_snprintf(ctx, buf, ", ");
            if (n < 0) return -1;
        }

        n = value(buf, &mem[i], ctx);
        if (n < 0) return -1;
    }

    return 0;
}

static int
option(buf_t *buf, const ndt_t *t, ndt_context_t *ctx)
{
    if (ndt_is_optional(t)) {
        return ndt_snprintf(ctx, buf, "?");
    }

    return 1;
}

static int
endian(buf_t *buf, const ndt_t *t, ndt_context_t *ctx)
{
    if (ndt_endian_is_set(t)) {
        if (ndt_is_little_endian(t)) {
            return ndt_snprintf(ctx, buf, "<");
        }
        return ndt_snprintf(ctx, buf, ">");
    }

    return 1;
}

static int
datashape(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int n;

    n = option(buf, t, ctx);
    if (n < 0) return -1;

    n = endian(buf, t, ctx);
    if (n < 0) return -1;

    switch (t->tag) {
        case Module: {
            n = ndt_snprintf(ctx, buf, "%s:: ", t->Module.name);
            if (n < 0) return -1;

            n = datashape(buf, t->Module.type, d, ctx);
            return n;
        }

        case Function: {
            ndt_t *pos = t->Function.pos;
            ndt_t *kwds = t->Function.kwds;

            n = ndt_snprintf(ctx, buf, "(");
            if (n < 0) return -1;

            if (pos->Tuple.shape > 0) {
                n = tuple_fields(buf, pos, d, ctx);
                if (n < 0) return -1;

                n = comma_variadic_flag(buf, pos->Tuple.flag, INT_MIN, ctx);
                if (n < 0) return -1;
            }
            else {
                n = variadic_flag(buf, pos->Tuple.flag, ctx);
                if (n < 0) return -1;
            }

            if (kwds->Record.shape > 0) {
                if (pos->Tuple.flag == Variadic || pos->Tuple.shape > 0) {
                    n = ndt_snprintf(ctx, buf, ", ");
                    if (n < 0) return -1;
                }

                n = record_fields(buf, kwds, INT_MIN, ctx);
                if (n < 0) return -1;

                n = comma_variadic_flag(buf, kwds->Record.flag, INT_MIN, ctx);
                if (n < 0) return -1;
            }
            else {
                n = variadic_flag(buf, kwds->Record.flag, ctx);
                if (n < 0) return -1;
            }

            n = ndt_snprintf(ctx, buf, ") -> ");
            if (n < 0) return -1;

            n = datashape(buf, t->Function.ret, d, ctx);
            return n;
        }

        case FixedDim: {
            n = ndt_snprintf(ctx, buf, "%" PRIi64 " * ", t->FixedDim.shape);
            if (n < 0) return -1;

            n = datashape(buf, t->FixedDim.type, d, ctx);
            return n;
        }

        case VarDim: {
            n = ndt_snprintf(ctx, buf, "var * ");
            if (n < 0) return -1;

            n = datashape(buf, t->VarDim.type, d, ctx);
            return n;
        }

        case SymbolicDim: {
            n = ndt_snprintf(ctx, buf, "%s * ", t->SymbolicDim.name);
            if (n < 0) return -1;

            n = datashape(buf, t->SymbolicDim.type, d, ctx);
            return n;
        }

        case EllipsisDim: {
            n = ndt_snprintf(ctx, buf, "%s... * ",
                    t->EllipsisDim.name ? t->EllipsisDim.name : "");
            if (n < 0) return -1;

            n = datashape(buf, t->EllipsisDim.type, d, ctx);
            return n;
        }

        case Tuple: {
            n = ndt_snprintf(ctx, buf, "(");
            if (n < 0) return -1;

            if (t->Tuple.shape > 0) {
                n = tuple_fields(buf, t, d, ctx);
                if (n < 0) return -1;

                n = comma_variadic_flag(buf, t->Tuple.flag, INT_MIN, ctx);
                if (n < 0) return -1;
            }
            else {
                n = variadic_flag(buf, t->Tuple.flag, ctx);
                if (n < 0) return -1;
            }

            n = ndt_snprintf(ctx, buf, ")");
            return n;
        }

        case Record: {
            n = ndt_snprintf(ctx, buf, "{");
            if (n < 0) return -1;

            if (d >= 0) {
                n = ndt_snprintf(ctx, buf, "\n");
                if (n < 0) return -1;
                n = indent(ctx, buf, d+2);
                if (n < 0) return -1;
            }

            if (t->Record.shape > 0) {
                n = record_fields(buf, t, d+2, ctx);
                if (n < 0) return -1;

                n = comma_variadic_flag(buf, t->Record.flag, d+2, ctx);
                if (n < 0) return -1;
            }
            else {
                n = variadic_flag(buf, t->Record.flag, ctx);
                if (n < 0) return -1;

            }

            if (d >= 0) {
                n = ndt_snprintf(ctx, buf, "\n");
                if (n < 0) return -1;
                n = indent(ctx, buf, d);
                if (n < 0) return -1;
            }

            n = ndt_snprintf(ctx, buf, "}");
            return n;
        }

        case Ref: {
            n = ndt_snprintf(ctx, buf, "ref(");
            if (n < 0) return -1;

            n = datashape(buf, t->Ref.type, d, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ")");
            return n;
        }

        case Constr: {
            n = ndt_snprintf(ctx, buf, "%s(", t->Constr.name);
            if (n < 0) return -1;

            n = datashape(buf, t->Constr.type, d, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ")");
            return n;
        }

        case Nominal: {
            n = ndt_snprintf(ctx, buf, "%s", t->Nominal.name);
            return n;
        }

        case Categorical: {
            n = ndt_snprintf(ctx, buf, "categorical(");
            if (n < 0) return -1;

            n = categorical(buf, t->Categorical.types, t->Categorical.ntypes, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ")");
            return n;
        }

        case FixedString: {
            if (t->FixedString.encoding == Utf8) {
                n = ndt_snprintf(ctx, buf, "fixed_string(%zu)",
                                 t->FixedString.size);
            }
            else {
                n = ndt_snprintf(ctx, buf, "fixed_string(%zu, %s)",
                                 t->FixedString.size,
                                 ndt_encoding_as_string(t->FixedString.encoding));
            }
            return n;
        }

        case FixedBytes: {
            if (t->FixedBytes.align == 1) {
                n = ndt_snprintf(ctx, buf, "fixed_bytes(size=%zu)",
                                 t->FixedBytes.size);
            }
            else {
                n = ndt_snprintf(ctx, buf, "fixed_bytes(size=%zu, align=%" PRIu8 ")",
                                 t->FixedBytes.size, t->FixedBytes.align);
            }
            return n;
        }

        case Bytes: {
            if (t->Bytes.target_align == 1) {
                n = ndt_snprintf(ctx, buf, "bytes()");
            }
            else {
                n = ndt_snprintf(ctx, buf, "bytes(align=%" PRIu8 ")", t->Bytes.target_align);
            }
            return n;
        }

        case Char: {
            n = ndt_snprintf(ctx, buf, "char(%s)",
                          ndt_encoding_as_string(t->Char.encoding));
            return n;
        }

        case Typevar: {
            n = ndt_snprintf(ctx, buf, "%s", t->Typevar.name);
            return n;
        }

        case AnyKind:
        case ScalarKind:
        case Void: case Bool:
        case SignedKind:
        case Int8: case Int16: case Int32: case Int64:
        case UnsignedKind:
        case Uint8: case Uint16: case Uint32: case Uint64:
        case FloatKind:
        case Float16: case Float32: case Float64:
        case ComplexKind:
        case Complex32: case Complex64: case Complex128:
        case FixedStringKind:
        case FixedBytesKind:
        case String:
            n = ndt_snprintf(ctx, buf, "%s", ndt_type_keyword(t));
            return n;
    }

    /* NOT REACHED: tags should be exhaustive. */
    ndt_internal_error("invalid tag");
}


/******************************************************************************/
/*                           Abstract syntax tree                             */
/******************************************************************************/

static int ast_datashape(buf_t *buf, const ndt_t *t, int d, int cont, ndt_context_t *ctx);

static int
ast_type_flags(buf_t *buf, const ndt_t *t, ndt_context_t *ctx)
{
    bool cont = 0;
    int n;

    if (t->flags & NDT_OPTION) {
        n = ndt_snprintf(ctx, buf, "%sOPTION", cont ? ", " : "");
        if (n > 0) return -1;
        cont = 1;
    }

    if (t->flags & NDT_SUBTREE_OPTION) {
        n = ndt_snprintf(ctx, buf, "%sSUBTREE_OPTION", cont ? ", " : "");
        if (n > 0) return -1;
        cont = 1;
    }

    if (t->flags & NDT_LITTLE_ENDIAN) {
        n = ndt_snprintf(ctx, buf, "%sLITTLE_ENDIAN", cont ? ", " : "");
        if (n > 0) return -1;
        cont = 1;
    }

    if (t->flags & NDT_BIG_ENDIAN) {
        n = ndt_snprintf(ctx, buf, "%sBIG_ENDIAN", cont ? ", " : "");
        if (n > 0) return -1;
        cont = 1;
    }

    if (t->flags & NDT_ELLIPSIS) {
        n = ndt_snprintf(ctx, buf, "%sELLIPSIS", cont ? ", " : "");
        if (n > 0) return -1;
    }

    return 0;
}

static int
ast_common_attributes(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int n;

    if (ndt_is_abstract(t)) {
        n = ndt_snprintf_d(ctx, buf, d, "access=Abstract, ndim=%d, ", t->ndim);
    }
    else {
        n = ndt_snprintf_d(ctx, buf, d,
                "access=Concrete, ndim=%d, datasize=%" PRIi64 ", align=%" PRIu16 ", ",
                t->ndim, t->datasize, t->align);
    }

    if (n < 0) return -1;

    n = ndt_snprintf(ctx, buf, "flags=[");
    if (n < 0) return -1;

    ast_type_flags(buf, t, ctx);
    if (n < 0) return -1;

    return ndt_snprintf(ctx, buf, "]");
}

static int
ast_common_attributes_with_newline(buf_t *buf, const ndt_t *t, int d,
                                   ndt_context_t *ctx)
{
    int n;

    n = ast_common_attributes(buf, t, d, ctx);
    if (n < 0) return -1;

    return ndt_snprintf(ctx, buf, "\n");
}

static int
ast_tuple_fields(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int64_t i;
    int n;

    assert(t->tag == Tuple);

    for (i = 0; i < t->Tuple.shape; i++) {
        if (i >= 1) {
            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;
        }

        n = ndt_snprintf_d(ctx, buf, d, "TupleField(\n");
        if (n < 0) return -1;

        n = ndt_snprintf_d(ctx, buf, d+2, "type=");
        if (n < 0) return -1;

        n = ast_datashape(buf, t->Tuple.types[i], d+5+2, 1, ctx);
        if (n < 0) return -1;

        n = ndt_snprintf(ctx, buf, "%s\n", ndt_is_concrete(t) ? "," : "");
        if (n < 0) return -1;

        if (ndt_is_concrete(t)) {
            n = ndt_snprintf_d(ctx, buf, d+2,
                               "offset=%zu, align=%" PRIu16 ", pad=%" PRIu16 "\n",
                               t->Concrete.Tuple.offset[i], t->Concrete.Tuple.align[i],
                               t->Concrete.Tuple.pad[i]);
            if (n < 0) return -1;
        }

        n = ndt_snprintf_d(ctx, buf, d, ")");
        if (n < 0) return -1;
    }

    return 0;
}

static int
ast_record_fields(buf_t *buf, const ndt_t *t, int d, ndt_context_t *ctx)
{
    int64_t i;
    int n;

    assert(t->tag == Record);

    for (i = 0; i < t->Record.shape; i++) {
        if (i >= 1) {
            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;
        }

        n = ndt_snprintf_d(ctx, buf, d, "RecordField(\n");
        if (n < 0) return -1;

        n = ndt_snprintf_d(ctx, buf, d+2, "name='%s',\n", t->Record.names[i]);
        if (n < 0) return -1;

        n = ndt_snprintf_d(ctx, buf, d+2, "type=");
        if (n < 0) return -1;

        n = ast_datashape(buf, t->Record.types[i], d+5+2, 1, ctx);
        if (n < 0) return -1;

        n = ndt_snprintf(ctx, buf, "%s\n", ndt_is_concrete(t) ? "," : "");
        if (n < 0) return -1;

        if (ndt_is_concrete(t)) {
            n = ndt_snprintf_d(ctx, buf, d+2,
                "offset=%zu, align=%" PRIu16 ", pad=%" PRIu16 "\n",
                t->Concrete.Record.offset[i], t->Concrete.Record.align[i],
                t->Concrete.Record.pad[i]);
            if (n < 0) return -1;
        }

        n = ndt_snprintf_d(ctx, buf, d, ")");
        if (n < 0) return -1;
    }

    return 0;
}

static int
ast_variadic_flag(buf_t *buf, enum ndt_variadic flag, int d, ndt_context_t *ctx)
{
    if (flag == Variadic) {
        return ndt_snprintf_d(ctx, buf, d, "variadic=true,\n");
    }

    return 0;
}

static int
ast_comma_ast_variadic_flag(buf_t *buf, enum ndt_variadic flag, int d, ndt_context_t *ctx)
{
    if (flag == Variadic) {
        int n = ndt_snprintf(ctx, buf, ",\n");
        if (n < 0) return -1;

        return ndt_snprintf_d(ctx, buf, d, "variadic=true");
    }

    return 0;
}

static int
ast_value(buf_t *buf, const ndt_value_t *mem, ndt_context_t *ctx)
{
    switch (mem->tag) {
    case ValBool:
        return ndt_snprintf(ctx, buf, mem->ValBool ? "true" : "false");
    case ValInt64:
        return ndt_snprintf(ctx, buf, "%" PRIi64, mem->ValInt64);
    case ValFloat64:
        return ndt_snprintf(ctx, buf, "%g", mem->ValFloat64);
    case ValString:
        return ndt_snprintf(ctx, buf, "'%s'", mem->ValString);
    case ValNA:
        return ndt_snprintf(ctx, buf, "NA");
    }

    /* NOT REACHED: tags should be exhaustive. */
    ndt_err_format(ctx, NDT_RuntimeError, "invalid value tag");
    return 0;
}

static int
ast_categorical(buf_t *buf, ndt_value_t *mem, size_t ntypes, ndt_context_t *ctx)
{
    size_t i;
    int n;

    for (i = 0; i < ntypes; i++) {
        if (i >= 1) {
            n = ndt_snprintf(ctx, buf, ", ");
            if (n < 0) return -1;
        }

        n = ast_value(buf, &mem[i], ctx);
        if (n < 0) return -1;
    }

    return 0;
}

static int
ast_datashape(buf_t *buf, const ndt_t *t, int d, int cont, ndt_context_t *ctx)
{
    int n;

    switch (t->tag) {
        case FixedDim:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "FixedDim(\n");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->FixedDim.type, d+2, 0, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "shape=%zu", t->FixedDim.shape);

            if (ndt_is_abstract(t)) {
                n = ndt_snprintf(ctx, buf, ",\n");
                if (n < 0) return -1;
            }
            else {
                n = ndt_snprintf(ctx, buf,
                    ", itemsize=%" PRIi64 ", step=%zu,\n",
                    t->Concrete.FixedDim.itemsize,
                    t->Concrete.FixedDim.step);
            }
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case SymbolicDim:
            assert(ndt_is_abstract(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "SymbolicDim(\n");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->SymbolicDim.type, d+2, 0, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "name='%s',\n", t->SymbolicDim.name);
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case VarDim: {
            int i;

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "VarDim(\n");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->VarDim.type, d+2, 0, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            if (ndt_is_concrete(t)) {
                n = ndt_snprintf_d(ctx, buf, d+2, "offsets=[");
                if (n < 0) return -1;

                for (i = 0; i < t->Concrete.VarDim.noffsets; i++) {
                    n = ndt_snprintf(ctx, buf, "%" PRIi32 "%s",
                                     t->Concrete.VarDim.offsets[i],
                                     i==t->Concrete.VarDim.noffsets-1 ? "" : ", ");
                    if (n < 0) return -1;
                }

                n = ndt_snprintf(ctx, buf, "],\n");
                if (n < 0) return -1;

                n = ndt_snprintf_d(ctx, buf, d+2, "slices=[");
                if (n < 0) return -1;

                for (i = 0; i < t->Concrete.VarDim.nslices; i++) {
                    n = ndt_snprintf(ctx, buf, "%" PRIi64 ":%" PRIi64 ":%" PRIi64 "%s",
                                     t->Concrete.VarDim.slices[i].start,
                                     t->Concrete.VarDim.slices[i].stop,
                                     t->Concrete.VarDim.slices[i].step,
                                     i==t->Concrete.VarDim.nslices-1 ? "" : ", ");
                    if (n < 0) return -1;
                }

                n = ndt_snprintf(ctx, buf, "],\n");
                if (n < 0) return -1;

                n = ndt_snprintf_d(ctx, buf, d+2,
                    "itemsize=%" PRIi64 ",\n", t->Concrete.VarDim.itemsize);
                if (n < 0) return -1;
            }

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");
        }

        case EllipsisDim:
            assert(ndt_is_abstract(t));
            assert(!ndt_is_optional(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "EllipsisDim(\n");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->EllipsisDim.type, d+2, 0, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, "\n");
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Nominal:
            assert(ndt_is_concrete(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Nominal(\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "name='%s',\n", t->Nominal.name);
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Module:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Module(\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "name='%s',\n", t->Module.name);
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "type=");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Module.type, d+5+2, 1, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, "\n");
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d, ")");
            return n;

        case Constr:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Constr(\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "name='%s',\n", t->Constr.name);
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "type=");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Constr.type, d+5+2, 1, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, "\n");
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d, ")");
            return n;

        case Tuple:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Tuple(\n");
            if (n < 0) return -1;

            if (t->Tuple.shape > 0) {
                n = ast_tuple_fields(buf, t, d+2, ctx);
                if (n < 0) return -1;

                n = ast_comma_ast_variadic_flag(buf, t->Tuple.flag, d+2, ctx);
                if (n < 0) return -1;

                n = ndt_snprintf(ctx, buf, ",\n");
                if (n < 0) return -1;
            }
            else {
                n = ast_variadic_flag(buf, t->Tuple.flag, d+2, ctx);
                if (n < 0) return -1;
            }

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Record:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Record(\n");
            if (n < 0) return -1;

            if (t->Record.shape > 0) {
                n = ast_record_fields(buf, t, d+2, ctx);
                if (n < 0) return -1;

                n = ast_comma_ast_variadic_flag(buf, t->Record.flag, d+2, ctx);
                if (n < 0) return -1;

                n = ndt_snprintf(ctx, buf, ",\n");
                if (n < 0) return -1;
            }
            else {
                n = ast_variadic_flag(buf, t->Record.flag, d+2, ctx);
                if (n < 0) return -1;

            }

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Function:
            assert(ndt_is_abstract(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Function(\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "pos=");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Function.pos, d+4+2, 1, ctx);

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "kwds=");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Function.kwds, d+5+2, 1, ctx);

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d+2, "ret=");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Function.ret, d+4+2, 1, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Typevar:
            assert(ndt_is_abstract(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Typevar(");
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, "name='%s', ", t->Typevar.name);
            if (n < 0) return -1;

            n = ast_common_attributes(buf, t, 0, ctx);
            if (n < 0) return -1;

            return ndt_snprintf(ctx, buf, ")");

        case AnyKind: case ScalarKind:
        case Void: case Bool:
        case SignedKind:
        case Int8: case Int16: case Int32: case Int64:
        case UnsignedKind:
        case Uint8: case Uint16: case Uint32: case Uint64:
        case FloatKind:
        case Float16: case Float32: case Float64:
        case ComplexKind:
        case Complex32: case Complex64: case Complex128:
        case FixedStringKind: case FixedBytesKind:
        case String: case FixedString: case FixedBytes:
            assert(ndt_is_concrete(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "%s(", ndt_type_name(t));
            if (n < 0) return -1;

            n = ast_common_attributes(buf, t, 0, ctx);
            if (n < 0) return -1;

            return ndt_snprintf(ctx, buf, ")");

        case Char:
            assert(ndt_is_concrete(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Char(%s,\n",
                               ndt_encoding_as_string(t->Char.encoding));

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Bytes:
            assert(ndt_is_concrete(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d,
                    "Bytes(target_align=%" PRIu16 ",\n", t->Bytes.target_align);
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            return ndt_snprintf_d(ctx, buf, d, ")");

        case Categorical:
            assert(ndt_is_concrete(t));

            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Categorical(");
            if (n < 0) return -1;

            n = ast_categorical(buf, t->Categorical.types, t->Categorical.ntypes, ctx);
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ")");
            return n;

        case Ref:
            n = ndt_snprintf_d(ctx, buf, cont ? 0 : d, "Ref(\n");
            if (n < 0) return -1;

            n = ast_datashape(buf, t->Ref.type, d+2, 0, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf(ctx, buf, ",\n");
            if (n < 0) return -1;

            n = ast_common_attributes_with_newline(buf, t, d+2, ctx);
            if (n < 0) return -1;

            n = ndt_snprintf_d(ctx, buf, d, ")");
            return n;

        default:
            ndt_err_format(ctx, NDT_ValueError, "invalid tag");
            return -1;
    }
}


/******************************************************************************/
/*                  API: string to primitive type conversions                 */
/******************************************************************************/

bool
ndt_strtobool(const char *v, ndt_context_t *ctx)
{
    if (strcmp(v, "true") == 0) {
        return 1;
    }
    else if (strcmp(v, "false") == 0) {
        return 0;
    }
    else {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "valid values for bool are 'true' or 'false'");
        return 0;
    }
}

char
ndt_strtochar(const char *v, ndt_context_t *ctx)
{
    if (strlen(v) == 1) {
        return v[0];
    }
    else {
        ndt_err_format(ctx, NDT_InvalidArgumentError, "invalid char");
        return 0;
    }
}

long
ndt_strtol(const char *v, long min, long max, ndt_context_t *ctx)
{
    char *endptr;
    long ld;

    errno = 0;
    ld = strtol(v, &endptr, 10);

    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid integer: '%s'", v);
    }
    if (errno == ERANGE || ld < min || ld > max) {
        ndt_err_format(ctx, NDT_ValueError,
                       "out of range: '%s'", v);
    }

    return ld;
}

long long
ndt_strtoll(const char *v, long long min, long long max, ndt_context_t *ctx)
{
    char *endptr;
    long long lld;

    errno = 0;
    lld = strtoll(v, &endptr, 10);
    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid integer: '%s'", v);
    }
    if (errno == ERANGE || lld < min || lld > max) {
        ndt_err_format(ctx, NDT_ValueError,
                       "out of range: '%s'", v);
    }

    return lld;
}

unsigned long
ndt_strtoul(const char *v, unsigned long max, ndt_context_t *ctx)
{
    char *endptr;
    unsigned long lu;

    errno = 0;
    lu = strtoul(v, &endptr, 10);
    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid integer: '%s'", v);
    }
    if (errno == ERANGE || lu > max) {
        ndt_err_format(ctx, NDT_ValueError,
                       "out of range: '%s'", v);
    }

    return lu;
}

unsigned long long
ndt_strtoull(const char *v, unsigned long long max, ndt_context_t *ctx)
{
    char *endptr;
    unsigned long long llu;

    errno = 0;
    llu = strtoull(v, &endptr, 10);
    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid integer: '%s'", v);
    }
    if (errno == ERANGE || llu > max) {
        ndt_err_format(ctx, NDT_ValueError,
                       "out of range: '%s'", v);
    }

    return llu;
}

/* Read a float.  Overflow or underflow is an error. */
float
ndt_strtof(const char *v, ndt_context_t *ctx)
{
    char *endptr;
    float f;

    errno = 0;
    f = strtof(v, &endptr);
    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid float: '%s'", v);
    }
    if (errno == ERANGE) {
        ndt_err_format(ctx, NDT_ValueError,
                       "%s: '%s'", f == 0 ? "underflow" : "overflow", v);
    }

    return f;
}

/* Read a double.  Overflow or underflow is an error. */
double
ndt_strtod(const char *v, ndt_context_t *ctx)
{
    char *endptr;
    double d;

    errno = 0;
    d = strtod(v, &endptr);
    if (*v == '\0' || *endptr != '\0') {
        ndt_err_format(ctx, NDT_InvalidArgumentError,
                       "invalid double: '%s'", v);
    }
    if (errno == ERANGE) {
        ndt_err_format(ctx, NDT_ValueError,
                       "%s: '%s'", d == 0 ? "underflow" : "overflow", v);
    }

    return d;
}


/******************************************************************************/
/*                       API: type to string conversions                      */
/******************************************************************************/

char *
ndt_as_string(ndt_t *t, ndt_context_t *ctx)
{
    buf_t buf = {0, 0, NULL};
    char *s;
    size_t count;

    if (datashape(&buf, t, INT_MIN, ctx) < 0) {
        return NULL;
    }

    count = buf.count;
    buf.count = 0;
    buf.size = count+1;

    buf.cur = s = ndt_alloc(1, count+1);
    if (buf.cur == NULL) {
        return ndt_memory_error(ctx);
    }

    if (datashape(&buf, t, INT_MIN, ctx) < 0) {
        ndt_free(s);
        return NULL;
    }
    s[count] = '\0';

    return s;
}

char *
ndt_indent(ndt_t *t, ndt_context_t *ctx)
{
    buf_t buf = {0, 0, NULL};
    char *s;
    size_t count;

    if (datashape(&buf, t, 0, ctx) < 0) {
        return NULL;
    }

    count = buf.count;
    buf.count = 0;
    buf.size = count+1;

    buf.cur = s = ndt_alloc(1, count+1);
    if (buf.cur == NULL) {
        return ndt_memory_error(ctx);
    }

    if (datashape(&buf, t, 0, ctx) < 0) {
        ndt_free(s);
        return NULL;
    }
    s[count] = '\0';

    return s;
}

char *
ndt_as_string_with_meta(ndt_t *t, ndt_context_t *ctx)
{
    buf_t buf = {0, 0, NULL};
    char *s;
    size_t count;

    if (ast_datashape(&buf, t, 0, 0, ctx) < 0) {
        return NULL;
    }

    count = buf.count;
    buf.count = 0;
    buf.size = count+1;

    buf.cur = s = ndt_alloc(1, count+1);
    if (buf.cur == NULL) {
        return ndt_memory_error(ctx);
    }

    if (ast_datashape(&buf, t, 0, 0, ctx) < 0) {
        ndt_free(s);
        return NULL;
    }
    s[count] = '\0';

    return s;
}
