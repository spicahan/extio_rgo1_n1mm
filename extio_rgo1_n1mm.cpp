#include <iostream>
#include <cstdint>
#include <windows.h>

// Define the function pointer type
typedef void (__stdcall *CloseHWFunc)(void);
typedef long (__stdcall *GetHWLOFunc)(void);
typedef long (__stdcall *GetHWSRFunc)(void);
typedef int (__stdcall *GetStatusFunc)(void);
typedef void (__stdcall *HideGUIFunc)(void);
typedef bool (__stdcall *InitHWFunc)(char *name, char *model, int &index);
typedef bool (__stdcall *OpenHWFunc)(void);
typedef void (__stdcall *SetCallbackFunc)(void (* Callback)(int, int, float, void *));
typedef int (__stdcall *SetHWLOFunc)(long LOfreq);
typedef void (__stdcall *ShowGUIFunc)(void);
typedef int (__stdcall *StartHWFunc)(long freq);
typedef void (__stdcall *StopHWFunc)(void);

typedef void (*Callback)(int, int, float, void *);

static HMODULE hModule;
static long fakeFreq = 0;
static int hwType = 0;
#define FIXED_IF 8999050

static Callback realCallback = nullptr;

void SwapIQCallback(int cnt, int status, float IQoffs, void *IQdata) {
    // Check if it's for IQ data available. If yes, swap IQ
    if (cnt > 0) {
        // TODO handle the case where I/Q sample is 16 bits
        for (int i = 0; i < cnt; ++i)
        {
            if (hwType == 3)
            {
                uint16_t *sampleI = reinterpret_cast<uint16_t *>(IQdata) + i * 2;
                uint16_t *sampleQ = reinterpret_cast<uint16_t *>(IQdata) + i * 2 + 1;
                uint16_t I = *sampleI;
                *sampleI = *sampleQ;
                *sampleQ = I;
            }
            else
            {
                uint32_t *sampleI = reinterpret_cast<uint32_t *>(IQdata) + i * 2;
                uint32_t *sampleQ = reinterpret_cast<uint32_t *>(IQdata) + i * 2 + 1;
                uint32_t I = *sampleI;
                *sampleI = *sampleQ;
                *sampleQ = I;
            }
        }
    }
    realCallback(cnt, status, IQoffs, IQdata);
}

static CloseHWFunc realCloseHW = nullptr;
static GetHWLOFunc realGetHWLO = nullptr;
static GetHWSRFunc realGetHWSR = nullptr;
static GetStatusFunc realGetStatus = nullptr;
static HideGUIFunc realHideGUI = nullptr;
static InitHWFunc realInitHW = nullptr;
static OpenHWFunc realOpenHW = nullptr;
static SetCallbackFunc realSetCallback = nullptr;
static SetHWLOFunc realSetHWLO = nullptr;
static ShowGUIFunc realShowGUI = nullptr;
static StartHWFunc realStartHW = nullptr;
static StopHWFunc realStopHW = nullptr;

#define loadFunction(func) \
do { \
    if (!loadFromDll(#func, real ## func)) \
    { \
        FreeLibrary(hModule); \
        return -1; \
    } \
} while(0)

template <typename FuncT>
bool loadFromDll(const char *funcName, FuncT &funcPtr) {
    funcPtr = reinterpret_cast<FuncT>(GetProcAddress(hModule, funcName));
    if (!funcPtr) {
        std::cerr << "Failed to find function: " << funcName << " in the DLL!";
        return false;
    }
    return true;
}

bool loadDll(const char * dllFileName) {
    hModule = LoadLibrary(dllFileName);  // Load the DLL
    if (!hModule) {
        std::cerr << "Failed to load DLL: " << dllFileName << std::endl;
        return false;
    }
    return true;
}

bool loadFunctions() {
    loadFunction(CloseHW); 
    loadFunction(GetHWLO); 
    loadFunction(GetHWSR); 
    loadFunction(GetStatus); 
    loadFunction(HideGUI); 
    loadFunction(InitHW); 
    loadFunction(OpenHW); 
    loadFunction(SetCallback); 
    loadFunction(SetHWLO); 
    loadFunction(ShowGUI); 
    loadFunction(StartHW); 
    loadFunction(StopHW); 
    return true;
}

extern "C" {
void __stdcall __declspec(dllexport) CloseHW(void) {
    realCloseHW();
    FreeLibrary(hModule);
}

long __stdcall __declspec(dllexport) GetHWLO(void) {
    // Fake it
    // return realGetHWLO();
    return fakeFreq;
}

long __stdcall __declspec(dllexport) GetHWSR(void) {
    return realGetHWSR();
}

int __stdcall __declspec(dllexport) GetStatus(void) {
    return realGetStatus();
}

void __stdcall __declspec(dllexport) HideGUI(void) {
    realHideGUI();
}

bool __stdcall __declspec(dllexport) InitHW(char *name, char *model, int &index) {
    if (!loadDll("_extio_real.dll"))
    {
        return FALSE;
    }

    if (!loadFunctions())
    {
        return FALSE;
    }
    bool rc = realInitHW(name, model, index);
    hwType = index;
    return rc;
}

bool __stdcall __declspec(dllexport) OpenHW(void) {
    return realOpenHW();
}

void __stdcall __declspec(dllexport) SetCallback(Callback cb) {
    realCallback = cb;
    realSetCallback(SwapIQCallback);
}

int __stdcall __declspec(dllexport) SetHWLO(long LOfreq) {
    // Do nothing, only save the fake freq
    fakeFreq = LOfreq;
    return realSetHWLO(FIXED_IF);
}

void __stdcall __declspec(dllexport) ShowGUI(void) {
    realShowGUI();
}

int __stdcall __declspec(dllexport) StartHW(long freq) {
    // Force it to be the fixed IF
    fakeFreq = freq;
    return realStartHW(FIXED_IF);
}

void __stdcall __declspec(dllexport) StopHW(void) {
    realStopHW();
}
}