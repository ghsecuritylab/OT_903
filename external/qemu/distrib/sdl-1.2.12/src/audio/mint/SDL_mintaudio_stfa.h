
#include "SDL_config.h"


#ifndef _SDL_mintaudio_stfa_h
#define _SDL_mintaudio_stfa_h

/*--- Defines ---*/

#define C_STFA	0x53544641L	/* Sound treiber f�r atari (seb/The removers) */

#define STFA_PLAY_ENABLE	(1<<0)
#define STFA_PLAY_DISABLE	(0<<0)
#define STFA_PLAY_REPEAT	(1<<1)
#define STFA_PLAY_SINGLE	(0<<1)

#define STFA_FORMAT_SIGNED		(1<<15)
#define STFA_FORMAT_UNSIGNED	(0<<15)
#define STFA_FORMAT_STEREO		(1<<14)
#define STFA_FORMAT_MONO		(0<<14)
#define STFA_FORMAT_16BIT		(1<<13)
#define STFA_FORMAT_8BIT		(0<<13)
#define STFA_FORMAT_LITENDIAN	(1<<9)
#define STFA_FORMAT_BIGENDIAN	(0<<9)
#define STFA_FORMAT_FREQ_MASK	0x0f
enum {
	STFA_FORMAT_F4995=0,
	STFA_FORMAT_F6269,
	STFA_FORMAT_F7493,
	STFA_FORMAT_F8192,

	STFA_FORMAT_F9830,
	STFA_FORMAT_F10971,
	STFA_FORMAT_F12538,
	STFA_FORMAT_F14985,

	STFA_FORMAT_F16384,
	STFA_FORMAT_F19819,
	STFA_FORMAT_F21943,
	STFA_FORMAT_F24576,

	STFA_FORMAT_F30720,
	STFA_FORMAT_F32336,
	STFA_FORMAT_F43885,
	STFA_FORMAT_F49152
};

/*--- Types ---*/

typedef struct {
	unsigned short sound_enable;
	unsigned short sound_control;
	unsigned short sound_output;
	unsigned long sound_start;
	unsigned long sound_current;
	unsigned long sound_end;
	unsigned short version;
	void *old_vbl;
	void *old_timera;
	unsigned long old_mfp_status;
	void *new_vbl;
	void *drivers_list;
	void *play_stop;
	unsigned short frequency;
	void *set_frequency;
	
	unsigned short frequency_threshold;
	unsigned short *custom_freq_table;
	unsigned short stfa_on_off;
	void *new_drivers_list;
	unsigned long old_bit_2_of_cookie_snd;
	void (*stfa_it)(void);
} cookie_stfa_t __attribute__((packed));

#endif /* _SDL_mintaudio_stfa_h */
