#include "../inc/Response.hpp"

void Response::composeResponse(){
	// getPath voor welke html page > wordt de body van response
	statusLine = statusLine + httpRequest.getStatusCode();
}