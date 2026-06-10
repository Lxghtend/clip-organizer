package main

import "fmt"
import "os"
import "os/exec"
import "time"
import "encoding/json"

type Window struct {
	Title string `json:"title"`
	Fullscreen int `json:"Fullscreen"`
}

func getFormattedTime() string {
	current_time := time.Now()
	
	return (current_time.Format("January-02-2006"))
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

	if err != nil {
		return false
	}

	return info.IsDir()
}

func main() {
	var home, err = os.UserHomeDir()
	
	if (err != nil) {
		return
	}

	var path string = home + "/Replays"
				
	if (checkForDir(path)) {
		err = os.Mkdir(path + "/hi", 0755)
	}

	fmt.Println(getFormattedTime())

	win := getActiveWindow()
	fmt.Println(win.Title)
}
