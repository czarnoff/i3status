// vim:ts=4:sw=4:expandtab
/*! 
 * \mainpage i3Status bar
 * \section mpd_sec print_mpd module
 * \author Jeffery Williams
 * \copyright GNU Public License
 
 * This code reads mpd status and adds it to the status bar.
 */

#include <stdio.h>
#include <string.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>
#include "i3status.h"

/* include from libclientmpd */
#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/message.h>


/*! print song info to the status bar */
static void print_tag(struct mpd_song **song, enum mpd_tag_type type,
	  char **outwalk)
{
	unsigned idx = 0;
	const char *value;

    if (*song == NULL) {
        return;
    }


    // Walk through the tag ids that match; only copy 50 characters
	while ((value = mpd_song_get_tag(*song, type, idx++)) != NULL)
    {
		*outwalk += snprintf(*outwalk, 50, "%s", value);
    }
}

/* Get information from mpd */
static char mpd_output(struct mpd_connection **mpd_conn, struct mpd_song **song)
{
	static struct mpd_status *status;
    int tmp_status=0;

    // if we don't have a connection, try to make one.
    if (*mpd_conn == NULL)
    {
        *mpd_conn = mpd_connection_new(NULL, 0, 30000);
    }

    //If the connection is bad, clean up and return.
    if (mpd_connection_get_error(*mpd_conn) !=MPD_ERROR_SUCCESS)
    {
        mpd_connection_free(*mpd_conn);
        *mpd_conn=NULL;
        return 0;
    }
    else //if the connection is good get the song and status
    {
		*song = mpd_run_current_song(*mpd_conn);

        //if we don't have a song loaded, return
        if (*song == NULL)
        {
            return 0;
        }


        status = mpd_run_status(*mpd_conn);

        // if the satus is good, read it and clean up.
        if (status != NULL)
        {
            tmp_status=mpd_status_get_state(status);
            mpd_status_free(status);
            return tmp_status;
        }
        else //return
        {
            return 0;
        }


    }

    return 0;
}

/*! 
 * \brief Add mpd info to the status bar */
void print_mpd(yajl_gen json_gen, char *buffer, const char *title, const char *format, const char *format_down) {
    char running = 0;
    const char *walk;
    char *outwalk = buffer;

    //mpd connection and song pointers
    static struct mpd_connection *mpd_conn ;
    static struct mpd_song *song;

    /* Pass the mpd_conn pointer and song pointer by reference */
    running = mpd_output(&mpd_conn, &song);

    //Choose the output format.
    if (running || format_down == NULL) {
        walk = format;
    } else {
        walk = format_down;
    }

    /*! \subsection color Colors
     * Choose the color based on status
     */
    switch (running) {

        case 0 : //!Unknown state (no song loaded): Bad color
            START_COLOR("color_bad");
            break;

        case 1 : //!Stopped: Degraded color
            START_COLOR("color_degraded");
            break;

        case 2 : //!Playing: Good color
            START_COLOR("color_good");
            break;

        case 3 : //! Paused: Default color
            break;

        default : //How did you get here?
            START_COLOR("color_bad");
            walk="Error";
    }

    //Put it all together.
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

    //clean up the song memory.
    if (song != NULL){
	   mpd_song_free(song);
    }

    END_COLOR;
    OUTPUT_FULL_TEXT(buffer);
}
