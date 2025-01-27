TEMPLATE = subdirs

SUBDIRS += \
    anchors \
    dials \
    dialogbuttons \
    invoker \
    inputpanel \
    images \
    shadows \
    shapes

qtHaveModule(webengine) {

    SUBDIRS += \
        webview
}

qtHaveModule(quickwidgets) {

    SUBDIRS += \
        grids
}
