// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/accessibility/ax_enums.h"

#include "base/logging.h"

namespace ui {

std::string ToString(AXEvent enum_param) {
  switch (enum_param) {
    case AX_EVENT_ACTIVEDESCENDANTCHANGED:
      return "activedescendantchanged";
    case AX_EVENT_ALERT:
      return "alert";
    case AX_EVENT_ARIA_ATTRIBUTE_CHANGED:
      return "ariaAttributeChanged";
    case AX_EVENT_AUTOCORRECTION_OCCURED:
      return "autocorrectionOccured";
    case AX_EVENT_BLUR:
      return "blur";
    case AX_EVENT_CHECKED_STATE_CHANGED:
      return "checkedStateChanged";
    case AX_EVENT_CHILDREN_CHANGED:
      return "childrenChanged";
    case AX_EVENT_FOCUS:
      return "focus";
    case AX_EVENT_HIDE:
      return "hide";
    case AX_EVENT_HOVER:
      return "hover";
    case AX_EVENT_INVALID_STATUS_CHANGED:
      return "invalidStatusChanged";
    case AX_EVENT_LAYOUT_COMPLETE:
      return "layoutComplete";
    case AX_EVENT_LIVE_REGION_CHANGED:
      return "liveRegionChanged";
    case AX_EVENT_LOAD_COMPLETE:
      return "loadComplete";
    case AX_EVENT_LOCATION_CHANGED:
      return "locationChanged";
    case AX_EVENT_MENU_END:
      return "menuEnd";
    case AX_EVENT_MENU_LIST_ITEM_SELECTED:
      return "menuListItemSelected";
    case AX_EVENT_MENU_LIST_VALUE_CHANGED:
      return "menuListValueChanged";
    case AX_EVENT_MENU_POPUP_END:
      return "menuPopupEnd";
    case AX_EVENT_MENU_POPUP_START:
      return "menuPopupStart";
    case AX_EVENT_MENU_START:
      return "menuStart";
    case AX_EVENT_ROW_COLLAPSED:
      return "rowCollapsed";
    case AX_EVENT_ROW_COUNT_CHANGED:
      return "rowCountChanged";
    case AX_EVENT_ROW_EXPANDED:
      return "rowExpanded";
    case AX_EVENT_SCROLL_POSITION_CHANGED:
      return "scrollPositionChanged";
    case AX_EVENT_SCROLLED_TO_ANCHOR:
      return "scrolledToAnchor";
    case AX_EVENT_SELECTED_CHILDREN_CHANGED:
      return "selectedChildrenChanged";
    case AX_EVENT_SELECTION:
      return "selection";
    case AX_EVENT_SELECTION_ADD:
      return "selectionAdd";
    case AX_EVENT_SELECTION_REMOVE:
      return "selectionRemove";
    case AX_EVENT_SHOW:
      return "show";
    case AX_EVENT_TEXT_CHANGED:
      return "textChanged";
    case AX_EVENT_TEXT_SELECTION_CHANGED:
      return "textSelectionChanged";
    case AX_EVENT_VALUE_CHANGED:
      return "valueChanged";
    case AX_EVENT_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXEvent ParseAXEvent(const std::string& enum_string) {
  if (enum_string == "activedescendantchanged")
    return AX_EVENT_ACTIVEDESCENDANTCHANGED;
  if (enum_string == "alert") return AX_EVENT_ALERT;
  if (enum_string == "aria_attribute_changed")
    return AX_EVENT_ARIA_ATTRIBUTE_CHANGED;
  if (enum_string == "autocorrection_occured")
    return AX_EVENT_AUTOCORRECTION_OCCURED;
  if (enum_string == "blur") return AX_EVENT_BLUR;
  if (enum_string == "checked_state_changed")
    return AX_EVENT_CHECKED_STATE_CHANGED;
  if (enum_string == "children_changed") return AX_EVENT_CHILDREN_CHANGED;
  if (enum_string == "focus") return AX_EVENT_FOCUS;
  if (enum_string == "hide") return AX_EVENT_HIDE;
  if (enum_string == "hover") return AX_EVENT_HOVER;
  if (enum_string == "invalid_status_changed")
    return AX_EVENT_INVALID_STATUS_CHANGED;
  if (enum_string == "layout_complete") return AX_EVENT_LAYOUT_COMPLETE;
  if (enum_string == "live_region_changed") return AX_EVENT_LIVE_REGION_CHANGED;
  if (enum_string == "load_complete") return AX_EVENT_LOAD_COMPLETE;
  if (enum_string == "location_changed") return AX_EVENT_LOCATION_CHANGED;
  if (enum_string == "menu_end") return AX_EVENT_MENU_END;
  if (enum_string == "menu_list_item_selected")
    return AX_EVENT_MENU_LIST_ITEM_SELECTED;
  if (enum_string == "menu_list_value_changed")
    return AX_EVENT_MENU_LIST_VALUE_CHANGED;
  if (enum_string == "menu_popup_end") return AX_EVENT_MENU_POPUP_END;
  if (enum_string == "menu_popup_start") return AX_EVENT_MENU_POPUP_START;
  if (enum_string == "menu_start") return AX_EVENT_MENU_START;
  if (enum_string == "row_collapsed") return AX_EVENT_ROW_COLLAPSED;
  if (enum_string == "row_count_changed") return AX_EVENT_ROW_COUNT_CHANGED;
  if (enum_string == "row_expanded") return AX_EVENT_ROW_EXPANDED;
  if (enum_string == "scroll_position_changed")
    return AX_EVENT_SCROLL_POSITION_CHANGED;
  if (enum_string == "scrolled_to_anchor") return AX_EVENT_SCROLLED_TO_ANCHOR;
  if (enum_string == "selected_children_changed")
    return AX_EVENT_SELECTED_CHILDREN_CHANGED;
  if (enum_string == "selection") return AX_EVENT_SELECTION;
  if (enum_string == "selection_add") return AX_EVENT_SELECTION_ADD;
  if (enum_string == "selection_remove") return AX_EVENT_SELECTION_REMOVE;
  if (enum_string == "show") return AX_EVENT_SHOW;
  if (enum_string == "text_changed") return AX_EVENT_TEXT_CHANGED;
  if (enum_string == "text_selection_changed")
    return AX_EVENT_TEXT_SELECTION_CHANGED;
  if (enum_string == "value_changed") return AX_EVENT_VALUE_CHANGED;
  return AX_EVENT_NONE;
}

std::string ToString(AXRole enum_param) {
  switch (enum_param) {
    case AX_ROLE_ALERT_DIALOG:
      return "alertDialog";
    case AX_ROLE_ALERT:
      return "alert";
    case AX_ROLE_ANNOTATION:
      return "annotation";
    case AX_ROLE_APPLICATION:
      return "application";
    case AX_ROLE_ARTICLE:
      return "article";
    case AX_ROLE_BANNER:
      return "banner";
    case AX_ROLE_BROWSER:
      return "browser";
    case AX_ROLE_BUSY_INDICATOR:
      return "busyIndicator";
    case AX_ROLE_BUTTON:
      return "button";
    case AX_ROLE_BUTTON_DROP_DOWN:
      return "buttonDropDown";
    case AX_ROLE_CANVAS:
      return "canvas";
    case AX_ROLE_CELL:
      return "cell";
    case AX_ROLE_CHECK_BOX:
      return "checkBox";
    case AX_ROLE_CLIENT:
      return "client";
    case AX_ROLE_COLOR_WELL:
      return "colorWell";
    case AX_ROLE_COLUMN_HEADER:
      return "columnHeader";
    case AX_ROLE_COLUMN:
      return "column";
    case AX_ROLE_COMBO_BOX:
      return "comboBox";
    case AX_ROLE_COMPLEMENTARY:
      return "complementary";
    case AX_ROLE_CONTENT_INFO:
      return "contentInfo";
    case AX_ROLE_DATE:
      return "date";
    case AX_ROLE_DATE_TIME:
      return "dateTime";
    case AX_ROLE_DEFINITION:
      return "definition";
    case AX_ROLE_DESCRIPTION_LIST_DETAIL:
      return "descriptionListDetail";
    case AX_ROLE_DESCRIPTION_LIST:
      return "descriptionList";
    case AX_ROLE_DESCRIPTION_LIST_TERM:
      return "descriptionListTerm";
    case AX_ROLE_DESKTOP:
      return "desktop";
    case AX_ROLE_DETAILS:
      return "details";
    case AX_ROLE_DIALOG:
      return "dialog";
    case AX_ROLE_DIRECTORY:
      return "directory";
    case AX_ROLE_DISCLOSURE_TRIANGLE:
      return "disclosureTriangle";
    case AX_ROLE_DIV:
      return "div";
    case AX_ROLE_DOCUMENT:
      return "document";
    case AX_ROLE_DRAWER:
      return "drawer";
    case AX_ROLE_EDITABLE_TEXT:
      return "editableText";
    case AX_ROLE_EMBEDDED_OBJECT:
      return "embeddedObject";
    case AX_ROLE_FIGCAPTION:
      return "figcaption";
    case AX_ROLE_FIGURE:
      return "figure";
    case AX_ROLE_FOOTER:
      return "footer";
    case AX_ROLE_FORM:
      return "form";
    case AX_ROLE_GRID:
      return "grid";
    case AX_ROLE_GROUP:
      return "group";
    case AX_ROLE_GROW_AREA:
      return "growArea";
    case AX_ROLE_HEADING:
      return "heading";
    case AX_ROLE_HELP_TAG:
      return "helpTag";
    case AX_ROLE_HORIZONTAL_RULE:
      return "horizontalRule";
    case AX_ROLE_IFRAME:
      return "iframe";
    case AX_ROLE_IGNORED:
      return "ignored";
    case AX_ROLE_IMAGE_MAP_LINK:
      return "imageMapLink";
    case AX_ROLE_IMAGE_MAP:
      return "imageMap";
    case AX_ROLE_IMAGE:
      return "image";
    case AX_ROLE_INCREMENTOR:
      return "incrementor";
    case AX_ROLE_INLINE_TEXT_BOX:
      return "inlineTextBox";
    case AX_ROLE_LABEL_TEXT:
      return "labelText";
    case AX_ROLE_LEGEND:
      return "legend";
    case AX_ROLE_LINK:
      return "link";
    case AX_ROLE_LIST_BOX_OPTION:
      return "listBoxOption";
    case AX_ROLE_LIST_BOX:
      return "listBox";
    case AX_ROLE_LIST_ITEM:
      return "listItem";
    case AX_ROLE_LIST_MARKER:
      return "listMarker";
    case AX_ROLE_LIST:
      return "list";
    case AX_ROLE_LOCATION_BAR:
      return "locationBar";
    case AX_ROLE_LOG:
      return "log";
    case AX_ROLE_MAIN:
      return "main";
    case AX_ROLE_MARQUEE:
      return "marquee";
    case AX_ROLE_MATH_ELEMENT:
      return "mathElement";
    case AX_ROLE_MATH:
      return "math";
    case AX_ROLE_MATTE:
      return "matte";
    case AX_ROLE_MENU_BAR:
      return "menuBar";
    case AX_ROLE_MENU_BUTTON:
      return "menuButton";
    case AX_ROLE_MENU_ITEM:
      return "menuItem";
    case AX_ROLE_MENU_ITEM_CHECK_BOX:
      return "menuItemCheckBox";
    case AX_ROLE_MENU_ITEM_RADIO:
      return "menuItemRadio";
    case AX_ROLE_MENU_LIST_OPTION:
      return "menuListOption";
    case AX_ROLE_MENU_LIST_POPUP:
      return "menuListPopup";
    case AX_ROLE_MENU:
      return "menu";
    case AX_ROLE_METER:
      return "meter";
    case AX_ROLE_NAVIGATION:
      return "navigation";
    case AX_ROLE_NOTE:
      return "note";
    case AX_ROLE_OUTLINE:
      return "outline";
    case AX_ROLE_PANE:
      return "pane";
    case AX_ROLE_PARAGRAPH:
      return "paragraph";
    case AX_ROLE_POP_UP_BUTTON:
      return "popUpButton";
    case AX_ROLE_PRESENTATIONAL:
      return "presentational";
    case AX_ROLE_PROGRESS_INDICATOR:
      return "progressIndicator";
    case AX_ROLE_RADIO_BUTTON:
      return "radioButton";
    case AX_ROLE_RADIO_GROUP:
      return "radioGroup";
    case AX_ROLE_REGION:
      return "region";
    case AX_ROLE_ROOT_WEB_AREA:
      return "rootWebArea";
    case AX_ROLE_ROW_HEADER:
      return "rowHeader";
    case AX_ROLE_ROW:
      return "row";
    case AX_ROLE_RULER_MARKER:
      return "rulerMarker";
    case AX_ROLE_RULER:
      return "ruler";
    case AX_ROLE_SVG_ROOT:
      return "svgRoot";
    case AX_ROLE_SCROLL_AREA:
      return "scrollArea";
    case AX_ROLE_SCROLL_BAR:
      return "scrollBar";
    case AX_ROLE_SEAMLESS_WEB_AREA:
      return "seamlessWebArea";
    case AX_ROLE_SEARCH:
      return "search";
    case AX_ROLE_SHEET:
      return "sheet";
    case AX_ROLE_SLIDER:
      return "slider";
    case AX_ROLE_SLIDER_THUMB:
      return "sliderThumb";
    case AX_ROLE_SPIN_BUTTON_PART:
      return "spinButtonPart";
    case AX_ROLE_SPIN_BUTTON:
      return "spinButton";
    case AX_ROLE_SPLIT_GROUP:
      return "splitGroup";
    case AX_ROLE_SPLITTER:
      return "splitter";
    case AX_ROLE_STATIC_TEXT:
      return "staticText";
    case AX_ROLE_STATUS:
      return "status";
    case AX_ROLE_SYSTEM_WIDE:
      return "systemWide";
    case AX_ROLE_TAB_GROUP:
      return "tabGroup";
    case AX_ROLE_TAB_LIST:
      return "tabList";
    case AX_ROLE_TAB_PANEL:
      return "tabPanel";
    case AX_ROLE_TAB:
      return "tab";
    case AX_ROLE_TABLE_HEADER_CONTAINER:
      return "tableHeaderContainer";
    case AX_ROLE_TABLE:
      return "table";
    case AX_ROLE_TEXT_AREA:
      return "textArea";
    case AX_ROLE_TEXT_FIELD:
      return "textField";
    case AX_ROLE_TIME:
      return "time";
    case AX_ROLE_TIMER:
      return "timer";
    case AX_ROLE_TITLE_BAR:
      return "titleBar";
    case AX_ROLE_TOGGLE_BUTTON:
      return "toggleButton";
    case AX_ROLE_TOOLBAR:
      return "toolbar";
    case AX_ROLE_TREE_GRID:
      return "treeGrid";
    case AX_ROLE_TREE_ITEM:
      return "treeItem";
    case AX_ROLE_TREE:
      return "tree";
    case AX_ROLE_UNKNOWN:
      return "unknown";
    case AX_ROLE_TOOLTIP:
      return "tooltip";
    case AX_ROLE_VALUE_INDICATOR:
      return "valueIndicator";
    case AX_ROLE_WEB_AREA:
      return "webArea";
    case AX_ROLE_WINDOW:
      return "window";
    case AX_ROLE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXRole ParseAXRole(const std::string& enum_string) {
  if (enum_string == "alert_dialog") return AX_ROLE_ALERT_DIALOG;
  if (enum_string == "alert") return AX_ROLE_ALERT;
  if (enum_string == "annotation") return AX_ROLE_ANNOTATION;
  if (enum_string == "application") return AX_ROLE_APPLICATION;
  if (enum_string == "article") return AX_ROLE_ARTICLE;
  if (enum_string == "banner") return AX_ROLE_BANNER;
  if (enum_string == "browser") return AX_ROLE_BROWSER;
  if (enum_string == "busy_indicator") return AX_ROLE_BUSY_INDICATOR;
  if (enum_string == "button") return AX_ROLE_BUTTON;
  if (enum_string == "button_drop_down") return AX_ROLE_BUTTON_DROP_DOWN;
  if (enum_string == "canvas") return AX_ROLE_CANVAS;
  if (enum_string == "cell") return AX_ROLE_CELL;
  if (enum_string == "check_box") return AX_ROLE_CHECK_BOX;
  if (enum_string == "client") return AX_ROLE_CLIENT;
  if (enum_string == "color_well") return AX_ROLE_COLOR_WELL;
  if (enum_string == "column_header") return AX_ROLE_COLUMN_HEADER;
  if (enum_string == "column") return AX_ROLE_COLUMN;
  if (enum_string == "combo_box") return AX_ROLE_COMBO_BOX;
  if (enum_string == "complementary") return AX_ROLE_COMPLEMENTARY;
  if (enum_string == "content_info") return AX_ROLE_CONTENT_INFO;
  if (enum_string == "date") return AX_ROLE_DATE;
  if (enum_string == "date_time") return AX_ROLE_DATE_TIME;
  if (enum_string == "definition") return AX_ROLE_DEFINITION;
  if (enum_string == "description_list_detail")
    return AX_ROLE_DESCRIPTION_LIST_DETAIL;
  if (enum_string == "description_list") return AX_ROLE_DESCRIPTION_LIST;
  if (enum_string == "description_list_term")
    return AX_ROLE_DESCRIPTION_LIST_TERM;
  if (enum_string == "desktop") return AX_ROLE_DESKTOP;
  if (enum_string == "details") return AX_ROLE_DETAILS;
  if (enum_string == "dialog") return AX_ROLE_DIALOG;
  if (enum_string == "directory") return AX_ROLE_DIRECTORY;
  if (enum_string == "disclosure_triangle") return AX_ROLE_DISCLOSURE_TRIANGLE;
  if (enum_string == "div") return AX_ROLE_DIV;
  if (enum_string == "document") return AX_ROLE_DOCUMENT;
  if (enum_string == "drawer") return AX_ROLE_DRAWER;
  if (enum_string == "editable_text") return AX_ROLE_EDITABLE_TEXT;
  if (enum_string == "embedded_object") return AX_ROLE_EMBEDDED_OBJECT;
  if (enum_string == "figcaption") return AX_ROLE_FIGCAPTION;
  if (enum_string == "figure") return AX_ROLE_FIGURE;
  if (enum_string == "footer") return AX_ROLE_FOOTER;
  if (enum_string == "form") return AX_ROLE_FORM;
  if (enum_string == "grid") return AX_ROLE_GRID;
  if (enum_string == "group") return AX_ROLE_GROUP;
  if (enum_string == "grow_area") return AX_ROLE_GROW_AREA;
  if (enum_string == "heading") return AX_ROLE_HEADING;
  if (enum_string == "help_tag") return AX_ROLE_HELP_TAG;
  if (enum_string == "horizontal_rule") return AX_ROLE_HORIZONTAL_RULE;
  if (enum_string == "iframe") return AX_ROLE_IFRAME;
  if (enum_string == "ignored") return AX_ROLE_IGNORED;
  if (enum_string == "image_map_link") return AX_ROLE_IMAGE_MAP_LINK;
  if (enum_string == "image_map") return AX_ROLE_IMAGE_MAP;
  if (enum_string == "image") return AX_ROLE_IMAGE;
  if (enum_string == "incrementor") return AX_ROLE_INCREMENTOR;
  if (enum_string == "inline_text_box") return AX_ROLE_INLINE_TEXT_BOX;
  if (enum_string == "label_text") return AX_ROLE_LABEL_TEXT;
  if (enum_string == "legend") return AX_ROLE_LEGEND;
  if (enum_string == "link") return AX_ROLE_LINK;
  if (enum_string == "list_box_option") return AX_ROLE_LIST_BOX_OPTION;
  if (enum_string == "list_box") return AX_ROLE_LIST_BOX;
  if (enum_string == "list_item") return AX_ROLE_LIST_ITEM;
  if (enum_string == "list_marker") return AX_ROLE_LIST_MARKER;
  if (enum_string == "list") return AX_ROLE_LIST;
  if (enum_string == "location_bar") return AX_ROLE_LOCATION_BAR;
  if (enum_string == "log") return AX_ROLE_LOG;
  if (enum_string == "main") return AX_ROLE_MAIN;
  if (enum_string == "marquee") return AX_ROLE_MARQUEE;
  if (enum_string == "math_element") return AX_ROLE_MATH_ELEMENT;
  if (enum_string == "math") return AX_ROLE_MATH;
  if (enum_string == "matte") return AX_ROLE_MATTE;
  if (enum_string == "menu_bar") return AX_ROLE_MENU_BAR;
  if (enum_string == "menu_button") return AX_ROLE_MENU_BUTTON;
  if (enum_string == "menu_item") return AX_ROLE_MENU_ITEM;
  if (enum_string == "menu_item_check_box") return AX_ROLE_MENU_ITEM_CHECK_BOX;
  if (enum_string == "menu_item_radio") return AX_ROLE_MENU_ITEM_RADIO;
  if (enum_string == "menu_list_option") return AX_ROLE_MENU_LIST_OPTION;
  if (enum_string == "menu_list_popup") return AX_ROLE_MENU_LIST_POPUP;
  if (enum_string == "menu") return AX_ROLE_MENU;
  if (enum_string == "meter") return AX_ROLE_METER;
  if (enum_string == "navigation") return AX_ROLE_NAVIGATION;
  if (enum_string == "note") return AX_ROLE_NOTE;
  if (enum_string == "outline") return AX_ROLE_OUTLINE;
  if (enum_string == "pane") return AX_ROLE_PANE;
  if (enum_string == "paragraph") return AX_ROLE_PARAGRAPH;
  if (enum_string == "pop_up_button") return AX_ROLE_POP_UP_BUTTON;
  if (enum_string == "presentational") return AX_ROLE_PRESENTATIONAL;
  if (enum_string == "progress_indicator") return AX_ROLE_PROGRESS_INDICATOR;
  if (enum_string == "radio_button") return AX_ROLE_RADIO_BUTTON;
  if (enum_string == "radio_group") return AX_ROLE_RADIO_GROUP;
  if (enum_string == "region") return AX_ROLE_REGION;
  if (enum_string == "root_web_area") return AX_ROLE_ROOT_WEB_AREA;
  if (enum_string == "row_header") return AX_ROLE_ROW_HEADER;
  if (enum_string == "row") return AX_ROLE_ROW;
  if (enum_string == "ruler_marker") return AX_ROLE_RULER_MARKER;
  if (enum_string == "ruler") return AX_ROLE_RULER;
  if (enum_string == "svg_root") return AX_ROLE_SVG_ROOT;
  if (enum_string == "scroll_area") return AX_ROLE_SCROLL_AREA;
  if (enum_string == "scroll_bar") return AX_ROLE_SCROLL_BAR;
  if (enum_string == "seamless_web_area") return AX_ROLE_SEAMLESS_WEB_AREA;
  if (enum_string == "search") return AX_ROLE_SEARCH;
  if (enum_string == "sheet") return AX_ROLE_SHEET;
  if (enum_string == "slider") return AX_ROLE_SLIDER;
  if (enum_string == "slider_thumb") return AX_ROLE_SLIDER_THUMB;
  if (enum_string == "spin_button_part") return AX_ROLE_SPIN_BUTTON_PART;
  if (enum_string == "spin_button") return AX_ROLE_SPIN_BUTTON;
  if (enum_string == "split_group") return AX_ROLE_SPLIT_GROUP;
  if (enum_string == "splitter") return AX_ROLE_SPLITTER;
  if (enum_string == "static_text") return AX_ROLE_STATIC_TEXT;
  if (enum_string == "status") return AX_ROLE_STATUS;
  if (enum_string == "system_wide") return AX_ROLE_SYSTEM_WIDE;
  if (enum_string == "tab_group") return AX_ROLE_TAB_GROUP;
  if (enum_string == "tab_list") return AX_ROLE_TAB_LIST;
  if (enum_string == "tab_panel") return AX_ROLE_TAB_PANEL;
  if (enum_string == "tab") return AX_ROLE_TAB;
  if (enum_string == "table_header_container")
    return AX_ROLE_TABLE_HEADER_CONTAINER;
  if (enum_string == "table") return AX_ROLE_TABLE;
  if (enum_string == "text_area") return AX_ROLE_TEXT_AREA;
  if (enum_string == "text_field") return AX_ROLE_TEXT_FIELD;
  if (enum_string == "time") return AX_ROLE_TIME;
  if (enum_string == "timer") return AX_ROLE_TIMER;
  if (enum_string == "title_bar") return AX_ROLE_TITLE_BAR;
  if (enum_string == "toggle_button") return AX_ROLE_TOGGLE_BUTTON;
  if (enum_string == "toolbar") return AX_ROLE_TOOLBAR;
  if (enum_string == "tree_grid") return AX_ROLE_TREE_GRID;
  if (enum_string == "tree_item") return AX_ROLE_TREE_ITEM;
  if (enum_string == "tree") return AX_ROLE_TREE;
  if (enum_string == "unknown") return AX_ROLE_UNKNOWN;
  if (enum_string == "tooltip") return AX_ROLE_TOOLTIP;
  if (enum_string == "value_indicator") return AX_ROLE_VALUE_INDICATOR;
  if (enum_string == "web_area") return AX_ROLE_WEB_AREA;
  if (enum_string == "window") return AX_ROLE_WINDOW;
  return AX_ROLE_NONE;
}

std::string ToString(AXState enum_param) {
  switch (enum_param) {
    case AX_STATE_BUSY:
      return "busy";
    case AX_STATE_CHECKED:
      return "checked";
    case AX_STATE_COLLAPSED:
      return "collapsed";
    case AX_STATE_DEFAULT:
      return "default";
    case AX_STATE_DISABLED:
      return "disabled";
    case AX_STATE_EDITABLE:
      return "editable";
    case AX_STATE_ENABLED:
      return "enabled";
    case AX_STATE_EXPANDED:
      return "expanded";
    case AX_STATE_FOCUSABLE:
      return "focusable";
    case AX_STATE_FOCUSED:
      return "focused";
    case AX_STATE_HASPOPUP:
      return "haspopup";
    case AX_STATE_HOVERED:
      return "hovered";
    case AX_STATE_INDETERMINATE:
      return "indeterminate";
    case AX_STATE_INVISIBLE:
      return "invisible";
    case AX_STATE_LINKED:
      return "linked";
    case AX_STATE_MULTISELECTABLE:
      return "multiselectable";
    case AX_STATE_OFFSCREEN:
      return "offscreen";
    case AX_STATE_PRESSED:
      return "pressed";
    case AX_STATE_PROTECTED:
      return "protected";
    case AX_STATE_READ_ONLY:
      return "readOnly";
    case AX_STATE_REQUIRED:
      return "required";
    case AX_STATE_SELECTABLE:
      return "selectable";
    case AX_STATE_SELECTED:
      return "selected";
    case AX_STATE_VERTICAL:
      return "vertical";
    case AX_STATE_VISITED:
      return "visited";
    case AX_STATE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXState ParseAXState(const std::string& enum_string) {
  if (enum_string == "busy") return AX_STATE_BUSY;
  if (enum_string == "checked") return AX_STATE_CHECKED;
  if (enum_string == "collapsed") return AX_STATE_COLLAPSED;
  if (enum_string == "default") return AX_STATE_DEFAULT;
  if (enum_string == "disabled") return AX_STATE_DISABLED;
  if (enum_string == "editable") return AX_STATE_EDITABLE;
  if (enum_string == "enabled") return AX_STATE_ENABLED;
  if (enum_string == "expanded") return AX_STATE_EXPANDED;
  if (enum_string == "focusable") return AX_STATE_FOCUSABLE;
  if (enum_string == "focused") return AX_STATE_FOCUSED;
  if (enum_string == "haspopup") return AX_STATE_HASPOPUP;
  if (enum_string == "hovered") return AX_STATE_HOVERED;
  if (enum_string == "indeterminate") return AX_STATE_INDETERMINATE;
  if (enum_string == "invisible") return AX_STATE_INVISIBLE;
  if (enum_string == "linked") return AX_STATE_LINKED;
  if (enum_string == "multiselectable") return AX_STATE_MULTISELECTABLE;
  if (enum_string == "offscreen") return AX_STATE_OFFSCREEN;
  if (enum_string == "pressed") return AX_STATE_PRESSED;
  if (enum_string == "protected") return AX_STATE_PROTECTED;
  if (enum_string == "read_only") return AX_STATE_READ_ONLY;
  if (enum_string == "required") return AX_STATE_REQUIRED;
  if (enum_string == "selectable") return AX_STATE_SELECTABLE;
  if (enum_string == "selected") return AX_STATE_SELECTED;
  if (enum_string == "vertical") return AX_STATE_VERTICAL;
  if (enum_string == "visited") return AX_STATE_VISITED;
  return AX_STATE_NONE;
}

std::string ToString(AXStringAttribute enum_param) {
  switch (enum_param) {
    case AX_ATTR_DOC_URL:
      return "docUrl";
    case AX_ATTR_DOC_TITLE:
      return "docTitle";
    case AX_ATTR_DOC_MIMETYPE:
      return "docMimetype";
    case AX_ATTR_DOC_DOCTYPE:
      return "docDoctype";
    case AX_ATTR_ACCESS_KEY:
      return "accessKey";
    case AX_ATTR_ACTION:
      return "action";
    case AX_ATTR_CONTAINER_LIVE_RELEVANT:
      return "containerLiveRelevant";
    case AX_ATTR_CONTAINER_LIVE_STATUS:
      return "containerLiveStatus";
    case AX_ATTR_DESCRIPTION:
      return "description";
    case AX_ATTR_DISPLAY:
      return "display";
    case AX_ATTR_HELP:
      return "help";
    case AX_ATTR_HTML_TAG:
      return "htmlTag";
    case AX_ATTR_NAME:
      return "name";
    case AX_ATTR_LIVE_RELEVANT:
      return "liveRelevant";
    case AX_ATTR_LIVE_STATUS:
      return "liveStatus";
    case AX_ATTR_ROLE:
      return "role";
    case AX_ATTR_SHORTCUT:
      return "shortcut";
    case AX_ATTR_TEXT_INPUT_TYPE:
      return "textInputType";
    case AX_ATTR_URL:
      return "url";
    case AX_ATTR_VALUE:
      return "value";
    case AX_STRING_ATTRIBUTE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXStringAttribute ParseAXStringAttribute(const std::string& enum_string) {
  if (enum_string == "doc_url") return AX_ATTR_DOC_URL;
  if (enum_string == "doc_title") return AX_ATTR_DOC_TITLE;
  if (enum_string == "doc_mimetype") return AX_ATTR_DOC_MIMETYPE;
  if (enum_string == "doc_doctype") return AX_ATTR_DOC_DOCTYPE;
  if (enum_string == "access_key") return AX_ATTR_ACCESS_KEY;
  if (enum_string == "action") return AX_ATTR_ACTION;
  if (enum_string == "container_live_relevant")
    return AX_ATTR_CONTAINER_LIVE_RELEVANT;
  if (enum_string == "container_live_status")
    return AX_ATTR_CONTAINER_LIVE_STATUS;
  if (enum_string == "description") return AX_ATTR_DESCRIPTION;
  if (enum_string == "display") return AX_ATTR_DISPLAY;
  if (enum_string == "help") return AX_ATTR_HELP;
  if (enum_string == "html_tag") return AX_ATTR_HTML_TAG;
  if (enum_string == "name") return AX_ATTR_NAME;
  if (enum_string == "live_relevant") return AX_ATTR_LIVE_RELEVANT;
  if (enum_string == "live_status") return AX_ATTR_LIVE_STATUS;
  if (enum_string == "role") return AX_ATTR_ROLE;
  if (enum_string == "shortcut") return AX_ATTR_SHORTCUT;
  if (enum_string == "text_input_type") return AX_ATTR_TEXT_INPUT_TYPE;
  if (enum_string == "url") return AX_ATTR_URL;
  if (enum_string == "value") return AX_ATTR_VALUE;
  return AX_STRING_ATTRIBUTE_NONE;
}

std::string ToString(AXIntAttribute enum_param) {
  switch (enum_param) {
    case AX_ATTR_SCROLL_X:
      return "scrollX";
    case AX_ATTR_SCROLL_X_MIN:
      return "scrollXMin";
    case AX_ATTR_SCROLL_X_MAX:
      return "scrollXMax";
    case AX_ATTR_SCROLL_Y:
      return "scrollY";
    case AX_ATTR_SCROLL_Y_MIN:
      return "scrollYMin";
    case AX_ATTR_SCROLL_Y_MAX:
      return "scrollYMax";
    case AX_ATTR_TEXT_SEL_START:
      return "textSelStart";
    case AX_ATTR_TEXT_SEL_END:
      return "textSelEnd";
    case AX_ATTR_TABLE_ROW_COUNT:
      return "tableRowCount";
    case AX_ATTR_TABLE_COLUMN_COUNT:
      return "tableColumnCount";
    case AX_ATTR_TABLE_HEADER_ID:
      return "tableHeaderId";
    case AX_ATTR_TABLE_ROW_INDEX:
      return "tableRowIndex";
    case AX_ATTR_TABLE_ROW_HEADER_ID:
      return "tableRowHeaderId";
    case AX_ATTR_TABLE_COLUMN_INDEX:
      return "tableColumnIndex";
    case AX_ATTR_TABLE_COLUMN_HEADER_ID:
      return "tableColumnHeaderId";
    case AX_ATTR_TABLE_CELL_COLUMN_INDEX:
      return "tableCellColumnIndex";
    case AX_ATTR_TABLE_CELL_COLUMN_SPAN:
      return "tableCellColumnSpan";
    case AX_ATTR_TABLE_CELL_ROW_INDEX:
      return "tableCellRowIndex";
    case AX_ATTR_TABLE_CELL_ROW_SPAN:
      return "tableCellRowSpan";
    case AX_ATTR_HIERARCHICAL_LEVEL:
      return "hierarchicalLevel";
    case AX_ATTR_TITLE_UI_ELEMENT:
      return "titleUiElement";
    case AX_ATTR_ACTIVEDESCENDANT_ID:
      return "activedescendantId";
    case AX_ATTR_COLOR_VALUE_RED:
      return "colorValueRed";
    case AX_ATTR_COLOR_VALUE_GREEN:
      return "colorValueGreen";
    case AX_ATTR_COLOR_VALUE_BLUE:
      return "colorValueBlue";
    case AX_ATTR_TEXT_DIRECTION:
      return "textDirection";
    case AX_INT_ATTRIBUTE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXIntAttribute ParseAXIntAttribute(const std::string& enum_string) {
  if (enum_string == "scroll_x") return AX_ATTR_SCROLL_X;
  if (enum_string == "scroll_x_min") return AX_ATTR_SCROLL_X_MIN;
  if (enum_string == "scroll_x_max") return AX_ATTR_SCROLL_X_MAX;
  if (enum_string == "scroll_y") return AX_ATTR_SCROLL_Y;
  if (enum_string == "scroll_y_min") return AX_ATTR_SCROLL_Y_MIN;
  if (enum_string == "scroll_y_max") return AX_ATTR_SCROLL_Y_MAX;
  if (enum_string == "text_sel_start") return AX_ATTR_TEXT_SEL_START;
  if (enum_string == "text_sel_end") return AX_ATTR_TEXT_SEL_END;
  if (enum_string == "table_row_count") return AX_ATTR_TABLE_ROW_COUNT;
  if (enum_string == "table_column_count") return AX_ATTR_TABLE_COLUMN_COUNT;
  if (enum_string == "table_header_id") return AX_ATTR_TABLE_HEADER_ID;
  if (enum_string == "table_row_index") return AX_ATTR_TABLE_ROW_INDEX;
  if (enum_string == "table_row_header_id") return AX_ATTR_TABLE_ROW_HEADER_ID;
  if (enum_string == "table_column_index") return AX_ATTR_TABLE_COLUMN_INDEX;
  if (enum_string == "table_column_header_id")
    return AX_ATTR_TABLE_COLUMN_HEADER_ID;
  if (enum_string == "table_cell_column_index")
    return AX_ATTR_TABLE_CELL_COLUMN_INDEX;
  if (enum_string == "table_cell_column_span")
    return AX_ATTR_TABLE_CELL_COLUMN_SPAN;
  if (enum_string == "table_cell_row_index")
    return AX_ATTR_TABLE_CELL_ROW_INDEX;
  if (enum_string == "table_cell_row_span") return AX_ATTR_TABLE_CELL_ROW_SPAN;
  if (enum_string == "hierarchical_level") return AX_ATTR_HIERARCHICAL_LEVEL;
  if (enum_string == "title_ui_element") return AX_ATTR_TITLE_UI_ELEMENT;
  if (enum_string == "activedescendant_id") return AX_ATTR_ACTIVEDESCENDANT_ID;
  if (enum_string == "color_value_red") return AX_ATTR_COLOR_VALUE_RED;
  if (enum_string == "color_value_green") return AX_ATTR_COLOR_VALUE_GREEN;
  if (enum_string == "color_value_blue") return AX_ATTR_COLOR_VALUE_BLUE;
  if (enum_string == "text_direction") return AX_ATTR_TEXT_DIRECTION;
  return AX_INT_ATTRIBUTE_NONE;
}

std::string ToString(AXFloatAttribute enum_param) {
  switch (enum_param) {
    case AX_ATTR_DOC_LOADING_PROGRESS:
      return "docLoadingProgress";
    case AX_ATTR_VALUE_FOR_RANGE:
      return "valueForRange";
    case AX_ATTR_MIN_VALUE_FOR_RANGE:
      return "minValueForRange";
    case AX_ATTR_MAX_VALUE_FOR_RANGE:
      return "maxValueForRange";
    case AX_FLOAT_ATTRIBUTE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXFloatAttribute ParseAXFloatAttribute(const std::string& enum_string) {
  if (enum_string == "doc_loading_progress")
    return AX_ATTR_DOC_LOADING_PROGRESS;
  if (enum_string == "value_for_range") return AX_ATTR_VALUE_FOR_RANGE;
  if (enum_string == "min_value_for_range") return AX_ATTR_MIN_VALUE_FOR_RANGE;
  if (enum_string == "max_value_for_range") return AX_ATTR_MAX_VALUE_FOR_RANGE;
  return AX_FLOAT_ATTRIBUTE_NONE;
}

std::string ToString(AXBoolAttribute enum_param) {
  switch (enum_param) {
    case AX_ATTR_DOC_LOADED:
      return "docLoaded";
    case AX_ATTR_BUTTON_MIXED:
      return "buttonMixed";
    case AX_ATTR_CONTAINER_LIVE_ATOMIC:
      return "containerLiveAtomic";
    case AX_ATTR_CONTAINER_LIVE_BUSY:
      return "containerLiveBusy";
    case AX_ATTR_LIVE_ATOMIC:
      return "liveAtomic";
    case AX_ATTR_LIVE_BUSY:
      return "liveBusy";
    case AX_ATTR_ARIA_READONLY:
      return "ariaReadonly";
    case AX_ATTR_CAN_SET_VALUE:
      return "canSetValue";
    case AX_ATTR_UPDATE_LOCATION_ONLY:
      return "updateLocationOnly";
    case AX_ATTR_CANVAS_HAS_FALLBACK:
      return "canvasHasFallback";
    case AX_ATTR_IS_AX_TREE_HOST:
      return "isAxTreeHost";
    case AX_BOOL_ATTRIBUTE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXBoolAttribute ParseAXBoolAttribute(const std::string& enum_string) {
  if (enum_string == "doc_loaded") return AX_ATTR_DOC_LOADED;
  if (enum_string == "button_mixed") return AX_ATTR_BUTTON_MIXED;
  if (enum_string == "container_live_atomic")
    return AX_ATTR_CONTAINER_LIVE_ATOMIC;
  if (enum_string == "container_live_busy") return AX_ATTR_CONTAINER_LIVE_BUSY;
  if (enum_string == "live_atomic") return AX_ATTR_LIVE_ATOMIC;
  if (enum_string == "live_busy") return AX_ATTR_LIVE_BUSY;
  if (enum_string == "aria_readonly") return AX_ATTR_ARIA_READONLY;
  if (enum_string == "can_set_value") return AX_ATTR_CAN_SET_VALUE;
  if (enum_string == "update_location_only")
    return AX_ATTR_UPDATE_LOCATION_ONLY;
  if (enum_string == "canvas_has_fallback") return AX_ATTR_CANVAS_HAS_FALLBACK;
  if (enum_string == "is_ax_tree_host") return AX_ATTR_IS_AX_TREE_HOST;
  return AX_BOOL_ATTRIBUTE_NONE;
}

std::string ToString(AXIntListAttribute enum_param) {
  switch (enum_param) {
    case AX_ATTR_INDIRECT_CHILD_IDS:
      return "indirectChildIds";
    case AX_ATTR_CONTROLS_IDS:
      return "controlsIds";
    case AX_ATTR_DESCRIBEDBY_IDS:
      return "describedbyIds";
    case AX_ATTR_FLOWTO_IDS:
      return "flowtoIds";
    case AX_ATTR_LABELLEDBY_IDS:
      return "labelledbyIds";
    case AX_ATTR_OWNS_IDS:
      return "ownsIds";
    case AX_ATTR_LINE_BREAKS:
      return "lineBreaks";
    case AX_ATTR_CELL_IDS:
      return "cellIds";
    case AX_ATTR_UNIQUE_CELL_IDS:
      return "uniqueCellIds";
    case AX_ATTR_CHARACTER_OFFSETS:
      return "characterOffsets";
    case AX_ATTR_WORD_STARTS:
      return "wordStarts";
    case AX_ATTR_WORD_ENDS:
      return "wordEnds";
    case AX_INT_LIST_ATTRIBUTE_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXIntListAttribute ParseAXIntListAttribute(const std::string& enum_string) {
  if (enum_string == "indirect_child_ids") return AX_ATTR_INDIRECT_CHILD_IDS;
  if (enum_string == "controls_ids") return AX_ATTR_CONTROLS_IDS;
  if (enum_string == "describedby_ids") return AX_ATTR_DESCRIBEDBY_IDS;
  if (enum_string == "flowto_ids") return AX_ATTR_FLOWTO_IDS;
  if (enum_string == "labelledby_ids") return AX_ATTR_LABELLEDBY_IDS;
  if (enum_string == "owns_ids") return AX_ATTR_OWNS_IDS;
  if (enum_string == "line_breaks") return AX_ATTR_LINE_BREAKS;
  if (enum_string == "cell_ids") return AX_ATTR_CELL_IDS;
  if (enum_string == "unique_cell_ids") return AX_ATTR_UNIQUE_CELL_IDS;
  if (enum_string == "character_offsets") return AX_ATTR_CHARACTER_OFFSETS;
  if (enum_string == "word_starts") return AX_ATTR_WORD_STARTS;
  if (enum_string == "word_ends") return AX_ATTR_WORD_ENDS;
  return AX_INT_LIST_ATTRIBUTE_NONE;
}

std::string ToString(AXTextDirection enum_param) {
  switch (enum_param) {
    case AX_TEXT_DIRECTION_LR:
      return "textDirectionLr";
    case AX_TEXT_DIRECTION_RL:
      return "textDirectionRl";
    case AX_TEXT_DIRECTION_TB:
      return "textDirectionTb";
    case AX_TEXT_DIRECTION_BT:
      return "textDirectionBt";
    case AX_TEXT_DIRECTION_NONE:
      return "";
  }
  NOTREACHED();
  return "";
}

AXTextDirection ParseAXTextDirection(const std::string& enum_string) {
  if (enum_string == "text_direction_lr") return AX_TEXT_DIRECTION_LR;
  if (enum_string == "text_direction_rl") return AX_TEXT_DIRECTION_RL;
  if (enum_string == "text_direction_tb") return AX_TEXT_DIRECTION_TB;
  if (enum_string == "text_direction_bt") return AX_TEXT_DIRECTION_BT;
  return AX_TEXT_DIRECTION_NONE;
}

}  // namespace ui
