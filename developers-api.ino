#include <ESPmDNS.h>
#include <WiFi.h>
#include <ArduinoNvs.h>
#include <aJSON.h>
#include "aWOT.h"

const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPass";

WiFiServer server(80);
Application app;

bool ok; 
char* developerFilter[7] = {"id", "name", "team", "skills", "createdAt", "updatedAt", NULL};
String notFound = "{\"errorType\": \"notFound\",\"message\": \"No name found\"}";
String internalError = "{\"errorType\": \"internalError\",\"message\": \"An internal error has occured\"}";
String badRequst = "{\"errorType\": \"badRequest\",\"message\": \"Application received a bad request\"}";

void send400(Response & res){
  res.set("Content-Type", "application/json");
  res.status(404);
  res.print(badRequst);
}

void send404(Response & res){
  res.set("Content-Type", "application/json");
  res.status(404);
  res.print(notFound);
}

void send500(Response & res){
  res.set("Content-Type", "application/json");
  res.status(500);
  res.print(internalError);
}

void send204(Response & res){
  res.sendStatus(204);
}


/*
 * Persistence Helpers Below
 */
String getDeveloper(String id){
  Serial.println("Retrieving Developer with ID: " + id);
  return NVS.getString (id);
}

bool saveDeveloper(aJsonObject* developer) {
  String body = aJson.print(developer);
  String id = getDeveloperId(developer);
  Serial.println("Storing Developer with ID: " + id);
  return NVS.setString (id, body);
}

bool removeDeveloper(String id) {
  Serial.println("Deleting Developer with ID: " + id);
  return NVS.erase (id);
}

/*
 * JSON Helper Functions below
 */
char* getDeveloperId(aJsonObject* developer) {
  return aJson.getObjectItem(developer, "id")->valuestring;
}

char* validateDeveloper(aJsonObject* developer){ 
  aJsonObject* id = aJson.getObjectItem(developer, "id");
  if (!id || id->type != aJson_String) {
    return "{\"errorType\": \"BadRequest\",\"message\": \"Body missing field: id\"}";
  }

  aJsonObject* name = aJson.getObjectItem(developer, "name");
  if (!name || name->type != aJson_String) {
    return "{\"errorType\": \"BadRequest\",\"message\": \"Body missing field: name\"}"; 
  }

  aJsonObject* team = aJson.getObjectItem(developer, "team");
  if (!team || team->type != aJson_String) {
    return "{\"errorType\": \"BadRequest\",\"message\": \"Body missing field: team\"}"; 
  }

  aJsonObject* createdAt = aJson.getObjectItem(developer, "createdAt");
  if (!createdAt || createdAt->type != aJson_String) {
    return "{\"errorType\": \"BadRequest\",\"message\": \"Body missing field: createdAt\"}"; 
  }

  aJsonObject* updatedAt = aJson.getObjectItem(developer, "updatedAt");
  if (!updatedAt || updatedAt->type != aJson_String) {
    return "{\"errorType\": \"BadRequest\",\"message\": \"Body missing field: updatedAt\"}";
  }

  return NULL;
}

/*
 * Routing Functions Below
 */
// GET developers
void searchDevelopers(Request & req, Response & res) {
  // Set Response
  res.set("Content-Type", "application/json");
  res.sendStatus(501);
}

// GET developers/{id}
void readDeveloper(Request & req, Response & res) {
  // Parse Path Param
  char devId[64];
  req.route("devId", devId, 64);

  // Retrieve developer from NVS
  String developer = getDeveloper(devId);
  if (developer == NULL) {
    send404(res);
  }

  // Set Response
  res.set("Content-Type", "application/json");
  res.status(200);
  res.print(developer);
}

// POST developers
void createDeveloper(Request & req, Response & res) {
  aJsonStream stream(&req);

  aJsonObject* developer = aJson.parse(&stream, developerFilter);
  if (!developer) {
    return res.sendStatus(400);
  }

  String error = validateDeveloper(developer);
  if (error != NULL){
    send400(res);
    return;
  }
  
  if (!saveDeveloper(developer)){
    send500(res);
    return;
  }

  res.set("Content-Type", "application/json");
  res.set("Location", getDeveloperId(developer));
  res.status(201);
  aJson.print(developer, &stream);
}

// PATCH developers/{id}
void updateDeveloper(Request & req, Response & res) {
  // Parse Path Param
  char devId[64];
  req.route("devId", devId, 64);

  // Retrieve developer from NVS
  String developer = getDeveloper(devId);
  if (developer == NULL) {
    send404(res);
    return;
  }

  return createDeveloper(req, res);
}

//DELETE developers/{id}
void deleteDeveloper(Request & req, Response & res) {
  // Parse Path Param
  char devId[64];
  req.route("devId", devId, 64);

  // Retrieve developer from NVS
  String developer = getDeveloper(devId);
  if (developer == NULL) {
    send404(res);
    return;
  }

  // Delete from NVS
  if (!removeDeveloper(devId)) {
    send500(res);
  }
  
  send204(res);
  return;
}

/*
 * Arduino Functions Below
 */

void setup() {
  Serial.begin(115200);

  NVS.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());

  if(!MDNS.begin("micromicro")) {
     Serial.println("Error starting mDNS");
     return;
  }

  app.post("/developers", &createDeveloper);
  app.get("/developers", &searchDevelopers);
  app.get("/developers/:devId", &readDeveloper);
  app.patch("/developers/:devId", &updateDeveloper);
  app.del("/developers/:devId", &deleteDeveloper);  

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client.connected()) {
    app.process(&client);
  }
}