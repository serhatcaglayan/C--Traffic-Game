#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <ncurses.h>
#include <iostream>
#include <queue>
#include <ctime>
#include <string.h>

#define wHeight 40 // height of the road
#define wWidth 100 // width of the road
#define lineX 45 // x coordinate of the middle line
#define lineLEN 10 // distance of the middle line from the beginning and the end
#define EXITY 35 // coordinate showing the end of the road
#define leftKeyArrow 260 // ASCII code of the left arrow key
#define RightKeyArrow 261 // ASCII code of the right arrow key
#define leftKeyA 97 // ASCII code of A
#define RightKeyD 100 // ASCII code of D
#define ESC 27 // ASCII code of the ESC key
#define ENTER 10 // ASCII code of the ENTER key
#define KEYPUP 259 // ASCII code of the up arrow key
#define KEYDOWN 258 // ASCII code of the down arrow key
#define KEYERROR -1 // ASCII code returned if an incorrect key is pressed
#define SAVEKEY 115 // ASCII code of S
#define levelBound 300 // To increase level after 300 points
#define MAXSLEVEL 5 // maximum level
#define ISPEED 500000 // initial value for game moveSpeed
#define DRATESPEED 100000 // to decrease moveSpeed after each new level
#define MINX 5 // minimum x coordinate value when creating cars
#define MINY 10 // the maximum y coordinate value when creating the cars, then we multiply it by -1 and take its inverse
#define MINH 5 // minimum height when creating cars
#define MINW 5 // minimum width when creating cars
#define SPEEDOFCAR 3 // speed of the car driven by the player
#define YOFCAR 34 // y coordinate of the car used by the player
#define XOFCAR 45 // x coordinate of the car used by the player
#define IDSTART 10 // initial value for cars ID
#define IDMAX 20 // maximum value for cars ID
#define COLOROFCAR 3 // color value of the car used by the player
#define POINTX 91 // x coordinate where the point is written
#define POINTY 42 // y coordinate where the point is written
#define MENUX 10 // x coordinate for the starting row of the menus
#define MENUY 5 // y coordinate for the starting row of the menus
#define MENUDIF 2 // difference between menu rows
#define MENUDIFX 20 // difference between menu columns
#define MENSLEEPRATE 200000 // sleep time for menu input
#define GAMESLEEPRATE 250000 // sleep time for player arrow keys
#define EnQueueSleep 1 // EnQueue sleep time
#define DeQueueSleepMin 2 // DeQueue minimum sleep time
#define numOfcolors 4 // maximum color value that can be selected for cars
#define maxCarNumber 5 // maximum number of cars in the queue
#define numOfChars 3 // maximum number of patterns that can be selected for cars
#define settingMenuItem 2 // number of options in the setting menu
#define mainMenuItem 6 // number of options in the main menu

using namespace std;

typedef struct Car {
    int ID;
    int x;
    int y;
    int height;
    int width;
    int speed;
    int clr;
    bool isExist;
    char chr;
} Car;

typedef struct Game {
    int leftKey;
    int rightKey;
    queue<Car> cars;
    queue<Car> serhat;
    bool IsGameRunning;
    bool IsSaveClicked;
    int counter;
    pthread_mutex_t mutexFile;
    Car current;
    int level;
    int moveSpeed;
    int points;
} Game;

Game playingGame; // Global variable used for new game


const char *gameTxt = "game.txt";
const char *CarsTxt = "cars.txt";
const char *pointsTxt = "points.txt";

// Array with options for the Setting menu
const char *settingMenu[50] = {"Play with < and > arrow keys", "Play with A and D keys"};
// Array with options for the Main menu
const char *mainMenu[50] = {"New Game", "Load the last game", "Instructions", "Settings", "Points", "Exit"};

const char shapesCar[numOfChars] ={'*', '#', '+'};

void drawCar(Car c, int type, int direction ); // prints or removes the given car on the screen
void printWindow(); // Draws the road on the screen
void *newGame(void *); // manages new game
void initGame(); // Assigns initial values to all control parameters for the new game
void initWindow(); // Creates a new window and sets I/O settings
void *enqueueCar(void *);
void *dequeueCar(void*);
void *moveCar(void* car);
bool checkCollision(Car c1, Car c2); 
void mainMenuScreen();
void instructionsScreen();
void pointsScreen();
void settingsMenuScreen();
void saveGame();
void writePoint();
void loadGame();
void startGame();
void loadGame();


int main() {

    srand(time(NULL));
    playingGame.leftKey = leftKeyArrow;
    playingGame.rightKey = RightKeyArrow;

    initGame();
    initWindow();
    mainMenuScreen();

    endwin();
    return 0;
}

void initGame() {
    playingGame.cars = queue<Car>();
    playingGame.counter = IDSTART;
    playingGame.mutexFile = PTHREAD_MUTEX_INITIALIZER; // assigns the initial value for the mutex
    playingGame.level = 1;
    playingGame.moveSpeed = ISPEED;
    playingGame.points = 0;
    playingGame.IsSaveClicked = false;
    playingGame.IsGameRunning = true;
    playingGame.current.ID = IDSTART - 1;
    playingGame.current.height = MINH;
    playingGame.current.width = MINW;
    playingGame.current.speed = SPEEDOFCAR;
    playingGame.current.x = XOFCAR;
    playingGame.current.y = YOFCAR;
    playingGame.current.clr = COLOROFCAR;
    playingGame.current.chr = '*';
    playingGame.current.isExist = true;
}

void *newGame(void *) {
    printWindow();
    drawCar(playingGame.current, 2,1); // oyuncunun sürdüğü arabayı başlangıc konumunda ekrana cizer 2 = çizme 1 = kullanıcının arabası
    int key;
    while (playingGame.IsGameRunning) { // continue until the game is over
        key = getch(); // kullanıcı girdisini alır
        if (key != KEYERROR) {
            if (key == playingGame.leftKey && playingGame.current.x >4) { // sol tuşa basmıssa
                drawCar(playingGame.current, 1,1); // mevcut arabanın konumunu siler
                playingGame.current.x -= playingGame.current.speed; // arabanın konumunu sola gunceler
                drawCar(playingGame.current, 2,1); // arabayı yeni konumunda tekrar çizer
            } else if (key == playingGame.rightKey && playingGame.current.x <= wWidth - 11) { // If the right key is pressed
                drawCar(playingGame.current, 1,1); // removes player's car from screen
                playingGame.current.x += playingGame.current.speed; // update position
                drawCar(playingGame.current, 2,1); // draw player's car with new position
            }
            else if(key == ESC){
                playingGame.IsGameRunning = false;
                mainMenuScreen();
            }
            else if(key == SAVEKEY){
                saveGame();
                
            }
        }
        usleep(GAMESLEEPRATE);
    }
    return NULL;
}

void initWindow() {
    initscr(); // terminali ncurses moduna geçirir
    start_color(); // renk desteğini açar
    keypad(stdscr, true); // etışların girişini kolaylaştırır
    nodelay(stdscr, true); // set the getch() function to non-blocking mode
    curs_set(0); // imleci gizler
    cbreak(); // enter basmadan girdiyi algılar
    noecho(); // kullanıcının bastığı değeri ekranda gizler
    clear(); // clear the screen
    sleep(1);
}

void printWindow() {


    for (int i = 1; i < wHeight - 1; ++i) {
        // mvprintw: Used to print text on the window, parameters order: y, x, string
        mvprintw(i, 2, "*"); // left side of the road
        mvprintw(i, 0, "*");
        mvprintw(i, wWidth - 1, "*"); // right side of the road
        mvprintw(i, wWidth - 3, "*");
    }
    for (int i = lineLEN; i < wHeight - lineLEN; ++i) { // line in the middle of the road
        mvprintw(i, lineX, "#");
    }

    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    int treex = wWidth +5, treey = 10;

    for(int i = 0; i < 3; i++) //tree
    {

        attron(COLOR_PAIR(1));
        mvprintw(treey++, treex, "  *  ");
        mvprintw(treey++, treex, " * *  ");
        mvprintw(treey++, treex, "* * *");
        attroff(COLOR_PAIR(1));
        attron(COLOR_PAIR(2));
        mvprintw(treey++, treex, "  #  ");
        mvprintw(treey,   treex, "  #  ");
        attroff(COLOR_PAIR(2));
        treey+=6;
    }


}

void drawCar(Car c, int type, int direction )
{
	//If the user does not want to exit the game and the game continues
    if(playingGame.IsSaveClicked!=true && playingGame.IsGameRunning==true)
    {
            init_pair(c.ID, c.clr, 0);// Creates a color pair: init_pair(short pair ID, short foregroundcolor, short backgroundcolor);
            //0: Black (COLOR_BLACK)
			//1: Red (COLOR_RED)
			//2: Green (COLOR_GREEN)
			//3: Yellow (COLOR_YELLOW)
			//4: Blue (COLOR_BLUE)
			attron(COLOR_PAIR(c.ID));//enable color pair
            char drawnChar;
            if (type == 1 )
               drawnChar = ' '; // to remove car
            else
               drawnChar= c.chr; //  to draw char
		    //mvhline: used to draw a horizontal line in the window
			//shallow. : mvhline(int y, int x, chtype ch, int n)
			//y: horizontal coordinate
			//x: vertical coordinate
			//ch: character to use
			//n: Length of the line
            mvhline(c.y, c.x, drawnChar, c.width);// top line of rectangle
            mvhline(c.y + c.height - 1, c.x, drawnChar, c.width); //bottom line of rectangle
            if(direction == 0) // If it is any car on the road
                mvhline(c.y + c.height, c.x, drawnChar, c.width);
            else //player's card
                mvhline(c.y -1, c.x, drawnChar, c.width);
		    //mvvline: used to draw a vertical line in the window
			//shallow. : mvhline(int y, int x, chtype ch, int n)
			//y: horizontal coordinate
			//x: vertical coordinate
			//ch: character to use
			//n: Length of the line
            mvvline(c.y, c.x, drawnChar, c.height); //left line of rectangle
            mvvline(c.y, c.x + c.width - 1, drawnChar, c.height); //right line of rectangle
            char text[5];
            if (type == 1 )
                sprintf(text,"  "); //to remove point
            else
                 sprintf(text,"%d",c.height * c.width); // to show car's point in rectangle
            mvprintw(c.y+1, c.x +1, text);// display car's point in rectangle
            attroff(COLOR_PAIR(c.ID));// disable color pair
    }
}

void *enqueueCar(void *) { // kuyruğa araba ekler
    while (playingGame.IsGameRunning) {
        if (playingGame.cars.size() <= maxCarNumber -1) { 
            Car c;
            if(playingGame.counter >=20){
                playingGame.counter = 10;
            }
            c.ID = playingGame.counter++;
            c.height = MINH + rand() % 3; // yükseklik
            c.width = MINW + rand() % 3;  // genislik

            c.speed = c.height /2;
            do{
                c.x = MINX + rand() % 86;
                if((c.x + c.width < lineX || c.x > lineX) && c.x + c.width < wWidth -3) // şeritin dişinda araba üretmesin
                    break;
            }while(1);
            c.y = -(rand() % 11);
            c.clr = 1 + rand() % numOfcolors;
            c.chr = shapesCar[(rand() % numOfChars)];
            c.isExist = true;
            pthread_mutex_lock(&playingGame.mutexFile); //arabalar listesine erişimi kitler
            playingGame.cars.push(c); //yeni arabayı ekler

            pthread_mutex_unlock(&playingGame.mutexFile); //kilidi açar
        }
        sleep(EnQueueSleep);
    }
}

void *dequeueCar(void*) //  moveCar fonksiyonuna araba yollar
{

    while (playingGame.IsGameRunning) {
       Car c;

        pthread_mutex_lock(&playingGame.mutexFile); // başka bir threadı engeller
        c = playingGame.cars.front();
        playingGame.cars.pop();
        pthread_mutex_unlock(&playingGame.mutexFile);
          c.isExist = true;

        pthread_t thMove;
        
        pthread_create(&thMove, NULL, moveCar, (void *)&c); // moveCar fonkisyonuna araba gönderir.

        int rndSleep = rand() % DeQueueSleepMin; // 0 veya 1 üretier

            if(rndSleep == 0 ){
            sleep(DeQueueSleepMin);
        }

        else{
            sleep(2*DeQueueSleepMin);
        }
    }
}

void *moveCar(void* car) // arabanın yolda hareketini sağlar
{
    Car c =  *(Car*)car;
    while (playingGame.IsGameRunning && c.isExist && !playingGame.IsSaveClicked){
        drawCar(c, 1, 0); 
        c.y += 1 + rand() % c.speed;// Arabanın hareketi
        drawCar(c, 2, 0); 
        writePoint(); //puan yazdırır
        if (c.y > EXITY) {   //arabanın yolu bitirdiğini kontrol eder
            drawCar(c, 1, 0);
            c.isExist = false;      
            playingGame.points += c.height * c.width; // puannı günceller
            writePoint();

            if (playingGame.points >= playingGame.level * levelBound && playingGame.level < MAXSLEVEL) { // Seviye yükseltmeyi kontrol eder
                playingGame.level++;
                playingGame.moveSpeed -= DRATESPEED;
            }
        }
        else{

            if(checkCollision(playingGame.current, c)){
                c.isExist = false; 
                playingGame.IsGameRunning = false; 
                FILE *writePoint = fopen(pointsTxt, "a+"); // puanı txt dosaysına ekler
                fprintf(writePoint, "%d\n", playingGame.points);
                fclose(writePoint);
                clear();

            }
        }

        usleep(playingGame.moveSpeed);

    }
    if(playingGame.IsSaveClicked && c.isExist)
    {
       // ekrandaki arabaları yeni bir kuyruğa ekler
        pthread_mutex_lock(&playingGame.mutexFile);
        playingGame.serhat.push(c);
        pthread_mutex_unlock(&playingGame.mutexFile);
    }
   


}


bool checkCollision(Car c1, Car c2) { //çarpışmaları kontrol eder.
    if(c1.x +c1.width < c2.x || c1.x > c2.x + c2.width || c1.y > c2.y + c2.height || c1.y + c1.height < c2.y)
    {
        return false;
    }
    return true;
}

void mainMenuScreen() { //menu ekranı hazırlama
    clear();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    int choice = 0; // hangi indexte
    char arrow[5];
    sprintf(arrow,"->");

    while (1) { 
        for (int i = 0; i < mainMenuItem; ++i) // imleç neredeeyse rengini değiştir
        {
            if (i == choice)
            {
                attron(COLOR_PAIR(2));
                mvprintw(MENUY + i * MENUDIF, MENUX - 2, arrow); //imleç yazar
                mvprintw(MENUY + i * MENUDIF, MENUX, mainMenu[i]); // satırı kırmızı yazaar
                attroff(COLOR_PAIR(2));
            }
            else{
                attron(COLOR_PAIR(1));
                mvprintw(MENUY + i * MENUDIF, MENUX-2, "    ");
                mvprintw(MENUY + i * MENUDIF, MENUX, mainMenu[i]);
                attroff(COLOR_PAIR(1));
            }
        }

        int key = getch(); // kullanıcıdan input alır
        switch (key) {
            case KEYPUP: 

                if(choice == 0) // imleç  yukardan aşagıya geçmesin 
                choice = 0;
                else
                    --choice;
                break;
            case KEYDOWN: 

                if(choice == mainMenuItem -1){
                choice = mainMenuItem -1;
                }
                else{
                    ++choice;
                }
                break;

            case ENTER: 
                switch (choice) {
                    case 0: // New Game
                        clear();
                        initGame(); // init değerleri atar
                        startGame(); //  threadleri başlatır
                        break;
                    case 1:
                       clear();
                       loadGame(); // son kaydedilen değerleri oyuna atar
                       startGame(); 
                        
                        break;
                    case 2:

                        instructionsScreen();
                        break;
                    case 3:

                        settingsMenuScreen();
                        break;
                    case 4:
                        pointsScreen();
                        break;
                    case 5:
                        exit(0);
                    break;
                }

            break;
        }
        usleep(MENSLEEPRATE); // sleep
    }
    endwin(); 
}
void startGame()
{
                         

                        pthread_t th1, th2, th3, th4; // thread oluşturur
                        pthread_create(&th1, NULL, newGame, NULL); 
                        pthread_create(&th2, NULL, enqueueCar, NULL); 
                        pthread_create(&th3, NULL, dequeueCar, NULL);

                        pthread_join(th1, NULL); // threaadlerin bitmesini bekler
                        pthread_join(th2, NULL);
                        pthread_join(th3, NULL);
                        endwin();
}

void instructionsScreen() {
    clear();
    attron(COLOR_PAIR(1));
    mvprintw(MENUY, MENUX, "< or A: moves the car to left");
    mvprintw(MENUY + MENUDIF, MENUX, "> or D: moves the car to right");
    mvprintw(MENUY + 2 * MENUDIF, MENUX, "ESC: exits the game without saving");
    mvprintw(MENUY + 3 * MENUDIF, MENUX, "S: saves and exits the game");
    attroff(COLOR_PAIR(1));
    refresh(); 
    sleep(5);
    mainMenuScreen();
}

void pointsScreen() {// puan ekranını yazaar
    clear();
    FILE *pointsFile = fopen(pointsTxt, "r"); 
    attron(COLOR_PAIR(1));
    if (pointsFile == NULL) {
        mvprintw(MENUY, MENUX, "points not found");
    } else {
        char line[256];
        int y = MENUY, 
        x = MENUX - MENUDIFX ;  // x konumunu düzenler 
        int count = 1; // kaçıncı puan oldugunu yazar
        while (fgets(line, sizeof(line), pointsFile)) { // fileden puan alır
            if((count % 10)  == 1){ // 10 satır olmasını sağlar
                x += MENUDIFX;
                y = MENUY;
                
            }
            mvprintw(y, x, "Game %d: %s", count, line);
            count++;
            y += MENUDIF;
        }
        fclose(pointsFile);
    }
    attroff(COLOR_PAIR(1));
    refresh();
    sleep(5);
    mainMenuScreen();
}

void settingsMenuScreen() {//düzenle
    clear();
    int choice = 0;
    bool menuRunning = true;
    char arrow[5];
    sprintf(arrow,"->");
    while (menuRunning) {
        for (int i = 0; i < settingMenuItem; ++i) //settıng menu yazıyor
            {
            if (i == choice) { // seçileni kırmızı yapar
                attron(COLOR_PAIR(2));
                mvprintw(MENUY + i * MENUDIF, MENUX - 2, arrow);
                mvprintw(MENUY + i * MENUDIF, MENUX, settingMenu[i]);
                attroff(COLOR_PAIR(2));
            }
            else{
                attron(COLOR_PAIR(1));
                mvprintw(MENUY + i * MENUDIF, MENUX-2, "            ");
                mvprintw(MENUY + i * MENUDIF, MENUX, settingMenu[i]);
                attroff(COLOR_PAIR(1));
            }

        }

        int key = getch();
        switch (key) {
            case KEYPUP:

                if (choice == 0) { // üstten aşağı gitmesin
                    choice =0;
                }
                else
                    choice--;
                break;
            case KEYDOWN:

                if (choice == settingMenuItem -1) {
                    choice = settingMenuItem -1;
                }
                else
                    choice++;
                break;
            case ENTER:
                switch (choice) {
                    case 0: 
                        playingGame.leftKey = leftKeyArrow;
                        playingGame.rightKey = RightKeyArrow;
                        break;
                    case 1:
                        playingGame.leftKey = leftKeyA;
                        playingGame.rightKey = RightKeyD;
                        break;
                }
                menuRunning = false;
                break;
        }
        usleep(MENSLEEPRATE);
    }
    mainMenuScreen(); 
}

void saveGame() // Game ve Car strucları dosyaya yazar. Kuyrukta düzenlemeler yapar.
{

    playingGame.IsSaveClicked = true;
    playingGame.IsGameRunning = false;
     sleep(1);
    pthread_mutex_lock(&playingGame.mutexFile);
   

    FILE *writeGame = fopen(gameTxt, "wb");
    FILE *writeCars = fopen(CarsTxt,"wb");



    while(!playingGame.serhat.empty()) // ekrandaki arabaları dosyaya yazar
    {
        Car c = playingGame.serhat.front();
        fwrite(&c, sizeof(Car), 1, writeCars);
        playingGame.serhat.pop();
        usleep(250000);
    }
    while(!playingGame.cars.empty()) // oyundaki Cars kuyrugunu boşaltır . Çünkü load dan sonra ekrandakiler bu kuyruga verilecek. 
    {
        Car c = playingGame.cars.front();
        playingGame.cars.pop();
        usleep(250000);
    }
    fwrite(&playingGame, sizeof(playingGame), 1, writeGame); 
    pthread_mutex_unlock(&playingGame.mutexFile);
    fclose(writeGame);
    fclose(writeCars);
    mainMenuScreen();
}

void writePoint()
{
    char text[20];
    sprintf(text, "Point: %d", playingGame.points);
    mvprintw(POINTY, POINTX, text);
}

void loadGame() { // Dosyadan kaydedeilen verileri oyuna ekler    
    FILE *gameFile = fopen(gameTxt, "rb");
    if (gameFile == NULL) {       
        sleep(1);
       startGame();
    }
    else {
        fread(&playingGame, sizeof(Game), 1, gameFile);
        fclose(gameFile);

        FILE *carsFile = fopen(CarsTxt, "rb");   
        Car c;
        size_t readCount;
         while ((readCount = fread(&c, sizeof(Car), 1, carsFile)) > 0)
        {         
            playingGame.cars.push(c); // dosyadaki arabaları cars a atar.
            usleep(250000);
        }
            fclose(carsFile);
        
    playingGame.IsSaveClicked = false; 
    playingGame.IsGameRunning = true;
    sleep(1);
    }
}


