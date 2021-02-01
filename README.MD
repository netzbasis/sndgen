sndgen(1) utility

Uses the OpenBSD build system and can be installed by

    $ make
    $ doas make install

# NAME

**sndgen** - generate sound

# SYNOPSIS

**sndgen**
\[**-lr**]
\[**-s**&nbsp;*hz*]

# DESCRIPTION

**sndgen**
is used to generate stereo sound and directly play it using sndiod(8).
The default output is white noise.

The options are as follows:

**-l**

> Only play the left channel.

**-r**

> Only play the right channel.

**-s** *hz*

> Play a sine wave at the specified frequency.

# SEE ALSO

aucat(1),
sndiod(8),
sio\_open(3)
