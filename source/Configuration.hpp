#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>

class Configuration
{
public:
    static void initialize(const std::string& configFileName);

    static bool getAudioEnabled();
    static int getAudioFrequency();
    static int getFrameRate();
    static const std::string& getPaletteFileName();
    static int getRenderScale();
    static const std::string& getRomFileName();
    static bool getScanlinesEnabled();
    static bool getVsyncEnabled();

private:
    static bool audioEnabled;
    static int audioFrequency;
    static int frameRate;
    static std::string paletteFileName;
    static int renderScale;
    static std::string romFileName;
    static bool scanlinesEnabled;
    static bool vsyncEnabled;
};

#endif
