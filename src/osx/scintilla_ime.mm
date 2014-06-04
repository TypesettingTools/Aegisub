// Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//
// Aegisub Project http://www.aegisub.org/

#import <objc/runtime.h>
#import <wx/osx/private.h>
#import <wx/stc/stc.h>

// from src/osx/cocoa/window.mm
@interface wxNSView : NSView <NSTextInputClient> {
    BOOL _hasToolTip;
    NSTrackingRectTag _lastToolTipTrackTag;
    id _lastToolTipOwner;
    void *_lastUserData;
}
@end

@interface IMEState : NSObject
@property (nonatomic) NSRange markedRange;
@property (nonatomic) bool undoActive;
@end

@implementation IMEState
- (id)init {
    self = [super init];
    self.markedRange = NSMakeRange(NSNotFound, 0);
    self.undoActive = false;
    return self;
}
@end

@interface ScintillaNSView : wxNSView <NSTextInputClient>
@property (nonatomic, readonly) wxStyledTextCtrl *stc;
@property (nonatomic, readonly) IMEState *state;
@end

@implementation ScintillaNSView
- (Class)superclass {
    return [wxNSView superclass];
}

- (wxStyledTextCtrl *)stc {
    return static_cast<wxStyledTextCtrl *>(wxWidgetImpl::FindFromWXWidget(self)->GetWXPeer());
}

- (IMEState *)state {
    return objc_getAssociatedObject(self, [IMEState class]);
}

- (void)invalidate {
    self.state.markedRange = NSMakeRange(NSNotFound, 0);
    [self.inputContext discardMarkedText];
}

#pragma mark - NSTextInputClient

- (NSAttributedString *)attributedSubstringForProposedRange:(NSRange)aRange
                                                actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point {
    return self.stc->PositionFromPoint(wxPoint(point.x, point.y));
}

- (BOOL)drawsVerticallyForCharacterAtIndex:(NSUInteger)charIndex {
    return NO;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
{
    auto stc = self.stc;
    int line = stc->LineFromPosition(range.location);
    int height = stc->TextHeight(line);
    auto pt = stc->PointFromPosition(range.location);

    int width = 0;
    if (range.length > 0) {
        // If the end of the range is on the next line, the range should be
        // truncated to the current line and actualRange should be set to the
        // truncated range
        int end_line = stc->LineFromPosition(range.location + range.length);
        if (end_line > line) {
            range.length = stc->PositionFromLine(line + 1) - 1 - range.location;
            *actualRange = range;
        }

        auto end_pt = stc->PointFromPosition(range.location + range.length);
        width = end_pt.x - pt.x;
    }

    auto rect = NSMakeRect(pt.x, pt.y, width, height);
    rect = [self convertRect:rect toView:nil];
    return [self.window convertRectToScreen:rect];
}

- (BOOL)hasMarkedText {
    return self.state.markedRange.length > 0;
}

- (void)insertText:(id)str replacementRange:(NSRange)replacementRange {
    [self unmarkText];
    [super insertText:str replacementRange:replacementRange];
}

- (NSRange)markedRange {
    return self.state.markedRange;
}

- (NSRange)selectedRange {
    long from = 0, to = 0;
    self.stc->GetSelection(&from, &to);
    return NSMakeRange(from, to - from);
}

- (void)setMarkedText:(id)str
        selectedRange:(NSRange)range
     replacementRange:(NSRange)replacementRange
{
    if ([str isKindOfClass:[NSAttributedString class]])
        str = [str string];

    auto stc = self.stc;
    auto state = self.state;

    int pos = stc->GetInsertionPoint();
    if (state.markedRange.length > 0) {
        pos = state.markedRange.location;
        stc->DeleteRange(pos, state.markedRange.length);
        stc->SetSelection(pos, pos);
    } else {
        state.undoActive = stc->GetUndoCollection();
        if (state.undoActive)
            stc->SetUndoCollection(false);
    }

    auto utf8 = [str UTF8String];
    auto utf8len = strlen(utf8);
    stc->AddTextRaw(utf8, utf8len);

    state.markedRange = NSMakeRange(pos, utf8len);

    stc->SetIndicatorCurrent(1);
    stc->IndicatorFillRange(pos, utf8len);

    // Re-enable undo if we got a zero-length string as that means we're done
    if (!utf8len && state.undoActive)
        stc->SetUndoCollection(true);
    else {
        int start = pos;
        // Range is in utf-16 code units
        if (range.location > 0)
            start += [[str substringToIndex:range.location] lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        int length = [[str substringWithRange:range] lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
        stc->SetSelection(start, start + length);
    }
}

- (void)unmarkText {
    auto state = self.state;
    if (state.markedRange.length > 0) {
        self.stc->DeleteRange(state.markedRange.location, state.markedRange.length);
        state.markedRange = NSMakeRange(NSNotFound, 0);
        if (state.undoActive)
            self.stc->SetUndoCollection(true);
    }
}

- (NSArray *)validAttributesForMarkedText {
    return @[];
}
@end

namespace osx { namespace ime {
void inject(wxStyledTextCtrl *ctrl) {
    id view = (id)ctrl->GetHandle();
    object_setClass(view, [ScintillaNSView class]);

    auto state = [IMEState new];
    objc_setAssociatedObject(view, [IMEState class], state,
                             OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    [state release];
}

void invalidate(wxStyledTextCtrl *ctrl) {
    [(ScintillaNSView *)ctrl->GetHandle() invalidate];
}

bool process_key_event(wxStyledTextCtrl *ctrl, wxKeyEvent &evt) {
    if (evt.GetModifiers() != 0) return false;
    if (evt.GetKeyCode() != WXK_RETURN && evt.GetKeyCode() != WXK_TAB) return false;
    if (![(ScintillaNSView *)ctrl->GetHandle() hasMarkedText]) return false;

    evt.Skip();
    return true;
}

} }
