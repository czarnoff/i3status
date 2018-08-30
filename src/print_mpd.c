// vim:ts=4:sw=4:expandtab
#include <stdio.h>
#include <string.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>
#include "i3status.h"

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/message.h>

static void
print_tag(struct mpd_song **song, enum mpd_tag_type type,
	  const char *label, char **outwalk)
{
	unsigned i = 0;
	const char *value;

    if (*song == NULL) {
        return;
    }

	while ((value = mpd_song_get_tag(*song, type, i++)) != NULL)
		*outwalk += snprintf(*outwalk, 50, "%s:%s", label, value);
}

static bool mpd_output(struct mpd_connection **mpd_conn, struct mpd_song **song)
{
    if (*mpd_conn == NULL)
    {

        *mpd_conn = mpd_connection_new(NULL, 0, 30000);
    }

    if (mpd_connection_get_error(*mpd_conn) !=MPD_ERROR_SUCCESS)
    {
        mpd_connection_free(*mpd_conn);
        *mpd_conn=NULL;
        return false;
    }
    else
    {
		*song = mpd_run_current_song(*mpd_conn);
        
        if (*song == NULL)
            return false;
        
        return true;
    }

}

void print_mpd(yajl_gen json_gen, char *buffer, const char *title, const char *format, const char *format_down) {
    bool running = false;
    const char *walk;
    char *outwalk = buffer;
    
    static struct mpd_connection *mpd_conn ;
    static struct mpd_song *song;
   
    running = mpd_output(&mpd_conn, &song);

    if (running || format_down == NULL) {
        walk = format;
    } else {
        walk = format_down;
    }
    


    START_COLOR((running ? "color_good" : "color_bad"));
    

    for (; *walk != '\0'; walk++) {
        if (*walk != '%') {
            *(outwalk++) = *walk;

        } else if (BEGINS_WITH(walk + 1, "artist")) {
			print_tag(&song, MPD_TAG_ARTIST, "art", &outwalk);
            walk += strlen("artist");

        } else if (BEGINS_WITH(walk + 1, "title")) {
			print_tag(&song, MPD_TAG_TITLE, "tit", &outwalk);
            walk += strlen("title");

        } else if (BEGINS_WITH(walk + 1, "track")) {
			print_tag(&song, MPD_TAG_TRACK, "tra", &outwalk);
            walk += strlen("track");

        } else if (BEGINS_WITH(walk + 1, "album")) {
			print_tag(&song, MPD_TAG_ALBUM, "alb", &outwalk);
            walk += strlen("album");

        } else {
            *(outwalk++) = '%';
        }
    }

    END_COLOR;
    OUTPUT_FULL_TEXT(buffer);
}
