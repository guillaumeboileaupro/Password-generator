APP=mdp-generator
CXX ?= g++
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
DESKTOPDIR ?= $(PREFIX)/share/applications
ICONDIR ?= $(PREFIX)/share/icons/hicolor/256x256/apps
CPPFLAGS += -I.
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)
APP_SOURCES = mdp_gui.cpp password_core.cpp
TEST_APP = password-core-tests
TEST_SOURCES = tests/password_core_test.cpp password_core.cpp
CORE_LIBS = -lssl -lcrypto
COVERAGE_MIN ?= 95
APP_CXXFLAGS += $(GTK_CFLAGS)
APP_LDFLAGS += $(GTK_LIBS) -lasound $(CORE_LIBS)
TEST_LDFLAGS += $(CORE_LIBS)

.PHONY: all test coverage coverage-check install clean deb snap

all: $(APP)

$(APP): $(APP_SOURCES) password_core.h
	$(CXX) $(CPPFLAGS) $(APP_CXXFLAGS) $(APP_SOURCES) -o $(APP) $(APP_LDFLAGS)

$(TEST_APP): $(TEST_SOURCES) password_core.h
	$(CXX) $(CPPFLAGS) $(TEST_SOURCES) -o $(TEST_APP) $(TEST_LDFLAGS)

test: $(TEST_APP)
	./$(TEST_APP)

coverage: CPPFLAGS += --coverage
coverage: APP_CXXFLAGS += --coverage
coverage: TEST_LDFLAGS += --coverage
coverage: clean $(TEST_APP)
	./$(TEST_APP)
	bash scripts/coverage_report.sh

coverage-check: CPPFLAGS += --coverage
coverage-check: APP_CXXFLAGS += --coverage
coverage-check: TEST_LDFLAGS += --coverage
coverage-check: clean $(TEST_APP)
	./$(TEST_APP)
	bash scripts/coverage_report.sh $(COVERAGE_MIN)

install: all
	install -d $(DESTDIR)$(BINDIR)
	install -m 0755 $(APP) $(DESTDIR)$(BINDIR)/$(APP)
	install -d $(DESTDIR)$(DESKTOPDIR)
	install -m 0644 mdp-generator.desktop $(DESTDIR)$(DESKTOPDIR)/mdp-generator.desktop
	install -d $(DESTDIR)$(ICONDIR)
	install -m 0644 mdp-logo.png $(DESTDIR)$(ICONDIR)/mdp-logo.png

clean:
	rm -f $(APP) $(TEST_APP) *.gcda *.gcno *.gcov *-*.gcda *-*.gcno *-*.gcov tests/*.gcda tests/*.gcno tests/*.gcov

deb: all
	./build_deb.sh

snap:
	snapcraft
