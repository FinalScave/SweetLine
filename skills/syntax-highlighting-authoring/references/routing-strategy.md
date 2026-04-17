# Routing Strategy

Do not assume file extension routing is sufficient.

## Prefer plain extensions when

- The extension maps cleanly to one grammar.
- The same extension is not used by another syntax in the project.

Examples:
- `.proto`
- `.graphql`
- `.rb`

## Use exact file names when

- The syntax is tied to conventional file names.
- The extension alone is too broad.

Examples:
- `Dockerfile`
- `CMakeLists.txt`
- `Makefile`
- `build.gradle`
- `settings.gradle.kts`

## Use file name suffixes when

- The meaningful part is at the end of a multi-part file name.
- The project needs to distinguish specialized files that share the last extension.

Examples:
- `.gradle.kts`
- `.blade.php`
- `.arm64.S`

## Use file name patterns when

- Naming conventions carry architecture, platform, or generator intent.
- Exact names and suffixes are still too narrow.

Examples:
- `.*(arm64|aarch64).*`
- `.*\\.generated\\..*`

## Required checks

- Verify the current repository actually supports the routing metadata you plan to use.
- If routing support does not exist, treat routing as part of the task rather than silently degrading behavior.
- Add analyzer-selection coverage for every non-trivial routing rule.
