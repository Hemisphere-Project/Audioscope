#include "AudioFileSourceSD.h"
//#include "AudioFileSourcePROGMEM.h"
//#include "viola.h"

//#include "AudioGeneratorMP3.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include <SD.h>

//AudioGeneratorMP3 *gen;
AudioGeneratorWAV *gen;

AudioFileSourceSD *file;
//AudioFileSourcePROGMEM *file;

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

    // List media
    /*File dir = SD.open("/");
    while (true) {
      File entry =  dir.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      LOGINL(entry.name());
      if (!entry.isDirectory()) {
        // files have sizes, directories do not
        LOGINL("\t\t");
        LOG(entry.size());
      }
      entry.close();
    }*/
    
    audio_sdOK = true;
  }
  
  out = new AudioOutputI2S();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain( settings_get("gain") );  

  //gen = new AudioGeneratorMP3();
  gen = new AudioGeneratorWAV();

  audio_volume(0);
  
  return audio_sdOK;
}

bool audio_play(String filePath) 
{
  if (gen->isRunning()) audio_stop();
  if (filePath == "") return false;

  filePath = "/"+filePath;
  //filePath = "/marimba.wav";

  if (gen->isRunning()) gen->stop();
  file = new AudioFileSourceSD(filePath.c_str());
  //file = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  if (gen->begin(file, out)) {
    audio_currentFile = filePath;
    audio_errorPlayer = "";
    LOG("play: "+filePath);
    audio_volume(100);
    return true;
  }
  else {
    LOG("not found: "+filePath);
    audio_errorPlayer = "not found ("+filePath+")";
    audio_stop();
    return false;
  }
}

void audio_stop() 
{
  if (!gen->isRunning()) return;
  audio_volume(0);
  //gen->stop();
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
    if (gen->loop()) return true;                             // playback is still running
    else if (audio_loopMedia && audio_currentFile != "") {    // file end: should i loop ?
      //audio_play(currentFile);
      file->seek(0,SEEK_SET);
      LOG("loop: "+audio_currentFile);
      return true;
    }
    else audio_stop();
  }
  return false;
}

bool audio_running() {
  return gen->isRunning();
}

String audio_media(){
  return audio_currentFile;
}
