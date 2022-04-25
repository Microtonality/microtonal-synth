# Microtonal Synthesizer
A software based modular synthesizer that can recreate microtones.
## How to Install
The latest version can be installed from the <a href="https://github.com/jakedco/microtonal-synth/releases" target="_blank">releases</a> page.
## Developer Quick Start
### All Operating Systems
   1. Download the JUCE Framework from this <a href="https://juce.com/get-juce/download" target="_blank">link</a>.
   2. Download the Plugin GUI Magic framework from this <a href="https://github.com/ffAudio/foleys_gui_magic/" target="_blank">link</a>.
      * This folder will have to be renamed from ```foleys_gui_magic-main``` to ```foleys_gui_magic```
      * Create a new folder in the JUCE folder called ```user_modules``` and move ```foleys_gui_magic``` to this folder
         * Example using Windows: ```C:\Path\To\Folder\juce-*\user_modules\foleys_gui_magic``` 
   3. Extract the contents and open the Projucer application.
   4. In the Projucer application:
       * Go to ```File > Open```, choose the ```MicrotonalKeyboardComponent.jucer``` file from this repo, and click ```Open```
       * Go to ```File > Global Paths...```
         * Change ```Path to JUCE``` to the JUCE folder you downloaded using the ```...``` button. 
           * Example using Windows: ```C:\Path\To\Folder\juce-*```
         * Change ```JUCE Modules``` to the modules folder using the ```...``` button.
           * Example using Windows: ```C:\Path\To\Folder\juce-*\modules```
         * Change ```User Modules``` to the JUCE folder named ```user_modules``` using the ```...``` button.
            * This folder will have to be manually created and should be in the JUCE folder 
            * Example using Windows: ```C:\Path\To\Folder\juce-*\user_modules```         
         * Close the ```Global Paths...``` window.

### Windows
   1. Choose Visual Studio 2019 (<a href="https://visualstudio.microsoft.com/vs/" target="_blank">download link</a>) as ```Selected exporter``` and click on the logo.  <img src="https://user-images.githubusercontent.com/68195709/139920230-ff2ca47c-8c65-49bf-adfc-e58c5a05546d.png" alt="drawing" width="25"/>
   2. Once Visual Studio 2019 is open, click <img src="https://user-images.githubusercontent.com/68195709/139921166-bfed9fe9-3452-4e62-b50f-7916a08391a6.png" alt="drawing" width="150"/> and the application will start.
### macOS
   1. Download and install the Projucer, Xcode, ```foleys_gui_magic``` (and place it anywhere in your filesystem), and this repository.
   2. Double click ```Microtonal Synth.jucer```, and your Mac should know to use the projucer to open the file.
   3. Under modules, fix the path for ```foleys_gui_magic``` to where you placed it when you downloaded it.
   4. Under exporters, click the ```+``` button, and add ```Xcode (macOS)``` as an exporter.
   6. At the top of the Projucer, in the ```Selected Exporter``` dropdown, switch to Xcode, and then click the Xcode button to export.
   7. You should be able to build and run. If you run into any issues, double check your build targets.
### Linux
   1. Download the ```Projucer``` and ```Code::Blocks``` applications from your distribution's package manager or software center.
      * You need to open Code::Blocks once before JUCE projects can be loaded into it.
   2. Open the Projucer application:
       * Go to ```File > Open```, choose the ```MicrotonalKeyboardComponent.jucer``` file from this repo, and click ```Open```
       * Choose Code::Blocks (Linux) as ```Selected exporter```
       * Go to ```File > Save``` then ```File > Save Project```
       * Navigate to the new folder that was created by saving the project ```/Path/To/Folder/microtonal-synth/Builds/CodeBlocksLinux```
       * Click on ```Microtonal Synth.cbp```
         *  This is the exported file for loading the project in an IDE
         *  If Code::Blocks does not open, try opening Code::Blocks first before opening this file
   3. Once Code::Blocks is open, click on the ```settings cog``` to build the project
   4. Once the build is complete, click the ```play button``` to run.
