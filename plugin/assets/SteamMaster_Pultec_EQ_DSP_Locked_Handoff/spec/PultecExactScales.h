// SteamMaster Pultec labels — reference values from the existing JUCE parameter layout.
// This header does not define parameters or modify the DSP.
#pragma once
#include <array>
#include <string_view>
namespace SteamMasterPultecUI {
struct ChoiceScale { std::string_view parameterID; std::array<std::string_view,3> labels; };
inline constexpr ChoiceScale LowFrequency {"pultec_low_freq", {"30", "60", "100"}};
inline constexpr ChoiceScale HighBoostFrequency {"pultec_high_boost_freq", {"8k", "12k", "16k"}};
inline constexpr ChoiceScale HighAttenuationFrequency {"pultec_high_atten_freq", {"5k", "10k", "20k"}};
static_assert(LowFrequency.labels.size()==3);
static_assert(HighBoostFrequency.labels.size()==3);
static_assert(HighAttenuationFrequency.labels.size()==3);
}
