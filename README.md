# Pitch-Shifter-Audio-Plugin

A simple harmonizing pitch-shifter audio plugin that generates up to **three transposed voices** from an input signal (e.g., voice). Each voice has an independent **semitone transposition control**, plus a **Preset ComboBox** for quick harmonization setups.

---

## Features

- **3 independent pitch-shift voices**
  - Voice 1 transposition (semitones)
  - Voice 2 transposition (semitones)
  - Voice 3 transposition (semitones)
- **Harmonization Presets (ComboBox)**
  - Select from **at least 4** preset chord/interval stacks
- Designed for fast, musical harmonies (typical use: vocals)

---

## Controls

### Pitch Transposition (Semitones)
Each voice has a semitone control:

- **Voice 1 (semitones)**: shifts the pitch of Voice 1 relative to the input
- **Voice 2 (semitones)**: shifts the pitch of Voice 2 relative to the input
- **Voice 3 (semitones)**: shifts the pitch of Voice 3 relative to the input

**Notes**
- Positive values shift **up** in pitch.
- Negative values shift **down** in pitch.
- Values are in **semitones** (12 semitones = 1 octave).

### Harmonization Preset (ComboBox)
Select a preset to set all three voices at once. You can then fine-tune each voice using the semitone controls.

---

## Presets (Example Set)

You can implement any 4+ presets; here’s a clean starter set that works well for common vocal harmonies:

| Preset Name | Voice 1 | Voice 2 | Voice 3 | Musical Meaning |
|------------|---------:|--------:|--------:|-----------------|
| Unison + Octaves | 0 | +12 | -12 | Unison with octave up/down |
| Power Harmony | +7 | +12 | 0 | 5th + octave + unison |
| 3rd–5th–7th (Example) | +4 | +7 | +10 | Maj 3rd + P5 + Min 7th above input |
| Low Stack | -3 | -7 | -12 | Minor 3rd + 5th down + octave down |

**Required example** (as specified):  
A preset that sets **Voice 1 = +4**, **Voice 2 = +7**, **Voice 3 = +10** semitones (Maj 3rd, P5, Min 7th above the input).

---

## Usage

1. Insert the plugin on a vocal track (or any monophonic source for best results).
2. Pick a harmonization **Preset** from the ComboBox.
3. Adjust **Voice 1/2/3 semitone** controls to taste.
4. Record/print the effect or automate preset/interval changes for arrangement movement.

---

## Implementation Notes (Recommended Behavior)

- Preset selection should:
  - Update the 3 semitone parameters immediately.
  - Keep UI and internal parameter state consistent (host automation friendly).
- Semitone parameters should remain editable after preset selection (presets as “starting points”).
- If you support automation:
  - Make sure preset changes don’t break parameter automation (common approach: presets write parameter values; automation still controls them afterward).

---

## Roadmap (Optional Enhancements)

- Mix level per voice (dry/wet + per-voice gain)
- Pan per voice for stereo width
- Formant preservation option (more natural vocal harmonies)
- Smoothing/interpolation to reduce zipper noise when changing semitones
- Latency reporting (if your pitch algorithm introduces latency)

---

## License
Add your license here (MIT, GPL, proprietary, etc.).

---

## Credits
Created by: **[Your Name]**  
Course / Context: **[Class / Institution]** (optional)

