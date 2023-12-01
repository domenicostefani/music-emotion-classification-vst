#include <JuceHeader.h>

class EmotionalDB : public juce::ChangeListener{
    // 1 Emotion
    std::vector<std::string> aggr;
    std::vector<std::string> rel;
    std::vector<std::string> hap;
    std::vector<std::string> sad;
    // 2 Emotions
    std::vector<std::string> aggr_rel;
    std::vector<std::string> aggr_hap;
    std::vector<std::string> aggr_sad;
    std::vector<std::string> rel_hap;
    std::vector<std::string> rel_sad;
    std::vector<std::string> hap_sad;
    // 3 Emotions
    std::vector<std::string> aggr_rel_hap;
    std::vector<std::string> aggr_rel_sad;
    std::vector<std::string> aggr_hap_sad;
    std::vector<std::string> rel_hap_sad;
    // All 4 Emotions
    std::vector<std::string> aggr_rel_hap_sad;

    // Create map with emption combination (In form of one hot ecoding for aggr, rel, hap, sad - e.g. 1010 for aggr_rel) to vector of strings
    std::map<std::string, std::vector<std::string>> emotion_map;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    } state;
    
    juce::Button* playbutton;
public:
    EmotionalDB() {
        {
            aggr.push_back("249_aggressive_i2_classical-guitar_CesSam_fingers_new4_20200720.wav");
            aggr.push_back("332_aggressive_i1_classical-guitar_AntDel_pick_new8_20200831.wav");
            aggr.push_back("237_aggressive_i2_classical-guitar_FedCer_fingers_new3_20200714.wav");
            aggr.push_back("87_aggressive_i3_steelstring-guitar_TomCan_pick_new1_20200707.wav");
            aggr.push_back("160_aggressive_i1_steelstring-guitar_MasChi_pick_new5_20200729.wav");
            aggr.push_back("162_aggressive_i3_steelstring-guitar_MasChi_pick_new5_20200729.wav");
            aggr.push_back("163_aggressive_i4_steelstring-guitar_MasChi_pick_new5_20200729.wav");
            aggr.push_back("85_aggressive_i1_steelstring-guitar_TomCan_pick_new1_20200707.wav");
            aggr.push_back("236_aggressive_i1_classical-guitar_FedCer_fingers_new3_20200714.wav");
            aggr.push_back("73_aggressive_i1_classical-guitar_MatRig_fingers_new3_20200618.wav");
            aggr.push_back("176_aggressive_i1_steelstring-guitar_FilMel_pick_new6_20200729.wav");
            aggr.push_back("178_aggressive_i3_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr.push_back("322_aggressive_i3_steelstring-guitar_NicCon_pick_new4_20200804.wav");
            aggr.push_back("74_aggressive_i2_classical-guitar_MatRig_fingers_new3_20200618.wav");
            aggr.push_back("214_aggressive_i3_classical-guitar_SalOli_pick_new3_20200714.wav");
            aggr.push_back("190_aggressive_i3_classical-guitar_GioAcq_fingers_new3_20200706.wav");
            aggr.push_back("321_aggressive_i2_steelstring-guitar_NicCon_pick_new4_20200804.wav");
            aggr.push_back("50_aggressive_i2_steelstring-guitar_ValFui_pick_new5_20200627.wav");
            aggr.push_back("213_aggressive_i2_classical-guitar_SalOli_pick_new3_20200714.wav");
            aggr_rel.push_back("103_relaxed_i1_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            aggr_rel.push_back("77_happy_i2_classical-guitar_MatRig_fingers_new3_20200618.wav");
            aggr_rel.push_back("186_sad_i2_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr_rel.push_back("323_happy_i1_steelstring-guitar_NicCon_pick_new4_20200804.wav");
            aggr_rel_hap.push_back("181_happy_i3_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr_rel_hap.push_back("77_happy_i2_classical-guitar_MatRig_fingers_new3_20200618.wav");
            aggr_rel_hap.push_back("179_happy_i1_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr_rel_hap.push_back("323_happy_i1_steelstring-guitar_NicCon_pick_new4_20200804.wav");
            aggr_rel_hap_sad.push_back("187_sad_i3_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr_rel_hap_sad.push_back("56_relaxed_i2_steelstring-guitar_ValFui_fingers_new5_20200627.wav");
            aggr_rel_hap_sad.push_back("348_happy_i2_classical-guitar_NicLat_pick_new7_20200730.wav");
            aggr_rel_hap_sad.push_back("358_aggressive_i3_steelstring-guitar_LucFra_fingers_new6_20200914.wav");
            aggr_rel_hap_sad.push_back("292_relaxed_i3_steelstring-guitar_FraBen_pick_new6_20200801.wav");
            aggr_rel_sad.push_back("278_relaxed_i1_steelstring-guitar_DavRos_pick_new3_20200901.wav");
            aggr_rel_sad.push_back("198_sad_i2_classical-guitar_GioAcq_fingers_new3_20200706.wav");
            aggr_rel_sad.push_back("186_sad_i2_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            aggr_rel_sad.push_back("56_relaxed_i2_steelstring-guitar_ValFui_fingers_new5_20200627.wav");
            aggr_hap.push_back("27_aggressive_i3_classical-guitar_DavBen_pick_new8_20200526.wav");
            aggr_hap.push_back("109_aggressive_i1_steelstring-guitar_SteRom_fingers_new3_20200715.wav");
            aggr_hap.push_back("215_happy_i1_classical-guitar_SalOli_pick_new3_20200714.wav");
            aggr_hap.push_back("274_aggressive_i3_steelstring-guitar_DavRos_pick_new3_20200901.wav");
            aggr_hap.push_back("28_happy_i1_classical-guitar_DavBen_pick_new8_20200526.wav");
            aggr_hap.push_back("149_happy_i1_steelstring-guitar_AleMar_pick_new9_20200724.wav");
            aggr_hap_sad.push_back("370_aggressive_i3_classical-guitar_AngLoi_fingers_new5_20200917.wav");
            aggr_hap_sad.push_back("348_happy_i2_classical-guitar_NicLat_pick_new7_20200730.wav");
            aggr_hap_sad.push_back("109_aggressive_i1_steelstring-guitar_SteRom_fingers_new3_20200715.wav");
            aggr_hap_sad.push_back("274_aggressive_i3_steelstring-guitar_DavRos_pick_new3_20200901.wav");
            aggr_sad.push_back("331_sad_i3_steelstring-guitar_NicCon_pick_new4_20200804.wav");
            aggr_sad.push_back("59_sad_i2_steelstring-guitar_ValFui_fingers_new5_20200627.wav");
            aggr_sad.push_back("393_aggressive_i2_classical-guitar_MarPia_fingers_new5_20201001.wav");
            aggr_sad.push_back("353_sad_i1_classical-guitar_NicLat_pick_new7_20200730.wav");
            aggr_sad.push_back("358_aggressive_i3_steelstring-guitar_LucFra_fingers_new6_20200914.wav");
            aggr_sad.push_back("173_sad_i2_steelstring-guitar_MasChi_pick_new5_20200729.wav");
            aggr_sad.push_back("212_aggressive_i1_classical-guitar_SalOli_fingers_new3_20200714.wav");
            aggr_sad.push_back("107_sad_i2_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            aggr_sad.push_back("158_sad_i3_steelstring-guitar_AleMar_pick_new9_20200724.wav");
            aggr_sad.push_back("95_sad_i2_steelstring-guitar_TomCan_pick_new1_20200707.wav");
            aggr_sad.push_back("188_aggressive_i1_classical-guitar_GioAcq_fingers_new3_20200706.wav");
            rel.push_back("399_relaxed_i2_classical-guitar_MarPia_fingers_new5_20201001.wav");
            rel.push_back("302_relaxed_i1_steelstring-guitar_GiaFer_fingers_new9_20200803.wav");
            rel.push_back("45_relaxed_i3_steelstring-guitar_OweWin_pick_new5_20200629.wav");
            rel.push_back("290_relaxed_i1_steelstring-guitar_FraBen_pick_new6_20200801.wav");
            rel.push_back("292_relaxed_i3_steelstring-guitar_FraBen_pick_new6_20200801.wav");
            rel.push_back("315_relaxed_i2_steelstring-guitar_GioDic_fingers_new5_20200821.wav");
            rel.push_back("339_relaxed_i2_classical-guitar_AntDel_fingers_new8_20200831.wav");
            rel.push_back("43_relaxed_i1_steelstring-guitar_OweWin_pick_new5_20200629.wav");
            rel.push_back("55_relaxed_i1_steelstring-guitar_ValFui_pick_new5_20200627.wav");
            rel.push_back("9_relaxed_i3_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            rel.push_back("153_relaxed_i2_steelstring-guitar_AleMar_pick_new9_20200724.wav");
            rel.push_back("155_relaxed_i4_steelstring-guitar_AleMar_pick_new9_20200724.wav");
            rel.push_back("91_relaxed_i1_steelstring-guitar_TomCan_pick_new1_20200707.wav");
            rel.push_back("183_relaxed_i2_steelstring-guitar_FilMel_fingers_new6_20200729.wav");
            rel_hap.push_back("337_happy_i3_classical-guitar_AntDel_fingers_new8_20200831.wav");
            rel_hap.push_back("277_happy_i3_steelstring-guitar_DavRos_pick_new3_20200901.wav");
            rel_hap.push_back("16_happy_i1_steelstring-guitar_DavBen_fingers_new4_20200623.wav");
            rel_hap.push_back("112_happy_i1_steelstring-guitar_SteRom_fingers_new3_20200715.wav");
            rel_hap.push_back("265_happy_i3_steelstring-guitar_AntPao_pick_new9_20200801.wav");
            rel_hap.push_back("42_happy_i3_steelstring-guitar_OweWin_fingers_new5_20200629.wav");
            rel_hap.push_back("396_happy_i2_classical-guitar_MarPia_fingers_new5_20201001.wav");
            rel_hap.push_back("105_relaxed_i3_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            rel_hap.push_back("312_happy_i2_steelstring-guitar_GioDic_fingers_new5_20200821.wav");
            rel_hap.push_back("359_happy_i1_steelstring-guitar_LucFra_fingers_new6_20200914.wav");
            rel_hap.push_back("395_happy_i1_classical-guitar_MarPia_fingers_new5_20201001.wav");
            rel_hap.push_back("397_happy_i3_classical-guitar_MarPia_fingers_new5_20201001.wav");
            rel_hap.push_back("21_relaxed_i3_steelstring-guitar_DavBen_fingers_new4_20200623.wav");
            rel_hap_sad.push_back("376_relaxed_i3_classical-guitar_AngLoi_fingers_new5_20200917.wav");
            rel_hap_sad.push_back("364_relaxed_i3_steelstring-guitar_LucFra_fingers_new6_20200914.wav");
            rel_hap_sad.push_back("117_relaxed_i3_steelstring-guitar_SteRom_fingers_new3_20200715.wav");
            rel_hap_sad.push_back("19_relaxed_i1_steelstring-guitar_DavBen_pick_new4_20200623.wav");
            rel_hap_sad.push_back("206_relaxed_i1_classical-guitar_TizBol_fingers_new7_20200630.wav");
            rel_hap_sad.push_back("255_relaxed_i2_classical-guitar_CesSam_fingers_new4_20200720.wav");
            rel_hap_sad.push_back("67_relaxed_i1_steelstring-guitar_AdoLaV_pick_new8_20200619.wav");
            rel_sad.push_back("378_sad_i2_classical-guitar_AngLoi_fingers_new5_20200917.wav");
            rel_sad.push_back("10_sad_i1_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            rel_sad.push_back("390_sad_i2_steelstring-guitar_AngLoi_fingers_new3_20200917.wav");
            rel_sad.push_back("377_sad_i1_classical-guitar_AngLoi_fingers_new5_20200917.wav");
            rel_sad.push_back("47_sad_i2_steelstring-guitar_OweWin_pick_new5_20200629.wav");
            rel_sad.push_back("268_relaxed_i3_steelstring-guitar_AntPao_pick_new9_20200801.wav");
            rel_sad.push_back("8_relaxed_i2_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            rel_sad.push_back("281_sad_i1_steelstring-guitar_DavRos_pick_new3_20200901.wav");
            rel_sad.push_back("70_sad_i1_steelstring-guitar_AdoLaV_pick_new8_20200619.wav");
            rel_sad.push_back("71_sad_i2_steelstring-guitar_AdoLaV_pick_new8_20200619.wav");
            rel_sad.push_back("72_sad_i3_steelstring-guitar_AdoLaV_pick_new8_20200619.wav");
            rel_sad.push_back("304_relaxed_i3_steelstring-guitar_GiaFer_fingers_new9_20200803.wav");
            rel_sad.push_back("80_relaxed_i2_classical-guitar_MatRig_fingers_new3_20200618.wav");
            rel_sad.push_back("11_sad_i2_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            rel_sad.push_back("82_sad_i1_classical-guitar_MatRig_fingers_new3_20200618.wav");
            hap.push_back("4_happy_i1_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            hap.push_back("88_happy_i1_steelstring-guitar_TomCan_pick_palm_mute_new1_20200707.wav");
            hap.push_back("40_happy_i1_steelstring-guitar_OweWin_pick_new5_20200629.wav");
            hap.push_back("101_happy_i2_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            hap.push_back("289_happy_i3_steelstring-guitar_FraBen_fingers_new6_20200801.wav");
            hap.push_back("216_happy_i2_classical-guitar_SalOli_fingers_new3_20200714.wav");
            hap.push_back("217_happy_i3_classical-guitar_SalOli_pick_new3_20200714.wav");
            hap.push_back("191_happy_i1_classical-guitar_GioAcq_fingers_new3_20200706.wav");
            hap.push_back("165_happy_i2_steelstring-guitar_MasChi_pick_new5_20200729.wav");
            hap.push_back("137_happy_i2_steelstring-guitar_SamLor_pick_new2_20200715.wav");
            hap.push_back("264_happy_i2_steelstring-guitar_AntPao_pick_new9_20200801.wav");
            hap.push_back("193_happy_i3_classical-guitar_GioAcq_fingers_new3_20200706.wav");
            hap.push_back("300_happy_i2_steelstring-guitar_GiaFer_pick_new9_20200803.wav");
            hap.push_back("29_happy_i2_classical-guitar_DavBen_fingers_new8_20200526.wav");
            hap_sad.push_back("126_happy_i3_steelstring-guitar_SimArm_fingers_new5_20200711.wav");
            hap_sad.push_back("373_happy_i3_classical-guitar_AngLoi_fingers_new5_20200917.wav");
            hap_sad.push_back("403_sad_i3_classical-guitar_MarPia_fingers_new5_20201001.wav");
            hap_sad.push_back("203_happy_i1_classical-guitar_TizBol_fingers_new7_20200630.wav");
            hap_sad.push_back("222_sad_i2_classical-guitar_SalOli_fingers_new3_20200714.wav");
            sad.push_back("94_sad_i1_steelstring-guitar_TomCan_pick_new1_20200707.wav");
            sad.push_back("108_sad_i3_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            sad.push_back("12_sad_i3_steelstring-guitar_LucTur_pick_new1_20200625.wav");
            sad.push_back("130_sad_i1_steelstring-guitar_SimArm_fingers_new5_20200711.wav");
            sad.push_back("159_sad_i4_steelstring-guitar_AleMar_pick_new9_20200724.wav");
            sad.push_back("106_sad_i1_steelstring-guitar_TizCam_pick_new5_20200715.wav");
            sad.push_back("403_sad_i3_classical-guitar_MarPia_fingers_new5_20201001.wav");
            sad.push_back("222_sad_i2_classical-guitar_SalOli_fingers_new3_20200714.wav");
            sad.push_back("402_sad_i2_classical-guitar_MarPia_fingers_new5_20201001.wav");
            sad.push_back("245_sad_i1_classical-guitar_FedCer_fingers_new3_20200714.wav");
            sad.push_back("257_sad_i1_classical-guitar_CesSam_fingers_new4_20200720.wav");
            sad.push_back("258_sad_i2_classical-guitar_CesSam_fingers_new4_20200720.wav");
            sad.push_back("259_sad_i3_classical-guitar_CesSam_fingers_new4_20200720.wav");
            sad.push_back("270_sad_i2_steelstring-guitar_AntPao_pick_new9_20200801.wav");
            sad.push_back("307_sad_i3_steelstring-guitar_GiaFer_pick_new9_20200803.wav");
        }
        emotion_map["1000"] = aggr;
        emotion_map["0100"] = rel;
        emotion_map["0010"] = hap;
        emotion_map["0001"] = sad;
        emotion_map["1100"] = aggr_rel;
        emotion_map["1010"] = aggr_hap;
        emotion_map["1001"] = aggr_sad;
        emotion_map["0110"] = rel_hap;
        emotion_map["0101"] = rel_sad;
        emotion_map["0011"] = hap_sad;
        emotion_map["1110"] = aggr_rel_hap;
        emotion_map["1101"] = aggr_rel_sad;
        emotion_map["1011"] = aggr_hap_sad;
        emotion_map["0111"] = rel_hap_sad;
        emotion_map["1111"] = aggr_rel_hap_sad;

        formatManager.registerBasicFormats();
        transportSource.addChangeListener (this);
    }

    EmotionalDB(juce::Button& playbutton) : EmotionalDB() {
        this->playbutton = &playbutton;
    }

    

    void playTracksEmo(int emoOneHot, size_t n) {
        // play tracks from the emotional database
        // n is the number of tracks to play
        // emoOneHot is the one hot encoding of the emotion
        // 1000 = aggressive (aggr)
        // 0100 = relaxed (rel)
        // 0010 = happy (hap)
        // 0001 = sad (sad)
        // (plus all combinations from 2 to all 4 emotions)

        juce::File file(emotion_map[std::to_string(emoOneHot)][0]);

        auto* reader = formatManager.createReaderFor (file);                 // [10]
 
        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);   // [11]
            transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
            // playButton.setEnabled (true);                                                      // [13]
            if (playbutton != nullptr) playbutton->setEnabled(true);
            readerSource.reset (newSource.release());                                          // [14]
        }
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else
                changeState (Stopped);
        }
    }

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;
 
            switch (state)
            {
                case Stopped:                           // [3]
                    // stopButton.setEnabled (false);
                    // playButton.setEnabled (true);
                    if (playbutton != nullptr) playbutton->setEnabled(true);
                    transportSource.setPosition (0.0);
                    break;
 
                case Starting:                          // [4]
                    // playButton.setEnabled (false);
                    if (playbutton != nullptr) playbutton->setEnabled(false);
                    transportSource.start();
                    break;
 
                case Playing:                           // [5]
                    // stopButton.setEnabled (true);
                    break;
 
                case Stopping:                          // [6]
                    // transportSource.stop();
                    break;
            }
        }
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) 
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
 
        transportSource.getNextAudioBlock (bufferToFill);
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) 
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void releaseResources() 
    {
        transportSource.releaseResources();
    }
};
