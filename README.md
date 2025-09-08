# PowerHouse

PowerHouse is a C++ DLL that hosts the PowerShell engine directly through COM interop, allowing applications to execute PowerShell commands without launching powershell.exe. The DLL provides two primary entry points for PowerShell execution, supports colorful console output, and can execute scripts from base64-encoded data files.

![PowerHouse Banner](https://i.imgur.com/placeholder.png)

## Features

- **Direct PowerShell Execution**: Hosts the PowerShell engine using COM interop instead of starting powershell.exe
- **Interactive Console Mode**: Provides a colorful interactive PowerShell console
- **Base64 Encoded Script Execution**: Reads and executes PowerShell scripts from base64-encoded .dat files
- **Colorful User Interface**: Includes a custom ASCII banner and colored terminal output
- **Error Handling**: Captures and displays both output and error streams
- **No Console Crash Protection**: Gracefully handles situations where no console is attached

## Entry Points

PowerHouse exposes two primary entry point functions:

### `console()`

Interactive PowerShell console mode that:
- Creates a console if one doesn't exist
- Displays a colorful ASCII banner
- Reads commands from the console
- Executes them through the PowerShell engine
- Displays results with proper formatting
- Exits when the user types "exit" or an empty command

### `dat(const char* filePath)`

Base64-encoded script execution mode that:
- Reads the contents of the specified .dat file
- Decodes the content from base64
- Executes the decoded text as a PowerShell script
- Displays output with proper formatting

## Build Requirements

- Visual Studio 2019 or higher
- C++14 compatible compiler
- Windows SDK 10.0 or higher
- ATL and COM support

## Usage Examples

### From C/C++

```cpp
// Example of using the PowerHouse DLL from C/C++
#include <windows.h>

typedef void (__stdcall *ConsoleFunc)();
typedef void (__stdcall *DatFunc)(const char*);

int main() {
    // Load the DLL
    HMODULE hDll = LoadLibrary("powerhouse.dll");
    if (!hDll) {
        return 1;
    }

    // Get function pointers
    ConsoleFunc consoleFunc = (ConsoleFunc)GetProcAddress(hDll, "console");
    DatFunc datFunc = (DatFunc)GetProcAddress(hDll, "dat");

    // Use the console mode
    if (consoleFunc) {
        consoleFunc();
    }

    // Or execute a script from a .dat file
    if (datFunc) {
        datFunc("C:\\path\\to\\script.dat");
    }

    // Unload the DLL
    FreeLibrary(hDll);
    return 0;
}
```

### From PowerShell

```powershell
# Load the DLL
$dllPath = "C:\path\to\powerhouse.dll"
Add-Type -Path $dllPath

# Call the entry points
[powerhouse.Naruto]::console()
[powerhouse.Naruto]::dat("C:\path\to\script.dat")
```

### Creating Base64-encoded Script Files

```powershell
# Create a base64-encoded script file
$script = @'
Write-Host "Hello from PowerHouse!" -ForegroundColor Cyan
Get-Process | Where-Object { $_.CPU -gt 10 } | Format-Table Name, CPU
'@

$bytes = [System.Text.Encoding]::UTF8.GetBytes($script)
$base64 = [Convert]::ToBase64String($bytes)
$base64 | Out-File -FilePath "script.dat" -Encoding ascii
```

## Implementation Details

- Uses COM automation with WScript.Shell to execute PowerShell commands
- Implements base64 decoding using XML DOM
- Supports ANSI color codes for rich console output
- Handles both standard output and error streams
- Implements proper cleanup of COM resources
- Uses anime character names for classes (Naruto, Goku)

