// File: source/Main.cpp
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "Configuration.hpp"
#include "Constants.hpp"
#include "Emulation/Controller.hpp"
#include "SMB/SMBEngine.hpp"
#include "Util/Video.hpp"

static SDL_Window* g_window = nullptr;
static SDL_Renderer* g_renderer = nullptr;
static SDL_Texture* g_texture = nullptr;
static SMBEngine* g_smbEngine = nullptr;
static uint32_t g_renderBuffer[RENDER_WIDTH * RENDER_HEIGHT];
static bool g_romLoaded = false;
static bool g_running = true;

// Bitmask state for iPad touch controls (buttons 0..7 matching ControllerButton enum)
static uint8_t g_touchMask = 0;

static void AudioCallback(void* userdata, uint8_t* buffer, int len) {
    if (!g_romLoaded || !g_smbEngine) {
        std::memset(buffer, 0, len);
        return;
    }
    g_smbEngine->audioCallback(buffer, len);
}

static bool loadRomIntoBuffer(const char* path, std::vector<uint8_t>& outRom) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::printf("[SMBC-WASM] Failed to open ROM at: %s\n", path);
        return false;
    }
    std::streamsize size = file.tellg();
    if (size < 16 + 32768 + 8192) {
        std::printf("[SMBC-WASM] Invalid NES ROM size (%ld bytes)\n", static_cast<long>(size));
        return false;
    }
    file.seekg(0, std::ios::beg);
    outRom.resize(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(outRom.data()), size)) {
        return false;
    }
    return true;
}

static void updateInputs() {
    if (!g_smbEngine) return;

    Controller& ctrl1 = g_smbEngine->getController1();
    const uint8_t* keys = SDL_GetKeyboardState(nullptr);

    bool a      = keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_Z] || keys[SDL_SCANCODE_L] || (g_touchMask & (1 << BUTTON_A));
    bool b      = keys[SDL_SCANCODE_C] || keys[SDL_SCANCODE_K] || keys[SDL_SCANCODE_LSHIFT] || (g_touchMask & (1 << BUTTON_B));
    bool select = keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_SPACE] || (g_touchMask & (1 << BUTTON_SELECT));
    bool start  = keys[SDL_SCANCODE_RETURN] || (g_touchMask & (1 << BUTTON_START));
    bool up     = keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W] || (g_touchMask & (1 << BUTTON_UP));
    bool down   = keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S] || (g_touchMask & (1 << BUTTON_DOWN));
    bool left   = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A] || (g_touchMask & (1 << BUTTON_LEFT));
    bool right  = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D] || (g_touchMask & (1 << BUTTON_RIGHT));

    ctrl1.setButtonState(BUTTON_A, a);
    ctrl1.setButtonState(BUTTON_B, b);
    ctrl1.setButtonState(BUTTON_SELECT, select);
    ctrl1.setButtonState(BUTTON_START, start);
    ctrl1.setButtonState(BUTTON_UP, up);
    ctrl1.setButtonState(BUTTON_DOWN, down);
    ctrl1.setButtonState(BUTTON_LEFT, left);
    ctrl1.setButtonState(BUTTON_RIGHT, right);
}

static void mainLoopTick() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g_running = false;
        }
    }

#ifdef __EMSCRIPTEN__
    // Fixed 60 updates/s whatever the display refresh rate (iPad ProMotion = 120Hz
    // would otherwise run the game at double speed / feel laggy).
    {
        static double lastMs = 0.0;
        static double accMs = 0.0;
        const double stepMs = 1000.0 / 60.0;
        double nowMs = emscripten_get_now();
        if (lastMs == 0.0) lastMs = nowMs;
        accMs += nowMs - lastMs;
        lastMs = nowMs;
        if (accMs > 100.0) accMs = 100.0;
        if (accMs < stepMs * 0.85) return;
        accMs -= stepMs;
        if (accMs > stepMs) accMs = stepMs;
    }
#endif

    if (!g_romLoaded || !g_smbEngine) {
        SDL_SetRenderDrawColor(g_renderer, 12, 12, 18, 255);
        SDL_RenderClear(g_renderer);
        SDL_RenderPresent(g_renderer);
        return;
    }

    updateInputs();
    g_smbEngine->update();
    g_smbEngine->render(g_renderBuffer);

    SDL_UpdateTexture(g_texture, nullptr, g_renderBuffer, sizeof(uint32_t) * RENDER_WIDTH);
    SDL_RenderClear(g_renderer);
    SDL_RenderCopy(g_renderer, g_texture, nullptr, nullptr);
    SDL_RenderPresent(g_renderer);
}

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int startGameWithRom(const char* romPath) {
    static std::vector<uint8_t> romData;
    if (!loadRomIntoBuffer(romPath, romData)) {
        return 0;
    }

    if (g_smbEngine) {
        delete g_smbEngine;
        g_smbEngine = nullptr;
    }

    g_smbEngine = new SMBEngine(romData.data());
    g_smbEngine->reset();
    g_romLoaded = true;

    if (Configuration::getAudioEnabled()) {
        SDL_PauseAudio(0);
    }

    std::printf("[SMBC-WASM] ROM loaded and SMBEngine started: %s\n", romPath);
    return 1;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void setTouchButton(int buttonId, int pressed) {
    if (buttonId < 0 || buttonId > 7) return;
    if (pressed) {
        g_touchMask |= static_cast<uint8_t>(1 << buttonId);
    } else {
        g_touchMask &= static_cast<uint8_t>(~(1 << buttonId));
    }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
void resetGame() {
    if (g_smbEngine && g_romLoaded) {
        g_smbEngine->reset();
    }
}

} // extern "C"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    Configuration::initialize(CONFIG_FILE_NAME);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    int windowWidth = RENDER_WIDTH * Configuration::getRenderScale();
    int windowHeight = RENDER_HEIGHT * Configuration::getRenderScale();

    g_window = SDL_CreateWindow(
        "Super Mario Bros. C (WASM)",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    g_renderer = SDL_CreateRenderer(
        g_window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    SDL_RenderSetLogicalSize(g_renderer, RENDER_WIDTH, RENDER_HEIGHT);

    g_texture = SDL_CreateTexture(
        g_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        RENDER_WIDTH,
        RENDER_HEIGHT
    );

    if (Configuration::getAudioEnabled()) {
        SDL_AudioSpec desiredSpec;
        SDL_zero(desiredSpec);
        desiredSpec.freq = Configuration::getAudioFrequency();
        desiredSpec.format = AUDIO_U8;
        desiredSpec.channels = 1;
        desiredSpec.samples = 2048;
        desiredSpec.callback = AudioCallback;
        desiredSpec.userdata = nullptr;

        if (SDL_OpenAudio(&desiredSpec, nullptr) < 0) {
            std::printf("SDL_OpenAudio warning: %s\n", SDL_GetError());
        }
    }

#ifdef __EMSCRIPTEN__
    // On WebAssembly, wait for the user to drop/select the .nes ROM via HTML UI
    emscripten_set_main_loop(mainLoopTick, 0, 1);
#else
    startGameWithRom(Configuration::getRomFileName().c_str());
    const uint32_t frameDelay = 1000 / Configuration::getFrameRate();
    while (g_running) {
        uint32_t startTicks = SDL_GetTicks();
        mainLoopTick();
        uint32_t elapsed = SDL_GetTicks() - startTicks;
        if (elapsed < frameDelay) {
            SDL_Delay(frameDelay - elapsed);
        }
    }
#endif

    if (Configuration::getAudioEnabled()) {
        SDL_CloseAudio();
    }
    delete g_smbEngine;
    SDL_DestroyTexture(g_texture);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    return 0;
}
