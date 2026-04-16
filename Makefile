CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -O2
SCRIPTS = scripts

NLOHMANN = -I$(SCRIPTS)
BOOST_FS = -lboost_filesystem -lboost_system

.PHONY: all clean install

all: \
	$(SCRIPTS)/selection-info.out \
	$(SCRIPTS)/search-result-menu-init.out \
	$(SCRIPTS)/search-info-menu-init.out \
	$(SCRIPTS)/active-selection-info-menu-init.out \
	$(SCRIPTS)/kbselection-daemon \
	$(SCRIPTS)/taskbar-ctl.out \
	$(SCRIPTS)/wl-launch \
	$(SCRIPTS)/a.out

$(SCRIPTS)/selection-info.out: $(SCRIPTS)/selection-info.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) -I$(SCRIPTS)/inipp $< -o $@ $(BOOST_FS)

$(SCRIPTS)/search-result-menu-init.out: $(SCRIPTS)/search-result-menu-init.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

$(SCRIPTS)/search-info-menu-init.out: $(SCRIPTS)/search-info-menu-init.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

$(SCRIPTS)/active-selection-info-menu-init.out: $(SCRIPTS)/active-selection-info-menu-init.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

$(SCRIPTS)/kbselection-daemon: $(SCRIPTS)/kbselection-daemon.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

$(SCRIPTS)/taskbar-ctl.out: $(SCRIPTS)/taskbar-ctl.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

$(SCRIPTS)/wl-launch: $(SCRIPTS)/wl-launch.c
	$(CC) -O2 $< -o $@ -lwayland-client

$(SCRIPTS)/a.out: $(SCRIPTS)/kbselection.cpp
	$(CXX) $(CXXFLAGS) $(NLOHMANN) $< -o $@

clean:
	rm -f $(SCRIPTS)/*.out $(SCRIPTS)/wl-launch $(SCRIPTS)/kbselection-daemon

install: all
	mkdir -p ~/.config/eww/scripts/dashboard
	mkdir -p ~/.config/eww/images
	mkdir -p ~/.config/eww/cache
	cp eww.yuck eww.scss ~/.config/eww/
	cp -r images/* ~/.config/eww/images/
	cp -r scripts/* ~/.config/eww/scripts/
	chmod +x ~/.config/eww/scripts/*
	chmod +x ~/.config/eww/scripts/dashboard/*
	@echo ""
	@echo "Installed to ~/.config/eww"
	@echo ""
	@echo "Before using, edit these files for your setup:"
	@echo "  eww.yuck         - social links, monitor resolution (1920x1080)"
	@echo "  scripts/taskbar-daemon    - OUT_W/OUT_H, OUTPUT_NAME (DP-1)"
	@echo "  scripts/taskbar-ctl.cpp   - resolution, output name, wfctl path"
	@echo "  scripts/dashboard/insta-* - Instagram usernames"
	@echo ""
	@echo "Then rebuild with: cd ~/.config/eww && make"
