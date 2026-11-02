from pycparser import parse_file, c_ast, CParser
import json

import cffi
import os

ast = parse_file( "far2lcffi.h", use_cpp=True, cpp_path="cpp", cpp_args=["-I.", "-D__attribute__(x)=", "-D__declspec(x)=", ] )

ir = {
    "structs": {},
    "enums": {},
    "functions": {}
}

# typedef alias map: {"ULONG": "uint32_t", ...}
typedefs = {}

def resolve_type(t):
    """Resolve typedef chains: ULONG → uint32_t → unsigned int"""
    while t in typedefs:
        t = typedefs[t]
    return t

def parse_struct(node):
    fields = []
    for decl in node.decls or []:
        # decl.type.type.names gives the underlying type tokens
        typename = " ".join(decl.type.type.names)
        typename = resolve_type(typename)
        fields.append({
            "name": decl.name,
            "type": typename
        })
    return fields

def parse_enum(node):
    values = {}
    for enumerator in node.values.enumerators:
        values[enumerator.name] = int(enumerator.value.value)
    return values

def parse_funcptr(node):
    # typedef RETURN (*NAME)(args...)
    name = node.type.type.declname
    ret = " ".join(node.type.type.type.type.names)
    ret = resolve_type(ret)

    args = []
    for param in node.type.args.params:
        if hasattr(param.type, "type") and hasattr(param.type.type, "names"):
            t = " ".join(param.type.type.names)
        else:
            t = "void*"
        t = resolve_type(t)
        args.append({
            "name": param.name,
            "type": t
        })

    return name, ret, args

# --- AST traversal ---
for ext in ast.ext:

    # typedef alias: typedef uint32_t ULONG;
    if isinstance(ext, c_ast.Typedef):
        t = ext.type

        # simple alias typedef
        if isinstance(t, c_ast.TypeDecl) and hasattr(t.type, "names"):
            base = " ".join(t.type.names)
            typedefs[ext.name] = base
            continue

        # struct typedef
        if isinstance(t.type, c_ast.Struct):
            struct_name = ext.name
            ir["structs"][struct_name] = {
                "fields": parse_struct(t.type)
            }
            continue

        # function pointer typedef
        if isinstance(t.type, c_ast.PtrDecl) and isinstance(t.type.type, c_ast.FuncDecl):
            name, ret, args = parse_funcptr(ext)
            ir["functions"][name] = {
                "return": ret,
                "args": args
            }
            continue

    # enum
    if isinstance(ext, c_ast.Enum):
        ir["enums"][ext.name] = parse_enum(ext)

# --- Normalize struct field types after typedefs collected ---
for sname, s in ir["structs"].items():
    for f in s["fields"]:
        f["type"] = resolve_type(f["type"])

# --- Normalize function pointer types ---
for fname, f in ir["functions"].items():
    f["return"] = resolve_type(f["return"])
    for a in f["args"]:
        a["type"] = resolve_type(a["type"])

print(json.dumps(ir, indent=2))
