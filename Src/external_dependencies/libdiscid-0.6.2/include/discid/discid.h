/* --------------------------------------------------------------------------

   MusicBrainz -- The Internet music metadatabase

   Copyright (C) 2013 Johannes Dewender
   Copyright (C) 2006-2010 Lukas Lalinsky
   Copyright (C) 2006 Matthias Friedrich
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

--------------------------------------------------------------------------- */
#ifndef MUSICBRAINZ_DISC_ID_H
#define MUSICBRAINZ_DISC_ID_H

#if (defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
#	ifdef libdiscid_EXPORTS
#		define LIBDISCID_API __declspec(dllexport)
#	else
#		define LIBDISCID_API __declspec(dllimport)
#	endif
#	define LIBDISCID_INTERNAL
#elif (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)
#	define LIBDISCID_API
#	define LIBDISCID_INTERNAL __attribute__((visibility("hidden")))
#elif defined(__SUNPRO_C)
#	define LIBDISCID_API __global
#	define LIBDISCID_INTERNAL __hidden
#else
#	define LIBDISCID_API
#	define LIBDISCID_INTERNAL
#endif

#if (defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__))
#define LIBDISCID_DEPRECATED __declspec(deprecated)
#elif (defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))) || defined(__clang__)
#define LIBDISCID_DEPRECATED __attribute__((deprecated))
#else
#define LIBDISCID_DEPRECATED
#endif

#define DISCID_VERSION_MAJOR @libdiscid_MAJOR@
#define DISCID_VERSION_MINOR @libdiscid_MINOR@
#define DISCID_VERSION_PATCH @libdiscid_PATCH@
#define DISCID_VERSION_NUM @libdiscid_VERSION_NUM@

#ifdef __cplusplus
  extern "C" {
#endif

/*!
 * \mainpage libdiscid
 * \section intro Introduction
 *
 * Libdiscid is a C library for calculating DiscIDs
 * (<a href="http://musicbrainz.org/doc/Disc ID">MusicBrainz</a>
 * and <a href="http://freedb.org">freedb</a>)
 * for Audio CDs.
 * Additionally the library can extract the MCN/UPC/EAN and the
 * <a href="http://musicbrainz.org/doc/ISRC">ISRCs</a> from disc.
 *
 * The idea is to have an easy to use library without any dependencies
 * that can be used from scripting languages.
 *
 * The API is documented in discid.h.
 *
 * \section examples Examples
 *
 * This is an example of the most basic usage:
 *
 * \code
 *
 * DiscId *disc = discid_new();
 *
 * if ( discid_read_sparse(disc, "/dev/cdrom", 0) == 0 ) {
 *     fprintf(stderr, "Error: %s\n", discid_get_error_msg(disc));
 *     return 1;
 * }
 * 
 * printf("DiscID        : %s\n", discid_get_id(disc));
 * printf("Submit via    : %s\n", discid_get_submission_url(disc));
 *
 * discid_free(disc); 
 *
 * \endcode
 *
 * \section Building
 *
 * libdiscid provides a pkg-config script that returns the necessary compiler and linker flags, as well as the
 * version number.  To build a small sample program one would use:
 *
 * @par
 * <tt>gcc libdiscid-test.c \`pkg-config libdiscid --cflags --libs\` -o libdiscid-test</tt>
 *
 * \section Contact
 *
 *  - <a href="http://lists.musicbrainz.org/mailman/listinfo/musicbrainz-devel">MusicBrainz Development Mailing List</a>
 *  - <a href="http://tickets.musicbrainz.org/browse/LIB">MusicBrainz Bug Tracker</a>
 *  - <a href="http://musicbrainz.org/doc/libdiscid">MusicBrainz Documentation</a>
 *  - <a href="https://github.com/metabrainz/libdiscid">Github Repository</a>
 *
 */ 


/**
 * A transparent handle for an Audio CD.
 *
 * This is returned by discid_new() and has to be passed as the first
 * parameter to all discid_*() functions.
 */
typedef void *DiscId;


/**
 * Return a handle for a new DiscId object.
 *
 * If no memory could be allocated, NULL is returned. Don't use the created
 * DiscId object before calling discid_read() or discid_put().
 *
 * @return a DiscId object, or NULL.
 */
LIBDISCID_API DiscId *discid_new();


/**
 * Release the memory allocated for the DiscId object.
 *
 * @param d a DiscId object created by discid_new()
 */
LIBDISCID_API void discid_free(DiscId *d);


/**
 * Read all supported features of the disc in the given CD-ROM/DVD-ROM drive.
 *
 * This function reads the disc in the drive specified by the given device
 * identifier. If the device is NULL, the default drive, as returned by
 * discid_get_default_device() is used.
 *
 * If you do not require all features provided by libdiscid, such as MCN
 * or ISRC reading, you should consider using discid_read_sparse() instead
 * of discid_read() for performance reasons.
 *
 * On error, this function returns false and sets the error message which you
 * can access using discid_get_error_msg(). In this case, the other functions
 * won't return meaningful values and should not be used.
 *
 * This function may be used multiple times with the same DiscId object.
 *
 * @param d a DiscId object created by discid_new()
 * @param device an operating system dependent device identifier, or NULL
 * @return true if successful, or false on error.
 */
LIBDISCID_API int discid_read(DiscId *d, const char *device);


/**
 * Read the disc in the given CD-ROM/DVD-ROM drive
 * extracting only the TOC and additionally specified features.
 *
 * This function will always read the TOC, but additional features
 * like ::DISCID_FEATURE_MCN and ::DISCID_FEATURE_ISRC can be set
 * using the features parameter.
 * Multiple features can be set using bitwise OR.
 *
 * If you only want to generate a disc ID, you only need the TOC,
 * so set features to 0:
 * \code
 * discid_read_sparse(disc, device, 0)
 * \endcode
 * This is a bit more verbose, but equivalent since ::DISCID_FEATURE_READ
 * is always implied:
 * \code
 * discid_read_sparse(disc, device, DISCID_FEATURE_READ)
 * \endcode
 *
 * If you want to read all features available, you can use discid_read().
 *
 * On error, this function returns false and sets the error message which you
 * can access using discid_get_error_msg(). In this case, the other functions
 * won't return meaningful values and should not be used.
 *
 * This function may be used multiple times with the same DiscId object.
 *
 * \since libdiscid 0.5.0
 *
 * @param d a DiscId object created by discid_new()
 * @param device an operating system dependent device identifier, or NULL
 * @param features a list of bit flags from the enum ::discid_feature
 * @return true if successful, or false on error.
 */
LIBDISCID_API int discid_read_sparse(DiscId *d, const char *device,
				     unsigned int features);

#define DISCID_HAVE_SPARSE_READ

/**
 * Provides the TOC of a known CD.
 *
 * This function may be used if the TOC has been read earlier and you
 * want to calculate the disc ID afterwards, without accessing the disc
 * drive. It replaces the discid_read function in this case.
 *
 * On error, this function returns false and sets the error message which you
 * can access using discid_get_error_msg(). In this case, the other functions
 * won't return meaningful values and should not be used.
 *
 * The offsets parameter points to an array which contains the track offsets
 * for each track. The first element, offsets[0], is the leadout track. It
 * must contain the total number of sectors on the disc.
 *
 * For discs with additional data tracks, the trailing data tracks
 * should be ignored. offset[0] should then be the last sector of the last
 * audio track.
 * Make sure the length of the last audio track as returned by libdiscid
 * after a put is the same as the length of your last audio track.
 * Depending on your tools you might need to substract 11400 (2:32 min.).
 * See also:
 * <a href="http://musicbrainz.org/doc/Disc_ID_Calculation">Disc ID Calculation</a>
 *
 *
 * @param d a DiscID object created by discid_new()
 * @param first the number of the first audio track on disc (usually one)
 * @param last the number of the last audio track on the disc
 * @param offsets a pointer to an array of 100 track offsets
 * @return true if the given data was valid, and false on error
 */
LIBDISCID_API int discid_put(DiscId *d, int first, int last, int *offsets);


/**
 * Return a human-readable error message.
 *
 * This function may only be used if discid_read() failed. The returned
 * error message is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string describing the error that occurred
 */
LIBDISCID_API char *discid_get_error_msg(DiscId *d);


/**
 * Return a MusicBrainz DiscID.
 *
 * The returned string is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing a MusicBrainz DiscID
 */
LIBDISCID_API char *discid_get_id(DiscId *d);


/**
 * Return a FreeDB DiscID.
 *
 * The returned string is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing a FreeDB DiscID
 */
LIBDISCID_API char *discid_get_freedb_id(DiscId *d);

/**
 * Return a string representing CD Table Of Contents (TOC).
 *
 * The string has following values separated by space:
 * first track number
 * last track number
 * total length in sectors
 * offset of 1st track
 * offset of 2nd track
 * ...
 *
 * Example:
 * 1 7 164900 150 22460 50197 80614 100828 133318 144712
 *
 * The returned string is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing TOC information
 */
LIBDISCID_API char *discid_get_toc_string(DiscId *d);

/**
 * Return an URL for submitting the DiscID to MusicBrainz.
 *
 * The URL leads to an interactive disc submission wizard that guides the
 * user through the process of associating this disc's DiscID with a
 * release in the MusicBrainz database.
 *
 * The returned string is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing an URL
 */
LIBDISCID_API char *discid_get_submission_url(DiscId *d);

/**
 * Return an URL for retrieving CD information from MusicBrainz' web service
 *
 * The URL provides the CD information in XML. 
 * See http://musicbrainz.org/development/mmd for details.
 *
 * The returned string is only valid as long as the DiscId object exists.
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing an URL
 *
 * @deprecated This function is deprecated. Please use libmusicbrainz instead
 * since this function returns an URL referring the deprecated webservice.
 */
LIBDISCID_API LIBDISCID_DEPRECATED char *discid_get_webservice_url(DiscId *d);

/**
 * Return the name of the default disc drive for this machine.
 * This isn't constant, but possibly depends on the drives currently
 * attached, depending on the platform.
 * For this reason you should call this once and save it
 * when you want to make sure to use the same drive for
 * multiple operations.
 *
 * The returned string is thread local and owned by libdiscid internally.
 *
 * @return a string containing an operating system dependent device identifier
 */
LIBDISCID_API char *discid_get_default_device(void);


/**
 * Return the number of the first track on this disc.
 *
 * @param d a DiscId object created by discid_new()
 * @return the number of the first track
 */
LIBDISCID_API int discid_get_first_track_num(DiscId *d);


/**
 * Return the number of the last audio track on this disc.
 *
 * @param d a DiscId object created by discid_new()
 * @return the number of the last track
 */
LIBDISCID_API int discid_get_last_track_num(DiscId *d);


/**
 * Return the length of the disc in sectors.
 *
 * @param d a DiscId object created by discid_new()
 * @return the length of the disc in sectors
 */
LIBDISCID_API int discid_get_sectors(DiscId *d);


/**
 * Return the sector offset of a track.
 *
 * Only track numbers between (and including) discid_get_first_track_num()
 * and discid_get_last_track_num() may be used.
 *
 * @param d a DiscId object created by discid_new()
 * @param track_num the number of a track
 * @return sector offset of the specified track
 */
LIBDISCID_API int discid_get_track_offset(DiscId *d, int track_num);


/**
 * Return the length of a track in sectors.
 *
 * Only track numbers between (and including) discid_get_first_track_num()
 * and discid_get_last_track_num() may be used.
 *
 * @param d a DiscId object created by discid_new()
 * @param track_num the number of a track
 * @return length of the specified track
 */
LIBDISCID_API int discid_get_track_length(DiscId *d, int track_num);

/**
 * Return the Media Catalogue Number (MCN) for the disc.
 *
 * This is essentially an EAN (= UPC with 0 prefix).
 *
 * \since libdiscid 0.3.0
 *
 * @param d a DiscId object created by discid_new()
 * @return a string containing an Media Catalogue Number of the disk
 */
LIBDISCID_API char* discid_get_mcn(DiscId *d);

/**
 * Return the ISRC for a track.
 *
 * Only track numbers between (and including) discid_get_first_track_num()
 * and discid_get_last_track_num() may be used.
 *
 * \since libdiscid 0.3.0
 *
 * @param d a DiscId object created by discid_new()
 * @param track_num the number of a track
 * @return a string containing an ISRC for the specified track
 */
LIBDISCID_API char* discid_get_track_isrc(DiscId *d, int track_num);


/**
 * PLATFORM-DEPENDENT FEATURES
 *
 * The platform dependent features are currently:
 *   - "read"	read TOC from disc
 *   - "mcn"	read MCN from disc
 *   - "isrc"	read ISRC from disc
 *
 * A table in the
 * <a href="http://musicbrainz.org/doc/libdiscid">MusicBrainz Documentation</a>
 * specifies which features are available on which platform in what version.
 *
 * In the code you can use discid_get_feature_list() or discid_has_feature()
 * below to get the features for your platform in this version.
 */

enum discid_feature {
	DISCID_FEATURE_READ = 1 << 0,
	DISCID_FEATURE_MCN  = 1 << 1,
	DISCID_FEATURE_ISRC = 1 << 2,
};
/**
 * Check if a certain feature is implemented on the current platform.
 *
 * This only works for single features, not bit masks with multiple features.
 *
 * \since libdiscid 0.4.0
 *
 * @param feature as enum ::discid_feature
 * @return 1 if the feature is implemented and 0 if not.
 */
LIBDISCID_API int discid_has_feature(enum discid_feature feature);

#define DISCID_FEATURE_STR_READ		"read"
#define DISCID_FEATURE_STR_MCN		"mcn"
#define DISCID_FEATURE_STR_ISRC		"isrc"
#define DISCID_FEATURE_LENGTH		32
/**
 * Return a list of features supported by the current platform.
 * The array of length ::DISCID_FEATURE_LENGTH should be allocated by the user.
 * After the call each element of the array is either NULL
 * or a pointer to a static string.
 *
 * \since libdiscid 0.4.0
 *
 * @param[out] features a static string array of length ::DISCID_FEATURE_LENGTH
 */
LIBDISCID_API void discid_get_feature_list(
		char *features[DISCID_FEATURE_LENGTH]);

/**
 * Return the full version string of this library, including the name.
 * This can be used for debug output.
 * Don't use this to test for features, see discid_has_feature().
 *
 * \since libdiscid 0.4.0
 *
 * @return a string containing the version of libdiscid.
 */
LIBDISCID_API char *discid_get_version_string(void);


#ifdef __cplusplus
  }
#endif

#endif /* MUSICBRAINZ_DISC_ID_H */
