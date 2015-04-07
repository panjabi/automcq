package main

import (
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"log"
	"net/http"
	"os"
)

// mainpage of the website
func mainpage(resp http.ResponseWriter, req *http.Request) {
	renderPage(resp, "../html/main.html")	
}

// endpoint to download mcq template
func downloadTemplate(resp http.ResponseWriter, req *http.Request) {
	data, err := ioutil.ReadFile("../assets/mcqtemplate.jpg")
	if err != nil {
		log.Println("could not read mcq tamplate: " + err.Error())
		return
	}
	resp.Header().Set("Content-Disposition", "attachment; filename = template.jpg")
	resp.Header().Set("Content-Type", ".jpeg")
	resp.Write(data)
}

// endpoint to handle images .zip file
func handleZip(resp http.ResponseWriter, req *http.Request) {
	reader, err := req.MultipartReader()
	if err != nil {
		http.Error(resp, err.Error(), http.StatusInternalServerError)
		return
	}
	part, _ := reader.NextPart()
	tmpDir,_ := ioutil.TempDir("",part.FileName())
	dst,_ := os.Create(tmpDir)
	fmt.Println(tmpDir)
	io.Copy(dst, part)
	// unzip
	// python kernel
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