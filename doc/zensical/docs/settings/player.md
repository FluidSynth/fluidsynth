# MIDI player settings


## `player.reset-synth` {#settings_player_reset-synth}

!!! tip "Real-time"
    This setting can be changed at run time.

| Property | Value |
|----------|-------|
| Type | `bool` |
| Default | `1 (TRUE)` |

If true, reset the synth after the end of a MIDI song, so that the state of a previous song can't affect the next song. Turn it off for seamless looping of a song.

## `player.timing-source` {#settings_player_timing-source}

| Property | Value |
|----------|-------|
| Type | `str` |
| Default | `sample` |

Determines the timing source of the player sequencer. 'sample' uses the sample clock (how much audio has been output) to sequence events, in which case audio is synchronized with MIDI events. 'system' uses the system clock, audio and MIDI are not synchronized exactly.
