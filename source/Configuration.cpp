// File: source/Configuration.cpp
#include "Configuration.hpp"

bool Configuration::audioEnabled = true;
int Configuration::audioFrequency = 48000;
int Configuration::frameRate = 60;
std::string Configuration::paletteFileName = "";
int Configuration::renderScale = 3;
std::string Configuration::romFileName = "/smb.nes";
bool Configuration::scanlinesEnabled = false;
bool Configuration::vsyncEnabled = true;

void Configuration::initialize(const std::string& configFileName) {
    (void)configFileName;
}

bool Configuration::getAudioEnabled() { return audioEnabled; }
int Configuration::getAudioFrequency() { return audioFrequency; }
int Configuration::getFrameRate() { return frameRate; }
const std::string& Configuration::getPaletteFileName() { return paletteFileName; }
int Configuration::getRenderScale() { return renderScale; }
const std::string& Configuration::getRomFileName() { return romFileName; }
bool Configuration::getScanlinesEnabled() { return scanlinesEnabled; }
bool Configuration::getVsyncEnabled() { return vsyncEnabled; }
