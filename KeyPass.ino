  /* LIBS */
  #include <SPFD5408_Adafruit_GFX.h>    // Core graphics library 
  #include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
  #include <SPFD5408_TouchScreen.h> //TouchScreen

  #include <ListLib.h> //Gestor de arrays
  #include <Keyboard.h>
  /* END LIBS */

  /* PINS */
  #define LCD_CS A3 // Chip Select goes to Analog 3
  #define LCD_CD A2 // Command/Data goes to Analog 2
  #define LCD_WR A1 // LCD Write goes to Analog 1
  #define LCD_RD A0 // LCD Read goes to Analog 0
  #define LCD_RESET A4

  /* END PINS */
  Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

  //COLORS
  #define	BLACK   0x0000
  #define	BLUE    0x001F
  #define	RED     0xF800
  #define	GREEN   0x07E0
  #define CYAN    0x07FF
  #define MAGENTA 0xF81F
  #define YELLOW  0xFFE0
  #define WHITE   0xFFFF

  // TOUCH 
  #define YP A1  // must be an analog pin, use "An" notation!
  #define XM A2  // must be an analog pin, use "An" notation!
  #define YM 7   // can be a digital pin
  #define XP 6   // can be a digital pin

  #define MINPRESSURE 10
  #define MAXPRESSURE 1000

  // Calibrate values
  #define TS_MINX 125
  #define TS_MINY 85
  #define TS_MAXX 965
  #define TS_MAXY 905
  TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

  //Interface
  #define BOXSIZE 50
  #define PENRADIUS 3

  //DATA BASE

  List<int> dbservice;
  List<int> dbusername;
  List<String> dbpassword;
  List<int> page (6); // page def
  int Cursor = 0;

void setup() { //Init Device
  bootLoader();
}

void loop() { //Update and Events 
  getTouchData();  
  loopLoader();
}

void bootLoader(){ //metodo de boot.
    tft.reset(); 
    displayInit();  

    loadDataBase();
    initialScreen();
    Keyboard.begin();
    Serial.begin(9600);
}

void loopLoader(){ // metodo de montar loop
    ui_loop_button_next();
    ui_loop_button_back();
    ui_loop_button_exec();
    ui_loop_button_load();
}


String recortarChar(char caracter, int posicion, int longitud) {
  // Convierte el caracter en una cadena
  String cadena = String(caracter);
  // Recorta la cadena y devuelve el trozo recortado
  return cadena.substring(posicion, longitud);
}

//encrypt
void aes256_enc_single(const uint8_t* key,  void* data);
void aes256_des_single(const uint8_t* key,  void* data);

//DISPLAY
void displayInit(){      
  tft.begin(0x9341);
  tft.setRotation(0); // Need for the Mega, please changed for your choice or rotation initial
}

TSPoint p;
void getTouchData(){ // obtener la informacion de la pantalla tactil
  p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { //si hay presion actualiza la pos de x y
     p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width()); 
     p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
  }
}

void initialScreen(){
    tft.fillScreen(WHITE);
    drawBorder();
    
    tft.setCursor (170,20);
    tft.setTextSize (1);
    tft.setTextColor(BLACK);
    tft.println("VaOs 0.01");
    ui_botons();
}

void ui_botons(){
  tft.fillRect( 10, 260 , BOXSIZE, BOXSIZE, BLUE); 
  tft.fillRect( 60, 260 , BOXSIZE, BOXSIZE, BLUE);
  tft.fillRect( 110, 260 , BOXSIZE, BOXSIZE, RED);
  tft.fillRect( 160, 260 , 70, BOXSIZE, RED);
  //Labels
  tft.setCursor(20,275);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.println("<");

  tft.setCursor(70,275);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.println(">");

  tft.setCursor(125,275);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.println("Do");

  tft.setCursor(175,275);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.println("Send");
}

//Eventos
bool validLoopEventYX( int Ymin , int Ymax, int Xmin, int Xmax ){
  if(p.z > MINPRESSURE && p.z < MAXPRESSURE){
        if( p.y > Ymin && p.y  < Ymax ){
          if(  p.x > Xmin  && p.x  <  Xmax ){
             return true;
          }else{
            return false;
          }     
        }else{
            return false;
        }
   }else {
     return false;
   }
}

//Botones
void ui_loop_button_back(){ // boton atras
  if( validLoopEventYX( 0 , 50 , 170 , 210 ) == true ){
      navegatePage(false);
      delay(1000);
    }
}

void ui_loop_button_next(){ // boton adelante
  if( validLoopEventYX( 0 , 50 , 130, 170 ) == true ){
      navegatePage(true);
      delay(1000);
    }
}

void ui_loop_button_exec(){ // boton adelante
  if( validLoopEventYX( 0 , 50 , 90 , 120  ) == true ){
      Serial.println("Down!");
      delay(1000);
    }
}

void ui_loop_button_load(){ // boton adelante
  if( validLoopEventYX( 0 , 50 , 20, 70 ) == true ){
      Serial.println("Send!");
      Keyboard.print ( dbpassword[Cursor]  );
      delay(1000);
   }
}

void seeXYZtouch(){
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
      Serial.print("X = "); Serial.print(p.x);
      Serial.print("\tY = "); Serial.print(p.y);
      Serial.print("\tPressure = "); 
      Serial.println(p.z);
    }
}

const unsigned int MAX_MESSAGE_LENGTH = 12;
void terminal(){
   while (Serial.available() > 0){
    //Create a place to hold the incoming message
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    //Read the next available byte in the serial receive buffer
    char inByte = Serial.read();

    //Message coming in (check not terminating character) and guard for over message size
    if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) ){
      //Add the incoming byte to our message
      message[message_pos] = inByte;
      message_pos++;
    }
    //Full message received...
    else{
      //Add null character to string
      message[message_pos] = '\0';

      //Print the message (or do other things)
      Serial.println(message);
      //Reset for the next message
      message_pos = 0;
    }
  }
}

void progmemPrint(const char *str) {
  char c;
  while(c = pgm_read_byte(str++)) Serial.print(c);
}

void progmemPrintln(const char *str) {
  progmemPrint(str);
  Serial.println();
}

void display_log_str(String msg ){
  tft.setTextSize(2); // Especificamos el tamaño del texto
  tft.println(msg); // Escribimos por pantalla  
}

void display_log(const char *msg ){
  tft.println(msg); // Escribimos por pantalla  
}

void drawBorder() {
  // Draw a border
  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;
  uint8_t border = 10;

  tft.fillScreen(BLACK);
  tft.fillRect(border, border, (width - border * 2), (height - border * 2), WHITE);  
}

void navegatePage( bool desc ){ 

  Serial.println("a");
  int Min = 0;
  int Max = dbusername.Count() - 1;

  if( desc ){
    if( Cursor < Max ){
      Cursor++;
    }
  } else {
    if( Cursor > Min ){
      Cursor--;
    }
  }  
  showAccountListPage();

}

void showAccountListPage(){ 
    tft.fillRect( 20, 70 , 208, 80, WHITE );  //clean display
    tft.setTextColor(BLACK);
    tft.setCursor(40, 70);
    tft.setTextSize(3);     
    display_log( dbservice[Cursor] );
    tft.setCursor (40, 90);
    tft.setTextSize (2);      
    display_log( dbusername[Cursor] ); // Escribimos por pantalla  
}

void newAccount(const char *service, const char *username, String password){ 
    dbservice.Add( service );
    dbusername.Add( username );
    dbpassword.Add( password );
}

void loadDataBase(){

    //gmail
    newAccount("gmail","miaw@gmail.com", "$hola#!-pass2");

    //facebook
    newAccount("facebook","miaw@gmail.com", "$hola#!-pass2");

    //instagram
    newAccount("instagram","miaw@gmail.com", "$hola#!-pass2");

    //twitter
    newAccount("twitter","miaw@gmail.com", "$hola#!-pass2");

}