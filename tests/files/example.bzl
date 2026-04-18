load("//tools:defs.bzl", "demo_alias", "demo_helper")
load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")
load("@rules_java//java:defs.bzl", "java_library")

DEMO_TAGS = ["demo", "syntax", "starlark"]
DEMO_FLAGS = struct(enabled = True, name = "starlark", retries = 3)

DemoInfo = provider(fields = ["name", "srcs", "deps", "tags"])

def _join_labels(items):
    result = []
    for item in items:
        result.append(str(item))
    return result

def _impl(ctx):
    out = ctx.actions.declare_file(ctx.label.name + ".txt")
    text = []
    text.append("name: " + ctx.label.name)
    text.append("srcs: " + str(len(ctx.files.srcs)))
    text.append("deps: " + str(len(ctx.attr.deps)))
    text.append("tags: " + ",".join(ctx.attr.tags))
    ctx.actions.write(
        output = out,
        content = "\n".join(text),
    )
    if not ctx.attr.enabled:
        fail("demo rule disabled")
    return [
        DefaultInfo(files = depset([out])),
        DemoInfo(
            name = ctx.label.name,
            srcs = ctx.files.srcs,
            deps = ctx.attr.deps,
            tags = ctx.attr.tags,
        ),
    ]

demo_rule = rule(
    implementation = _impl,
    attrs = {
        "srcs": attr.label_list(allow_files = True),
        "deps": attr.label_list(),
        "tags": attr.string_list(),
        "enabled": attr.bool(default = True),
    },
)

def demo_macro(name, srcs = [], deps = [], tags = [], enabled = True):
    merged_tags = DEMO_TAGS + tags
    demo_rule(
        name = name,
        srcs = srcs,
        deps = deps,
        tags = merged_tags,
        enabled = enabled,
    )

def demo_binary_macro(name, srcs = [], deps = [], main = None):
    cc_binary(
        name = name,
        srcs = srcs,
        deps = deps,
        main = main,
    )

def demo_java_library(name, srcs = [], deps = []):
    java_library(
        name = name,
        srcs = srcs,
        deps = deps,
        visibility = ["//visibility:public"],
    )

def demo_select_flag(enabled):
    return select({
        "//conditions:default": "disabled",
        ":enabled": "enabled",
    })

def demo_manifest(srcs):
    info = {
        "srcs": _join_labels(srcs),
        "count": len(srcs),
    }
    return struct(
        srcs = srcs,
        info = info,
    )

def demo_feature_matrix():
    values = {
        "primary": "starlark",
        "secondary": "bazel",
    }
    values.update({"version": "1.2.2"})
    return values

def demo_rule_list():
    items = [
        "//app:lib",
        "//app:test",
        "//tools:helper",
    ]
    return items

def demo_string_table():
    table = {
        "alpha": "one",
        "beta": "two",
        "gamma": "three",
    }
    return table

def demo_guard(name):
    if name == "":
        fail("name must not be empty")
    return name

def demo_print_summary(name, srcs, deps):
    print("target: " + demo_guard(name))
    print("srcs: " + str(len(srcs)))
    print("deps: " + str(len(deps)))
    print("flags: " + str(DEMO_FLAGS))

def demo_bundle(name, srcs = [], deps = []):
    demo_print_summary(name, srcs, deps)
    demo_macro(
        name = name,
        srcs = srcs,
        deps = deps,
        tags = ["bundle"],
    )

