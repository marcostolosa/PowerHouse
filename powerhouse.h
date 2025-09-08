#pragma once

#include <string>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <comdef.h>
#include <atlbase.h>
#include <msxml6.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

namespace powerhouse {
    
    // Helper class for PowerShell interaction
    class Goku {
    public:
        // Initialize COM environment
        static bool Initialize() {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            return SUCCEEDED(hr);
        }

        // Uninitialize COM environment
        static void Uninitialize() {
            CoUninitialize();
        }

        // Check if console is attached
        static bool HasConsole() {
            return ::GetConsoleWindow() != NULL;
        }

        // Create a console if one doesn't exist
        static bool EnsureConsole() {
            if (!HasConsole()) {
                // Try to create a console
                if (!AllocConsole()) {
                    return false;
                }
                
                // Redirect standard I/O to console
                FILE* fpstdin = stdin;
                FILE* fpstdout = stdout;
                FILE* fpstderr = stderr;
                
                freopen_s(&fpstdin, "CONIN$", "r", stdin);
                freopen_s(&fpstdout, "CONOUT$", "w", stdout);
                freopen_s(&fpstderr, "CONOUT$", "w", stderr);
                
                // Also set C++ streams
                std::ios::sync_with_stdio();
            }
            return true;
        }

        // Display PowerHouse ASCII banner
        static void DisplayAsciiBanner() {
            std::cout << "\033[1;36m" << std::endl; // Bright cyan color
            std::cout << "  _____                        _    _                        " << std::endl;
            std::cout << " |  __ \\                      | |  | |                       " << std::endl;
            std::cout << " | |__) |____      _____ _ __ | |__| | ___  _   _ ___  ___   " << std::endl;
            std::cout << " |  ___/ _ \\ \\ /\\ / / _ \\ '_ \\|  __  |/ _ \\| | | / __|/ _ \\  " << std::endl;
            std::cout << " | |  | (_) \\ V  V /  __/ | | | |  | | (_) | |_| \\__ \\  __/  " << std::endl;
            std::cout << " |_|   \\___/ \\_/\\_/ \\___|_| |_|_|  |_|\\___/ \\__,_|___/\\___|  " << std::endl;
            std::cout << "                                                             " << std::endl;
            std::cout << "\033[0m"; // Reset color
            std::cout << "\033[1;33m"; // Bright yellow for subtitle
            std::cout << "         PowerShell Host DLL by Ishan Saha           " << std::endl;
            std::cout << "\033[0m"; // Reset color
            std::cout << "\033[0;32m"; // Green for divider
            std::cout << "==============================================================" << std::endl;
            std::cout << "\033[0m"; // Reset color
        }

        // Configure console settings
        static void SetupConsole() {
            // Get console handle
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (hConsole == INVALID_HANDLE_VALUE)
                return;

            // Set console mode to enable virtual terminal processing
            DWORD dwMode = 0;
            if (GetConsoleMode(hConsole, &dwMode)) {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hConsole, dwMode);
            }

            // Set console title
            SetConsoleTitleA("PowerHouse - PowerShell Host");
        }

        // Convert wide string to string
        static std::string WideToString(const std::wstring& wstr) {
            if (wstr.empty()) return std::string();
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
            return strTo;
        }

        // Convert string to wide string
        static std::wstring StringToWide(const std::string& str) {
            if (str.empty()) return std::wstring();
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
            std::wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
            return wstrTo;
        }

        // Execute PowerShell script
        static bool ExecutePowerShell(const std::string& script, bool printOutput) {
            bool success = false;
            
            try {
                if (!Initialize()) {
                    return false;
                }

                // Create Windows Script Host Shell object
                CComPtr<IDispatch> spShell;
                HRESULT hr = spShell.CoCreateInstance(L"WScript.Shell");
                if (FAILED(hr)) {
                    Uninitialize();
                    return false;
                }

                // Get Exec method
                DISPID dispid;
                LPOLESTR name = const_cast<LPOLESTR>(L"Exec");
                hr = spShell->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &dispid);
                if (FAILED(hr)) {
                    Uninitialize();
                    return false;
                }

                // Create PowerShell command line with encoded command
                std::string encodedCommand = "powershell.exe -ExecutionPolicy Bypass -NoLogo -NoProfile -Command \"& { ";
                encodedCommand += script;
                encodedCommand += " }\"";

                // Convert to wide string
                std::wstring wCommand = StringToWide(encodedCommand);
                
                // Setup parameters for Exec method
                DISPPARAMS params;
                VARIANT arg;
                VARIANT result;
                VariantInit(&arg);
                VariantInit(&result);
                
                arg.vt = VT_BSTR;
                arg.bstrVal = SysAllocString(wCommand.c_str());
                
                params.rgvarg = &arg;
                params.rgdispidNamedArgs = NULL;
                params.cArgs = 1;
                params.cNamedArgs = 0;
                
                // Call Exec method
                hr = spShell->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &result, NULL, NULL);
                SysFreeString(arg.bstrVal);
                
                if (SUCCEEDED(hr)) {
                    // Process the result if we need to print output
                    if (printOutput) {
                        // Make sure we have a console if we need to print
                        if (EnsureConsole()) {
                            // Get the WshExec object from the result
                            if (result.vt == VT_DISPATCH && result.pdispVal != NULL) {
                                // Get StdOut property
                                LPOLESTR stdOutName = const_cast<LPOLESTR>(L"StdOut");
                                DISPID stdOutId;
                                hr = result.pdispVal->GetIDsOfNames(IID_NULL, &stdOutName, 1, LOCALE_USER_DEFAULT, &stdOutId);
                                
                                if (SUCCEEDED(hr)) {
                                    // Read StdOut
                                    DISPPARAMS noParams = { NULL, NULL, 0, 0 };
                                    VARIANT stdOutVar;
                                    VariantInit(&stdOutVar);
                                    
                                    hr = result.pdispVal->Invoke(stdOutId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &noParams, &stdOutVar, NULL, NULL);
                                    
                                    if (SUCCEEDED(hr) && stdOutVar.vt == VT_DISPATCH) {
                                        // Get ReadAll method
                                        LPOLESTR readAllName = const_cast<LPOLESTR>(L"ReadAll");
                                        DISPID readAllId;
                                        hr = stdOutVar.pdispVal->GetIDsOfNames(IID_NULL, &readAllName, 1, LOCALE_USER_DEFAULT, &readAllId);
                                        
                                        if (SUCCEEDED(hr)) {
                                            // Call ReadAll method
                                            VARIANT readResult;
                                            VariantInit(&readResult);
                                            
                                            hr = stdOutVar.pdispVal->Invoke(readAllId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &noParams, &readResult, NULL, NULL);
                                            
                                            if (SUCCEEDED(hr) && readResult.vt == VT_BSTR) {
                                                // Print the output
                                                std::wcout << readResult.bstrVal;
                                                // Only add a newline if the output doesn't end with one
                                                if (SysStringLen(readResult.bstrVal) > 0 && 
                                                    readResult.bstrVal[SysStringLen(readResult.bstrVal) - 1] != L'\n') {
                                                    std::wcout << std::endl;
                                                }
                                            }
                                            
                                            VariantClear(&readResult);
                                        }
                                    }
                                    
                                    VariantClear(&stdOutVar);
                                }
                                
                                // Get StdErr property
                                LPOLESTR stdErrName = const_cast<LPOLESTR>(L"StdErr");
                                DISPID stdErrId;
                                hr = result.pdispVal->GetIDsOfNames(IID_NULL, &stdErrName, 1, LOCALE_USER_DEFAULT, &stdErrId);
                                
                                if (SUCCEEDED(hr)) {
                                    // Read StdErr
                                    DISPPARAMS noParams = { NULL, NULL, 0, 0 };
                                    VARIANT stdErrVar;
                                    VariantInit(&stdErrVar);
                                    
                                    hr = result.pdispVal->Invoke(stdErrId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &noParams, &stdErrVar, NULL, NULL);
                                    
                                    if (SUCCEEDED(hr) && stdErrVar.vt == VT_DISPATCH) {
                                        // Get ReadAll method
                                        LPOLESTR readAllName = const_cast<LPOLESTR>(L"ReadAll");
                                        DISPID readAllId;
                                        hr = stdErrVar.pdispVal->GetIDsOfNames(IID_NULL, &readAllName, 1, LOCALE_USER_DEFAULT, &readAllId);
                                        
                                        if (SUCCEEDED(hr)) {
                                            // Call ReadAll method
                                            VARIANT readResult;
                                            VariantInit(&readResult);
                                            
                                            hr = stdErrVar.pdispVal->Invoke(readAllId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &noParams, &readResult, NULL, NULL);
                                            
                                            if (SUCCEEDED(hr) && readResult.vt == VT_BSTR && SysStringLen(readResult.bstrVal) > 0) {
                                                // Print the error
                                                std::wcerr << L"\033[1;31mError: \033[0m" << readResult.bstrVal;
                                                // Only add a newline if the output doesn't end with one
                                                if (readResult.bstrVal[SysStringLen(readResult.bstrVal) - 1] != L'\n') {
                                                    std::wcerr << std::endl;
                                                }
                                            }
                                            
                                            VariantClear(&readResult);
                                        }
                                    }
                                    
                                    VariantClear(&stdErrVar);
                                }
                            }
                        }
                    }
                    
                    success = true;
                }
                
                VariantClear(&result);
            }
            catch (...) {
                // Silently handle exceptions
            }
            
            Uninitialize();
            return success;
        }

        // Decode base64 to text
        static std::string DecodeBase64(const std::string& base64String) {
            if (base64String.empty()) {
                return "";
            }

            try {
                Initialize();

                // Create XML document (XML DOM supports base64 decoding)
                CComPtr<IXMLDOMDocument> xmlDoc;
                HRESULT hr = xmlDoc.CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
                if (FAILED(hr)) {
                    Uninitialize();
                    return "";
                }

                // Create base64 element
                CComBSTR bstrXml(L"<base64 />");
                VARIANT_BOOL isSuccessful;
                xmlDoc->loadXML(bstrXml, &isSuccessful);
                if (isSuccessful != VARIANT_TRUE) {
                    Uninitialize();
                    return "";
                }

                // Get the root element
                CComPtr<IXMLDOMElement> xmlElement;
                xmlDoc->get_documentElement(&xmlElement);
                if (!xmlElement) {
                    Uninitialize();
                    return "";
                }

                // Set the base64 data
                CComBSTR bstrData(StringToWide(base64String).c_str());
                xmlElement->put_dataType(CComBSTR(L"bin.base64"));
                xmlElement->put_text(bstrData);

                // Get the decoded data
                VARIANT varValue;
                VariantInit(&varValue);
                xmlElement->get_nodeTypedValue(&varValue);

                std::string result;
                if (varValue.vt == (VT_ARRAY | VT_UI1)) {
                    // Get data from SAFEARRAY
                    SAFEARRAY* pArray = V_ARRAY(&varValue);
                    BYTE* pData = NULL;
                    HRESULT hr = SafeArrayAccessData(pArray, (void**)&pData);
                    
                    if (SUCCEEDED(hr)) {
                        LONG lLower, lUpper;
                        SafeArrayGetLBound(pArray, 1, &lLower);
                        SafeArrayGetUBound(pArray, 1, &lUpper);
                        
                        // Convert to string
                        result = std::string((char*)pData, lUpper - lLower + 1);
                        
                        SafeArrayUnaccessData(pArray);
                    }
                }

                VariantClear(&varValue);
                Uninitialize();
                return result;
            }
            catch (...) {
                Uninitialize();
                return "";
            }
        }
    };

    // Entry point class
    class Naruto {
    public:
        // Interactive console mode
        static void console() {
            try {
                // Ensure there's a console attached
                if (!Goku::EnsureConsole()) {
                    return; // Cannot create a console, exit silently
                }

                // Configure console
                Goku::SetupConsole();
                
                // Display ASCII banner
                Goku::DisplayAsciiBanner();
                
                // Display help message
                std::cout << "\033[0;37mType \033[1;37mcommands\033[0;37m to execute or \033[1;37mexit\033[0;37m to quit\033[0m" << std::endl;
                std::cout << "\033[0;32m--------------------------------------------------------------\033[0m" << std::endl;

                while (true) {
                    std::cout << "\033[1;36mPS>\033[0m ";
                    std::string command;
                    std::getline(std::cin, command);

                    // Exit if command is empty or "exit"
                    if (command.empty() || command == "exit") {
                        std::cout << "\033[0;33mExiting PowerHouse...\033[0m" << std::endl;
                        break;
                    }

                    // Execute the command
                    Goku::ExecutePowerShell(command, true);
                }
            }
            catch (...) {
                // Silent exception handling to prevent crashes
            }
        }

        // Execute script from base64-encoded .dat file
        static void dat(const char* filePath) {
            try {
                // Check if file exists
                std::ifstream file(filePath);
                if (!file.good()) {
                    return;
                }
                file.close();

                // Ensure there's a console for output
                Goku::EnsureConsole();
                
                // Configure console
                Goku::SetupConsole();
                
                // Display ASCII banner
                Goku::DisplayAsciiBanner();
                
                std::cout << "\033[0;36mExecuting PowerShell script from: \033[1;36m" << filePath << "\033[0m" << std::endl;
                std::cout << "\033[0;32m--------------------------------------------------------------\033[0m" << std::endl;

                // Read the base64 encoded content from the .dat file
                std::ifstream inFile(filePath);
                std::string base64Content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
                inFile.close();
                
                // Decode the base64 content
                std::string decodedScript = Goku::DecodeBase64(base64Content);

                // Execute the decoded PowerShell script
                Goku::ExecutePowerShell(decodedScript, true);
                
                std::cout << "\033[0;32m--------------------------------------------------------------\033[0m" << std::endl;
                std::cout << "\033[0;36mScript execution completed. Press Enter to exit...\033[0m" << std::endl;
                std::cin.get(); // Wait for user to press Enter
            }
            catch (...) {
                // Silent exception handling to prevent crashes
            }
        }
    };
}