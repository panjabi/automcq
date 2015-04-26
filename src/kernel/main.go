package main

import (
	"archive/zip"
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

// mainpage of the website
func mainpage(resp http.ResponseWriter, req *http.Request) {
	renderPage(resp, "../ui/main.html")	
}

// endpoint to download mcq template
func downloadTemplate(resp http.ResponseWriter, req *http.Request) {
	data, err := ioutil.ReadFile("../../assets/template.jpg")
	if err != nil {
		log.Println("could not read mcq tamplate: " + err.Error())
		return
	}
	resp.Header().Set("Content-Disposition", "attachment; filename = template.jpg")
	resp.Header().Set("Content-Type", ".jpeg")
	resp.Write(data)
}

// to unzip a zip file
func unzip(src string) error {
    r, err := zip.OpenReader(src)
    if err != nil {
        return err
    }
    defer func() {
        if err := r.Close(); err != nil {
            panic(err)
        }
    }()
    dest := filepath.Dir(src)

    // Closure to address file descriptors issue with all the deferred .Close() methods
    extractAndWriteFile := func(f *zip.File) error {
        rc, err := f.Open()
        if err != nil {
            return err
        }
        defer func() {
            if err := rc.Close(); err != nil {
                panic(err)
            }
        }()
        path := filepath.Join(dest, f.Name)

        if f.FileInfo().IsDir() {
            os.MkdirAll(path, f.Mode())
        } else {
            f, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, f.Mode())
            if err != nil {
                return err
            }
            defer func() {
                if err := f.Close(); err != nil {
                    panic(err)
                }
            }()
            _, err = io.Copy(f, rc)
            if err != nil {
                return err
            }
        }
        return nil
    }

    for _, f := range r.File {
        err := extractAndWriteFile(f)
        if err != nil {
            return err
        }
    }

    return nil
}

// endpoint to handle images .zip file
func handleZip(resp http.ResponseWriter, req *http.Request) {
	reader, err := req.MultipartReader()
	if err != nil {
		http.Error(resp, err.Error(), http.StatusInternalServerError)
		return
	}
	part, err := reader.NextPart()
	if err != nil {
		log.Println("Can not read uploaded file" + err.Error())
		return
	}
	
	tmpDir,_ := ioutil.TempDir(".","")
	fmt.Println(tmpDir)
	dst,_ := os.Create(filepath.Join(tmpDir, part.FileName()))

	// check for the file being a .zip and maximum size of the file
	if part.Header["Content-Type"][0] != "application/zip" {
		return
	}
	io.Copy(dst, part)

	// unzip the zip file
	if err = unzip(filepath.Join(tmpDir,part.FileName())); err != nil {
		http.Error(resp, err.Error(), http.StatusBadRequest)
		return
	}

	// extracting names of files in folder
    files, _ := ioutil.ReadDir(tmpDir)
    for _, file := range files {
    	fmt.Println(file)
	    // add other extensions by using or condition
	    if strings.Contains(file.Name(),".jpg") {
    		// binary kernel
			cmd := exec.Command("./main", "photo")
			err = cmd.Run()
			if err != nil {
				fmt.Println(err.Error())
			}
	    }
    }

	os.RemoveAll(tmpDir)

	// respond with .zip file with results(.csv) in it
}

// rendering HTML page
func renderPage(resp http.ResponseWriter, temp string){
	t, err := template.ParseFiles(temp)
	if err != nil {
		log.Println("Could not load htmlpage: " + err.Error())
		return
	}
	t.Execute(resp,"")
}

func main() {

	http.HandleFunc("/", mainpage)
	http.HandleFunc("/downloadtemplate", downloadTemplate)
	http.HandleFunc("/handlezip",handleZip)
	http.ListenAndServe(":8080", nil)
}