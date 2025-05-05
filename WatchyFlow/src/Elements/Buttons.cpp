#include "Buttons.h"

LayoutButtonLabels::LayoutButtonLabels(Watchy *watchy, String back,
                                       String select, String up, String down,
                                       const GFXfont *font, uint16_t color,
                                       bool verticalLabels,
                                       const LayoutElement &child) {
  if (verticalLabels) {
    layout_ =
        LayoutColumns(
            {
                LayoutEntry(LayoutRows({
                    LayoutEntry(LayoutSpacer(5)),
                    LayoutEntry(LayoutCenter(LayoutRotate(
                        LayoutText(watchy->buttonConfig() ==
                                           BUTTONS_SELECT_BACK_RIGHT
                                       ? up
                                       : back,
                                   font, color),
                        1))),
                    LayoutEntry(LayoutFill(), true),
                    LayoutEntry(LayoutCenter(LayoutRotate(
                        LayoutText(watchy->buttonConfig() ==
                                           BUTTONS_SELECT_BACK_RIGHT
                                       ? down
                                       : select,
                                   font, color),
                        1))),
                })),
                LayoutEntry(LayoutSpacer(5)),
                LayoutEntry(
                    LayoutBorder(child, false, true, false, true, color), true),
                LayoutEntry(LayoutSpacer(5)),
                LayoutEntry(LayoutRows({
                    LayoutEntry(LayoutSpacer(5)),
                    LayoutEntry(LayoutCenter(LayoutRotate(
                        LayoutText(watchy->buttonConfig() ==
                                           BUTTONS_SELECT_BACK_RIGHT
                                       ? back
                                       : up,
                                   font, color),
                        3))),
                    LayoutEntry(LayoutFill(), true),
                    LayoutEntry(LayoutCenter(LayoutRotate(
                        LayoutText(watchy->buttonConfig() ==
                                           BUTTONS_SELECT_BACK_RIGHT
                                       ? select
                                       : down,
                                   font, color),
                        3))),
                })),
            })
            .clone();
    return;
  }
  layout_ =
      LayoutRows(
          {
              LayoutEntry(LayoutColumns({
                  LayoutEntry(LayoutCenter(LayoutText(
                      watchy->buttonConfig() == BUTTONS_SELECT_BACK_RIGHT
                          ? up
                          : back,
                      font, color))),
                  LayoutEntry(LayoutFill(), true),
                  LayoutEntry(LayoutCenter(LayoutText(
                      watchy->buttonConfig() == BUTTONS_SELECT_BACK_RIGHT ? back
                                                                          : up,
                      font, color))),
              })),
              LayoutEntry(LayoutSpacer(5)),
              LayoutEntry(LayoutBorder(child, true, false, true, false, color),
                          true),
              LayoutEntry(LayoutSpacer(5)),
              LayoutEntry(LayoutColumns({
                  LayoutEntry(LayoutCenter(LayoutText(
                      watchy->buttonConfig() == BUTTONS_SELECT_BACK_RIGHT
                          ? down
                          : select,
                      font, color))),
                  LayoutEntry(LayoutFill(), true),
                  LayoutEntry(LayoutCenter(LayoutText(
                      watchy->buttonConfig() == BUTTONS_SELECT_BACK_RIGHT
                          ? select
                          : down,
                      font, color))),
              })),
          })
          .clone();
}
