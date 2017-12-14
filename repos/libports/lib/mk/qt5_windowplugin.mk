include $(REP_DIR)/lib/import/import-qt5_windowplugin.mk

SHARED_LIB = yes

include $(REP_DIR)/lib/mk/qt5_windowplugin_generated.inc

QT_DEFINES += -UQT_STATICPLUGIN

include $(REP_DIR)/lib/mk/qt5.inc

LIBS += qt5_qml mesa

# install the QtQuick QML plugin

QTQUICK_INSTALL_DIR := $(BUILD_BASE_DIR)/bin/qt5_fs/qt/qml/QtQuick/Window.2
QTQUICK_QMLDIR      := $(QTQUICK_INSTALL_DIR)/qmldir
QTQUICK_PLUGIN_NAME := qt5_windowplugin.lib.so
QTQUICK_PLUGIN      := $(QTQUICK_INSTALL_DIR)/$(QTQUICK_PLUGIN_NAME)

$(QTQUICK_INSTALL_DIR):
	$(VERBOSE)mkdir -p $@

$(QTQUICK_QMLDIR): $(QTQUICK_INSTALL_DIR)
	$(VERBOSE)cp $(QT5_CONTRIB_DIR)/qtdeclarative/src/imports/window/qmldir $(QTQUICK_INSTALL_DIR)

$(QTQUICK_PLUGIN): $(QTQUICK_INSTALL_DIR)
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/bin/$(QTQUICK_PLUGIN_NAME) $(QTQUICK_INSTALL_DIR)/$(QTQUICK_PLUGIN_NAME)

ifneq ($(call select_from_ports,qt5),)
all: $(QTQUICK_QMLDIR) $(QTQUICK_PLUGIN)
endif

#
# unfortunately, these clean rules don't trigger
#

clean-qtquick_install_dir:
	rm -rf $(QTQUICK_INSTALL_DIR)

clean: clean-qtquick_install_dir
