

#include <iostream>


int main()
{
    char* str = "# emotionally-aware-"
   "SMIs\r\n"
   "Embedded implementat"
   "ion of an emotion cl"
   "assifier for a Smart"
   " Musical Instrument "
   "(electric guitar, ac"
   "oustic guitar and pi"
   "ano).\r\n\r\n"
   "Repository organizat"
   "ion\r\n"
   "- Folder `EmotionCla"
   "ssificationPlugin` c"
   "ontains a JUCE plugi"
   "n that records audio"
   ", extracts features "
   "and executes inferen"
   "ce with a custom mod"
   "el. It also contains"
   " the subfolder `pyth"
   "on-osc-server` with "
   "the OSC server to ru"
   "n on a Elk Audio OS "
   "board, and `libs` wi"
   "th the dependencies."
   "\r\n"
   "- Folder `OSCcontrol"
   "ler` contains a Pure"
   " Data patch and acco"
   "mpanying abstraction"
   " to control the plug"
   "in via OSC messages."
   "\r\n"
   "- Folder `docs/image"
   "s` contains images i"
   "ncluded in this READ"
   "ME.\r\n\r\n"
   "AAA (EmotionClassifi"
   "cationPlugin/libs)\r"
   "\n\r\n\r\n";
//   "- **eigen** [3.3.4]("
//   "https://gitlab.com/l"
//   "ibeigen/eigen/-/rele"
//   "ases/3.3.4) ([gitlab"
//   ".com/libeigen/eigen]"
//   "(https://gitlab.com/"
//   "libeigen/eigen) comm"
//   "it [`3dc3a0ea2d0773a"
//   "f4c0ffd7bbcb21c608e2"
//   "8fcef`](https://gitl"
//   "ab.com/libeigen/eige"
//   "n/tree/3dc3a0ea2d077"
//   "3af4c0ffd7bbcb21c608"
//   "e28fcef))\r\n"
//   "- **ffmpeg** - [3.4."
//   "12](https://github.c"
//   "om/FFmpeg/FFmpeg/rel"
//   "eases/tag/n3.4.12) ("
//   "[github.com/FFmpeg/F"
//   "Fmpeg](https://githu"
//   "b.com/FFmpeg/FFmpeg)"
//   " commit [`872001459c"
//   "f0a20c6f44105f485d12"
//   "5c8e22fc76`](https:/"
//   "/github.com/FFmpeg/F"
//   "Fmpeg/tree/872001459"
//   "cf0a20c6f44105f485d1"
//   "25c8e22fc76))\r\n"
//   "- **essentia** - mai"
//   "n branch ([github.co"
//   "m/MTG/essentia](http"
//   "s://github.com/MTG/e"
//   "ssentia) commit [`32"
//   "376db9b39d8692509ac5"
//   "8036d0b539b7e`](http"
//   "s://github.com/MTG/e"
//   "ssentia/tree/32376db"
//   "9b39d8692509ac58036d"
//   "0b539b7e))\r\n"
//   "- **tflite** - 2.11."
//   "0 from [domenicostef"
//   "ani/deep-classf-runt"
//   "ime-wrappers](https:"
//   "//github.com/domenic"
//   "ostefani/deep-classf"
//   "-runtime-wrappers)\r"
//   "\n\r\n"
//   "Instruction to use t"
//   "he Emotion Classific"
//   "ation Plugin\r\n\r\n"
//   "The plugin is made t"
//   "o be used both as a "
//   "headless plugin in ["
//   "Elk Audio Os](https:"
//   "//www.elk.audio/star"
//   "t) and standalone/VS"
//   "T2/3 plugin on a reg"
//   "ular computer (curre"
//   "ntly only compiled f"
//   "or `linux amd64` tho"
//   "ugh).  \r\n"
//   "#GUI version\r\n"
//   "On a regular compute"
//   "r, the usage is stra"
//   "ightforward: open th"
//   "e plugin either as a"
//   " standalone or in a "
//   "DAW, select the fold"
//   "er where to save the"
//   " recordings and the "
//   "neural model, and pr"
//   "ess the record butto"
//   "n. Upon a second pre"
//   "ss, the recording st"
//   "ops and inference is"
//   " executed for every "
//   "3 seconds of audio."
//   "\r\n"
//   "#Elk OS Version\r\n"
//   "Since the execution "
//   "on the Elk board is "
//   "headless, the plugin"
//   " is controlled via O"
//   "SC messages from a d"
//   "esktop or laptop com"
//   "puter (we will refer"
//   " to this as *control"
//   "ler*). A Pure Data c"
//   "ontroller applicatio"
//   "n is provided in `OS"
//   "Ccontroller/`.\r\n\r"
//   "\n"
//   "##Step-by-step instr"
//   "uctions:\r\n"
//   "1. **Connect the Elk"
//   " board to a laptop/d"
//   "esktop computer via "
//   "ethernet cable**. Yo"
//   "u might need to crea"
//   "te a shared ethernet"
//   " network to communic"
//   "ate with the board a"
//   "nd have it set its c"
//   "urrent date properly"
//   ".  \r\n"
//   "**Turn on the board "
//   "after connecting it*"
//   "*.\r\n\r\n"
//   "2. **Find the IP add"
//   "ress of the board**:"
//   " Find the IP of the "
//   "board. On a Unix ter"
//   "minal, you can run `"
//   "$ ping elk-pi.local`"
//   ", or inspect the out"
//   "put of `$ arp -a`.  "
//   "\r\n"
//   "To make sure it\'s t"
//   "he correct address y"
//   "ou can connect via s"
//   "sh with `$ ssh mind@"
//   "elk-pi.local` (passw"
//   "ord is `elk`).  \r\n"
//   "![Alt readme_content](docs//im"
//   "ages/ip_board.png)  "
//   "\r\n\r\n"
//   "3. **Find the IP add"
//   "ress of the controll"
//   "er**: Find the IP of"
//   " the controller **in"
//   " the same network** "
//   "that is shared with "
//   "the board. You can d"
//   "o this by running `$"
//   " ifconfig` on a Unix"
//   " terminal and lookin"
//   "g for the IP address"
//   " that looks like the"
//   " one of the board, a"
//   "part from the last n"
//   "umber block. For exa"
//   "mple, if the board h"
//   "as IP `10.42.0.47`, "
//   "the controller might"
//   " have address `10.42"
//   ".0.1`. To confirm th"
//   "is you can ssh into "
//   "the controller (see "
//   "previous step) and p"
//   "ing the address, wai"
//   "ting for a positive "
//   "response.  \r\n"
//   "![Alt readme_content](docs//im"
//   "ages/ip_controller.p"
//   "ng)  \r\n\r\n"
//   "4. **Open ports on c"
//   "ontroller\'s firewal"
//   "l**: Add rules to th"
//   "e *controller*\'s fi"
//   "rewall to open the p"
//   "orts used for OSC co"
//   "mmunication. You sho"
//   "uld allow incoming t"
//   "raffic on ports `704"
//   "2` and `9042`. For m"
//   "ore security you can"
//   " limit the traffic t"
//   "o the IP address of "
//   "the board (make sure"
//   " it doesn\'t change "
//   "though) or the subne"
//   "t. On a Linux comput"
//   "er with the UFW fire"
//   "wall, if the board h"
//   "as address `10.42.0."
//   "47` and the controll"
//   "er has address `10.4"
//   "2.0.1` you can run t"
//   "he following command"
//   "s:  \r\n"
//   "`$ sudo ufw allow fr"
//   "om 10.42.0.0/16 to a"
//   "ny port 7042`  \r\n"
//   "`$ sudo ufw allow fr"
//   "om 10.42.0.0/16 to a"
//   "ny port 9042`  \r\n"
//   "These should also en"
//   "sure that, if the IP"
//   " addresses change, t"
//   "he ports are still o"
//   "pen given that the s"
//   "ubnet is quite loose"
//   ".\r\n"
//   "Result:\r\n"
//   "    ```\r\n"
//   "    $ ufw status\r\n"
//   "\r\n"
//   "    To              "
//   "           Action   "
//   "   From\r\n"
//   "    --              "
//   "           ------   "
//   "   ----             "
//   " \r\n"
//   "    7042            "
//   "           ALLOW    "
//   "   10.42.0.0/16     "
//   "         \r\n"
//   "    9042            "
//   "           ALLOW    "
//   "   10.42.0.0/16 \r\n"
//   "    ```\r\n"
//   "    Outgoing traffic"
//   " should be allowed b"
//   "y default on any gen"
//   "eral-purpose compute"
//   "r, but if not, you s"
//   "hould enable outgoin"
//   "g traffic for ports "
//   "`6042`, `8042` and `"
//   "24024`. More info in"
//   " [OSC connection not"
//   "es](#osc-connection-"
//   "notes)\r\n\r\n"
//   "5. **[Optional] Prep"
//   "are the list of file"
//   "names for your recor"
//   "ding session**: In `"
//   "OSCcontroller/data/f"
//   "ilenames.csv` you sh"
//   "ould write the list "
//   "of filenames to reco"
//   "rd. Follow the examp"
//   "le in the same folde"
//   "r and use search/rep"
//   "lace on a readme_content edito"
//   "r. It\'s suggested t"
//   "o have a few test na"
//   "mes at the beginning"
//   " to test levels.\r\n"
//   "\r\n"
//   "6. **Run the control"
//   "ler**: Open the Pure"
//   " Data patch `OSCcont"
//   "roller/main.pd` and "
//   "check on the console"
//   " that there are no e"
//   "rrors. The patch dep"
//   "ends on some externa"
//   "l libraries like `ze"
//   "xy`, `cyclone`, `iem"
//   "lib` and `mrpeach`."
//   "\r\n\r\n"
//   "7. **Plug the IP add"
//   "resses and connect**"
//   ": Put the correct IP"
//   " addresses in the tw"
//   "o message boxes on t"
//   "he top left. Do not "
//   "use hostnames like "
//   "\"elk-pi.local\". Th"
//   "en press the connect"
//   " button. If everythi"
//   "ng is correct, the "
//   "\"Connected\" toggle"
//   " in the monitoring a"
//   "rea will turn on. Th"
//   "e controller is now "
//   "communicating with a"
//   " server on the devic"
//   "e, no plugin is runn"
//   "ing yet.  \r\n"
//   "![Alt readme_content](docs//im"
//   "ages/connect.png)\r"
//   "\n"
//   "![Alt readme_content](docs//im"
//   "ages/connection_stat"
//   "e.png)  \r\n\r\n\r\n"
//   "8. **Run a plugin**:"
//   " Press one of the bu"
//   "ttons labeled as \"s"
//   "tart_[...]_plugin. A"
//   "fter a few seconds, "
//   "the toggle \"No plug"
//   "in started\" will tu"
//   "rn off while one of "
//   "the [...]_Plugin_sta"
//   "rted will turn on it"
//   ".  \r\n"
//   "at this point the le"
//   "vel meter **must** s"
//   "tart moving ever so "
//   "slightly. If it does"
//   "n\'t, something went"
//   " wrong and you shoul"
//   "d check the console "
//   "of the controller an"
//   "d the board.  \r\n"
//   "![Alt readme_content](docs//im"
//   "ages/monitor_level.p"
//   "ng)  \r\n"
//   "To access the board "
//   "server output you ca"
//   "n ssh into the board"
//   " and run `$ tmux a`."
//   " To detach the tmux "
//   "session use `$ ctrl+"
//   "b d`.\r\n\r\n"
//   "9. **Set the input g"
//   "ain**: Use the \"Rec"
//   "ording_gain\" slider"
//   " to set the input ga"
//   "in. The meter will s"
//   "how changes while so"
//   "und is fed to the bo"
//   "ard. It\'s still adv"
//   "ised to record a few"
//   " test take, move the"
//   "m to the computer (v"
//   "ia `scp` or `sftp`, "
//   "see next steps) and "
//   "check the levels the"
//   "re. **Make sure that"
//   " there is no clippin"
//   "g** and the level is"
//   " not too low.\r\n\r"
//   "\n"
//   "10. **Classify the i"
//   "ntended emotion of a"
//   "n improvisation**: P"
//   "ress the \"record\" "
//   "button on the contro"
//   "ller. The recording "
//   "will start and the "
//   "\"Plugin state\" che"
//   "ckbox array will go "
//   "from \"Idle\" to \"R"
//   "ecording\".  \r\n"
//   "When you are done, p"
//   "ress Stop_and_classi"
//   "fy. \r\n"
//   "The recording will s"
//   "top and the plugin w"
//   "ill start classifyin"
//   "g the audio. The \"P"
//   "lugin state\" checkb"
//   "ox array will go fro"
//   "m \"Recording\" to "
//   "\"Classifying\" and "
//   "then back to \"Idle"
//   "\".\r\n\r\n"
//   "11. **Inspect result"
//   "s**: After a short w"
//   "hile, the \"Results"
//   "\" canvas will show "
//   "the predicted emotio"
//   "n for the improvised"
//   " recording. There ca"
//   "n be a single emotio"
//   "n or multiple ones i"
//   "n the case of an amb"
//   "ivalent prediction. "
//   " \r\n"
//   "For a more in-depth "
//   "analysis or debut, y"
//   "ou can check the [Au"
//   "dacity](https://www."
//   "audacityteam.org/) l"
//   "abel files in the `/"
//   "udata/emoAwSMIs-reco"
//   "rdings folder on the"
//   " board`. These can b"
//   "e loaded in Audacity"
//   " along with the reco"
//   "rding found in the s"
//   "ame folder. For each"
//   " audio file, there a"
//   "re 2 label files whe"
//   "re the first one wil"
//   "l show the framewise"
//   " result, while the s"
//   "econd will show the "
//   "actual framewise net"
//   "work output (Which s"
//   "hould be a SoftMax a"
//   "rray).  \r\n"
//   "*Note:* A result wil"
//   "l be ambivalent if t"
//   "he maximum softmax o"
//   "utput (averaged over"
//   " every 3-second fram"
//   "e) is closer than a "
//   "thresholding distanc"
//   "e from the second ma"
//   "ximum. This threshol"
//   "d is set to 1/7 by d"
//   "efault. In this case"
//   ", all the emotions w"
//   "ithin the threshold "
//   "distance from the ma"
//   "ximum will be shown."
//   "\r\n\r\n"
//   "12. **[Optional] REN"
//   "AME THE RECORDING**:"
//   " If, for experiment "
//   "purposes, you want t"
//   "o give specific name"
//   "s to the recording ("
//   "with the list define"
//   "d in step 5) you sho"
//   "uld do it now. **War"
//   "ning:** every new re"
//   "cording is considere"
//   "d \"unnamed\" but it"
//   " is kept if you reco"
//   "rd multiple tracks. "
//   "**However**, to rena"
//   "me a track successfu"
//   "lly there must be on"
//   "e and only one unnam"
//   "ed track. This is to"
//   " make sure to avoid "
//   "renaming mistakes.  "
//   "\r\n"
//   "For this step, inspe"
//   "ct closely the outpu"
//   "t in the Pd console."
//   " Clear it if needed "
//   "(`Ctrl+Shift+L`).\r"
//   "\n"
//   "On the purple area o"
//   "f the controller, pr"
//   "ess Print to see the"
//   " next filename prepa"
//   "red. Press \"Rename "
//   "& prepare NextName\""
//   " to rename the last "
//   "and only unnamed rec"
//   "ording track.  \r\n"
//   "If there is an error"
//   " (no unnamed track, "
//   "or more than one) a "
//   "light will blink a f"
//   "ew times and the con"
//   "sole will show detai"
//   "ls.  \r\n"
//   "If the operation was"
//   " successful, the \"R"
//   "enamed OK\" toggle w"
//   "ill turn on.\r\n\r\n"
//   "13. **[Optional] Cop"
//   "y the recordings to "
//   "a computer**: To cop"
//   "y the recordings to "
//   "a computer, you can "
//   "use the `scp` comman"
//   "d on a Unix terminal"
//   ". If there was no er"
//   "ror in the process, "
//   "all the tracks are i"
//   "n the `/udata/emoAwS"
//   "MIs-recordings/RENAM"
//   "ED/` folder. This in"
//   "cludes also the Auda"
//   "city label files and"
//   " a log file with the"
//   " renaming details.\r"
//   "\n"
//   "If there was an erro"
//   "r in renaming, the t"
//   "racks with their ori"
//   "ginal names are stor"
//   "ed in `/udata/emoAwS"
//   "MIs-recordings/backu"
//   "p/`. Tracks deleted "
//   "with the \"Delete la"
//   "st track\" button ar"
//   "e stored in `/udata/"
//   "emoAwSMIs-recordings"
//   "/trashcan/`.\r\n"
//   "___\r\n\r\n"
//   "OSC connection notes"
//   "\r\n"
//   "Here are some additi"
//   "onal info about how "
//   "the OSC controller c"
//   "ommunicates with the"
//   " board and the plugi"
//   "n.\r\n"
//   "More details about t"
//   "he OSC communication"
//   " procedure can be fo"
//   "und in [docs/images/"
//   "OSCcomm1.png](docs/i"
//   "mages/OSCcomm1.png) "
//   "and [docs/images/OSC"
//   "comm2.png](docs/imag"
//   "es/OSCcomm2.png).\r"
//   "\n"
//   "- Ports `6042` and `"
//   "7042` are used to co"
//   "mmunicate with the P"
//   "ython OSC server tha"
//   "t oversees plugin st"
//   "art/stop operations,"
//   " file renaming and d"
//   "eletion (communicati"
//   "on uses a 2-way hand"
//   "shake (osc /pyshake)"
//   " to verify successfu"
//   "l connection).\r\n"
//   "- Ports `8042` and `"
//   "9042` are used to co"
//   "mmunicate with the p"
//   "lugin itself to rece"
//   "ive loudness meterin"
//   "g, plugin status and"
//   " classification resu"
//   "lts (communication u"
//   "ses a 2-way handshak"
//   "e (osc /handshake) t"
//   "o verify successful "
//   "plugin startup).\r\n"
//   "- Port `24024` is th"
//   "e default port used "
//   "by the Sushi DAW in "
//   "Elk Audio OS to set "
//   "the parameters expos"
//   "ed by VST plugins. I"
//   "t\'s used to start/s"
//   "top recording (setti"
//   "ng the recstate para"
//   "meter to 1 or 0), an"
//   "d input gain (linear"
//   ", [0-1]).\r\n\r\n"
//   "___\r\n"
//   "References\r\n\r\n"
//   "1. Turchet, L., & Pa"
//   "uwels, J. (2021). Mu"
//   "sic emotion recognit"
//   "ion: intention of co"
//   "mposers-performers v"
//   "ersus perception of "
//   "musicians, non-music"
//   "ians, and listening "
//   "machines. IEEE/ACM T"
//   "ransactions on Audio"
//   ", Speech, and Langua"
//   "ge Processing, 30, 3"
//   "05-316.";
   
    std::string readme_content(str);
    
    // remove markdown section from ## Dependencies to next ## section
    size_t pos = readme_content.find("## Dependencies");
    if (pos != std::string::npos) {
        size_t pos2 = readme_content.find("##", pos+1);
        if (pos2 != std::string::npos) {
            readme_content.erase(pos, pos2-pos);
        }
    }

    // Remove the main title (text following the first #)
    pos = readme_content.find("#");
    if (pos != std::string::npos) {
        pos2 = readme_content.find("\n", pos+1);
        if (pos2 != std::string::npos) {
            readme_content.erase(pos, pos2-pos);
        }
    }

    // with regex, remove all markdown links and replace with the link text
    regex link_regex("\\[(.*?)\\]\\((.*?)\\)");
    readme_content = regex_replace(readme_content, link_regex, "$1");

    // with regex, remove all markdown images
    std::regex image_regex("!\\[(.*?)\\]\\((.*?)\\)");
    readme_content = std::regex_replace(readme_content, image_regex, "");

    // with regex, remove all markdown bold and italic formatting
    std::regex bold_regex("\\*\\*(.*?)\\*\\*");
    readme_content = std::regex_replace(readme_content, bold_regex, "$1");



    std::cout << readme_content << std::endl;
    
    return 0;
}
