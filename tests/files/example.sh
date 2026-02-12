#!/bin/bash
# Shell 高亮示例

# 变量
NAME="World"
COUNT=42
PI=3.14

# 内置命令 (builtin)
echo "Hello, $NAME!"
printf "Count: %d\n" "$COUNT"
cd /tmp && pwd

# 函数定义
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

# 内置命令演示
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

# 条件和循环
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

# 变量展开
echo "HOME is ${HOME}"
echo "Default: ${UNSET:-fallback}"
echo "Length: ${#NAME}"
echo "Slice: ${NAME:0:3}"

# 特殊变量
echo "Script: $0"
echo "Args: $@"
echo "Count: $#"
echo "Status: $?"
echo "PID: $$"

# 命令替换
CURRENT=$(date +%s)
FILES=$(find . -name "*.sh" | wc -l)

# 双引号中的变量
echo "Current timestamp: ${CURRENT}"
echo "Found ${FILES} shell scripts"

# Here Document
cat <<EOF
Name: $NAME
Count: $COUNT
EOF

# 数组
arr=(one two three)
echo "${arr[0]}"
echo "${arr[@]}"

# 管道和重定向
echo "test" | grep "t" > /dev/null 2>&1

# 内置值
result=true
failed=false

# 后台执行
nohup sleep 10 &
wait

# 环境变量
export PATH="/usr/local/bin:$PATH"
env | grep HOME
printenv USER

# 数值运算
expr 10 + 20
echo "Scale: $(echo "scale=2; 22/7" | bc)"

# trap 信号处理
trap 'echo "Caught signal"' EXIT
