/*
 * tkMacOSXEvent.h --
 *
 *  Declarations of Macintosh specific functions for implementing the
 *      Mac OS X Notifier.
 *
 *      Copyright 2001, Apple Computer, Inc.
 *
 *      The following terms apply to all files originating from Apple
 *      Computer, Inc. ("Apple") and associated with the software
 *      unless explicitly disclaimed in individual files.
 *
 *
 *      Apple hereby grants permission to use, copy, modify,
 *      distribute, and license this software and its documentation
 *      for any purpose, provided that existing copyright notices are
 *      retained in all copies and that this notice is included
 *      verbatim in any distributions. No written agreement, license,
 *      or royalty fee is required for any of the authorized
 *      uses. Modifications to this software may be copyrighted by
 *      their authors and need not follow the licensing terms
 *      described here, provided that the new terms are clearly
 *      indicated on the first page of each file where they apply.
 *
 *
 *      IN NO EVENT SHALL APPLE, THE AUTHORS OR DISTRIBUTORS OF THE
 *      SOFTWARE BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL,
 *      INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF
 *      THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
 *      EVEN IF APPLE OR THE AUTHORS HAVE BEEN ADVISED OF THE
 *      POSSIBILITY OF SUCH DAMAGE.  APPLE, THE AUTHORS AND
 *      DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
 *      BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 *      FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS
 *      SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, AND APPLE,THE
 *      AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
 *      MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *      GOVERNMENT USE: If you are acquiring this software on behalf
 *      of the U.S. government, the Government shall have only
 *      "Restricted Rights" in the software and related documentation
 *      as defined in the Federal Acquisition Regulations (FARs) in
 *      Clause 52.227.19 (c) (2).  If you are acquiring the software
 *      on behalf of the Department of Defense, the software shall be
 *      classified as "Commercial Computer Software" and the
 *      Government shall have only "Restricted Rights" as defined in
 *      Clause 252.227-7013 (c) (1) of DFARs.  Notwithstanding the
 *      foregoing, the authors grant the U.S. Government and others
 *      acting in its behalf permission to use and distribute the
 *      software in accordance with the terms specified in this
 *      license.
 *
 * RCS: @(#) Id
 */

#ifndef _TKMACEVENT
#define _TKMACEVENT

#ifndef _TK
#include "tk.h"
#endif

typedef struct {
    int stopProcessing;
    int err;
    char errMsg[1024];
} MacEventStatus;

/*
 * The event information in passed in the following structures
 */
typedef struct {
    EventRef   eventRef;
    UInt32     eClass;  /* Defines the class of event : see CarbonEvents.h */
    UInt32     eKind;   /* Defines the kind of the event : see CarbonEvents.h */
    Tcl_Interp *interp; /* Interp to handle events in */
} TkMacOSXEvent;

OSStatus TkMacOSXReceiveAndProcessEvent();
void TkMacOSXFlushWindows(); 
int TkMacOSXProcessEvent(TkMacOSXEvent *eventPtr, 
        MacEventStatus *statusPtr);
int TkMacOSXProcessMouseEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXProcessWindowEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXProcessKeyboardEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXProcessApplicationEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXProcessMenuEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXProcessCommandEvent(TkMacOSXEvent *e,
        MacEventStatus *statusPtr);
int TkMacOSXKeycodeToUnicode(
        UniChar * uniChars, int maxChars,
        EventKind eKind,
        UInt32 keycode, UInt32 modifiers,
        UInt32 * deadKeyStatePtr);
OSStatus TkMacOSXStartTclEventLoopCarbonTimer();
OSStatus TkMacOSXStopTclEventLoopCarbonTimer();

#if !defined(MAC_OS_X_VERSION_10_3) || \
        (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_3)
/* Define constants only available on Mac OS X 10.3 or later */
enum { kEventAppAvailableWindowBoundsChanged = 110 };
#endif

#endif
