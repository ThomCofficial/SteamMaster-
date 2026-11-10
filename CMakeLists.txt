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
  target_sources(SteamMasterVST3 PRIVATE plugin/PluginProcessor.cpp plugin/PluginProcessor.h plugin/PluginEditor.cpp plugin/PluginEditor.h plugin/HeaderModule.cpp plugin/HeaderModule.h)
  target_link_libraries(SteamMasterVST3 PRIVATE steammaster_dsp juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_extra juce::juce_dsp)
  target_compile_definitions(SteamMasterVST3 PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  # Every source panel is embedded from the ORIGINAL uploaded GitHub asset folders.
  # Fail closed rather than shipping the 1x1 placeholder-artwork plugin.
  set(STEAMMASTER_ART_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/header_faceplate_blank_candidate.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Ocean_Furnace_Asset_Prep_v1/ocean_module_source.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Ocean_Furnace_Asset_Prep_v1/furnace_module_source.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_ParametricEQ_Asset_Prep_v1/parametric_eq_module_source.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Pultec_EQ_DSP_Locked_Handoff/artwork/Pultec_Approved_Reference.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Shadow_Complete_Module_Kit_v1/01_reference/Shadow_Source_Locked.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Exciter_Complete_Asset_Prep_v1/SteamMaster_Exciter_Complete_Asset_Prep_v1/source/Exciter_panel_uncorrected_1536x402.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_BaxandallEQ_Module_Kit_v1/01_reference/baxandall_module_trimmed_reference.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Tape_Complete_Module_Kit_v1/source/Tape_Module_Reference_1536x469.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_StereoImage_Limiter_Module_Kit_v1/00_source/stereo_limiter_approved_source.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/knob_input_silver.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/knob_output_silver.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/global_toggle_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/global_toggle_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_off_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_off_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_2x_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_2x_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_4x_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_4x_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_8x_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_8x_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_a_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_a_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_b_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_b_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_previous_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_previous_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_next_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_next_on.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_save_off.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/plugin/assets/SteamMaster_Global_Header_Complete_v2/button_save_on.png"
  )
  foreach(ART_FILE IN LISTS STEAMMASTER_ART_FILES)
    if(NOT EXISTS "${ART_FILE}")
      message(FATAL_ERROR "STEAMMASTER MISSING APPROVED ASSET: ${ART_FILE}")
    endif()
    file(SIZE "${ART_FILE}" ART_BYTES)
    if(ART_BYTES LESS 10240)
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
