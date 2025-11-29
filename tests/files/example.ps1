#!/usr/bin/env pwsh
# PowerShell syntax highlighting demo
using namespace System.IO

<#
  Block comment spanning
  multiple lines
#>

# Enum
enum LogLevel {
    Debug = 0; Info = 1
    Warning = 2; Error = 3
}

# Class with constructor, methods, static member
class Logger {
    [string]$Name
    [LogLevel]$Level
    hidden [int]$Count = 0

    Logger([string]$name) {
        $this.Name = $name
        $this.Level = [LogLevel]::Info
    }

    [void] Log([string]$msg) {
        $this.Count++
        Write-Host "[$([DateTime]::Now.ToString('HH:mm:ss'))] $($this.Name): $msg"
    }

    static [string] Format([string]$tpl, [object[]]$args) {
        return [string]::Format($tpl, $args)
    }
}

# Function with attributes and param block
function Get-Report {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory=$true, Position=0)]
        [string]$Name,
        [ValidateSet('Brief', 'Full')]
        [string]$Mode = 'Brief',
        [switch]$Detailed
    )
    begin { $result = @{} }
    process {
        $result['Name'] = $Name
        $result['OS'] = [Environment]::OSVersion.ToString()
        $result['PS'] = $PSVersionTable.PSVersion
        if ($Detailed) {
            $result['Procs'] = Get-Process |
                Sort-Object CPU -Descending |
                Select-Object -First 5
        }
    }
    end { return $result }
}

# Filter
filter Select-Large {
    if ($_.Length -gt 1mb) { $_ }
}

# Variables and scopes
$name = "PowerShell"
$global:Config = @{ Timeout = 30; Retries = 3 }
$script:Counter = 0
$env:PATH += ";C:\Tools"
${spaced var} = "braced variable"

# Built-in constants and automatic variables
$valid = $true -and ($null -ne $name)
$ok = $?; $dir = $PWD; $root = $PSScriptRoot

# Type literals, casting, static calls
[int]$port = 8080
[string[]]$hosts = @("alpha", "beta", "gamma")
$path = [System.IO.Path]::Combine($HOME, "data")
$max = [Math]::Max(10, 20)

# Numbers: integer, hex, binary, float, scientific, size suffix
$n = 42; $hex = 0xFF; $bin = 0b1011
$pi = 3.14159; $sci = 1.5e3; $buf = 64kb; $quota = 2gb

# Strings: single-quoted, double-quoted, here-strings
$s1 = 'literal: no $expansion, escape with '''
$s2 = "Hello $name v$($PSVersionTable.PSVersion)`nPath: $env:PATH"
$hd = @"
Here-string: $name with $($hosts.Count) hosts
"@
$hs = @'
Literal here-string: $name not expanded
'@

# Control flow
foreach ($h in $hosts) {
    if ($h -like "a*") {
        Write-Host "$h matches" -ForegroundColor Green
    } elseif ($h -match '^b') {
        Write-Host "$h regex" -ForegroundColor Yellow
    } else {
        Write-Host "$h other"
    }
}

switch ($name) {
    'PowerShell' { "exact match"; break }
    default      { "no match" }
}

for ($i = 0; $i -lt 10; $i++) {
    if ($i -eq 5) { continue }
    $script:Counter += $i
}
do { $n = [Math]::Floor($n / 2) } until ($n -le 1)

# Pipeline, cmdlets, splatting
$files = Get-ChildItem -Path $HOME -Recurse -Filter "*.log" |
    Where-Object { $_.LastWriteTime -gt (Get-Date).AddDays(-7) } |
    Sort-Object Length -Descending
$splat = @{ Path = "a.txt"; Destination = "b.txt"; Force = $true }
Copy-Item @splat

# Error handling
try {
    $data = [File]::ReadAllText("config.txt") | ConvertFrom-Json
} catch [FileNotFoundException] {
    Write-Error "Missing: $_"
} catch {
    Write-Warning "Error: $($_.Exception.Message)"
} finally {
    Write-Host "Cleanup done"
}

# Comparison, logical operators, range, join
$cmp = (10 -gt 5) -and ("hello" -ne "world")
$matched = "abc123" -match '\d+'
$evens = 1..20 | Where-Object { $_ % 2 -eq 0 }
$joined = $evens -join ", "

# .NET method calls and property access
$sb = [System.Text.StringBuilder]::new(256)
$sb.Append("Hello").Append(" World") | Out-Null
Write-Host "Len=$($sb.Length), Text=$($sb.ToString())" -ForegroundColor Cyan
