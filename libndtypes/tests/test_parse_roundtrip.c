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
#include "test.h"


/* Same as test_parse, but can only contain the canonical datashape forms. */
const char *parse_roundtrip_tests[] = {
  "2395344366 * Any",
  "10 * 2395344366 * Any",
  "10 * 10 * 2395344366 * Any",
  "L * Any",
  "10 * L * Any",
  "10 * 10 * L * Any",
  "var * Any",
  "... * Any",
  "10 * ... * Any",
  "10 * 10 * ... * Any",
  "10 * Any",
  "Any",
  "10 * ScalarKind",
  "ScalarKind",
  "10 * bool",
  "bool",
  "10 * SignedKind",
  "SignedKind",
  "10 * int8",
  "int8",
  "10 * int16",
  "int16",
  "10 * int32",
  "int32",
  "10 * int64",
  "int64",
  "10 * UnsignedKind",
  "UnsignedKind",
  "10 * uint8",
  "uint8",
  "10 * uint16",
  "uint16",
  "10 * uint32",
  "uint32",
  "10 * uint64",
  "uint64",
  "10 * FloatKind",
  "FloatKind",
  "10 * float32",
  "float32",
  "10 * float64",
  "float64",
  "10 * ComplexKind",
  "ComplexKind",
  "10 * complex64",
  "complex64",
  "10 * complex128",
  "complex128",
  "10 * char('ascii')",
  "char('ascii')",
  "10 * char('utf8')",
  "char('utf8')",
  "10 * char('utf16')",
  "char('utf16')",
  "10 * char('utf32')",
  "char('utf32')",
  "10 * char('ucs2')",
  "char('ucs2')",
  "10 * string",
  "string",
  "10 * FixedStringKind",
  "FixedStringKind",
  "10 * fixed_string(729742655, 'ascii')",
  "fixed_string(729742655, 'ascii')",
  "10 * fixed_string(3952068488)",
  "fixed_string(3952068488)",
  "10 * fixed_string(2949183030, 'utf16')",
  "fixed_string(2949183030, 'utf16')",
  "10 * fixed_string(375133997, 'utf32')",
  "fixed_string(375133997, 'utf32')",
  "10 * fixed_string(969064578, 'ucs2')",
  "fixed_string(969064578, 'ucs2')",
  "10 * bytes(align=16)",
  "bytes(align=16)",
  "10 * FixedBytesKind",
  "FixedBytesKind",
  "10 * fixed_bytes(size=1904128700, align=4)",
  "fixed_bytes(size=1904128700, align=4)",
  "10 * categorical(63)",
  "categorical(63)",
  "10 * categorical(10, 63)",
  "categorical(10, 63)",
  "10 * categorical(8, 10)",
  "categorical(8, 10)",
  "10 * categorical(18514)",
  "categorical(18514)",
  "10 * categorical(10, 18514)",
  "categorical(10, 18514)",
  "10 * categorical(-176354404)",
  "categorical(-176354404)",
  "10 * categorical(-176354404, 10)",
  "categorical(-176354404, 10)",
  "10 * categorical(500601201)",
  "categorical(500601201)",
  "10 * categorical(10, 500601201)",
  "categorical(10, 500601201)",
  "10 * categorical(135)",
  "categorical(135)",
  "10 * categorical(10, 135)",
  "categorical(10, 135)",
  "10 * categorical(22234)",
  "categorical(22234)",
  "10 * categorical(10, 22234)",
  "categorical(10, 22234)",
  "10 * categorical(3761672407)",
  "categorical(3761672407)",
  "10 * categorical(10, 3761672407)",
  "categorical(10, 3761672407)",
  "10 * categorical(2339269771)",
  "categorical(2339269771)",
  "10 * categorical(10, 2339269771)",
  "categorical(10, 2339269771)",
  "10 * categorical(1.2)",
  "categorical(-1.2e+33)",
  "10 * categorical(10, 1.05e+10)",
  "categorical(10, 1.05e+10)",
  "10 * categorical(1.05e+300)",
  "categorical(-1.05e+300)",
  "10 * categorical(10, 1.05e+300)",
  "categorical(10, 1.05e+300)",
  "10 * categorical('jRAMoBPQ')",
  "categorical('jRAMoBPQ')",
  "10 * categorical(10, 'jRAMoBPQ')",
  "categorical(10, 'jRAMoBPQ')",
  "10 * categorical(127)",
  "categorical(127)",
  "10 * categorical(10, 127)",
  "categorical(10, 127)",
  "10 * categorical(-128)",
  "categorical(-128)",
  "10 * categorical(10, -128)",
  "categorical(10, -128)",
  "10 * categorical(32767)",
  "categorical(32767)",
  "10 * categorical(10, 32767)",
  "categorical(10, 32767)",
  "10 * categorical(-32768)",
  "categorical(-32768)",
  "10 * categorical(10, -32768)",
  "categorical(10, -32768)",
  "10 * categorical(2147483647)",
  "categorical(2147483647)",
  "10 * categorical(10, 2147483647)",
  "categorical(10, 2147483647)",
  "10 * categorical(-2147483648)",
  "categorical(-2147483648)",
  "10 * categorical(10, -2147483648)",
  "categorical(10, -2147483648)",
  "10 * categorical(9223372036854775807)",
  "categorical(9223372036854775807)",
  "10 * categorical(10, 9223372036854775807)",
  "categorical(10, 9223372036854775807)",
  "10 * categorical(-9223372036854775808)",
  "categorical(-9223372036854775808)",
  "10 * categorical(-9223372036854775808, 10)",
  "categorical(-9223372036854775808, 10)",
  "10 * categorical(255)",
  "categorical(255)",
  "10 * categorical(10, 255)",
  "categorical(10, 255)",
  "10 * categorical(65535)",
  "categorical(65535)",
  "10 * categorical(10, 65535)",
  "categorical(10, 65535)",
  "10 * categorical(4294967295)",
  "categorical(4294967295)",
  "10 * categorical(10, 4294967295)",
  "categorical(10, 4294967295)",
  "10 * categorical(1.1755e-38)",
  "categorical(1.1755e-38)",
  "10 * categorical(10, 1.1755e-38)",
  "categorical(10, 1.1755e-38)",
  "10 * categorical(2.22508e-308)",
  "categorical(2.22508e-308)",
  "10 * categorical(10, 2.22508e-308)",
  "categorical(10, 2.22508e-308)",
  "10 * categorical(3.40282e+38)",
  "categorical(3.40282e+38)",
  "10 * categorical(10, 3.40282e+38)",
  "categorical(10, 3.40282e+38)",
  "10 * categorical(1.79769e+308)",
  "categorical(1.79769e+308)",
  "10 * categorical(10, 1.79769e+308)",
  "categorical(10, 1.79769e+308)",
  "10 * categorical('')",
  "categorical('')",
  "10 * categorical(10, '')",
  "categorical(10, '')",
  "10 * categorical('xyz')",
  "categorical('xyz')",
  "10 * categorical(10, 'xyz')",
  "categorical(10, 'xyz')",
  "10 * ref(Any)",
  "ref(Any)",
  "10 * (...)",
  "10 * (Any)",
  "(Any)",
  "10 * (int64, Any)",
  "(int64, Any)",
  "10 * (int64, int64, Any)",
  "(int64, int64, Any)",
  "(Any) -> complex64",
  "10 * (Any, ...)",
  "(Any, ...)",
  "10 * {...}",
  "{...}",
  "10 * {a : Any}",
  "{a : Any}",
  "10 * {x : int64, a : Any}",
  "{x : int64, a : Any}",
  "10 * {x : int64, x : int64, a : Any}",
  "{x : int64, x : int64, a : Any}",
  "10 * {a : int64}",
  "{a : int64}",
  "10 * {a : int64, ...}",
  "{a : int64, ...}",
  "10 * (...) -> Any",
  "(...) -> Any",
  "10 * () -> (...)",
  "() -> (...)",
  "10 * (Any) -> Any",
  "(Any) -> Any",
  "10 * () -> (Any)",
  "() -> (Any)",
  "10 * (int64, Any) -> Any",
  "(int64, Any) -> Any",
  "10 * () -> (int64, Any)",
  "() -> (int64, Any)",
  "10 * (int64, int64, Any) -> Any",
  "(int64, int64, Any) -> Any",
  "10 * () -> (int64, int64, Any)",
  "() -> (int64, int64, Any)",
  "10 * (Any, ...) -> Any",
  "(Any, ...) -> Any",
  "10 * () -> (Any, ...)",
  "() -> (Any, ...)",
  "10 * (a : Any, ...) -> Any",
  "(a : Any) -> int64",
  "(a : Any, ...) -> Any",
  "10 * (x : int64, a : Any, ...) -> Any",
  "(x : int64, a : Any, ...) -> Any",
  "10 * (x : int64, x : int64, a : Any, ...) -> Any",
  "(x : int64, x : int64, a : Any, ...) -> Any",
  "10 * (Any, a : Any) -> Any",
  "(Any, a : Any) -> Any",
  "(Any, ..., a : Any) -> Any",
  "10 * (int64, Any, a : Any) -> Any",
  "(int64, Any, a : Any) -> Any",
  "10 * (int64, Any, ..., a : Any) -> Any",
  "(int64, Any, ..., a : Any) -> Any",
  "10 * (int64, int64, Any, a : Any) -> Any",
  "(int64, int64, Any, a : Any) -> Any",
  "10 * (int64, int64, Any, ..., a : Any) -> Any",
  "(int64, int64, Any, ..., a : Any) -> Any",
  "10 * (Any, x : int64, a : Any) -> Any",
  "(Any, x : int64, a : Any) -> Any",
  "10 * (Any, x : int64, x : int64, a : Any) -> Any",
  "(Any, x : int64, x : int64, a : Any) -> Any",
  "10 * (Any, a : Any, ...) -> Any",
  "(Any, a : Any, ...) -> Any",
  "10 * (Any, ..., a : Any, ...) -> Any",
  "(Any, ..., a : Any, ...) -> Any",
  "10 * Up",
  "Up",
  "10 * X(Any)",
  "X(Any)",
  /* BEGIN RANDOM */
  "fixed_bytes(size=1342281600, align=16)",
  "var * var * var * (...)",
  "categorical(-84, 18772, 49062, 1619552300, 'a', 'l')",
  "categorical(24, 44038, 1763886348, 2840608325, 1.2e-32, 'jOt')",
  "categorical(-2049848438, 94, 3577056037, 3599739756, 1.2e-32)",
  "categorical(2753441703, 1.2e-32)",
  "var * var * ... * string",
  "1286044356 * LbR6JPFI * 2096958782 * uint8",
  "categorical(-25910, 4265882500, 1.2e-32, 'xM3Mys0XqH')",
  "var * var * FixedBytesKind",
  "var * ... * Nf",
  "var * var * var * fixed_bytes(size=2816010912, align=16)",
  "3663546803 * 1650974226 * fixed_bytes(size=912328236, align=2)",
  "var * var * var * ZcTmBXnKPi",
  "categorical(946986991, 43, 'omhwkoWVWw')",
  "var * float32",
  "var * var * var * uint16",
  "var * ... * int16",
  "var * var * var * ScalarKind",
  "fixed_bytes(size=280180300, align=2)",
  "fixed_string(1901614748, 'utf32')",
  "var * M7",
  "var * bytes(align=2)",
  "var * ... * E",
  "2948091211 * ... * categorical(-1869840431, 2052908559)",
  "fixed_bytes(size=1882259712, align=16)",
  "categorical(-83, -58, 1.2e-32)",
  "3739637665 * 1476113530 * 1701021063 * IKd * 450514382 * WXn",
  "ref(AilcKv4su1(FixedBytesKind))",
  "var * ... * complex128",
  "var * ... * R8KFFEabJ",
  "var * QoFb",
  "var * ... * fixed_string(1464831555, 'ucs2')",
  "var * fixed_string(1233383142, 'utf32')",
  "fixed_bytes(size=2882797900, align=2)",
  "fixed_bytes(size=1233209904, align=8)",
  "WhRsMHHXYp(categorical(145))",
  "var * var * bytes(align=2)",
  "categorical(-85, -49, -26791, 8094, 1780, 1.2e-32, 'pX')",
  "D6nfBg_(categorical(16278, 26651, 147170982, 1.2e-32, 'ylR7'))",
  "categorical(482331467, 3229733591, -1.2e-32, 1.2e-32)",
  /* END RANDOM */

  /* BEGIN MANUALLY GENERATED */
  "()",
  "{}",
  "() -> ()",
  "(...) -> ()",
  "(...) -> (...)",
  "(..., a : int64, ...) -> complex128",
  "() -> int32",
  "(int32) -> int32",
  "(int32, float64) -> int32",
  "(scale : float64) -> int32",
  "(scale : float64, color : float64) -> int32",
  "(int32, scale : float64) -> int32",
  "(int32, scale : float64, color : float64) -> int32",
  "(int32, float32, scale : float64, color : float64) -> int32",
  "(...) -> int32",
  "(int32, ...) -> int32",
  "(int32, float32, ...) -> int32",
  "(scale : float64, ...) -> int32",
  "(scale : float64, color : float64, ...) -> int32",
  "(..., scale : float64) -> int32",
  "(int32, ..., scale : float64) -> int32",
  "(int32, float32, ..., scale : float64) -> int32",
  "(int32, scale : float64, ...) -> int32",
  "(int32, float32, scale : float64, color : float64, ...) -> int32",
  "(..., scale : float64, ...) -> int32",
  "(int32, ..., scale : float64, color : float64, ...) -> int32",
  "(int32, float32, ..., scale : float64, color : float64, ...) -> int32",

  "10 * defined_t",
  "defined_t",
  "(int32, float32, ..., scale : defined_t, color : float64, ...) -> defined_t",
  "(a : ref({b : defined_t, c : (defined_t, int16) -> float64})) -> defined_t",

  "10 * foo_t",
  "foo_t",
  "(int32, float32, ..., scale : defined_t, color : float64, ...) -> foo_t",
  "(a : ref({b : defined_t, c : (foo_t, int16) -> float64})) -> foo_t",

  "() -> void",
  "(complex128, a : float64) -> void",
  "(int32, ...) -> void",

  "<int64",
  ">int64",
  "2 * 10 * >float64",

  /* END MANUALLY GENERATED */

  NULL
};
