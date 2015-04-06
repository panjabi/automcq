package main

import (
	// "fmt"
	"html/template"
	"io/ioutil"
	"log"
	"net/http"
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
	}
	resp.Header().Set("Content-Disposition", "attachment; filename = template.jpg")
	resp.Header().Set("Content-Type", ".jpeg")
	resp.Write(data)
}

// rendering HTML page
func renderPage(resp http.ResponseWriter, temp string){
	t, err := template.ParseFiles(temp)
	if err != nil {
		log.Println("Could not load htmlpage: " + err.Error())
	}
	t.Execute(resp,"")
}
func main() {
	http.HandleFunc("/", mainpage)
	http.HandleFunc("/downloadTemplate", downloadTemplate)
	http.ListenAndServe(":8080", nil)
}