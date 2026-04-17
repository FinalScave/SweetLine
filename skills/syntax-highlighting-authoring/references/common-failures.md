# Common Failures

Review this file before closing any syntax-authoring task.

## Routing failures

- The syntax only works by extension, but the real target needs exact file names.
- A multi-part file name falls back to the last extension and routes to the wrong syntax.
- Demo-side syntax maps still use extension-only logic and ignore the new routing behavior.
- A dialect split was added, but ambiguous extensions still collide.

## Tokenization failures

- A broad fallback rule consumes tokens that should be distinct.
- Token order causes an earlier generic rule to swallow a more specific token.
- The syntax only highlights declarations and leaves DSL heads, directives, tags, or operands unclassified.
- Rules with capturing groups use `style` where `styles` is required, or group counts do not align with styles and sub-states.
- Strings or comments support URLs conceptually, but the syntax forgot to reuse the URL fragment inside those states.
- The syntax emits default-style spans for meaningful tokens because a group or fallback is misconfigured.

## State and fragment failures

- Repeated string or comment logic was copied instead of extracted into fragments.
- A new state exists but nothing transitions into it.
- A multiline construct enters a state but never exits cleanly.
- An imported syntax is referenced before the base syntax is compiled.

## Example failures

- The example reaches the line target through repetition instead of branch coverage.
- The example demonstrates only the easy path and skips edge constructs.
- The example omits URLs even though URL highlighting is part of the syntax.
- The example covers top-level blocks but not nested or multiline forms.
- The example covers host syntax but not embedded syntax.

## Test failures

- The syntax compiles but analyzer selection is never tested.
- Highlight tests only check plain keywords or strings.
- Assertions verify existence of spans but not the exact style class that matters.
- A new file-routing rule was added without a matching selection test.

## Delivery failures

- Demo registration was skipped even though the syntax is intended to appear in examples or demos.
- Style names used by the syntax are not registered in the test or demo environment.
- The syntax works in isolation but the repository still ships outdated examples or routing tables.
