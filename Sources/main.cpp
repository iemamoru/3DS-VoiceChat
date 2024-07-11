#include <3ds.h>
#include "csvc.h"
#include <CTRPluginFramework.hpp>
#include "buffers.h"

#include <vector>

namespace CTRPluginFramework
{
    // This patch the NFC disabling the touchscreen when scanning an amiibo, which prevents ctrpf to be used
    static void    ToggleTouchscreenForceOn(void) {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern =
        {
            0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000,
            0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001,
            0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003
        };

        Result  res;
        Handle  processHandle;
        s64     textTotalSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
exit:
        svcCloseHandle(processHandle);
    }

    // This function is called before main and before the game starts
    // Useful to do code edits safely
    void    PatchProcess(FwkSettings &settings) {
        ToggleTouchscreenForceOn();
    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void) {
        ToggleTouchscreenForceOn();
    }

    void InitializeSockets(void) {
        Result ret = RL_SUCCESS;
        Color red = Color::Red;
        Color green = Color::Green;

        ret = svcControlMemoryUnsafe((u32 *)&soundBuffer, SOUND_BUFFER_ADDR, SOUND_BUFFER_SIZE, MemOp(MEMOP_REGION_SYSTEM | MEMOP_ALLOC), MemPerm(MEMPERM_READ | MEMPERM_WRITE));
        if (R_FAILED(ret) || !soundBuffer)
            OSD::Notify("alloc soundBuffer failed", red);
        else
            OSD::Notify("alloc soundBuffer success", green);

        ret = svcControlMemoryUnsafe((u32 *)&micBuffer, MIC_BUFFER_ADDR, MIC_BUFFER_SIZE, MemOp(MEMOP_REGION_SYSTEM | MEMOP_ALLOC), MemPerm(MEMPERM_READ | MEMPERM_WRITE));
        if (R_FAILED(ret) || !micBuffer)
            OSD::Notify("alloc micBuffer failed", red);
        else
            OSD::Notify("alloc micBuffer success", green);
    }

    void    InitMenu(PluginMenu &menu)
    {
    }

    int     main(void)
    {
        PluginMenu *menu = new PluginMenu("Action Replay", 0, 7, 4,
                                            "A blank template plugin.\nGives you access to the ActionReplay and others tools.");

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);

        // Init our menu entries & folders
        InitMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        // Exit plugin
        return (0);
    }
}
