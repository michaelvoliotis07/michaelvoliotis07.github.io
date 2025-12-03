# “The Soundwave Program, As Presented By Someone Who Swears It Worked Yesterday”

In the grand tradition of tools that do exactly what you tell them to (and nothing you meant to say), _soundwave_ is a c program concerened with the bytes of WAV files. It analyses and stores integers of all shapes and sizes, occasionally meddling with audio data in ways that defy common sense yet somehow make perfect sense.

If you happen to be curious, or politely bored, ```./soundwave --help ``` will whisper the secrets.

## What it will do (and usually does)

### Available Effects


| Command    | Parameters | What It Does (Brief, with flair) |
|------------|-----------|---------------------------------|
| `info`     | none      | Peeks into an audio file’s secrets: sample rate, duration, channels… basically its diary. |
| `rate`     | `<fp_rate>` | Persuades time to hurry or slow down. Higher → chipmunk urgency, lower → ponderous drama. |
| `volume`   | `<fp_volume>` | Turns the amplitude knob of reality. Above 1 → enthusiastic loudness; below 1 → polite whispers. |
| `generate` | `[--dur <s>] [--sr <Hz>] [--fm <Hz>] [--fc <Hz>] [--mi <index>] [--amp <value>]` | Conjures tones from pure math and CPU whimsy. All knobs optional; defaults are generous. |
| `fuzz`     | `<fp_fuzz>` | Hard clips the signal into a thick, nearly-square, buzzy waveform; polite tones go in, fuzzed tones roar out. |
| `drive`    | `<drive>` | Softly nudges audio beyond its comfort zone. Near 1 → oscillates with spirited enthusiasm. |
| `bitcrush` | `<bits>` | Strips audio down to bare digital bones. Lower bits → increasingly quantized despair. |
| `tremolo`  | `<rate> <depth>` | Volume wobbles gently like jelly on a bumpy road. Depth near 1 → full “now you hear me, now you don’t.” |
| `echo`     | `<delay_sec> <decay>` | Makes sound wander down a corridor and come back chatty. High decay → corridor won’t shut up. |
| `reverb`   | `<room>` | Throws your audio into rooms of questionable architecture. Higher → bigger, more judgmental rooms. |

> **Tip:** SoundWave stops at any mistake. Misbehave with your parameters, and it will raise a polite fuss rather than silently wreak havoc.


---

### When in need of guidance

For guidance on any effect, run:

```bash
./soundwave <effect> --help
```

### If the little voice in your head says ‘What’s the worst that could happen?’… try DJ.

DJ is the Time-Master. You point, flick, whisper ‘Apply that effect,’ and—BAM!—it’s done. DJ lets you bend your own audio reality. You are the master of you own fate,... or at least your audio's.

Small tip: When nothing works try ```./soundwave dj --help ```.

# A Guided Tour of the  Contraption

Ah, the explanation section. Every grand invention has one, usually written by someone who hopes you won’t notice the scorch marks. Here, soundwave stops being mesterious and starts talking, so you can understand what’s happening instead of just nodding wisely and hoping for the best (that is my job). By the end, it should all make sense… or at least you’ll know who to blame.

## The stuff everything else depends on

In this program, every byte passes through stdin with getchar() and makes its exit via putchar(). The only exceptions are when it stops for a gossip with printf() or complains to fprintf() about a mistake.
### Byte Reading/Writing Helpers

| Function        | In plain nglish (or something like it) |
|-----------------|-------------|
| `read4(unsigned char *buf)` | Reads **4 bytes** from `stdin` into a buffer. Exits on EOF. |
| `read2(unsigned char *buf)` | Reads **2 bytes** from `stdin` into a buffer. Exits on EOF. |
| `write_bytes(const unsigned char *buf, int n)` | Writes `n` bytes from a buffer to `stdout`. |

### Little-Endian Helpers

Since WAV files use **little-endian**, these functions read and write multi-byte values correctly:

| Function         | In plain nglish (or something like it)  |
|-----------------|-------------|
| `uint32_t read4_le()` | Reads **4 bytes** from `stdin` in little-endian order and returns a 32-bit integer. |
| `uint16_t read2_le()` | Reads **2 bytes** from `stdin` in little-endian order and returns a 16-bit integer. |
| `put4bytes(uint32_t val)` | Writes a 32-bit integer to `stdout` in little-endian order. |
| `put2bytes(uint16_t val)` | Writes a 16-bit integer to `stdout` in little-endian order. |

### How They Work

1. `read2` and `read4` simply call `getchar()` multiple times, storing each byte in a buffer.  
2. `read2_le` and `read4_le` combine bytes using bitwise shifts to respect little-endian order.  
3. `put2bytes` and `put4bytes` do the reverse: take an integer, split it into bytes, and write to `stdout`.  
4. `write_bytes` is a simple loop to dump raw bytes to the output stream.  

These helpers quietly fetch and arrange the bytes, sparing the program from endian and alignment headaches, so it can focus on making sound behave—or delightful misbehave—just as you wish.


##The functions the user calls upon

This is the all-singing, all-dancing part of the program—the section that answers when the user calls for info, rate, chanell,... even the ever-mysterious dj. Naturally, it also accommodates the venerable --help, because every program needs its polite companion.


### Info

`info` is delightfully simple:  
- We read input one character at a time using `getchar`.  
- Any mischief is reported immediately with `fprintf`.  
- If things go sideways, we exit politely with `exit(1)`.  
- Near the end, a quick check ensures nothing sneaky slipped by.  
- Finally, for each step everything checks out, we print the results right away—no dawdling.

Think of it as the program analysing everything for you before giving you the verdict.

#### Small interlude

Here we introduce four new helper functions: `read_header`, `write_header`, `copy_extra_bytes`, and `acheck_final_size`. Each does exactly what its name suggests—no surprises here.  

It’s really the same sort of careful, step-by-step work we did for `info`. We just don’t want to repeat ourselves too often or produce too much text. This is a brief pause to admire the helpers doing their quiet, essential work behind the scenes.

### Rate

Another simple one. We read the header, compute the new sample rate and bytes per second by multiplying the original values with the `fp_rate` the user provided—and voilà! A slower or faster file, exactly as requested.  

We print the new header, copy everything else unchanged, and usually, everything just works out —of course, performing all the necessary checks along the way.

###Channel

As before, we read the header, make the necessary changes, and rewrite it right away.  
Here is the big change: unlike our previous functions, we now need to process the samples themselves.  
To do that, we must understand how samples work — in other words, how 8-bit and 16-bit samples are stored, and what frames are.

**Frames:**  
A frame is one left sample and one right sample taken together.  
- For **8-bit** audio, a frame is simply: `Left, Right`.  
- For **16-bit** audio, a frame is made of two pairs:  
  `(leftLow, leftHigh)` and `(rightLow, rightHigh)`.

**Important:** In WAV files,  
- 8-bit samples are stored as: `Left, Right`  
- 16-bit samples are stored as: `leftLow, leftHigh, rightLow, rightHigh`

So we simply output only the left samples (as many as there are frames) or the right samples (again, as many as frames) as the user specifies, followed, of course, by the necessary checks.

### Mysound (aka generate)

Don’t be overwhelmed — this is actually a step back in complexity.  
Here, we don’t need any checks or extra fuss. We simply write a header and follow it by outputting only the samples, based on the mathematical formula:  

*`val = trunc(amp * sin(2 * M_PI * fc * t - mi * sin(2 * M_PI * fm * t)))`*

Worth mentioning: we produce **mono WAV files only**.

## Entering Mode DJ

The upcoming functions all live under the hood of `dj`.  
They are effects that can be applied either to the entire WAV file or to a specific section defined by the user (start second and end second).

You might be wondering how this actually works. Well… here’s the answer:  

```c
#define APPLY_EFFECT(i, dj, start, end) (!(dj) || ((i) >= (start) && (i) < (end)))
```
The start and end seconds provided by the user are converted to frames by multiplying by the sample rate.

For the most part, we:

- Read the header

- Write the header

- Process the samples using the logical equation above as a guide

- If dj = 0, obviously all samples are processed. This when the user types ```./soundwave <effect> <parameters> ```

Things get a bit more complicated for effects like echo and reverb, but we’ll get to those in time.

### Volume

Everything goes according to plan here. We read the header, compute the new values, convert seconds to frames, and multiply the samples we want by *`int new smaple = (int)trunc(sample * fp_volume)`*.  

A small note: for 8-bit samples we process them directly, but for 16-bit samples we split them into low and high 8-bit parts before reconstructing. This is because we check individual bytes. Of course doing so for every channel.

Also, from now on you will see some clamping happening. That’s because as we process samples, values may go out of bounds — something we want to avoid at all costs:  
- 8-bit samples must fit in 8 bits after editing  
- 16-bit samples must fit in 16 bits after editing

### Fuzz

We follow the same logic as `volume`, the only real difference is in the mathm:  

The process is simple and efficient:  
1. Convert the sample to a convenient signed range by subtracting the midpoint and dividing (for 8-bit samples, subtract 128 and divide by 128).  
2. Apply the cubic formula (`x*x*x`) to shape the distortion.  
3. Convert the result back to an integer in the original range.  
4. Output the sample in little-endian format (for 16-bit "low-high" samples).  

In short: **turn it into a manageable signed value → apply cubic shaping → convert back → output**.  
This allows quick and smooth fuzz effects without unnecessary computations.

### Overdive

In a similar fashion to `fuzz`, `overdrive` only changes the math to softly clip the sound:  

1. Convert the sample to a convenient signed range (just like in `fuzz`).  
2. Multiply by `drive` (`x *= drive`) to control the intensity.  
3. Apply the non-linear shaping using the tangent function (`x = tan(x)`) instead of the cubic formula.  
4. Convert back to the original integer range and output (8-bit or little-endian 16-bit).  

Everything else works exactly the same as `fuzz`.

### Tremolo

It's time for the logic to change a bit. Let's start with explaining *tremolo*.  
Think of it as a repeating wave that modulates the volume up and down — like a child who’s discovered the volume knob and is left unsupervised, turning it with great curiosity.

Here’s the math behind it:

We introduce a **phase**, which tells us where in the modulation wave we currently are.  
We start `phase = 0`, and increase it for every frame using:

```c
phase += 2.0 * M_PI * rate / sample_rate;
```
This essentially determines how fast the tremolo wave oscillates.


The core logic is still very similar to the previous functions — we convert samples, check whether the effect should apply, and clamp as usual.
The only difference is the tremolo formula:

```c
double lfo = (1.0 - depth) + depth * 0.5 * (1.0 + sin(phase)); 
sample *= lfo;
```

The lfo value oscillates between a lower and higher multiplier, creating that classic tremolo volume-shimmer effect. Also, we advance the phase index by the increment calculated above. Everything else  behaves exactly the same as before.

### Bitcrush

This is probably the simplest function we’ll discuss. There’s no math puzzle hiding here and no headaches to worry about. We follow the same routine as before, but this 
time we simply add one more clamp — the user-specified limit. That’s it. No tricks, no fancy formulas, just a straightforward cap on the sample values.

### Echo

Now things start to get really interesting — and just a little more complex. Unlike our previous, well-behaved effects, `echo` has a memory. It doesn’t just process the samples and forget them; it remembers, storing both the old and the new. Why? Because echo is essentially a ghost of the sound, replayed at the pace and volume you dictate.

#### Delay buffers — the echo’s memory

- `delay_samples = delay_sec * sample_rate` tells us how many frames to remember.  
- `double **delay_buf` — one magical little buffer per channel. Each buffer is a circular array that holds the most recent `delay_samples` frames.  
- `pos` keeps track of where we are in this circular memory.

Every channel gets its own private echo chamber, so left and right don’t talk to each other — they have their own secrets.

#### Processing loop — one frame at a time

1. **Read and normalize**  
   - 8-bit samples: unsigned, nudged to [-1, 1].  
   - 16-bit samples: signed, also scaled to [-1, 1].  

2. **Apply the echo**  
   ```c
   samples[ch] += decay * delay_buf[ch][pos];
   ```
   - Take the sample from delay_samples frames ago, scale it by decay, and add it to the current one.

   - Only applies if APPLY_EFFECT says so — the DJ has the final say.

3. Store the current sample

   After borrowing from the past, we tuck the current sample safely into the buffer for future echoes. This ensures the delay buffer always contains the most recent delay_samples frames.

4. Denormalize and write

   Now it's time to clamp all the samples we processed, as we did before, and then return them in proper form. The echo now officially exists in the new file.

5. Advance the buffer pointer

   ```c
   Copy code
   pos = (pos + 1) % delay_samples;
   ```
   Circular buffer! When we reach the end, we wrap around. Always fixed length, never losing a beat.

6. Cleanup

   At last, we free each channel’s buffer — ghosts should not linger forever.

#### Key ponits (in plain human)
- double **delay_buf → array of buffers, one for each channel.

- delay_buf[ch][pos] → the sample from the past, ready to haunt the present.

- Circular buffers #let us keep a fixed-length memory without moving anything around.

- Multi-channel audio is handled naturally: each channel has its own echo chamber.

- pos moves in lockstep across channels — the echo marches forward in unison.

### Reverb

 If you haven’t already — better brace yourself for a headache.  
This function is a wandering maze of buffers and arrays, ilustrating all sorts of eco-emmiting digital chambers.  

And I, because I’m clearly not well, and I hope you’re just as unwell, am going to break it down anyway.

#### Reverb Filters (A gentle introduction)

 Just enough to give you a peek at what’s coming without tripping over your own feet. Also I ought to mention that I am going to masterfully evade repeating anything I have explained above.

#### Comb Filter (Feedback Comb)
Think of it as shouting into a corridor and hearing your voice bounce back, a little louder each time.  
- Adds a delayed copy of itself to the current sound.  
- **Formula:**  
```text
y[n] = x[n] + g * y[n - D]
```
- g = feedback (how sticky the echoes are), D = delay in samples. 

Result: a series of neat, repetitive echoes — the skeleton of our “room.”

#### All-Pass Filter

Now imagine spreading those echoes around so they don’t all arrive at once, softening the hard edges.

- Passes all frequencies, just messes with timing.

- Formula:

```text
y[n] = -g * x[n] + x[n - D] + g * y[n - D]
```

Result: echoes turn into a smooth, flowing tail — the carpet on our corridor floor.

#### The Reverb Orchestra

- Parallel combs lay down the echoes.

- Series all-pass filters shuffle and soften them.

- Mix wet (processed) and dry (original) to taste.

In short: combs = echoes, all-pass = smoothing, and together they make your audio feel like it’s in a room rather than a cardboard box.

#### Reverb parameters

```c
int num_combs = 4 + (int)(4 * room); 
int num_aps   = 2 + (int)(2 * room); 
```

- Number of combs and all-pass filters depends on ```room```. Larger rooms → more filters → denser reverb.

Delay lengths:

```c
comb_delays[i] = (int)((0.05 + 0.45 * room) * sample_rate * (0.8 + 0.05 * i));
ap_delays[i]   = (int)((0.02 + 0.03 * room) * sample_rate * (0.9 + 0.05 * i));
```

- Delays scale with room size, with slight variation for each filter.

- Ensures each delay ≥ 1 sample to avoid modulo/division errors.

Feedback & mix:

```c
double comb_fb = 0.55 + room * 0.30;
double ap_fb   = 0.45 + room * 0.25;
double wet     = 0.2  + room * 0.30;
double dry     = 1.0 - wet;
```

- ```comb_fb```: feedback coefficient for combs (slower decay for larger rooms).

- ```ap_fb```: feedback for all-pass filters (controls diffusion).

- ```wet/dry```: mix between processed and original signal.

#### Allocate buffers

**Comb buffers**  

Now we summon our first set of magical digital chambers: `comb_buffers`. Imagine a grand hall for each channel, with a row of comb filters patiently waiting to echo every sound you throw at them.  

```c
double ***comb_buffers = calloc(channels, sizeof(double**));
int **comb_idx = calloc(channels, sizeof(int*)); // circular buffer positions
```

- ```comb_buffers[ch]``` → an array of pointers to each comb buffer for that channel.

- ```comb_idx[ch]``` → keeps track of where the next sample should go in each comb’s circular buffer.

- Every allocation is carefully checked: if any calloc refuses to work, the program tidies up the mess it made and gracefully exits.

Next, we furnish each hall with the comb filters themselves:

```c
for (int ch = 0; ch < channels; ch++) {
    comb_buffers[ch] = calloc(num_combs, sizeof(double*));
    comb_idx[ch] = calloc(num_combs, sizeof(int));
    for (int i = 0; i < num_combs; i++) {
        comb_buffers[ch][i] = calloc((size_t)comb_delays[i], sizeof(double));
        comb_idx[ch][i] = 0;
    }
}
```

- Each ```comb_buffers[ch][i]``` is a circular chamber of length comb_delays[i].

- Initialized to zero, because echoes should only appear after the first clap, not before.

- ```comb_idx[ch][i]``` starts at 0 and loops like a cheeky sprite around the chamber.

- Every channel has its own set of combs, so left and right don’t whisper secrets to each other.

**All-pass buffers**

And now, the same little ritual is performed for all-pass filters: their chambers are built in the same fashion, each channel independent, each buffer ready to tangle echoes just so.

```c
double ***ap_buffers = calloc(channels, sizeof(double**));
int **ap_idx = calloc(channels, sizeof(int*));
// Same pattern as comb_buffers
```

> **In short**: we now have a cathedral of circular chambers. Comb filters and all-pass filters, per channel, each waiting patiently for the waves of sound to come and dance within them. All arranged neatly in memory, without collisions, and with positions tracked carefully so no echo gets lost in the corridors.

## As We Approach The End

So we have wxplained all the diffucult functions. Only thing left to discuss is the --help and the main.

### Command Dispatch & Effect Handling

This is where **SoundWave listens politely, checks its notes, and executes orders**—sometimes across the whole file, sometimes only in a sneaky little corner of time.  

*Important and general*: 

- argc = number of arguments.

- argv = array of strings from the command line.

#### 1. `handle_effect()`

The full-file general:  

- Reads `argv[1]` to see what effect the user is asking for: `"info"`, `"volume"`, `"reverb"`, and the like.  
- Checks the arguments like a cautious librarian checking overdue books.  
- Converts strings to numbers (`atof()` / `atoi()`) with a polite nod.  
- Calls the effect function with `dj_mode = 0` (apply everywhere, no questions asked).  

Example: `volume` → convert `argv[2]` to `double`, ensure it’s positive, then `volume(fp_volume, 0, 0, 0)`.  

#### 2. `handle_dj_effect()`

The DJ mode: **for when SoundWave only wants to meddle in a slice of time**.  

- Takes `argv[2]` and `argv[3]` as start and end seconds.  
- Builds a tiny command array for the effect name and parameters.  
- Calls the same effect function as the full-file mode, but with `dj_mode = 1` and start/end frames.  

Think of it as SoundWave **sneaking in to tweak the volume while you’re not looking**.  

#### 3. `main()`

The gatekeeper and bouncer:  

- No arguments → hands you a polite help message.  
- `--help` → recites all global guidance.  
- `effect --help` → whispers the secret ways of that specific effect.  
- Dispatches wisely:  
  - `"dj"` → `handle_dj_effect()`  
  - Anything else → `handle_effect()`  

#### 4. Unified effect interface

All effects answer the same polite question:  

```effect_name(params..., int dj_mode, double start, double end)```


- `dj_mode = 0` → apply to the whole hall.  
- `dj_mode = 1` → only in the little corner you pointed out.  

This keeps SoundWave **tidy, modular, and not repeating itself unnecessarily**.  

#### 5. Argument parsing & safety

- `strcmp()` → recognizes which effect you meant.  
- `atoi()` / `atof()` → turns words into numbers.  
- Checks counts and ranges → prevents catastrophe.  
- Unknown options → politely points out the error and guides you back to sanity.  

 **In a nutshell:**  

- `handle_effect()` → everywhere, no exceptions.  
- `handle_dj_effect()` → sneaky, time-limited mischief.  
- `main()` → decides who gets attention.  
- All effects → reusable, checked, and ready to obey.
