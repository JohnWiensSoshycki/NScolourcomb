# NScolourcomb
This repo contains the free vst3 and source code to the NewShadesAudio ColourComb plugin.

## What is ColourComb?
ColourComb is a vst3 plugin designed for electronic music producers, primarily those associated with the genre of ColourBass/Future Riddim.
More specifically it aims to remove frequencies from a source that are not within a scale (isolation of undesired frequencies and the boosting of desired frequencies are the fundamental sound design technique of the genre).

ColourComb is essentially a tuned comb filter that targets a specific note at a time amongst multiple octaves at a time.
Although not currently implemented it will be able to filter out a total of 5 unique notes (amongst octaves) to accentuate a desired scale.

The plugin is coded with the Juce8 library in c++, the main header, editor and processor files can be found in this repo.

## Installation 
Rather straightforward if you have installed any other third party plugin for you daw of choice (the install wizard will usual find a viable path on its own), depending on the daw and the os there will either be a dedicated plugin folder or VST3 folder routed in the DAWs plugin paths (which you can change if necessary to use any folder), simply put the vst3 file in any of these folders.  

## NewShadesAudio
NewShadesAudio is a small audio company I, John Wiens-Soshycki am trying to create and grow.  The intention of the plugins and projects featured at NewShades are to create more accessible and affordable options to music producers specializing in ColourBass production, an environment where many plugins have non consumer friendly pricing.

## Development Timeline
Below is a rather loose timeline I have planned for this plugin.  Not all dates are strict but are rather personal goals for ColourComb's development.

2025/07/31
-Have all core logic dsp finished (including up to 5 notes, and corresponding octaves, being filtered.  More q function types and smoothing).

2025/08/31
-All gui touch ups being finished including custom knobs, reactive keys (active keys are highlighted), custom background and menu options, settings menu along with user background customizability).

2025/09/31 
-Full release of version 1.0 accessible both in this repo and the NewShades website.

## License
On god I don't know exactly what this license does but I'm pretty sure it allows you to mess around with the code so ye.
