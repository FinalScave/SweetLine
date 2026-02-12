-- Lua 高亮示例

--[[ 多行注释
     跨越多行 ]]

local function greet(name)
    print("Hello, " .. name .. "!")
end

-- 面向对象
local Animal = {}
Animal.__index = Animal

function Animal.new(name, sound)
    local self = setmetatable({}, Animal)
    self.name = name
    self.sound = sound
    return self
end

function Animal:speak()
    return self.name .. " says " .. self.sound
end

-- 内置函数
local nums = {3, 1, 4, 1, 5}
local len = #nums
local str = tostring(42)
local num = tonumber("3.14")
local tp = type(nums)

for i, v in ipairs(nums) do
    if v > 3 then
        print(v)
    end
end

for k, v in pairs({a = 1, b = 2}) do
    assert(v > 0)
end

-- 数字字面量
local hex = 0xFF
local float = 3.14
local sci = 1e10
local zero = 0

-- 字面量
local flag = true
local off = false
local empty = nil

-- 长字符串
local longStr = [[
this is a
multi-line string
]]

-- 字符串库
local upper = string.upper("hello")
local result = string.format("PI = %.2f", 3.14)
local found = string.find("hello world", "world")

-- 数学库
local abs = math.abs(-42)
local sqrt = math.sqrt(144)

-- 表操作
local t = {10, 20, 30}
table.insert(t, 40)
table.sort(t)

-- pcall 错误处理
local ok, err = pcall(function()
    error("something went wrong")
end)

-- require 模块
local json = require("json")

-- select 和 unpack
local first = select(1, "a", "b", "c")
local a, b = unpack({10, 20})
