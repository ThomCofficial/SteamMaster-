STEAMMASTER — EXCITER / THE LIGHTHOUSE

Artwork basis: your exact original user-supplied Exciter image, 1145 x 92 px.

VERIFIED AGAINST APPROVED CONTROL SPECIFICATION:
- 7 separate rotary knobs: Low Drive 0-20; Low Frequency 20-800 Hz; High Drive 0-20; High Frequency 1000-18000 Hz; Crossover 100-5000 Hz; Output -12 to +12 dB; Mix 0-100%.
- IN with ON and OFF sprites.
- Mode: Tube, Tape, Transformer with on/off button sprites.
- Center analog VU meter.

GEOMETRY
- Preserved reference dimensions and relative knob/button locations: Documentation/Exciter_Parameter_Layout_Map.json
- Faceplates/Exciter_Empty_Socket_Faceplate_4x.png is the most straightforward starter for JUCE if you want physical sockets; background has been enlarged from the small original.
- Faceplates/Exciter_Clean_Faceplate_OriginalGeometry_4x.png contains approximate control removal via local inpainting; inspect before shipping.
- Faceplates/Exciter_Faceplate_Transparent_Control_Cutouts_4x.png is the other assembly option.
- Use the separate Knobs/ and Buttons/ assets at code-defined positions, using the shared faceplate coordinate system and g.drawImageTransformed(..., transform, false) for full-color sprites.
- Value_Dials/ has seven accurate endpoint-and-unit overlays created deterministically, not generated numerical scales. Position these behind knobs if desired, but for final precision render ticks and labels in JUCE.
- Meter/ has a static crop of the original meter and an alternate rendered meter with its own needle sprite. The original source contains a baked-in needle.

IMPORTANT LIMITATIONS
- Actual SteamMaster DSP source code could not be audited; the manifest uses the parameters agreed in this conversation.
- Generated image concepts are stylistic alternatives ONLY: their parameter names and values are WRONG (HARMONICS/BRIGHTNESS/SOLID etc). They are sequestered in Concept_Only and should not be implemented.
- Since the original screenshot was just 92 pixels tall, the pixel-level faceplate cleanup and generated metal knobs require visual approval before deployment; these are NOT guaranteed pixel-perfect production assets.
- No DSP or VST3 build was modified or started.
