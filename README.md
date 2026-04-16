<h1 align="center">.dotfiles</h1>

<p align="center">
  <img src="https://img.shields.io/badge/Arch-Linux-1793D1?logo=archlinux&logoColor=white" />
  <img src="https://img.shields.io/badge/Wayfire-Compositor-f38ba8" />
  <img src="https://img.shields.io/badge/Eww-Widgets-a6e3a1?logo=gtk&logoColor=white" />
  <img src="https://img.shields.io/badge/Catppuccin-Mocha-f5e0dc?logo=data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iMjQiIGhlaWdodD0iMjQiIHZpZXdCb3g9IjAgMCAyNCAyNCIgZmlsbD0ibm9uZSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIgZmlsbD0iIzExMTExYiIvPjwvc3ZnPg==" />
  <img src="https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/License-MIT-89b4fa" />
</p>

<p align="center">
  Custom <a href="https://github.com/elkowar/eww">Eww</a> widgets for Wayland — a full desktop shell with a taskbar, start menu, app launcher, dashboard, and Instagram DM integration. Built for <a href="https://wayfire.org/">Wayfire</a> with Catppuccin Mocha theming.
</p>

---

### Desktop

<img src="screenshots/desktop.png" />

### Dashboard

<img src="screenshots/startmenu.png" />

### App Search

<img src="screenshots/search.png" />

### In Use

<img src="screenshots/busy.png" />

<details>
<summary>More screenshots</summary>

### Neofetch

<img src="screenshots/neofetch.png" />

### Wobbly Windows

<img src="screenshots/wobbly.png" />

</details>

---

## Features

- Bottom bar with weather, taskbar icons (open/minimized/closed state), and start menu toggle
- Start menu with keyboard-navigable fuzzy app search (KMP matching), submenu options per app, and a keybind legend
- Dashboard with clock, weather, music player controls (playerctl), CPU/RAM/disk/network stats, recent apps, quick links, and Instagram DMs
- Wayland-native app launcher (`wl-launch`) with `xdg-activation-v1` startup notification cursor
- Pacman hook to auto-refresh the app cache on install/remove

## Dependencies

| Dependency | Purpose |
|---|---|
| [eww](https://github.com/elkowar/eww) (Wayland) | Widget system |
| [Wayfire](https://wayfire.org/) + `wfctl` | Compositor, window management |
| `g++`, `gcc`, `make` | Building C/C++ helpers |
| `boost` (filesystem) | Desktop file scanning |
| `wayland-client` | `wl-launch` startup notifications |
| `playerctl` | Music controls |
| `perl` + `Linux::DesktopFiles`, `Tree::Trie`, `JSON`, `File::Slurp` | App search |
| `python3` + `wayfire` module | Taskbar daemon |
| Hack, Nimbus Sans, Montserrat, Verdana | Fonts |

`nlohmann/json` is vendored in `scripts/nlohmann/`.

## Install

```bash
git clone https://github.com/killmlana/.dotfiles.git
cd .dotfiles
./install.sh
```

Or manually:

```bash
make all
make install
```

Installs to `~/.config/eww/`.

## Configuration

Before running, edit these for your setup:

| File | What to change |
|---|---|
| `eww.yuck` | Social links, monitor width (`1920px`) in `bartest` and `dismiss-layer` |
| `scripts/taskbar-daemon` | `OUT_W`, `OUT_H`, `OUTPUT_NAME` |
| `scripts/taskbar-ctl.cpp` | Resolution, output name |
| `scripts/dashboard/insta-*` | Instagram usernames and thread ID |
| `scripts/weather_day`, `weatherText`, `temperature` | RapidAPI key and city |
| `scripts/eww-app-cache.hook` | Absolute path (for pacman hook) |

The taskbar app list lives in `eww.yuck` (`defvar tasklist`) and the `APPS` maps in `scripts/taskbar-daemon` and `scripts/taskbar-ctl.cpp`.

## Usage

```bash
eww daemon
eww open bartest
```

The start menu is toggled by `scripts/toggle-startmenu`. Bind it to a key in your Wayfire config.

## Pacman Hook

Auto-refresh the app cache when packages change:

```bash
sudo cp ~/.config/eww/scripts/eww-app-cache.hook /etc/pacman.d/hooks/eww-app-cache.hook
```

Edit the `Exec` path in the hook to your install location.

## Tech Stack

| Component | Technology |
|---|---|
| Widgets | Eww (Yuck + SCSS) |
| Compositor | Wayfire |
| App search | Perl (KMP matching) |
| Keyboard nav | C++ FIFO daemon |
| Taskbar | Python + Wayfire IPC |
| App launcher | C + Wayland `xdg-activation-v1` |
| DM integration | Python + Chrome DevTools Protocol |
| Theme | Catppuccin Mocha |

## License

MIT
