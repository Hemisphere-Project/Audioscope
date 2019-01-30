#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

AudioGeneratorWAV *gen;
AudioFileSourceSD *file;
AudioOutputI2S *out;
String audio_currentFile = "";
bool audio_loopMedia = false;
bool audio_sdOK = false;
String audio_errorPlayer = "";

bool audio_setup()
{
  // CS / SS  GPIO for SD module
  if (!SD.begin()) {
    LOG("SD card error");
  }
  else {
    LOG("SD card OK");
    audio_sdOK = true;
  }

  out = new AudioOutputI2S();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain( settings_get("gain") );

  gen = new AudioGeneratorWAV();

  audio_volume(100);

  return audio_sdOK;
}

bool audio_play(String filePath)
{
  if (gen->isRunning()) audio_stop();

  file = new AudioFileSourceSD(filePath.c_str());
  if (gen->begin(file, out)) {
    audio_currentFile = filePath;
    audio_errorPlayer = "";
    LOG("play: "+filePath);
    return true;
  }
  else {
    LOG("not found: "+filePath);
    audio_errorPlayer = "not found ("+filePath+")";
    audio_stop();
    return false;
  }
}

void audio_end()
{
  gen->stop();
  // out->stop(); // FIX to avoid hanging value : see https://github.com/earlephilhower/ESP8266Audio/issues/149
}

void audio_stop()
{
  audio_end();
  audio_currentFile = "";
  audio_errorPlayer = "";
  LOG("stop");
}

void audio_volume(int vol)
{
  LOGF("GAIN: %i\n", vol);
  float v = vol * settings_get("gain") / 10000.0;
  out->SetGain(v);
  LOGF("gain: %f\n", v);
}

void audio_loop(bool doLoop)
{
  audio_loopMedia = doLoop;
}

bool audio_run()
{
  if (gen->isRunning()) {
    if (gen->loop()) return true;
    else if (audio_loopMedia && audio_currentFile != "") {
      audio_play(audio_currentFile);
      //file->seek(0,SEEK_SET);
      LOG("loop: "+audio_currentFile);
      return true;
    }
    else audio_end();
  }
  return false;
}

bool audio_running() {
  return gen->isRunning();
}

String audio_media(){
  return audio_currentFile;
}
