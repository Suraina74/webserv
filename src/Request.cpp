#include "../inc/Request.hpp"

// void Request::extractReqLine(){
// 	int startGet = Input.find("GET");
// 	if (startGet){
// 		requestLine = Input.substr(startGet, );
// 	}
// }

// GET / HTTP/1.1 niets na /
// GET /index.html HTTP/1.1 specifieke html page na /
// GET /style.css HTTP/1.1
// GET /favicon.ico HTTP/1.1
// POST /delete.html HTTP/1.1 met body filename=


//Lijst van html pages maken. Array of strings met de namen en dan kijken of the html page na / in de lijst staat.
//Als er niets na / komt, dan moet de index page worden getoond

// Met GET weet je dat je alleen de gevraagde html page als string moet meegeven aan send.
// Met POST moet er ook nog iets worden gedaan met de file (wat in de body staat).

// Met GET een string returnen met de html page waarom wordt gevraagd.