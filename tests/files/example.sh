#!/bin/bash
# Shell sample

# variable
NAME="World"
COUNT=42
PI=3.14

# built-in command (builtin)
echo "Hello, $NAME!"
printf "Count: %d\n" "$COUNT"
cd /tmp && pwd

# function definition
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

# built-in command demonstration
ls -la /tmp
cp source.txt dest.txt
mkdir -p /tmp/test
touch /tmp/test/file.txt
cat /tmp/test/file.txt
head -n 5 /etc/hosts
tail -n 3 /etc/hosts
grep -r "pattern" /tmp
find /tmp -name "*.txt" -type f
sort /tmp/data.txt | uniq | wc -l
chmod 755 /tmp/test
date +"%Y-%m-%d %H:%M:%S"
sleep 1
which bash
type echo

# condition and loop
if [ -d "/tmp" ] && [ "$COUNT" -gt 0 ]; then
    echo "Directory exists"
elif [ "$NAME" = "World" ]; then
    echo "Default name"
else
    echo "Other"
fi

for i in $(seq 1 5); do
    echo "Iteration $i"
done

while read -r line; do
    echo "Line: $line"
done < /etc/hosts

case "$NAME" in
    World)
        echo "Global"
        ;;
    *)
        echo "Other: $NAME"
        ;;
esac

# variable expansion
echo "HOME is ${HOME}"
echo "Default: ${UNSET:-fallback}"
echo "Length: ${#NAME}"
echo "Slice: ${NAME:0:3}"

# special variable
echo "Script: $0"
echo "Args: $@"
echo "Count: $#"
echo "Status: $?"
echo "PID: $$"

# command substitution
CURRENT=$(date +%s)
FILES=$(find . -name "*.sh" | wc -l)

# varibale in double quote string
echo "Current timestamp: ${CURRENT}"
echo "Found ${FILES} shell scripts"

# Here Document
cat <<EOF
Name: $NAME
Count: $COUNT
EOF

# array
arr=(one two three)
echo "${arr[0]}"
echo "${arr[@]}"

# pipe and redirect
echo "test" | grep "t" > /dev/null 2>&1

# built-in value
result=true
failed=false

# background execution
nohup sleep 10 &
wait

# environment variable
export PATH="/usr/local/bin:$PATH"
env | grep HOME
printenv USER

# numeric operation
expr 10 + 20
echo "Scale: $(echo "scale=2; 22/7" | bc)"

# trap signal handling
trap 'echo "Caught signal"' EXIT
