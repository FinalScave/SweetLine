#!/bin/bash
# Shell sample

# Variable assignment
NAME="World"
ITEMS=()
APPEND+="extra"

# Built-in commands
echo "Hello, $NAME!"
printf "Count: %d\n" "$COUNT"
cd /tmp && pwd
read -r -p "Enter: " input

# Path arguments - keywords/builtins should NOT highlight in paths
ls -la /tmp/test
cat /var/log/do/done.log
head -n 5 /etc/hosts

# Function definitions
greet() {
    local name="$1"
    local upper
    upper=$(echo "$name" | tr 'a-z' 'A-Z')
    echo "Hello, $upper!"
}

function process_files() {
    local dir="${1:-.}"
    local count=0

    for file in "$dir"/*; do
        if [ -f "$file" ]; then
            local size
            size=$(stat -f%z "$file" 2>/dev/null)
            basename "$file"
            count=$((count + 1))
        fi
    done

    echo "Processed $count files"
    return 0
}

# Command substitution $()
CURRENT=$(date +%s)
FILES=$(find . -name "*.sh" | wc -l)
UPPER=$(echo "hello" | tr 'a-z' 'A-Z')

# Nested command substitution
COMPLEX=$(echo "$(date +%Y)-$(hostname)")

# Backtick command substitution
OLD_STYLE=`date +%s`
KERNEL=`uname -r`
LINE_COUNT=`wc -l < /etc/passwd`

# Arithmetic expansion $((...))
SUM=$((10 + 20))
PRODUCT=$((COUNT * 3))
REMAINDER=$((100 % 7))
INCREMENT=$((COUNT + 1))

# Variable expansion
echo "HOME is ${HOME}"
echo "Default: ${UNSET:-fallback}"
echo "Assign: ${UNSET:=default}"

# Special variables
echo "Script: $0"
echo "First arg: $1"
echo "All args: $@"
echo "All args: $*"

# String types
SINGLE='Hello, World! No $expansion here'
DOUBLE="Hello, $NAME! With ${COUNT} items"
ANSI=$'Line 1\nLine 2\tTabbed'
EMPTY=""

# Condition and loop
if [ -d "/tmp" ] && [ "$COUNT" -gt 0 ]; then
    echo "Directory exists"
elif [ "$NAME" = "World" ]; then
    echo "Default name"
else
    echo "Other"
fi

# Select menu
select opt in "Option1" "Option2" "Quit"; do
    case $opt in
        Quit) break ;;
        *) echo "Selected: $opt" ;;
    esac
done

# Here Document
cat <<EOF
Name: $NAME
Count: $COUNT
Home: ${HOME}
EOF

cat <<-'NOEXPAND'
	No $variable expansion
	Everything is literal: ${HOME}
NOEXPAND

# Numeric literals
DEC=42
FLOAT=3.14

# Array
arr=(one two three "four five")
echo "${arr[0]}"
echo "${arr[@]}"
echo "${#arr[@]}"
arr+=(six)

declare -A map
map[key1]="value1"
map[key2]="value2"

# Pipe and redirect
echo "test" | grep "t" > /dev/null 2>&1
cat < input.txt >> output.txt
ls -la 2>/dev/null | sort -k5 -n | head -n 10

# Environment variables
export PATH="/usr/local/bin:$PATH"
env | grep HOME
printenv USER
readonly CONSTANT="immutable"

# Subshell and group
(cd /tmp && ls)
{ echo "grouped"; echo "commands"; }

# Useful patterns
# Command chaining
mkdir -p /tmp/build && cd /tmp/build && make clean
command1 || echo "command1 failed"

# Process substitution
diff <(ls /dir1) <(ls /dir2)

# Brace expansion
echo {1..10}
mkdir -p /tmp/{src,build,dist}
