// log.cpp -- Logging routines.
//
// $Id: log.cpp,v 1.7 2003/02/17 01:59:21 sdennis Exp $
//

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"

#include <sys/types.h>

NAMETAB logdata_nametab[] =
{
    {"flags",           1,  0,  LOGOPT_FLAGS},
    {"location",        1,  0,  LOGOPT_LOC},
    {"owner",           1,  0,  LOGOPT_OWNER},
    {"timestamp",       1,  0,  LOGOPT_TIMESTAMP},
    { NULL,             0,  0,  0}
};

NAMETAB logoptions_nametab[] =
{
    {"accounting",      2,  0,  LOG_ACCOUNTING},
    {"all_commands",    2,  0,  LOG_ALLCOMMANDS},
    {"bad_commands",    2,  0,  LOG_BADCOMMANDS},
    {"buffer_alloc",    3,  0,  LOG_ALLOCATE},
    {"bugs",            3,  0,  LOG_BUGS},
    {"checkpoints",     2,  0,  LOG_DBSAVES},
    {"config_changes",  2,  0,  LOG_CONFIGMODS},
    {"create",          2,  0,  LOG_PCREATES},
    {"killing",         1,  0,  LOG_KILLS},
    {"logins",          1,  0,  LOG_LOGIN},
    {"network",         1,  0,  LOG_NET},
    {"problems",        1,  0,  LOG_PROBLEMS},
    {"security",        2,  0,  LOG_SECURITY},
    {"shouts",          2,  0,  LOG_SHOUTS},
    {"startup",         2,  0,  LOG_STARTUP},
    {"suspect",         2,  0,  LOG_SUSPECTCMDS},
    {"time_usage",      1,  0,  LOG_TIMEUSE},
    {"wizard",          1,  0,  LOG_WIZARD},
    { NULL,                     0,  0,  0}
};

/* ---------------------------------------------------------------------------
 * start_log: see if it is OK to log something, and if so, start writing the
 * log entry.
 */

bool start_log(const char *primary, const char *secondary)
{
    mudstate.logging++;
    if (  1 <= mudstate.logging
       && mudstate.logging <= 2)
    {
        if (!mudstate.bStandAlone)
        {
            // Format the timestamp.
            //
            char buffer[256];
            buffer[0] = '\0';
            if (mudconf.log_info & LOGOPT_TIMESTAMP)
            {
                CLinearTimeAbsolute ltaNow;
                ltaNow.GetLocal();
                FIELDEDTIME ft;
                ltaNow.ReturnFields(&ft);
                sprintf(buffer, "%d.%02d%02d:%02d%02d%02d ",ft.iYear,
                    ft.iMonth, ft.iDayOfMonth, ft.iHour, ft.iMinute,
                    ft.iSecond);
            }

            // Write the header to the log.
            //
            if (  secondary
               && *secondary)
            {
                Log.tinyprintf("%s%s %3s/%-5s: ", buffer, mudconf.mud_name,
                    primary, secondary);
            }
            else
            {
                Log.tinyprintf("%s%s %-9s: ", buffer, mudconf.mud_name,
                    primary);
            }
        }

        // If a recursive call, log it and return indicating no log.
        //
        if (mudstate.logging == 1)
        {
            return true;
        }
        Log.WriteString("Recursive logging request." ENDLINE);
    }
    mudstate.logging--;
    return false;
}

/* ---------------------------------------------------------------------------
 * end_log: Finish up writing a log entry
 */

void end_log(void)
{
    Log.WriteString(ENDLINE);
    Log.Flush();
    mudstate.logging--;
}

/* ---------------------------------------------------------------------------
 * log_perror: Write perror message to the log
 */

void log_perror(const char *primary, const char *secondary, const char *extra, const char *failing_object)
{
    start_log(primary, secondary);
    if (extra && *extra)
    {
        log_text("(");
        log_text(extra);
        log_text(") ");
    }

    // <Failing_object text>: <strerror() text>
    //
    Log.WriteString(failing_object);
    Log.WriteString(": ");
    Log.WriteString(strerror(errno));
#ifndef WIN32
    Log.WriteString(ENDLINE);
#endif // !WIN32
    Log.Flush();
    mudstate.logging--;
}

/* ---------------------------------------------------------------------------
 * log_text, log_number: Write text or number to the log file.
 */

void log_text(const char *text)
{
    Log.WriteString(strip_ansi(text));
}

void log_number(int num)
{
    Log.WriteInteger(num);
}

/* ---------------------------------------------------------------------------
 * log_name: write the name, db number, and flags of an object to the log.
 * If the object does not own itself, append the name, db number, and flags
 * of the owner.
 */

void log_name(dbref target)
{
    if (mudstate.bStandAlone)
    {
        Log.tinyprintf("%s(#%d)", Name(target), target);
    }
    else
    {
        char *tp;

        if (mudconf.log_info & LOGOPT_FLAGS)
        {
            tp = unparse_object(GOD, target, false);
        }
        else
        {
            tp = unparse_object_numonly(target);
        }
        Log.WriteString(strip_ansi(tp));
        free_lbuf(tp);
        if (  (mudconf.log_info & LOGOPT_OWNER)
           && target != Owner(target))
        {
            if (mudconf.log_info & LOGOPT_FLAGS)
            {
                tp = unparse_object(GOD, Owner(target), false);
            }
            else
            {
                tp = unparse_object_numonly(Owner(target));
            }
            Log.tinyprintf("[%s]", strip_ansi(tp));
            free_lbuf(tp);
        }
    }
}

/* ---------------------------------------------------------------------------
 * log_name_and_loc: Log both the name and location of an object
 */

void log_name_and_loc(dbref player)
{
    log_name(player);
    if (  (mudconf.log_info & LOGOPT_LOC)
       && Has_location(player))
    {
        log_text(" in ");
        log_name(Location(player));
    }
    return;
}

const char *OBJTYP(dbref thing)
{
    if (!Good_dbref(thing))
    {
        return "??OUT-OF-RANGE??";
    }
    switch (Typeof(thing))
    {
    case TYPE_PLAYER:
        return "PLAYER";
    case TYPE_THING:
        return "THING";
    case TYPE_ROOM:
        return "ROOM";
    case TYPE_EXIT:
        return "EXIT";
    case TYPE_GARBAGE:
        return "GARBAGE";
    default:
        return "??ILLEGAL??";
    }
}

void log_type_and_name(dbref thing)
{
    char nbuf[16];

    log_text(OBJTYP(thing));
    sprintf(nbuf, " #%d(", thing);
    log_text(nbuf);
    if (Good_obj(thing))
    {
        log_text(Name(thing));
    }
    log_text(")");
    return;
}

void do_log
(
    dbref executor,
    dbref caller,
    dbref enactor,
    int   key,
    int   nargs,
    char *whichlog,
    char *logtext
)
{
    bool bValid = true;

    // Strip the filename of all ANSI.
    //
    char *pFilename = strip_ansi(whichlog);

    // Restrict filename to a subdirectory to reduce the possibility
    // of a security hole.
    //
    char *temp_ptr = strrchr(pFilename, '/');
    if (temp_ptr)
    {
        pFilename = ++temp_ptr;
    }
    temp_ptr = strrchr(pFilename, '\\');
    if (temp_ptr)
    {
        pFilename = ++temp_ptr;
    }

    // Check for and disallow leading periods, empty strings
    // and filenames over 30 characters.
    //
    size_t n = strlen(pFilename);
    if (  n == 0
       || n > 30)
    {
        bValid = false;
    }
    else
    {
        unsigned int i;
        for (i = 0; i < n; i++)
        {
            if (!mux_isalnum(pFilename[i]))
            {
                bValid = false;
                break;
            }
        }
    }

    char *pFullName = NULL;
    char *pMessage = "";
    if (bValid)
    {
        pFullName = alloc_lbuf("do_log_filename");
        sprintf(pFullName, "logs/M-%s.log", pFilename);

        // Strip the message of all ANSI.
        //
        pMessage = strip_ansi(logtext);

        // Check for and disallow empty messages.
        //
        if (pMessage[0] == '\0')
        {
            bValid = false;
        }
    }
    if (!bValid)
    {
        if (pFullName) free_lbuf(pFullName);
        notify(executor, "Syntax: @log file=message");
        return;
    }

    FILE *hFile = fopen(pFullName, "r");
    if (hFile)
    {
        fclose(hFile);
        hFile = fopen(pFullName, "a");
    }
    if (hFile == NULL)
    {
        notify(executor, "Not a valid log file.");
        if (pFullName) free_lbuf(pFullName);
        return;
    }

    // Okay, at this point, the file exists.
    //
    fprintf(hFile, "%s" ENDLINE, pMessage);
    fclose(hFile);
    free_lbuf(pFullName);
}
