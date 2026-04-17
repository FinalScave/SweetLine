# SweetLine Common Failures

Review this file before closing any syntax task.

## Routing failures

- The syntax only works by the last suffix, but the real target needs exact file names.
- A multi-part file name falls back to a shorter suffix and routes to the wrong syntax.
- A file-name pattern was authored as a substring or path-aware match even though SweetLine matches the basename only and anchors patterns to the whole string.
- Equal-length suffixes or overlapping file-name patterns collide, and the analyzer is not created.
- Duplicate exact file names or duplicate syntax names were introduced and silently routed to whichever entry happened to win iteration.
- A shipped demo still exposes an outdated syntax set, built-in list, or manifest and never precompiles the syntax needed for exact-name or pattern routing.
- A dialect split was added, but ambiguous suffixes or other route-colliding variants still ship together.

## Tokenization failures

- A broad fallback rule consumes tokens that should be distinct.
- Token order causes an earlier generic rule to swallow a more specific token.
- The syntax only highlights declarations and leaves DSL heads, directives, tags, or operands unclassified.
- A helper capture that preserved downstream group numbering was rewritten to non-capturing and shifted the `styles` or `subStates` mapping.
- Capturing groups were added, removed, or reordered without revalidating the token-local capture indices consumed by `styles` or `subStates`.
- `style`, `styles`, and `subStates` do not match the real intent of the rule or the actual capture-group indices.
- Strings or comments contain embedded inline token classes, but the syntax forgot to reuse the needed fragment inside those states.
- Meaningful tokens fall back to default style because a group, boundary, ordering rule, or fallback is misconfigured.
- Inline-style rules reference a style name that was never declared in top-level `styles[]`.

## State and fragment failures

- Repeated string or comment logic was copied instead of extracted into fragments.
- A new state exists but nothing transitions into it.
- A multiline construct enters a state but never exits cleanly.
- `include` or `includes` was mixed with sibling token fields instead of being written as a standalone directive object.
- A fragment include chain depends on missing fragments or forms a cycle.
- An imported syntax is referenced before the base syntax is compiled.
- `importSyntax` depends on `#ifdef`, but the required macro was never defined during compilation.
- `onLineEndState` or `importSyntax` was written as if it could coexist with a normal token rule and still apply both behaviors.

## Example failures

- The example reaches the line target through repetition instead of branch coverage.
- The example demonstrates only the easy path and skips edge constructs.
- The example omits representative embedded inline tokens even though the syntax highlights them.
- The example covers top-level blocks but not nested or multiline forms.
- The example covers host syntax but not embedded syntax.

## Test failures

- The syntax compiles but analyzer selection is never tested.
- Highlight tests only check plain keywords or strings.
- Assertions verify existence of spans but not the exact style class that matters.
- Exact-style assertions exist, but the checked tokens are still mapped to the wrong shared style classes.
- A new file-routing rule was added without a matching selection test.

## Delivery failures

- Demo asset exposure was skipped even though the syntax is intended to appear in shipped examples or demos.
- Style names used by the syntax are not registered in the test or demo environment.
- Route-colliding syntax variants were bundled together in a shipped common-syntax set.
- The syntax works in isolation but the repository still ships outdated examples, built-in demo lists, or generated manifests.
- A syntax-only task silently expanded into engine, routing, or API changes before proving that the current SweetLine syntax model could not express the requirement.
