package main

import (
	"encoding/json"
	"fmt"
	"os"
	"os/exec"
	"time"

	"github.com/fsnotify/fsnotify"
)

type Layout struct {
	WindowSize [2]int `json:"window_size"`
}

type Window struct {
	Title  string `json:"title"`
	AppID  string `json:"app_id"`
	Layout Layout `json:"layout"`
	//Fullscreen int `json:"fullscreen"`
}

type Mode struct {
	Width  int `json:"width"`
	Height int `json:"height"`
}

type Output struct {
	Modes       []Mode `json:"modes"`
	CurrentMode int    `json:"current_mode"`
}

func getDay() string {
	current_time := time.Now()

	return (current_time.Format("January-02"))
}

func getFormattedTime() string {
	current_time := time.Now()

	return (current_time.Format("2006-01-02_15-04-05"))
}

func handleDirs(path string, win Window) string {
	var win_path string = path + "/" + win.AppID
	var dated_win_path string = win_path + "/" + getDay()

	if !(checkForDir(win_path)) {
		os.Mkdir(win_path, 0755)
	}

	if !(checkForDir(dated_win_path)) {
		os.Mkdir(dated_win_path, 0755)
	}

	return dated_win_path
}

func getActiveWindow() Window {
	cmd := exec.Command("niri", "msg", "-j", "focused-window")
	cmd.Env = os.Environ()
	output, err := cmd.Output()

	if err != nil {
		return Window{}
	}

	var win Window
	json.Unmarshal(output, &win)

	return win
}

func checkForDir(path string) bool {
	info, err := os.Stat(path)

	if err != nil {
		return false
	}

	return info.IsDir()
}

func getScreenSize() [2]int {
	cmd := exec.Command("niri", "msg", "-j", "focused-output")
	cmd.Env = os.Environ()
	coutput, err := cmd.Output()

	if err != nil {
		return [2]int{0, 0}
	}

	var out Output
	json.Unmarshal(coutput, &out)

	xy := out.Modes[out.CurrentMode]

	return [2]int{xy.Width, xy.Height}
}

func checkFullscreen(win Window) bool {
	screen := getScreenSize()
	if screen == win.Layout.WindowSize {
		return true
	}

	return false
}

func main() {
	var home, err = os.UserHomeDir()
	var path string = home + "/Replays"

	if (err != nil) || !(checkForDir(path)) {
		return
	}

	watcher, err := fsnotify.NewWatcher()

	if err != nil {
		return
	}

	watcher.Add(path)

	for {
		select {
		case event := <-watcher.Events:
			if event.Has(fsnotify.Create) {
				var win Window = getActiveWindow()

				if checkFullscreen(win) {
					newPath := handleDirs(path, win)

					var oldName string = event.Name

					var newName string = newPath + "/" + win.AppID + "-" + getFormattedTime() + ".mp4"

					os.Rename(oldName, newName)

					fmt.Println(newName)
				}
			}
		}
	}
}
