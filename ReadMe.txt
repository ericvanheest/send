Send - Sends keystrokes and windows messages to applications

======================================================================

Usage:  send [options] "application" "keys" [time]

"application" is a case-insensitive string that matches part of a window
caption.  For example, if you are running notepad, its title bar will be
something similar to "MyFile.txt - Notepad".  You could have send.exe type
the text "Hello" by using the following command:

    send notepad Hello

Spaces in the key string need to be quoted, as in:

    send notepad "Hello everyone!"
    
[time] is a string representation of a time at which you want the keys to
be sent to the application.  The application window need not exist until
that time.  For example, to send the F6 key to VirtualDub at 4:55 PM, use
the following syntax:

    send virtualdub \{F6} 4:55pm

The time string accepts several different time formats (it uses the Windows
function VarDateFromStr), including dates.  Dates with spaces must be
quoted, as in:

    send virtualdub \{F6} "April 1, 2001 10:55am"

Options:

    -a  Sends the commands to every window matching the expression, not
        just the first one.
        
    -b  Does not bring the specified window to the foreground.  Keystrokes
        will still be sent to whatever window is in the foreground, but
        you may send windows messages and position/size commands to a
        background window (useful for moving a window without bringing it
        to the foreground first).  Using this option implies -d.
    
    -c  Uses a case-sensitive search, so that "notepad" will not match
        "Notepad"

    -d  Disables switching back to the process that launched send.exe
        after sending the input string.  (Windows may not always
        permit the sending console to re-acquire focus either way.)

    -D  Introduces a delay before and after every virtual mouse or keyboard
        event.  This can help with applications that are polling they key
        state (e.g. DirectInput) and may otherwise miss the otherwise
        nearly-instantaneous sequential SendInput calls.

    -e  Searches every window on the system for the caption specified, even
        child windows.

    -f  Sends the specified string to the window repeatedly.  This is most
        useful when the keystrokes you are sending will cause the window to
        close, and you want to send the same keystrokes to the window again
        when it reappears (with the -w options).

    -F  Sends the keystring only if the specified app is currently in the
        foreground.  This can prevent the sending of keys and commands to
        other unintended windows when the desired app is minimized/etc.
        Implies -b.
        
    -g  Sends the specified string to whichever window happens to be in
        the foreground.  Useful for arbitrary mouse movements that do not
        need a particular window to be focused.  If you specify the "-g"
        and a window caption, it is ignored.
    
    -h  Includes hidden windows in the search.  Normally, a nonvisible
        window is not a desired target for input operations.

    -i  The window message is sent to the child window specified by
        the given dialog ID.
        
    -I  Outputs the dialog item ID found by the sequence provided.  This can
        be useful in conjunction with a program such as Spy++ to determine
        whether the correct target is being located.
        
    -j  Interpret "app" as a window handle instead of a caption.  Use
        0x as a prefix if it is hexadecimal.

    -J  Read a window handle from STDIN that can be used as \1 in the command
        line.  Useful for passing the result of -o to a new instance.

    -k  Sends commands only if the set of keys in the "waitkeys" string
        are all pressed.

    -K  Sends commands only if the set of keys in the "waitkeys" string
        are all released.

    -l  Lists all of the window captions that "send" finds before
        matching the search string.  Useful for discoving the precise
        titles of windows.

    -m  Waits for the specified number of milliseconds between moving the
        mouse cursor and clicking the button, if appropriate (see the
        "Mouse Commands" section).

    -M  Sends keyboard and mouse input via windows messages (i.e.
        WM_KEYDOWN, WM_MOUSEMOVE, etc.) instead of using SendInput calls.
        Note that windows messages must typically be directed to the exact
        child window desired (e.g. a textbox) rather than the main window.

    -n  Gives the found window a new caption.  This can be useful when you
        want to send an input string that will take some time to process,
        and then use another "send" command to wait for the new window
        caption to exist.
        
    -o  Find the first window specified by "app" and print its window handle
        without doing any further processing.  This is useful in a situation
        where finding a child window takes some time and you want to send
        it several commands later without re-finding it.

    -O  Similar to -o but prints a report of all windows that match the
        window specified by \"app\"

    -p  Sends the actual message to the parent of the indicated window
        (using -i) instead of the window itself.  When using messages that
        include notifications such as BN_CLICKED, this is typically useful.
        
    -P  Uses a given number of milliseconds as the time between window
        polling (notably when using the -K/-k or -f options.  Polling more
        frequently uses more resources but will quicken the response time
        when something send is using changes (for example, the caption of
        the window).  The default is 10 ms.

    -q  Suppresses stdout messages.

    -r  Uses a regular expression for the search string.  For example,
        "^notepad$" would match only a window titled exactly "Notepad" and
        not one name "Untitled - Notepad".  A regular expression of "."
        will match all windows that have a title of at least one character.

    -R  Repeats the entire sequence of keys the number of times specified
        after the -R option.

    -s  The window must also match the class name provided (with regular
        expression matching if -r is specified).

    -S  This option will attempt to use a common scan code for virtual keys
        which have a common physical location on a US 102-key keyboard.  This
        is necessary for some low-level applications (such as games).

    -t  Times out after the given number of ms when used in conjunction with
        the -w/-W/-x/-X/-f options

    -u  Sends commands only if the set of keys in the "waitkeys" string
        are all pressed and then released.

    -v  Prints some extra information about window messages.

    -w  Waits for the window to exist before sending the keystrokes.
    
    -W  Similar to -w, but waits for the window to be enabled.

    -x  Waits for the window to no longer exist before sending the keystrokes.

    -X  Similar to -x, but waits for the window to be disabled.

======================================================================

If you want to send a special key, such as F6 or Escape, use the following
tables (examples are for notepad or other such edit controls):

----------------------------------------------------------------------

Keyboard Modifiers:
    \a      Press the ALT key
    \A      Release the ALT key
    \s      Press the SHIFT key
    \S      Release the SHIFT key
    \c      Press the CONTROL key
    \C      Release the CONTROL key
    \p      Press the APPLICATIONS key
    \P      Release the APPLICATIONS key
    \w      Press the WINDOWS key
    \W      Release the WINDOWS key

For example, "\shello\S" would send "shift, h, e, l, l, o, unshift" and
create the word "HELLO"

----------------------------------------------------------------------

Arrow Keys:
    \u      UP arrow       
    \d      DOWN arrow       
    \l      LEFT arrow       
    \r      RIGHT arrow       

For example, "hllo\l\l\le" would write "hello" by writing "hllo,"
moving left three characters, and inserting the "e" last.

----------------------------------------------------------------------

Special Keys:
    \b      Backspace
    \e      Escape
    \t      Tab
    \n      Enter/Return

For example, "Username\tPassword\n" would type a username, push tab,
type a password, and hit enter.

----------------------------------------------------------------------

Named Keys:

    \{F1}       F1 key
    \{F2}       F2 key
    ...
    \{F24}      F24 key
    \{Delete}   Delete
    \{End}      End
    \{Home}     Home
    \{Insert}   Insert
    \{PgUp}     PageUp
    \{PgDn}     PageDown
    \{PrSc}     PrintScreen
    \{Pause}    Pause
    \{ScLk}     ScrollLock

Keys may be abbreviated when unambiguous; delete = "\{d}" etc.
For example, "\{pd}\{pd}\{e}" would send PageDown twice, then the "End" key.

The value \{Clipboard} will be replaced with the current (text) contents
of the Windows clipboard.

----------------------------------------------------------------------

Numeric Keypad:

    \{n0}       Keypad Zero
    \{n1}       Keypad One
    ...
    \{n9}       Keypad Nine
    \{n*}       Keypad *
    \{n+}       Keypad +
    \{n-}       Keypad -
    \{n/}       Keypad /
    \{n.}       Keypad .

----------------------------------------------------------------------

Mouse Commands:

Mouse commands may be given as the special key "m" followed by button index,
event, coordinates, and modifiers.  For example, to click the left
mouse button at client-local coordinates (200,100), use this syntax:

    \{m,l,click,200,100,local}

This could also be abbreviated to "\{m,l,c,200,100,l}"

"click" can be replaced with "down" or "up" if desired.  If the specified
button index is "wheel" (or simply "w"), then up/down means to roll the wheel
up or down, and "click" has no meaning (the wheel as a button is usually
the middle button).  To move the mouse without pressing a button,
specify "none" (or "n") as the action.  The buttons may be any of the
predefined buttons "l," "m," "r," "1," "2" (left, middle, right, extra 1,
extra 2).

if the "click" or "c" is capitalized ("Click" or "C") then the mouse cursor
will be repositioned at the point where it was before executing the click.

The last option can be "local" or "screen" ("l" or "s"), and makes the given
coordinates either local to the client window, or global (to the desktop).

Some more examples:

    Press and hold the right mouse button at screen coordinates (100,50):

       \{m,r,d,100,50,s}

    Scroll the mouse wheel up at local coordinates (0,0):

       \{m,w,u,0,0,l}

    Move the mouse to screen coordinates (400,300):

       \{m,n,n,400,300,s}
       
Relative movement:
       
    Move the mouse 100 pixels to the right of its current position:
    
       \{m,n,n,100r,0r,s}

    Move the mouse 100 pixels up and click the left button:
    
       \{m,l,c,0r,-100r,s}

By default, the mouse cursor is moved to the selected location and clicked
as a single action.  This can cause programs that rely on polling rather
than windows messages to fail to notice the click.  Use the -m option to
specify a number of milliseconds to wait between the move and click events
for all mouse commands in this instance of send.

If you want a delay between the pressing of the mouse button and the
releasing of it, add that delay at the end.  For example, to pause 50 ms
before releasing the mouse button:

    \{m,l,c,640,480,s,50}

You may use some simple math when positioning the cursor, if you like.
For example, to move the mouse to the middle of the screen:

    \{m,n,n,w*.5,h*.5,s}

To place the mouse at the position it was before executing any commands,
use 'o1' (horizontal) or 'o2' (vertical) as the value, for example:

    \{m,n,n,o1,o2,s}

----------------------------------------------------------------------

Repetition:

If you want to repeat a particular command a number of times, use
an R as the first item in the sequence.  For example:

    Move the mouse up and left 2 pixels, 10 times:

    \{R10,m,n,n,-2r,-2r,s}

    Send a WM_CHAR message 50 times:

    \[R50,WM_CHAR,0x71,0x00100001]

    Move the mouse right 1 pixel, 100 times, pausing 5 ms each time

    \{R100:5,m,n,n,1r,0r,s}

----------------------------------------------------------------------

Window Positioning:

The active window's size and position may be changed as shown in the
following examples:

    Move the active window to absolute position (200,100) and resize
    it to 640x480:
    
       \{wp,200,100}\{ws,640,480}
       
    Move the active window four pixels to the right and two pixels up
    
       \{wp,4r,-2r}
       
    Move the active window to absolute position (1000,1100) using the
    bottom-right corner of the window as the anchor.
    
       \{wp,1000x,1100x}
       
    Resize the current window to 320x240
    
       \{ws,320,240}
       
    Expand the current window by 100 pixels horizontally
    
       \{ws,100r,0}
       
Note that a zero window size is simply ignored.  A zero position,
however, indicates the top or left side of the screen.  You may also
place a position and size command in the same string, if you like
(this causes only one MoveWindow call to be made, which may or may
not be important).

    \{wp,200,100,640,480}
    
You may use the letters 'h' and 'w' in the numeric portion of the call
to represent the current height and width, respectively, of the Windows
desktop.  The following call would move the window to position (1500,800)
if your desktop resolution is 1600x1200:

    \{wp,w-100,h-400}
    
If you use the capital versions of 'H' or 'W' then the value used will be
the "work area" dimensions (those of the screen minus the task bar or
other toolbars.

If you use "ww" or "hh" then the current window width or height will be used.

Send uses a very simple mathematical parser that understands only these
operators:  + (add), - (subtract), * (multiply), / (divide), % (mod)
Proper order-of-operations is not followed; 1+2*3 is 9, not 7.  Use
parenthesis to force ordering if desired, e.g. 1+(2*3)

If you add a digit after a "w" or "h" then that index will be used to
determine which monitor's width or height is retrieved.  For example, to
move a window to the lower-right corner of monitor 2:

    \{wp,w2x,h2x}

Note that the "work area" does not apply to monitors other than "1"
    
----------------------------------------------------------------------

Windows Messages:

You may send arbitrary windows messages to the active window during
playback.  the wParam and lParam may be omitted if '0' is desired for their
values.  For example:

    send notepad "\[WM_SYSCOMMAND,SC_MINIMIZE]"

will minimize the notepad window.  Note that this will take focus away
from this window and further keystrokes may not be recognized. 
Further windows messages, however, will be sent.  Windows message
sending is generally for advanced users, but a few common ones are
listed here for convenience:

    \[WM_CLOSE]                     Will close the active window
    \[WM_SYSCOMMAND,SC_MINIMIZE]    Minimizes the active window
    \[WM_SYSCOMMAND,SC_MAXIMIZE]    Maximizes the active window
    \[WM_SYSCOMMAND,SC_RESTORE]     "Un-minimizes" the active window

You may specify decimal or hexadecimal numbers (with the "0x" prefix), such
as in the following example:

    \[WM_COMMAND,1,0x4007]
    
String pointers may be specified using single quotes.  For example, this will
directly set the text in the first child window of notepad (the edit control)
without using keystrokes.  Using a windows message like this allows you to
perform window operations without bringing the window to the foreground.
To insert single quotes, escape them with a backslash:

    send -i c notepad "\[WM_SETTEXT,0,'Insert this \'text\' here']"

If you wish to post the message to the window's message queue instead of
using SendMessage, include ",p" at the end of the command.  For example:

    \[WM_COMMAND,2,0,p]

This will post the WM_COMMAND message without waiting for a response.  You
must specify the LPARAM and WPARAM values if you are specifying 'p' to
use PostMessage.
    
----------------------------------------------------------------------

Pausing during sending of keystrokes:

You may have the program pause for a user-specified number of milliseconds
during playback of keystrokes by including \{sX} in the send string, where
X is an integer.  For example:

    send notepad "A\{s500}B\{s1000}C"

would send the "A" key, wait half a second, send the "B" key, wait 1
second, and then send the "C" key.

----------------------------------------------------------------------

Sending commands only if certain keys are pressed or released:

Using the -k (pressed) or -K (released) option will send the provided
command string only if the list of keys given are all down or all up.
This does not wait for the keys to be pressed or released; if the
contition is not met the commands are simply not sent and the program
will exit (unless -f is used).

The "waitkeys" string can be any ASCII characters on the keyboard or
any of the following predefined strings:
         \c, \s, \a, \w   (control, shift, alt, windows)
         \t, \n, \e  (tab, enter, escape)
         \l, \r, \u, \d  (arrow keys - left, right, up, down)
         \F#  Function key #, where # is a hex digit, 1-c
         \P#  Numeric keypad key (0-9 . / * - +)
         \v## Predefined virtual key, where ## is a hex value, 00-ff
         
For example:

    send -dfg -k "\c\Fc" "null" "\{m,l,c,0r,0r}"
    
This will repeatedly click the mouse at its current location while
Control+F12 is pressed.

----------------------------------------------------------------------

Sending messages to dialog IDs:

If you want to send a windows message (or window positioning command)
to a child window, you can specify a dialog ID or chain of IDs in the
following manner:

    send -i id,id,id,... <windowname>

Where "id" can either be a numeric dialog ID (decimal by default, or prefix
with 0x to use hex), a string surrounded by single-quotes (which will match
the window text of a control such as a button or checkbox), or one of these
characters:

    p   previous window
    n   next window
    f   first window
    l   last window
    c   first child window
    o   owner window

For example, to move the "Save As..." button in the Display Properties
window to the upper left corner of its parent client area, you could use
this command:

    send -i c,c,n,n,n "Display Properties" \{wp,0,0}

You can specify a dialog item's control ID or window handle with the special
text 'id' and 'hwnd'.  Also, the delimiter ":" can be used to represent a
hiword/loword split.

When sending notifications such as BN_CLICKED, you will most likely want to
use the -p option to send the actual notification to the parent of the
control, rather than the control itself.  For example:

    send -pi c,n,n,n,n,n,n "Run" "\[WM_COMMAND,BN_CLICKED:id,hwnd]"

With a bit of effort, you can reposition virtually any controls on any
window, modify text, etc.

<end of help text>
======================================================================


"Send" is copyright 2001-2021 by Eric VanHeest (edv_ws@vanheest.org). 
Feel free to modify this program to suit your needs.

