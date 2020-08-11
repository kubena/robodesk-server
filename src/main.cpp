#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <pins.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

ESP8266WebServer server(80);

int btn_pins[] = {BTN_UP, BTN_DOWN};
int assert_pins[] = {ASSERT_UP, ASSERT_DOWN};
bool override_movement = false;

// function prototypes for HTTP handlers
void handleRoot();              
void handleNotFound();
void handleStepMove();
void handleContinuousMove();
// function prototypes for helpers
void movingUp();
void movingDown();
void movingStop();


void setup(void){
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');
  //init
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(ASSERT_UP, OUTPUT);
  pinMode(ASSERT_DOWN, OUTPUT);

  Serial.print("Setting Access Point …");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/step", HTTP_GET, handleStepMove);
  server.on("/move", HTTP_GET, handleContinuousMove);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  if (not override_movement) {
    for(uint8_t i=0; i < ARRAY_SIZE(btn_pins); ++i) {
      digitalWrite(assert_pins[i], digitalRead(btn_pins[i]));
    }
  }
}

void handleRoot() {
  String status = "manual input status on page load: BTN UP: " + String(digitalRead(BTN_UP)) + " BTN DOWN: " + String(digitalRead(BTN_DOWN));

  server.send(200, "text/html",
  "<!DOCTYPE html><html>"
  "<head><style>"
  " html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}"
  " .button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;"
  " text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}"
  "</style></head>"
  "<body>"
  "<h1>ONovy controller</h1>"
  "<button type=\"button\" class=\"button\" onclick=\"loadContinuousMove('down')\">DOWN</button>"
  "<button type=\"button\" class=\"button\" onclick=\"loadStepMove('down')\">DOWN 3s</button>"
  "<button type=\"button\" class=\"button\" onclick=\"loadContinuousMove('stop')\">STOP</button>"
  "<button type=\"button\" class=\"button\" onclick=\"loadStepMove('up')\">UP 3s</button>"
  "<button type=\"button\" class=\"button\" onclick=\"loadContinuousMove('up')\">UP !</button>"
  "<br>"
  + status +
  "<script>"
  "  function loadStepMove(direction) {"
  "    var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"step?direction=\" + direction, true); xhttp.send(); }"
  "  function loadContinuousMove(direction) {"
  "    var xhttp = new XMLHttpRequest(); xhttp.open(\"GET\", \"move?direction=\" + direction, true); xhttp.send(); }"
  "</script>"
  "</body></html>");
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");
}

void handleStepMove() {
  override_movement = false;
  String direction = server.arg("direction");
  int duration_ms = server.arg("duration").toInt() * 1000;
  if(duration_ms == 0){
    duration_ms = 3000;
  }

  if( direction == "up" ){
    movingUp();
    delay(duration_ms);
    movingStop();
    server.send(200);
  }else if( direction == "down" ){
    movingDown();
    delay(duration_ms);
    movingStop();
    server.send(200);
  }else{
    server.send(404);
  }
}

void handleContinuousMove() {
  String direction = server.arg("direction");

  if( direction == "up" ){
    override_movement = true;
    movingUp();
    server.send(200);
  }else if( direction == "down" ){
    override_movement = true;
    movingDown();
    server.send(200);
  }else if( direction == "stop" ){
    override_movement = false;
    movingStop();
    server.send(200);
  }else{
    server.send(404);
  }
}

void movingUp()
{
  Serial.println("Table moving up");
  digitalWrite(ASSERT_UP, HIGH);
  digitalWrite(ASSERT_DOWN, LOW);
}

void movingDown()
{
  Serial.println("Table moving down");
  digitalWrite(ASSERT_UP, LOW);
  digitalWrite(ASSERT_DOWN, HIGH);
}

void movingStop()
{
  digitalWrite(ASSERT_UP, LOW);
  digitalWrite(ASSERT_DOWN, LOW);
  Serial.println("Table stop moving");
}
