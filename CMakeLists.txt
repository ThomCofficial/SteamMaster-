cmake_minimum_required(VERSION 3.20)
project(SteamMasterDSP VERSION 1.7.0 LANGUAGES CXX)
include(CTest)
add_library(steammaster_dsp STATIC src/DSP.cpp src/PultecCircuit.cpp src/Oversampling.cpp src/Mastering.cpp src/BaxShelf.cpp src/TapeMagnetics.cpp)
target_include_directories(steammaster_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(steammaster_dsp PUBLIC cxx_std_17)
if (MSVC)
  target_compile_options(steammaster_dsp PRIVATE /W4 /permissive-)
else()
  target_compile_options(steammaster_dsp PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()
if (BUILD_TESTING)
  add_executable(steammaster_tests tests/TestDSP.cpp)
  target_link_libraries(steammaster_tests PRIVATE steammaster_dsp)
  if (NOT MSVC)
    target_compile_options(steammaster_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endif()
  add_test(NAME SteamMasterDSPTests COMMAND steammaster_tests)
  add_executable(steammaster_oversampling_tests tests/TestOversampling.cpp)
  target_link_libraries(steammaster_oversampling_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterOversamplingTests COMMAND steammaster_oversampling_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_mastering_tests tests/TestMastering.cpp)
  target_link_libraries(steammaster_mastering_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterMasteringTests COMMAND steammaster_mastering_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_characterization_tests tests/TestCharacterization.cpp)
  target_link_libraries(steammaster_characterization_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterCharacterizationTests COMMAND steammaster_characterization_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_tp_probe tools/OfflineTruePeakProbe.cpp)
  target_link_libraries(steammaster_tp_probe PRIVATE steammaster_dsp)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_metering_tests tests/TestMetering.cpp)
  target_link_libraries(steammaster_metering_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterMeteringTests COMMAND steammaster_metering_tests)
endif()
if (BUILD_TESTING)
  add_executable(steammaster_analog_voicing_tests tests/TestAnalogVoicing.cpp)
  target_link_libraries(steammaster_analog_voicing_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterAnalogVoicingTests COMMAND steammaster_analog_voicing_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_hardware_reference_tests tests/TestHardwareReferences.cpp)
  target_link_libraries(steammaster_hardware_reference_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterHardwareReferenceTests COMMAND steammaster_hardware_reference_tests)
endif()
if (BUILD_TESTING)
  add_executable(steammaster_reference_curve_probe tools/ReferenceCurveProbe.cpp)
  target_link_libraries(steammaster_reference_curve_probe PRIVATE steammaster_dsp)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_pultec_circuit_tests tests/TestPultecCircuit.cpp)
  target_link_libraries(steammaster_pultec_circuit_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterPultecCircuitTests COMMAND steammaster_pultec_circuit_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_exciter_voice_tests tests/TestExciterVoices.cpp)
  target_link_libraries(steammaster_exciter_voice_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterExciterVoiceTests COMMAND steammaster_exciter_voice_tests)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_bax_reference_tests tests/TestBaxReference.cpp)
  target_link_libraries(steammaster_bax_reference_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterBaxReferenceTests COMMAND steammaster_bax_reference_tests)
  add_executable(steammaster_bax_response_probe tools/BaxResponseProbe.cpp)
  target_link_libraries(steammaster_bax_response_probe PRIVATE steammaster_dsp)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_tape_reference_tests tests/TestTapeReference.cpp)
  target_link_libraries(steammaster_tape_reference_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterTapeReferenceTests COMMAND steammaster_tape_reference_tests)
endif()
if (BUILD_TESTING)
  add_executable(steammaster_tape_response_probe tools/TapeResponseProbe.cpp)
  target_link_libraries(steammaster_tape_response_probe PRIVATE steammaster_dsp)
endif()

if (BUILD_TESTING)
  add_executable(steammaster_stereo_limiter_tests tests/TestStereoLimiter.cpp)
  target_link_libraries(steammaster_stereo_limiter_tests PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterStereoLimiterTests COMMAND steammaster_stereo_limiter_tests)
endif()

# Optional JUCE VST3 test bench. Keeps the DSP-only CI path independent of JUCE.
option(STEAMMASTER_BUILD_VST3 "Build SteamMaster VST3 plugin" OFF)
if(STEAMMASTER_BUILD_VST3)
  include(FetchContent)
  FetchContent_Declare(juce GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.10 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(juce)
  juce_add_plugin(SteamMasterVST3 COMPANY_NAME "ThomC" BUNDLE_ID "com.thomc.steammaster.testbench" PLUGIN_MANUFACTURER_CODE ThmC PLUGIN_CODE SmT1 FORMATS VST3 PRODUCT_NAME "SteamMaster" IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
  target_sources(SteamMasterVST3 PRIVATE plugin/PluginProcessor.cpp plugin/PluginProcessor.h plugin/PluginEditor.cpp plugin/PluginEditor.h)
  target_link_libraries(SteamMasterVST3 PRIVATE steammaster_dsp juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_dsp)
  target_compile_definitions(SteamMasterVST3 PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  # Every source panel is embedded from the ORIGINAL uploaded GitHub asset folders.
  # Fail closed rather than shipping the 1x1 placeholder-artwork plugin.
  # Embed the new, user-uploaded and audited asset folder structure.
  # These filenames are intentionally explicit so a missing visual blocks compilation.
  set(STEAMMASTER_ART_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/01_Header/abyssal_steampunk_mastering_faceplate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/01_Header/brass_steampunk_knob_sprite_sheet.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/01_Header/steampunk_brass_ui_button_sprite_sheet.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/01_Header/steampunk_global_in_toggle_set.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/02_Ocean_Furnace/01_Combined_Faceplate/Ocean_Furnace_Clean_Faceplate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/02_Ocean_Furnace/02_Knobs/Ocean_Furnace_Knobs_Sprite_Sheet.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/02_Ocean_Furnace/03_Buttons_Ornaments/Ocean_Furnace_Buttons_Porthole_Ornaments.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/02_Ocean_Furnace/04_Value_Dials/Ocean_Furnace_12_Value_Dial_Rings.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Faceplates/Parametric_EQ_Clean_Faceplate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band1_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band1_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band1_Q.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Output.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Mix.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Buttons/Band1_Peak_OriginalState.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Buttons/IN_OriginalState.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/04_Pultec_EQ/Faceplate/Pultec_EQ_Faceplate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/04_Pultec_EQ/Controls/Pultec_EQ_Knobs_and_IN_Button_Sheet.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/04_Pultec_EQ/Dials/Pultec_EQ_10_Value_Dials_Sheet.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Faceplates/Shadow_Clean_Faceplate_EmptySockets_4x.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Continuous_Large_Brass.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Continuous_Small_Brass.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Stepped_Lever_Brass.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Meter/GainReduction_Meter_Face_NoNeedle.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Meter/GainReduction_Needle.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Global_IN_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Global_IN_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Faceplates/Exciter_Empty_Socket_Faceplate_4x.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/Low_Drive_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Meter/VU_Meter_Face_No_Animated_Needle_Concept.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Meter/VU_Animated_Needle_Brass.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Tube_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Tube_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Tape_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Tape_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Transformer_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/Transformer_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Faceplate/Baxandall_Rack_Faceplate_Cropped.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_HPF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_LPF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_High_Shelf_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_High_Shelf_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_Low_Shelf_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_Low_Shelf_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Knobs/Bax_Output.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Buttons/Bax_Channel_IN_Lit.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Buttons/Bax_Channel_IN_Unlit.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Buttons/Bax_Link_On_Lit.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/07_Baxandall_EQ/SteamMaster_Baxandall_EQ_Asset_Pack/Buttons/Bax_Link_Off_Unlit.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Faceplate/TapeMachine_Faceplate_Clean_4x.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Knobs/Drive_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Knobs/Bias_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Knobs/HF_Trim_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Knobs/Mix_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Selectors/Type_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Selectors/Speed_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Reels/Reel_Left_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Reels/Reel_Right_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Buttons/IN_On_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/08_Tape_Machine_v3/SteamMaster_Tape_Machine_FrontOn_v3/Buttons/IN_Off_FrontOn.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Faceplate/StereoImage_Limiter_Clean_Redesigned.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Mono_Maker_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Stereo_Width_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Width_Frequency_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Threshold_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Ceiling_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Release_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Lookahead_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Knobs/Stereo_Link_FrontFacing_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Meters/Correlation_Meter_Static_NoNeedle.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Meters/Correlation_Needle_Separate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Meters/Gain_Reduction_Meter_Static_NoNeedle.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Meters/Gain_Reduction_Needle_Separate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_CLEAN_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_CLEAN_Off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_PUNCH_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_PUNCH_Off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_MODERN_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Buttons/Mode_MODERN_Off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/IN_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/IN_Off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/True_Peak_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/True_Peak_Off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/Auto_Release_On.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/09_Stereo_Limiter_v3/SteamMaster_Stereo_Image_Limiter_Redesigned_v3/Toggles/Auto_Release_Off.png"
  )
  list(APPEND STEAMMASTER_ART_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Optical_Threshold.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Optical_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Discrete_Threshold.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Discrete_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Discrete_Ratio.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Discrete_Attack.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Discrete_Recovery.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Knobs/Sidechain_HPF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/Low_Frequency_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/High_Drive_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/High_Frequency_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/Crossover_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/Output_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Knobs/Mix_Brass_Transparent.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band2_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band2_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band2_Q.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band3_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band3_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band3_Q.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band4_Gain.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band4_Frequency.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/03_Parametric_EQ/SteamMaster_Parametric_EQ_Asset_Pack/Sprites/Knobs/Band4_Q.png"
  )
  list(APPEND STEAMMASTER_ART_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Optical_IN_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Optical_IN_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Discrete_IN_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/05_Shadow_Compressor/SteamMaster_Shadow_Compressor_Asset_Pack/Buttons/Discrete_IN_OFF.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/IN_ON.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/06_Exciter/SteamMaster_Exciter_Asset_Pack/Buttons/IN_OFF.png"
  )
  foreach(ART_FILE IN LISTS STEAMMASTER_ART_FILES)
    if(NOT EXISTS "${ART_FILE}")
      message(FATAL_ERROR "STEAMMASTER MISSING APPROVED ASSET: ${ART_FILE}")
    endif()
    file(SIZE "${ART_FILE}" ART_BYTES)
    if(ART_BYTES LESS 400)
      message(FATAL_ERROR "STEAMMASTER INVALID SMALL ARTWORK: ${ART_FILE}")
    endif()
  endforeach()
  juce_add_binary_data(SteamMasterArtwork SOURCES ${STEAMMASTER_ART_FILES})
  target_link_libraries(SteamMasterVST3 PRIVATE SteamMasterArtwork)
  target_compile_definitions(SteamMasterVST3 PRIVATE STEAMMASTER_HAS_ARTWORK=1)
  juce_generate_juce_header(SteamMasterVST3)
  if(BUILD_TESTING)
    add_executable(steammaster_plugin_host_smoke tests/TestPluginHost.cpp)
    target_link_libraries(steammaster_plugin_host_smoke PRIVATE SteamMasterVST3 steammaster_dsp juce::juce_audio_utils)
    target_include_directories(steammaster_plugin_host_smoke PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin)
    add_test(NAME SteamMasterPluginHostSmoke COMMAND steammaster_plugin_host_smoke)
  endif()
endif()

if(BUILD_TESTING)
  add_executable(steammaster_full_chain_stress tests/TestFullChainStress.cpp)
  target_link_libraries(steammaster_full_chain_stress PRIVATE steammaster_dsp)
  add_test(NAME SteamMasterFullChainStress COMMAND steammaster_full_chain_stress)
endif()
