# SweetLine Common Failures

Review this file before closing any syntax task.

## Routing failures

- The syntax only works by the last suffix, but the real target needs exact file names.
- A multi-part file name falls back to a shorter suffix and routes to the wrong syntax.
- Demo-side syntax maps still use suffix-only logic and ignore exact-name or pattern routing.
- A dialect split was added, but ambiguous suffixes still collide.

## Tokenization failures

- A broad fallback rule consumes tokens that should be distinct.
- Token order causes an earlier generic rule to swallow a more specific token.
- The syntax only highlights declarations and leaves DSL heads, directives, tags, or operands unclassified.
- Structural regex groups are left as capturing groups instead of being converted to non-capturing groups.
- Capturing groups are introduced without a consumer in `styles` or `subStates`.
- `style`, `styles`, and `subStates` do not match the real intent of the rule or the actual capture-group indices.
- Strings or comments contain embedded inline token classes, but the syntax forgot to reuse the needed fragment inside those states.
- Meaningful tokens fall back to default style because a group, boundary, ordering rule, or fallback is misconfigured.

## State and fragment failures

- Repeated string or comment logic was copied instead of extracted into fragments.
- A new state exists but nothing transitions into it.
- A multiline construct enters a state but never exits cleanly.
- An imported syntax is referenced before the base syntax is compiled.

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

- Demo registration was skipped even though the syntax is intended to appear in examples or demos.
- Style names used by the syntax are not registered in the test or demo environment.
- The syntax works in isolation but the repository still ships outdated examples or routing tables.
- A syntax-only task silently expanded into engine, routing, or API changes before proving that the current SweetLine syntax model could not express the requirement.
