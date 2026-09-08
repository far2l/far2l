import json

ir = json.load(open("ir.json"))

# --- Generate struct → hashref converters ---
def gen_struct(struct_name, struct):
    out = []
    out.append(f"SV* to_perl_{struct_name}({struct_name}* s) {{")
    out.append("    HV* hv = newHV();")
    for f in struct["fields"]:
        out.append(f'    hv_stores(hv, "{f["name"]}", newSViv(s->{f["name"]}));')
    out.append("    return newRV_noinc((SV*)hv);")
    out.append("}")
    return "\n".join(out)

# --- Generate enum constants ---
def gen_enum(enum_name, values):
    out = []
    out.append(f"void register_enum_{enum_name}() {{")
    out.append(f'    HV* stash = gv_stashpv("Editor::{enum_name}", TRUE);')
    for k, v in values.items():
        out.append(f'    newCONSTSUB(stash, "{k}", newSViv({v}));')
    out.append("}")
    return "\n".join(out)

# --- Generate function pointer wrapper ---
def gen_funcptr(name, fn):
    out = []
    out.append(f"XS(Editor_{name}) {{")
    out.append("    dXSARGS;")
    out.append("    if (items != %d) XSRETURN_UNDEF;" % len(fn["args"]))
    for i, arg in enumerate(fn["args"]):
        out.append(f'    {arg["type"]} a{i} = ({arg["type"]})SvIV(ST({i}));')
    args = ", ".join([f"a{i}" for i in range(len(fn["args"]))])
    out.append(f"    {fn['return']} r = g_{name}({args});")
    out.append("    ST(0) = newSViv(r);")
    out.append("    XSRETURN(1);")
    out.append("}")
    return "\n".join(out)

# --- Emit everything ---
print("/* Auto-generated XS glue */\n")

for sname, s in ir["structs"].items():
    print(gen_struct(sname, s), "\n")

for ename, e in ir["enums"].items():
    print(gen_enum(ename, e), "\n")

for fname, f in ir["functions"].items():
    print(gen_funcptr(fname, f), "\n")
