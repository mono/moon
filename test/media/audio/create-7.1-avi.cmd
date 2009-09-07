
rem 
rem http://www.microsoft.com/windows/windowsmedia/howto/articles/creating71audio.aspx
rem get WavAviMux here: http://www.microsoft.com/downloads/details.aspx?FamilyId=72F6F2FA-0ABE-4A92-9DD0-FD35B966825C&displaylang=en
rem unzip to WAVAVIMUX_PATH below

set WAVAVIMUX_PATH=I:\Programas\Audio - Video\Windows Media Mono to Multichannel Wave Combiner\wavavimux_setup\
set WaVAVIMUX=%WAVAVIMUX_PATH%\WavAviMux.exe

set OUT=BigBuckBunny-9-16-7.1.avi
set FRONTLEFT=BigBuckBunny-DVDMaster-L-9-16.wav
set FRONTRIGHT=BigBuckBunny-DVDMaster-R-9-16.wav
set FRONTCENTER=BigBuckBunny-DVDMaster-C-9-16.wav
set LOWFREQ=BigBuckBunny-DVDMaster-LFE-9-16.wav
set BACKLEFT=BigBuckBunny-DVDMaster-LS-9-16.wav
set BACKRIGHT=BigBuckBunny-DVDMaster-RS-9-16.wav
set SIDELEFT=BigBuckBunny-DVDMaster-LS-9-16.wav
set SIDERIGHT=BigBuckBunny-DVDMaster-RS-9-16.wav


regsvr32 "%WAVAVIMUX_PATH%\spgi.ax" /s
"%WAVAVIMUX%" -mask 255 -o %OUT% -iwav 8 %FRONTLEFT% %FRONTRIGHT% %FRONTCENTER% %LOWFREQ% %BACKLEFT% %BACKRIGHT% %SIDELEFT% %SIDERIGHT%
regsvr32 "%WAVAVIMUX_PATH%\spgi.ax" /u /s