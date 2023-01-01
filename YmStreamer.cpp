#include <iostream>
#include <filesystem>
#include <chrono>
#include <windows.h>

#include "YmStream.h"
#include "YmDevice.h"
#include "W32Exception.h"

volatile bool running = true;

bool WINAPI consoleHandler(_In_ DWORD dwCtrlType) {
    if (dwCtrlType == CTRL_C_EVENT) {
        running = false;
        return true;
    }
    return false;
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc < 4) {
        return EXIT_FAILURE;
    }
    try {
        YmStream s1(argv[1]);
        YmStream s2(argv[2]);
        YmDevice d(argv[3], (argc > 4) ? argv[4] : NULL);

        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler, TRUE)) {
            throw W32Exception(L"SetConsoleCtrlHandler");
        }

        std::cout << "Version: " << s1.getVersion() << std::endl;
        std::cout << "Title: " << s1.getSongName() << std::endl;
        std::cout << "Artist: " << s1.getAuthorName() << std::endl;
        std::cout << "Remarks: " << s1.getSongComment() << std::endl;
        std::cout << "Length: " << s1.getDurationStr() << std::endl << std::endl;

        std::cout << "Version: " << s2.getVersion() << std::endl;
        std::cout << "Title: " << s2.getSongName() << std::endl;
        std::cout << "Artist: " << s2.getAuthorName() << std::endl;
        std::cout << "Remarks: " << s2.getSongComment() << std::endl;
        std::cout << "Length: " << s2.getDurationStr() << std::endl << std::endl;
        
        if (s1.getFrameRate() != s2.getFrameRate()) {
            throw std::invalid_argument("Files have different framerates");
        }

        for (uint32_t f = 0; f < max(s1.getFrameCount(), s2.getFrameCount()); f++) {
            using namespace std::chrono;
            if (!running) {
                break;
            }
            auto end = high_resolution_clock::now() + (1s / s1.getFrameRate());
            d.updateTurboFrame(s1.getFrame(f), s2.getFrame(f));
            while (high_resolution_clock::now() < end) { (void)0; }
        }

        d.mute();
    }
    catch (const W32Exception& e) {
        fwprintf(stderr, L"%s: %d %s\n", e.what(), e.code(), e.message());
        return e.code();
    }
    catch (const std::exception& e) {
        fprintf(stderr, "%s\n", e.what());
        return EXIT_FAILURE;
    }
}