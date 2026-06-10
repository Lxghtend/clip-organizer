package main

import "fmt"
import "os"
import "os/exec"
import "time"
import "encoding/json"
import "github.com/fsnotify/fsnotify"

type Window struct {
	Title string `json:"title"`
	InitTitle string `json:"initialTitle"`
	Fullscreen int `json:"fullscreen"`
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
	var win_path string = path + "/" + win.InitTitle
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
	cmd := exec.Command("hyprctl", "activewindow", "-j")
	cmd.Env = os.Environ()
	output, err := cmd.Output()
	
	if (err != nil) {
		return Window{}
	}

	var win Window
	json.Unmarshal(output, &win)
	
	return win
}

func checkForDir(path string) bool {
	info, err := os.Stat(path)

	if (err != nil) {
		return false
	}

	return info.IsDir()
}

func main() {
	var home, err = os.UserHomeDir()
	var path string = home + "/Replays"

	if (err != nil) || !(checkForDir(path)) {
		return
	}
		
	watcher, err := fsnotify.NewWatcher()

	if (err != nil) {
		return
	}

	watcher.Add(path)

	for {
		select {
		case event := <-watcher.Events:
			if (event.Has(fsnotify.Create)) {
				var win Window = getActiveWindow()
					
				if (win.Fullscreen == 2) {
					newPath := handleDirs(path, win)

					var oldName string = event.Name
					var newName string = newPath + "/" + win.InitTitle + "-" + getFormattedTime() + ".mp4"

					os.Rename(oldName, newName)

					fmt.Println(newName)
				}
			}
		}
	}
}
