// speech.cpp -- Commands which involve speaking.
//
// $Id: speech.cpp,v 1.20 2005/08/05 15:37:50 sdennis Exp $
//

#include "copyright.h"
#include "autoconf.h"
#include "config.h"
#include "externs.h"

#include "attrs.h"
#include "interface.h"
#include "powers.h"
#ifdef REALITY_LVLS
#include "levels.h"
#endif

char *modSpeech(dbref player, char *message, bool bWhich, char *command)
{
    dbref aowner;
    int aflags;
    char *mod = atr_get(player, bWhich ? A_SPEECHMOD : A_SAYSTRING,
        &aowner, &aflags);

    if (  mod[0] == '\0'
       || MuxAlarm.bAlarmed)
    {
        free_lbuf(mod);
        return NULL;
    }

    char *mod_orig = mod;
    char *new_message = alloc_lbuf("modspeech");
    char *t_ptr = new_message;
    char *args[2];
    args[0] = message;
    args[1] = command;
    mux_exec(new_message, &t_ptr, player, player, player,
        EV_FCHECK | EV_EVAL | EV_TOP, &mod, args, 2);
    free_lbuf(mod_orig);
    return new_message;
}

static int idle_timeout_val(dbref player)
{
    // If IDLETIMEOUT attribute is not present, the value
    // returned will be zero.
    //
    dbref aowner;
    int aflags;
    char *ITbuffer = atr_get(player, A_IDLETMOUT, &aowner, &aflags);
    int idle_timeout = mux_atol(ITbuffer);
    free_lbuf(ITbuffer);
    return idle_timeout;
}

bool sp_ok(dbref player)
{
    if (  Gagged(player)
       && !Wizard(player))
    {
        notify(player, "Sorry. Gagged players cannot speak.");
        return false;
    }

    if (!mudconf.robot_speak)
    {
        if (Robot(player) && !Controls(player, Location(player)))
        {
            notify(player, "Sorry, robots may not speak in public.");
            return false;
        }
    }
    if (Auditorium(Location(player)))
    {
        if (!could_doit(player, Location(player), A_LSPEECH))
        {
            notify(player, "Sorry, you may not speak in this place.");
            return false;
        }
    }
    return true;
}

void wall_broadcast(int target, dbref player, char *message)
{
    DESC *d;
    DESC_ITER_CONN(d)
    {
        switch (target)
        {
        case SHOUT_WIZARD:

            if (Wizard(d->player))
            {
                notify_with_cause(d->player, player, message);
            }
            break;

        case SHOUT_ADMIN:

            if (WizRoy(d->player))
            {
                notify_with_cause(d->player, player, message);
            }
            break;

        default:

            notify_with_cause(d->player, player, message);
            break;
        }
    }
}

static void say_shout(int target, const char *prefix, int flags,
    dbref player, char *message)
{
    char *p;
    if (flags & SAY_NOTAG)
    {
        p = tprintf("%s%s", Moniker(player), message);
    }
    else
    {
        p = tprintf("%s%s%s", prefix, Moniker(player), message);
    }
    wall_broadcast(target, player, p);
}

static const char *announce_msg = "Announcement: ";
static const char *broadcast_msg = "Broadcast: ";
static const char *admin_msg = "Admin: ";

void do_think(dbref executor, dbref caller, dbref enactor, int key,
    char *message)
{
    char *str, *buf, *bp;

    buf = bp = alloc_lbuf("do_think");
    str = message;
    mux_exec(buf, &bp, executor, caller, enactor, EV_FCHECK | EV_EVAL | EV_TOP,
             &str, (char **)NULL, 0);
    *bp = '\0';
    notify(executor, buf);
    free_lbuf(buf);
}

void do_say(dbref executor, dbref caller, dbref enactor, int key, char *message)
{
    // Make sure speaker is somewhere if speaking in a place
    //
    dbref loc = where_is(executor);
    if ( !(  Good_obj(loc)
            & sp_ok(executor)))
    {
        return;
    }

    int say_flags, depth;

    // Convert prefix-coded messages into the normal type
    //
    say_flags = key & (SAY_NOEVAL | SAY_NOTAG | SAY_HERE | SAY_ROOM | SAY_HTML);
    key &= ~(SAY_NOEVAL | SAY_NOTAG | SAY_HERE | SAY_ROOM | SAY_HTML);

    if (key == SAY_PREFIX)
    {
        switch (*message)
        {
        case '"':
            message++;
            key = SAY_SAY;
            break;

        case ':':
            message++;
            if (*message == ' ')
            {
                message++;
                key = SAY_POSE_NOSPC;
            }
            else
            {
                key = SAY_POSE;
            }
            break;

        case ';':
            message++;
            key = SAY_POSE_NOSPC;
            break;

        case '\\':
            message++;

            // FALLTHROUGH
            //

        default:
            key = SAY_EMIT;
            break;
        }
    }

    char *command = "";
    if (SAY_SAY == key)
    {
        command = "say";
    }
    else if (SAY_POSE == key || SAY_POSE_NOSPC == key)
    {
        command = "pose";
    }
    else if (SAY_EMIT == key)
    {
        command = "@emit";
    }

    // Parse speechmod if present.
    //
    char *messageOrig = message;
    char *messageNew = NULL;
    if (!(say_flags & SAY_NOEVAL))
    {
        messageNew = modSpeech(executor, message, true, command);
        if (messageNew)
        {
            message = messageNew;
        }
    }

    // Send the message on its way
    //
    char *saystring;
    switch (key)
    {
    case SAY_SAY:
        saystring = modSpeech(executor, messageOrig, false, command);
        if (saystring)
        {
            notify_saypose(executor, tprintf("%s %s \"%s\"",
                Moniker(executor), saystring, message));
#ifdef REALITY_LVLS
            notify_except_rlevel(loc, executor, executor, tprintf("%s %s \"%s\"", Moniker(executor), saystring, message), MSG_SAYPOSE);
#else
            notify_except(loc, executor, executor, tprintf("%s %s \"%s\"", Moniker(executor), saystring, message), MSG_SAYPOSE);
#endif
            free_lbuf(saystring);
        }
        else
        {
            notify_saypose(executor, tprintf("You say, \"%s\"", message));
#ifdef REALITY_LVLS
            notify_except_rlevel(loc, executor, executor, tprintf("%s says, \"%s\"", Moniker(executor), message), MSG_SAYPOSE);
#else
            notify_except(loc, executor, executor, tprintf("%s says, \"%s\"", Moniker(executor), message), MSG_SAYPOSE);
#endif
        }
        break;

    case SAY_POSE:
#ifdef REALITY_LVLS
        notify_except_rlevel(loc, executor, -1, tprintf("%s %s", Moniker(executor), message), MSG_SAYPOSE);
#else
        notify_all_from_inside_saypose(loc, executor, tprintf("%s %s", Moniker(executor), message));
#endif
        break;

    case SAY_POSE_NOSPC:
#ifdef REALITY_LVLS
        notify_except_rlevel(loc, executor, -1, tprintf("%s%s", Moniker(executor), message), MSG_SAYPOSE);
#else
        notify_all_from_inside_saypose(loc, executor, tprintf("%s%s", Moniker(executor), message));
#endif
        break;

    case SAY_EMIT:
        if (  (say_flags & SAY_HERE)
           || (say_flags & SAY_HTML)
           || !say_flags)
        {
            if (say_flags & SAY_HTML)
            {
                notify_all_from_inside_html(loc, executor, message);
            }
            else
            {

#ifdef REALITY_LVLS
                notify_except_rlevel(loc, executor, -1, message, SAY_EMIT);
#else
                notify_all_from_inside(loc, executor, message);
#endif
            }
        }
        if (say_flags & SAY_ROOM)
        {
            if (  isRoom(loc)
               && (say_flags & SAY_HERE))
            {
                if (messageNew)
                {
                    free_lbuf(messageNew);
                }
                return;
            }
            for (depth = 0; !isRoom(loc) && (depth < 20); depth++)
            {
                loc = Location(loc);
                if (  !Good_obj(loc)
                   || (loc == Location(loc)))
                {
                    if (messageNew)
                    {
                        free_lbuf(messageNew);
                    }
                    return;
                }
            }
            if (isRoom(loc))
            {
#ifdef REALITY_LVLS
                notify_except_rlevel(loc, executor, -1, message, -1);
#else
                notify_all_from_inside(loc, executor, message);
#endif
            }
        }
        break;
    }
    if (messageNew)
    {
        free_lbuf(messageNew);
    }
}


void do_shout(dbref executor, dbref caller, dbref enactor, int key,
    char *message)
{
    char *p;
    char *buf2, *bp;
    int say_flags = key & (SAY_NOTAG | SAY_HERE | SAY_ROOM | SAY_HTML);
    key &= ~(SAY_NOTAG | SAY_HERE | SAY_ROOM | SAY_HTML);

    // Parse speechmod if present.
    //
    char *messageNew = modSpeech(executor, message, true, "@wall");
    if (messageNew)
    {
        message = messageNew;
    }

    switch (key)
    {
    case SHOUT_SHOUT:
        switch (*message)
        {
        case ':':
            message[0] = ' ';
            say_shout(0, announce_msg, say_flags, executor, message);
            break;

        case ';':
            message++;
            say_shout(0, announce_msg, say_flags, executor, message);
            break;

        case '"':
            message++;

        default:
            buf2 = alloc_lbuf("do_shout.shout");
            bp = buf2;
            safe_str(" shouts, \"", buf2, &bp);
            safe_str(message, buf2, &bp);
            safe_chr('"', buf2, &bp);
            *bp = '\0';
            say_shout(0, announce_msg, say_flags, executor, buf2);
            free_lbuf(buf2);
        }
        STARTLOG(LOG_SHOUTS, "WIZ", "SHOUT");
        log_name(executor);
        log_text(" shouts: ");
        log_text(message);
        ENDLOG;
        break;

    case SHOUT_WIZSHOUT:
        switch (*message)
        {
        case ':':
            message[0] = ' ';
            say_shout(SHOUT_WIZARD, broadcast_msg, say_flags, executor,
                  message);
            break;
        case ';':
            message++;
            say_shout(SHOUT_WIZARD, broadcast_msg, say_flags, executor,
                  message);
            break;
        case '"':
            message++;
        default:
            buf2 = alloc_lbuf("do_shout.wizshout");
            bp = buf2;
            safe_str(" says, \"", buf2, &bp);
            safe_str(message, buf2, &bp);
            safe_chr('"', buf2, &bp);
            *bp = '\0';
            say_shout(SHOUT_WIZARD, broadcast_msg, say_flags, executor, buf2);
            free_lbuf(buf2);
        }
        STARTLOG(LOG_SHOUTS, "WIZ", "BCAST");
        log_name(executor);
        log_text(" broadcasts: '");
        log_text(message);
        log_text("'");
        ENDLOG;
        break;

    case SHOUT_ADMINSHOUT:
        switch (*message)
        {
        case ':':
            message[0] = ' ';
            say_shout(SHOUT_ADMIN, admin_msg, say_flags, executor,
                  message);
            break;
        case ';':
            message++;
            say_shout(SHOUT_ADMIN, admin_msg, say_flags, executor,
                  message);
            break;
        case '"':
            message++;
        default:
            buf2 = alloc_lbuf("do_shout.adminshout");
            bp = buf2;
            safe_str(" says, \"", buf2, &bp);
            safe_str(message, buf2, &bp);
            safe_chr('"', buf2, &bp);
            *bp = '\0';
            say_shout(SHOUT_ADMIN, admin_msg, say_flags, executor,
                  buf2);
            free_lbuf(buf2);
        }
        STARTLOG(LOG_SHOUTS, "WIZ", "ASHOUT");
        log_name(executor);
        log_text(" yells: ");
        log_text(message);
        ENDLOG;
        break;

    case SHOUT_WALLPOSE:
        if (say_flags & SAY_NOTAG)
        {
            p = tprintf("%s %s", Moniker(executor), message);
        }
        else
        {
            p = tprintf("Announcement: %s %s", Moniker(executor),
                message);
        }
        wall_broadcast(0, executor, p);
        STARTLOG(LOG_SHOUTS, "WIZ", "SHOUT");
        log_name(executor);
        log_text(" WALLposes: ");
        log_text(message);
        ENDLOG;
        break;

    case SHOUT_WIZPOSE:
        if (say_flags & SAY_NOTAG)
        {
            p = tprintf("%s %s", Moniker(executor), message);
        }
        else
        {
            p = tprintf("Broadcast: %s %s", Moniker(executor), message);
        }
        wall_broadcast(SHOUT_WIZARD, executor, p);
        STARTLOG(LOG_SHOUTS, "WIZ", "BCAST");
        log_name(executor);
        log_text(" WIZposes: ");
        log_text(message);
        ENDLOG;
        break;

    case SHOUT_WALLEMIT:
        if (say_flags & SAY_NOTAG)
        {
            p = tprintf("%s", message);
        }
        else
        {
            p = tprintf("Announcement: %s", message);
        }
        wall_broadcast(0, executor, p);
        STARTLOG(LOG_SHOUTS, "WIZ", "SHOUT");
        log_name(executor);
        log_text(" WALLemits: ");
        log_text(message);
        ENDLOG;
        break;

    case SHOUT_WIZEMIT:
        if (say_flags & SAY_NOTAG)
        {
            p = tprintf("%s", message);
        }
        else
        {
            p = tprintf("Broadcast: %s", message);
        }
        wall_broadcast(SHOUT_WIZARD, executor, p);
        STARTLOG(LOG_SHOUTS, "WIZ", "BCAST");
        log_name(executor);
        log_text(" WIZemit: ");
        log_text(message);
        ENDLOG;
        break;
    }
    if (messageNew)
    {
        free_lbuf(messageNew);
    }
}

/* ---------------------------------------------------------------------------
 * do_page: Handle the page command.
 * Page-pose code from shadow@prelude.cc.purdue.
 */

static void page_return(dbref player, dbref target, const char *tag,
    int anum, const char *dflt)
{
    if (MuxAlarm.bAlarmed)
    {
        return;
    }

    dbref aowner;
    int aflags;
    char *str, *str2, *buf, *bp;

    str = atr_pget(target, anum, &aowner, &aflags);
    if (*str)
    {
        str2 = bp = alloc_lbuf("page_return");
        buf = str;
        mux_exec(str2, &bp, target, player, player,
                 EV_FCHECK | EV_EVAL | EV_TOP | EV_NO_LOCATION, &buf,
                 (char **)NULL, 0);
        *bp = '\0';
        if (*str2)
        {
            CLinearTimeAbsolute ltaNow;
            ltaNow.GetLocal();
            FIELDEDTIME ft;
            ltaNow.ReturnFields(&ft);

            char *p = tprintf("%s message from %s: %s", tag,
                Moniker(target), str2);
#ifdef BT_ENABLED
            if (  Wizard(target)
               || !In_IC_Loc(target))
            {
                notify_with_cause_ooc(player, target, p);
            }
#else
            notify_with_cause_ooc(player, target, p);
#endif
            p = tprintf("[%d:%02d] %s message sent to %s.", ft.iHour,
                ft.iMinute, tag, Moniker(player));
            notify_with_cause_ooc(target, player, p);
        }
        free_lbuf(str2);
    }
    else if (dflt && *dflt)
    {
        notify_with_cause_ooc(player, target, dflt);
    }
    free_lbuf(str);
}

static bool page_check(dbref player, dbref target)
{
#ifdef BT_ENABLED
    if (  In_IC_Loc(player)
       && !WizRoy(target)
       && !WizRoy(player))
    {
        notify(player, NOPERM_MESSAGE);
        return false;
    }
#endif
    if (!payfor(player, Guest(player) ? 0 : mudconf.pagecost))
    {
        notify(player, tprintf("You don't have enough %s.", mudconf.many_coins));
    }
    else if (!Connected(target))
    {
        page_return(player, target, "Away", A_AWAY,
            tprintf("Sorry, %s is not connected.", Moniker(target)));
    }
#ifdef BT_ENABLED
    else if (  !could_doit(player, target, A_LPAGE)
            || (  !Wizard(player)
               && In_IC_Loc(target)
               && !Wizard(target)))
#else
    else if (!could_doit(player, target, A_LPAGE))
#endif
    {
        if (  Can_Hide(target)
           && Hidden(target)
           && !See_Hidden(player))
        {
            page_return(player, target, "Away", A_AWAY,
                tprintf("Sorry, %s is not connected.", Moniker(target)));
        }
        else
        {
            page_return(player, target, "Reject", A_REJECT,
                tprintf("Sorry, %s is not accepting pages.", Moniker(target)));
        }
    }
    else if (!could_doit(target, player, A_LPAGE))
    {
        char *p;
        if (Wizard(player))
        {
            p = tprintf("Warning: %s can't return your page.",
                Moniker(target));
            notify(player, p);
            return true;
        }
        else
        {
            p = tprintf("Sorry, %s can't return your page.",
                Moniker(target));
            notify(player, p);
            return false;
        }
    }
    else
    {
        return true;
    }
    return false;
}

// The combinations are:
//
//           nargs  arg1[0]  arg2[0]
//   ''        1      '\0'    '\0'      Report LastPaged to player.
//   'a'       1      'a'     '\0'      Page LastPaged with A
//   'a='      2      'a'     '\0'      Page A. LastPaged <- A
//   '=b'      2      '\0'    'b'       Page LastPaged with B
//   'a=b'     2      'a'     'b'       Page A with B. LastPaged <- A
//   'a=b1=[b2=]*...'                   All treated the same as 'a=b'.
//
void do_page
(
    dbref executor,
    dbref caller,
    dbref enactor,
    int   key,
    int   nargs,
    char *arg1,
    char *arg2
)
{
    int   nPlayers = 0;
    dbref aPlayers[(LBUF_SIZE+1)/2];

    // Either we have been given a recipient list, or we are relying on an
    // existing A_LASTPAGE.
    //
    bool bModified = false;
    if (  nargs == 2
       && arg1[0] != '\0')
    {
        bModified = true;

        char *p = arg1;
        while (*p != '\0')
        {
            char *q = strchr(p, '"');
            if (q)
            {
                *q = '\0';
            }

            // Decode space-delimited or comma-delimited recipients.
            //
            MUX_STRTOK_STATE tts;
            mux_strtok_src(&tts, p);
            mux_strtok_ctl(&tts, ", ");
            char *r;
            for (r = mux_strtok_parse(&tts); r; r = mux_strtok_parse(&tts))
            {
                dbref target = lookup_player(executor, r, true);
                if (target != NOTHING)
                {
                    aPlayers[nPlayers++] = target;
                }
                else
                {
                    notify(executor, tprintf("I don't recognize \"%s\".", r));
                }
            }

            if (q)
            {
                p = q + 1;

                // Handle quoted named.
                //
                q = strchr(p, '"');
                if (q)
                {
                    *q = '\0';
                }

                dbref target = lookup_player(executor, p, true);
                if (target != NOTHING)
                {
                    aPlayers[nPlayers++] = target;
                }
                else
                {
                    notify(executor, tprintf("I don't recognize \"%s\".", p));
                }

                if (q)
                {
                    p = q + 1;
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        // Need to decode the A_LASTPAGE.
        //
        dbref aowner;
        int   aflags;
        char *pLastPage = atr_get(executor, A_LASTPAGE, &aowner, &aflags);

        MUX_STRTOK_STATE tts;
        mux_strtok_src(&tts, pLastPage);
        mux_strtok_ctl(&tts, " ");
        char *p;
        for (p = mux_strtok_parse(&tts); p; p = mux_strtok_parse(&tts))
        {
            dbref target = mux_atol(p);
            if (  Good_obj(target)
               && isPlayer(target))
            {
                aPlayers[nPlayers++] = target;
            }
            else
            {
                notify(executor, tprintf("I don't recognize #%d.", target));
                bModified = true;
            }
        }
        free_lbuf(pLastPage);
    }

    int nValid = nPlayers;

    // Remove duplicate dbrefs.
    //
    int i;
    for (i = 0; i < nPlayers-1; i++)
    {
        if (aPlayers[i] != NOTHING)
        {
            int j;
            for (j = i+1; j < nPlayers; j++)
            {
                if (aPlayers[j] == aPlayers[i])
                {
                    aPlayers[j] = NOTHING;
                    bModified = true;
                    nValid--;
                }
            }
        }
    }

    // If we are doing more than reporting, we have some other dbref
    // validation to do.
    //
    if (  nargs == 2
       || arg1[0] != '\0')
    {
        for (i = 0; i < nPlayers; i++)
        {
            if (  Good_obj(aPlayers[i])
               && !page_check(executor, aPlayers[i]))
            {
                aPlayers[i] = NOTHING;
                bModified = true;
                nValid--;
            }
        }
    }

    if (bModified)
    {
        // Our aPlayers could be different than the one encoded on A_LASTPAGE.
        // Update the database.
        //
        ITL itl;
        char *pBuff = alloc_lbuf("do_page.lastpage");
        char *pBufc = pBuff;
        ItemToList_Init(&itl, pBuff, &pBufc);
        for (i = 0; i < nPlayers; i++)
        {
            if (  Good_obj(aPlayers[i])
               && !ItemToList_AddInteger(&itl, aPlayers[i]))
            {
                break;
            }
        }
        ItemToList_Final(&itl);
        atr_add_raw(executor, A_LASTPAGE, pBuff);
        free_lbuf(pBuff);
    }

    // Verify that the recipient list isn't empty.
    //
    if (nValid == 0)
    {
        if (  nargs == 1
           && arg1[0] == '\0')
        {
            notify(executor, "You have not paged anyone.");
        }
        else
        {
            notify(executor, "No one to page.");
        }
        return;
    }

    // Build a friendly representation of the recipient list.
    //
    char *aFriendly = alloc_lbuf("do_page.friendly");
    char *pFriendly = aFriendly;

    if (nValid > 1)
    {
        safe_chr('(', aFriendly, &pFriendly);
    }
    bool bFirst = true;
    for (i = 0; i < nPlayers; i++)
    {
        if (aPlayers[i] != NOTHING)
        {
            if (bFirst)
            {
                bFirst = false;
            }
            else
            {
                safe_copy_buf(", ", 2, aFriendly, &pFriendly);
            }
            safe_str(Moniker(aPlayers[i]), aFriendly, &pFriendly);
        }
    }
    if (nValid > 1)
    {
        safe_chr(')', aFriendly, &pFriendly);
    }
    *pFriendly = '\0';

    // We may be able to proceed directly to the reporting case.
    //
    if (  nargs == 1
       && arg1[0] == '\0')
    {
        notify(executor, tprintf("You last paged %s.", aFriendly));
        free_lbuf(aFriendly);
        return;
    }

    // Build messages.
    //
    char *omessage = alloc_lbuf("do_page.omessage");
    char *imessage = alloc_lbuf("do_page.imessage");
    char *omp = omessage;
    char *imp = imessage;

    char *pMessage;
    if (nargs == 1)
    {
        // 'page A' form.
        //
        pMessage = arg1;
    }
    else
    {
        // 'page A=', 'page =B', and 'page A=B' forms.
        //
        pMessage = arg2;
    }

    int pageMode;
    switch (*pMessage)
    {
    case '\0':
        pageMode = 1;
        break;

    case ':':
        pageMode = 2;
        pMessage++;
        break;

    case ';':
        pageMode = 3;
        pMessage++;
        break;

    case '"':
        pMessage++;

        // FALL THROUGH

    default:
        pageMode = 0;
    }

    char *newMessage = modSpeech(executor, pMessage, true, "page");
    if (newMessage)
    {
        pMessage = newMessage;
    }

    switch (pageMode)
    {
    case 1:
        // 'page A=' form.
        //
        if (nValid == 1)
        {
            safe_tprintf_str(omessage, &omp, "From afar, %s pages you.",
                Moniker(executor));
        }
        else
        {
            safe_tprintf_str(omessage, &omp, "From afar, %s pages %s.",
                Moniker(executor), aFriendly);
        }
        safe_tprintf_str(imessage, &imp, "You page %s.", aFriendly);
        break;

    case 2:
        safe_str("From afar, ", omessage, &omp);
        if (nValid > 1)
        {
            safe_tprintf_str(omessage, &omp, "to %s: ", aFriendly);
        }
        safe_tprintf_str(omessage, &omp, "%s %s", Moniker(executor), pMessage);
        safe_tprintf_str(imessage, &imp, "Long distance to %s: %s %s",
            aFriendly, Moniker(executor), pMessage);
        break;

    case 3:
        safe_str("From afar, ", omessage, &omp);
        if (nValid > 1)
        {
            safe_tprintf_str(omessage, &omp, "to %s: ", aFriendly);
        }
        safe_tprintf_str(omessage, &omp, "%s%s", Moniker(executor), pMessage);
        safe_tprintf_str(imessage, &imp, "Long distance to %s: %s%s",
            aFriendly, Moniker(executor), pMessage);
        break;

    default:
        if (nValid > 1)
        {
            safe_tprintf_str(omessage, &omp, "To %s, ", aFriendly);
        }
        safe_tprintf_str(omessage, &omp, "%s pages: %s", Moniker(executor),
            pMessage);
        safe_tprintf_str(imessage, &imp, "You paged %s with '%s'",
            aFriendly, pMessage);
        break;
    }
    free_lbuf(aFriendly);

    // Send message to recipients.
    //
    for (i = 0; i < nPlayers; i++)
    {
        dbref target = aPlayers[i];
        if (target != NOTHING)
        {
            notify_with_cause_ooc(target, executor, omessage);
            if (fetch_idle(target) >= idle_timeout_val(target))
            {
                page_return(executor, target, "Idle", A_IDLE, NULL);
            }
        }
    }
    free_lbuf(omessage);

    // Send message to sender.
    //
    notify(executor, imessage);
    free_lbuf(imessage);
    if (newMessage)
    {
        free_lbuf(newMessage);
    }
}

/* ---------------------------------------------------------------------------
 * do_pemit: Messages to specific players, or to all but specific players.
 */

void whisper_pose(dbref player, dbref target, char *message, bool bSpace)
{
    char *newMessage = modSpeech(player, message, true, "whisper");
    if (newMessage)
    {
        message = newMessage;
    }
    char *buff = alloc_lbuf("do_pemit.whisper.pose");
    strcpy(buff, Moniker(player));
    notify(player, tprintf("%s senses \"%s%s%s\"", Moniker(target), buff,
        bSpace ? " " : "", message));
    notify_with_cause(target, player, tprintf("You sense %s%s%s", buff,
        bSpace ? " " : "", message));
    free_lbuf(buff);
    if (newMessage)
    {
        free_lbuf(newMessage);
    }
}

void do_pemit_single
(
    dbref player,
    int key,
    bool bDoContents,
    int pemit_flags,
    char *recipient,
    int chPoseType,
    char *message
)
{
    dbref target, loc;
    char *buf2, *bp;
    int depth;
    bool ok_to_do = false;

    switch (key)
    {
    case PEMIT_FSAY:
    case PEMIT_FPOSE:
    case PEMIT_FPOSE_NS:
    case PEMIT_FEMIT:
        target = match_controlled(player, recipient);
        if (target == NOTHING)
        {
            return;
        }
        ok_to_do = true;
        break;

    default:
        init_match(player, recipient, TYPE_PLAYER);
        match_everything(0);
        target = match_result();
    }

    char *newMessage = NULL;
    char *saystring = NULL;

    char *p;
    switch (target)
    {
    case NOTHING:
        switch (key)
        {
        case PEMIT_WHISPER:
            notify(player, "Whisper to whom?");
            break;

        case PEMIT_PEMIT:
            notify(player, "Emit to whom?");
            break;

        case PEMIT_OEMIT:
            notify(player, "Emit except to whom?");
            break;

        default:
            notify(player, "Sorry.");
            break;
        }
        break;

    case AMBIGUOUS:
        notify(player, "I don't know who you mean!");
        break;

    default:

        // Enforce locality constraints.
        //
        if (  !ok_to_do
           && (  nearby(player, target)
              || Long_Fingers(player)
              || Controls(player, target)))
        {
            ok_to_do = true;
        }
        if (  !ok_to_do
           && key == PEMIT_PEMIT
           && isPlayer(target)
           && mudconf.pemit_players)
        {
            if (!page_check(player, target))
            {
                return;
            }
            ok_to_do = true;
        }
        if (  !ok_to_do
           && (  !mudconf.pemit_any
              || key != PEMIT_PEMIT))
        {
            notify(player, "You are too far away to do that.");
            return;
        }
        if (  bDoContents
           && !Controls(player, target)
           && !mudconf.pemit_any)
        {
            notify(player, NOPERM_MESSAGE);
            return;
        }
        loc = where_is(target);

        switch (key)
        {
        case PEMIT_PEMIT:
            if (bDoContents)
            {
                if (Has_contents(target))
                {
                    notify_all_from_inside(target, player, message);
                }
            }
            else
            {
                if (pemit_flags & PEMIT_HTML)
                {
                    notify_with_cause_html(target, player, message);
                }
                else
                {
                    notify_with_cause(target, player, message);
                }
            }
            break;

        case PEMIT_OEMIT:
            notify_except(Location(target), player, target, message, 0);
            break;

        case PEMIT_WHISPER:
            if (  isPlayer(target)
               && !Connected(target))
            {
                page_return(player, target, "Away", A_AWAY,
                    tprintf("Sorry, %s is not connected.", Moniker(target)));
                return;
            }
            switch (chPoseType)
            {
            case ':':
                message++;
                whisper_pose(player, target, message, true);
                break;

            case ';':
                message++;
                whisper_pose(player, target, message, false);
                break;

            case '"':
                message++;

            default:
                newMessage = modSpeech(player, message, true, "whisper");
                if (newMessage)
                {
                    message = newMessage;
                }
                notify(player, tprintf("You whisper \"%s\" to %s.", message,
                    Moniker(target)));
                notify_with_cause(target, player,
                    tprintf("%s whispers \"%s\"", Moniker(player), message));
                if (newMessage)
                {
                    free_lbuf(newMessage);
                }
            }
            if (  !mudconf.quiet_whisper
               && !Wizard(player))
            {
                loc = where_is(player);
                if (loc != NOTHING)
                {
                    buf2 = alloc_lbuf("do_pemit.whisper.buzz");
                    bp = buf2;
                    safe_str(Moniker(player), buf2, &bp);
                    safe_str(" whispers something to ", buf2, &bp);
                    safe_str(Moniker(target), buf2, &bp);
                    *bp = '\0';
                    notify_except2(loc, player, player, target, buf2);
                    free_lbuf(buf2);
                }
            }
            break;

        case PEMIT_FSAY:
            newMessage = modSpeech(target, message, true, "@fsay");
            if (newMessage)
            {
                message = newMessage;
            }
            notify(target, tprintf("You say, \"%s\"", message));
            if (loc != NOTHING)
            {
                saystring = modSpeech(target, message, false, "@fsay");
                if (saystring)
                {
                    p = tprintf("%s %s \"%s\"", Moniker(target),
                        saystring, message);
                    notify_except(loc, player, target, p, 0);
                }
                else
                {
                    p = tprintf("%s says, \"%s\"", Moniker(target),
                        message);
                    notify_except(loc, player, target, p, 0);
                }
            }
            if (saystring)
            {
                free_lbuf(saystring);
            }
            if (newMessage)
            {
                free_lbuf(newMessage);
            }
            break;

        case PEMIT_FPOSE:
            newMessage = modSpeech(target, message, true, "@fpose");
            if (newMessage)
            {
                message = newMessage;
            }
            p = tprintf("%s %s", Moniker(target), message);
            notify_all_from_inside(loc, player, p);
            if (newMessage)
            {
                free_lbuf(newMessage);
            }
            break;

        case PEMIT_FPOSE_NS:
            newMessage = modSpeech(target, message, true, "@fpose");
            if (newMessage)
            {
                message = newMessage;
            }
            p = tprintf("%s%s", Moniker(target), message);
            notify_all_from_inside(loc, player, p);
            if (newMessage)
            {
                free_lbuf(newMessage);
            }
            break;

        case PEMIT_FEMIT:
            if (  (pemit_flags & PEMIT_HERE)
               || !pemit_flags)
            {
                notify_all_from_inside(loc, player, message);
            }
            if (pemit_flags & PEMIT_ROOM)
            {
                if (  isRoom(loc)
                   && (pemit_flags & PEMIT_HERE))
                {
                    return;
                }
                depth = 0;
                while (  !isRoom(loc)
                      && depth++ < 20)
                {
                    loc = Location(loc);
                    if (  loc == NOTHING
                       || loc == Location(loc))
                    {
                        return;
                    }
                }
                if (isRoom(loc))
                {
                    notify_all_from_inside(loc, player, message);
                }
            }
            break;
        }
    }
}

void do_pemit_list
(
    dbref player,
    int key,
    bool bDoContents,
    int pemit_flags,
    char *list,
    int chPoseType,
    char *message
)
{
    // Send a message to a list of dbrefs. The list is destructively
    // modified.
    //
    if (  message[0] == '\0'
       || list[0] == '\0')
    {
        return;
    }

    char *p;
    MUX_STRTOK_STATE tts;
    mux_strtok_src(&tts, list);
    mux_strtok_ctl(&tts, " ");
    for (p = mux_strtok_parse(&tts); p; p = mux_strtok_parse(&tts))
    {
        do_pemit_single(player, key, bDoContents, pemit_flags, p, chPoseType,
            message);
    }
}

void do_pemit
(
    dbref executor,
    dbref caller,
    dbref enactor,
    int   key,
    int   nargs,
    char *recipient,
    char *message
)
{
    if (nargs < 2)
    {
        return;
    }

    // Decode PEMIT_CONENTS and PEMIT_LIST and remove from key.
    //
    bool bDoContents = false;
    if (key & PEMIT_CONTENTS)
    {
        bDoContents = true;
    }
    bool bDoList = false;
    if (key & PEMIT_LIST)
    {
        bDoList = true;
    }
    key &= ~(PEMIT_CONTENTS | PEMIT_LIST);


    // Decode PEMIT_HERE, PEMIT_ROOM, PEMIT_HTML and remove from key.
    //
    int mask = PEMIT_HERE | PEMIT_ROOM | PEMIT_HTML;
    int pemit_flags = key & mask;
    key &= ~mask;

    int chPoseType = *message;
    if (key == PEMIT_WHISPER && chPoseType == ':')
    {
        message[0] = ' ';
    }

    if (bDoList)
    {
        do_pemit_list(executor, key, bDoContents, pemit_flags, recipient,
            chPoseType, message);
    }
    else
    {
        do_pemit_single(executor, key, bDoContents, pemit_flags, recipient,
            chPoseType, message);
    }
}
