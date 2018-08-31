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

static void print_tag(struct mpd_song **song, enum mpd_tag_type type,
	  char **outwalk)
{
	unsigned i = 0;
	const char *value;

    if (*song == NULL) {
        return;
    }

	while ((value = mpd_song_get_tag(*song, type, i++)) != NULL)
		*outwalk += snprintf(*outwalk, 50, "%s", value);
}

static char mpd_output(struct mpd_connection **mpd_conn, struct mpd_song **song)
{
	struct mpd_status *status;
    int tmp_status=0;

    if (*mpd_conn == NULL)
    {

        *mpd_conn = mpd_connection_new(NULL, 0, 30000);
    }

    if (mpd_connection_get_error(*mpd_conn) !=MPD_ERROR_SUCCESS)
    {
        mpd_connection_free(*mpd_conn);
        *mpd_conn=NULL;
        return 0;
    }
    else
    {
		*song = mpd_run_current_song(*mpd_conn);
        
        if (*song == NULL)
        {
            return 0;
        }
        status = mpd_run_status(*mpd_conn);
        
        if (status != NULL)
        {
            tmp_status=mpd_status_get_state(status);
            mpd_status_free(status);
        }
        else
        {
            return 0;
        }


        return tmp_status;
    }

}

void print_mpd(yajl_gen json_gen, char *buffer, const char *title, const char *format, const char *format_down) {
    char running = 0;
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

    switch (running) {
        case 0 :
            START_COLOR("color_bad");
            break;
        case 1 :
            START_COLOR("color_degraded");
            break;
        case 2 :
            START_COLOR("color_good");
            break;
        case 3 :
            break;
        default :
            START_COLOR("color_bad");
            walk="Error";
    }

    for (; *walk != '\0'; walk++) {
        if (*walk != '%') {
            *(outwalk++) = *walk;

        } else if (BEGINS_WITH(walk + 1, "artist")) {
			print_tag(&song, MPD_TAG_ARTIST, &outwalk);
            walk += strlen("artist");

        } else if (BEGINS_WITH(walk + 1, "title")) {
			print_tag(&song, MPD_TAG_TITLE, &outwalk);
            walk += strlen("title");

        } else if (BEGINS_WITH(walk + 1, "track")) {
			print_tag(&song, MPD_TAG_TRACK, &outwalk);
            walk += strlen("track");

        } else if (BEGINS_WITH(walk + 1, "album")) {
			print_tag(&song, MPD_TAG_ALBUM, &outwalk);
            walk += strlen("album");

        } else {
            *(outwalk++) = '%';
        }
    }
    if (song != NULL){
	   mpd_song_free(song);
    }

    END_COLOR;
    OUTPUT_FULL_TEXT(buffer);
}
