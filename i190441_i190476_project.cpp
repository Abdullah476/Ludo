/*
Husnain Rasul Qadri-i190441
Abdullah-i190476
*/
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <wait.h>
#include <semaphore.h>

//STRUCT TO STORE TOKENS DATA;
struct tokens {
	sf::Sprite* image = new sf::Sprite;
	sf::IntRect position;
	sf::IntRect home_position;
	bool in_home;
	bool token_won;
	tokens(){
		in_home=true;
		token_won=false;
	}
};

struct display {
	sf::Sprite board;
	tokens token[4][4];
	bool won[4];
	display(){
		won[0]=false;
		won[1]=false;
		won[2]=false;
		won[3]=false;
	}
};

struct dice {
	int roll_num=1;
	int buffer[3];
	sf::Sprite image;
	sf::Text text;
	sf::IntRect position;
	sf::Texture texture_dice;
	sf::String str;
	void setStr(){
		str = "L";
		text.setString(str);
	}
};

struct path {
	int x;
	int y;
	sf::IntRect position;
	void update(){
		position.left=x;
		position.top=y;
	}
};

path token_path[4][18];
int round_values[4];

//DECLARING THREADS
pthread_t t_id;
pthread_t t_display;
pthread_t t_event;
pthread_t t_tokens;
pthread_t t_dice;
pthread_t t_player;

sem_t window_lock;
sem_t co_ordinates_lock;
sem_t turn_lock;
sem_t dice_lock;

sf::RenderWindow* window;
sf::Sprite* board_ptr;
sf::Sprite** tokens_ptr;
display* display_ptr;
dice* dice_ptr;
int turn = 1;
sf::RectangleShape whos_turn_display;
sf::Vector2i co_ordinates;
void* TokenDisplay(void*arg);

//THIS IS THE FUNCTION TO UPDATE TOKEN POSITION
//DESTINATION POSITION IS ALSO PASSED I.E. SECTION AND INDEX
//THE IDTIFICATION OF TOKEN IS ALSO PASSED TOKEN AND NUMBER_OF_TOKEN
void update_token_position(int section,int index,int token_,int number_of_token) {
	display_ptr->token[token_][number_of_token].position.left=token_path[section][index].x;
	display_ptr->token[token_][number_of_token].position.top=token_path[section][index].y;
	display_ptr->token[token_][number_of_token].image->setPosition(display_ptr->token[token_][number_of_token].position.left,display_ptr->token[token_][number_of_token].position.top);
}

//THIS IS A SIMPLE FUNCTION THAT RETURNS THE INDEX FOR GIVEN X AND Y CO ORDINATES
int find_index(sf::IntRect val){
	//int index;
	for (int i=0;i<4;i++) {
		for (int j=0;j<18;j++) {
			if (val.contains(token_path[i][j].position.left,token_path[i][j].position.top)){
				return j;
			}
		}
	}
	return -1;
}

//THIS IS THE THREAD THAT GENERATES A RANDOM NUMBER BETWEEN 1 - 6
//THIS THREAD HAVE A LOOP ACTUALLY SIMULATING DICE ROLL
void* dice_roll(void* ard) {
	srand (time(NULL));
	for (int i=0; i<10 ; i++) {
		dice_ptr->roll_num = rand() % 6 + 1;
		dice_ptr->setStr();
		switch (dice_ptr->roll_num) {
			case 1:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_1.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			case 2:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_2.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			case 3:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_3.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			case 4:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_4.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			case 5:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_5.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			case 6:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/dice_6.png");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;

			default:
			dice_ptr->texture_dice.loadFromFile("../i190441_i190476/ludo_pic.jpg");
			dice_ptr->image.setTexture(dice_ptr->texture_dice);
			break;
		}
		sleep(1);
	}
	pthread_exit(NULL);
}

//THIS FUNCTION CALLS DICE ROLL THREAD
//THIS FUNCTION TAKES CARE OF THE CONDITION IF 6 COMES WE HAVE TO ROLL AGAIN
//STORES ALL 3 VALUES IF THEY COME IN AN ARRAY
void roll_the_dice() {
	sem_wait(&dice_lock);
	dice_ptr->buffer[0]=0;
	dice_ptr->buffer[1]=0;
	dice_ptr->buffer[2]=0;
	pthread_create(&t_dice,NULL,dice_roll,NULL);
	pthread_join(t_dice,NULL);
	dice_ptr->buffer[0]=dice_ptr->roll_num;
	std::cout<<"DICE 1:"<<dice_ptr->buffer[0]<<std::endl;
	if (dice_ptr->roll_num==6){
		pthread_create(&t_dice,NULL,dice_roll,NULL);
		pthread_join(t_dice,NULL);
		dice_ptr->buffer[1]=dice_ptr->roll_num;
		std::cout<<"DICE 2:"<<dice_ptr->buffer[1]<<std::endl;
		if (dice_ptr->roll_num==6) {
			pthread_create(&t_dice,NULL,dice_roll,NULL);
			pthread_join(t_dice,NULL);
			if (dice_ptr->roll_num==6) {
				dice_ptr->roll_num=0;
				dice_ptr->buffer[0]=0;
				dice_ptr->buffer[1]=0;
				dice_ptr->buffer[2]=0;
			}
			else {
				dice_ptr->buffer[2]=dice_ptr->roll_num;
			}
			std::cout<<"DICE 3:"<<dice_ptr->buffer[2]<<std::endl;
		}
	}
	sem_post(&dice_lock);
}

//WE HAVE CREATED A BOX THAT DISPLAY THE COLOR OF THE PLAYER WHOS TURN IT IS
void set_whos_turn_display(){
	if (turn == 0) {
		whos_turn_display.setFillColor(sf::Color::Green);
	}
	else if (turn == 1) {
		whos_turn_display.setFillColor(sf::Color::Blue);
	}
	else if (turn == 2) {
		whos_turn_display.setFillColor(sf::Color::Red);
	}
	else if (turn == 3) {
		whos_turn_display.setFillColor(sf::Color::Yellow);
	}
}

//THIS FUNCTION IS MADE TO MOVE THE TOKEN
//IT TAKES CARE OF THE VARIOUS CONDITIONS THAT MAY OCCUR
void move_token(int number_of_token,int dice_val){
	if (display_ptr->token[turn][number_of_token].in_home) {
		if(dice_val==6){
			update_token_position(turn,13,turn,number_of_token);
			display_ptr->token[turn][number_of_token].in_home=false;
		}
	}
	else if (display_ptr->token[turn][number_of_token].position==token_path[0][17].position) {
		update_token_position(3,0,turn,number_of_token);
		update_token_position(3,dice_val-1,turn,number_of_token);
	}
	else if (display_ptr->token[turn][number_of_token].position==token_path[1][17].position) {
		update_token_position(2,0,turn,number_of_token);
		update_token_position(2,dice_val-1,turn,number_of_token);
	}
	else if (display_ptr->token[turn][number_of_token].position==token_path[2][17].position) {
		update_token_position(0,0,turn,number_of_token);
		update_token_position(0,dice_val-1,turn,number_of_token);
	}
	else if (display_ptr->token[turn][number_of_token].position==token_path[3][17].position) {
		update_token_position(1,0,turn,number_of_token);
		update_token_position(1,dice_val-1,turn,number_of_token);
	}
	else if ( display_ptr->token[turn][number_of_token].position==token_path[0][6].position ) {
		if (turn==0){
			update_token_position(0,7,turn,number_of_token);
			update_token_position(0,7+(dice_val-1),turn,number_of_token);
		}
		else {
			update_token_position(0,12,turn,number_of_token);
			update_token_position(0,12+(dice_val-1),turn,number_of_token);
		}
	}	
	else if (display_ptr->token[turn][number_of_token].position==token_path[1][6].position) {
		if (turn==1){
			update_token_position(1,7,turn,number_of_token);
			update_token_position(1,7+(dice_val-1),turn,number_of_token);
		}
		else {
			update_token_position(1,12,turn,number_of_token);	
			update_token_position(1,12+(dice_val-1),turn,number_of_token);	
		}
	}
	else if (display_ptr->token[turn][number_of_token].position==token_path[2][6].position) {
		if (turn==2){
			update_token_position(2,12,turn,number_of_token);
			update_token_position(2,12+(dice_val-1),turn,number_of_token);
		}
		else {
			update_token_position(2,12,turn,number_of_token);
			update_token_position(2,12+(dice_val-1),turn,number_of_token);
			}
	}
	else if	(display_ptr->token[turn][number_of_token].position==token_path[3][6].position) {
		if (turn==3){
			update_token_position(3,12,turn,number_of_token);
			update_token_position(3,12+(dice_val-1),turn,number_of_token);
		}
		else {
			update_token_position(3,12,turn,number_of_token);
			update_token_position(3,12+(dice_val-1),turn,number_of_token);
		}
	} 
	else {
		if (find_index(display_ptr->token[turn][number_of_token].position)+dice_val>17) {
			int temp = 17 - find_index(display_ptr->token[turn][number_of_token].position);
			dice_val -= temp;
			switch (turn) {
				case 0:
					update_token_position(3,dice_val,turn,number_of_token);
				break;
				case 1:
					update_token_position(2,dice_val,turn,number_of_token);
				break;
				case 2:
					update_token_position(0,dice_val,turn,number_of_token);
				break;
				case 3:
					update_token_position(1,dice_val,turn,number_of_token);
				break;
				default:
				break;
			}
		}
	}
}

//THREAD FOR THE PLAYER 
//WE ROLL THE DICE, TAKE CARE OF WHICH TOKEN IS PICKED BY THE PLAYER ETC
void* player(void* arg) {
	set_whos_turn_display();
	for (int i=0;i<10;i++) {
		sleep(1);
		sem_wait(&co_ordinates_lock);
		if (dice_ptr->position.contains(co_ordinates)) {
			roll_the_dice();
			co_ordinates.x=0;
			co_ordinates.y=0;
			sem_post(&co_ordinates_lock);
			break;
		}
		else if (i==9){
			sem_post(&co_ordinates_lock);
			pthread_exit(NULL);
		}
		sem_post(&co_ordinates_lock);
	}
	sem_wait(&turn_lock);
	sem_wait(&dice_lock);
	if (	dice_ptr->buffer[0]!=6 &&
		display_ptr->token[turn][0].in_home==true &&
		display_ptr->token[turn][1].in_home==true && 
		display_ptr->token[turn][2].in_home==true &&
		display_ptr->token[turn][3].in_home==true)
	{
		sem_post(&turn_lock);
		sem_post(&dice_lock);
		pthread_exit(NULL);
	}
	sem_post(&turn_lock);
	sem_post(&dice_lock);

	for (int k=0;k<3;k++) {
		if (dice_ptr->buffer[k]==0) {
			break;
		}
		for (int i=0;i<20;i++) {
			sleep(2);
			sem_wait(&co_ordinates_lock);
			sem_wait(&turn_lock);
			if (display_ptr->token[turn][0].position.contains(co_ordinates.x,co_ordinates.y)) {
				move_token(0,dice_ptr->buffer[k]);
				co_ordinates.x=0;
				co_ordinates.y=0;
				sem_post(&co_ordinates_lock);
				sem_post(&turn_lock);
				break;
			}
			else if (display_ptr->token[turn][1].position.contains(co_ordinates.x,co_ordinates.y)) {
				move_token(1,dice_ptr->buffer[k]);
				co_ordinates.x=0;
				co_ordinates.y=0;
				sem_post(&co_ordinates_lock);
				sem_post(&turn_lock);
				break;
			}
			else if(display_ptr->token[turn][2].position.contains(co_ordinates.x,co_ordinates.y)) {
				move_token(2,dice_ptr->buffer[k]);
				co_ordinates.x=0;
				co_ordinates.y=0;
				sem_post(&co_ordinates_lock);
				sem_post(&turn_lock);
				break;
			}
			else if (display_ptr->token[turn][3].position.contains(co_ordinates.x,co_ordinates.y)) {
				move_token(3,dice_ptr->buffer[k]);
				co_ordinates.x=0;
				co_ordinates.y=0;
				sem_post(&co_ordinates_lock);
				sem_post(&turn_lock);
				break;
			}
			else if (i==19){
				sem_post(&co_ordinates_lock);
				sem_post(&turn_lock);
				pthread_exit(NULL);
			}
			sem_post(&co_ordinates_lock);
			sem_post(&turn_lock);
		}
	}
	pthread_exit(NULL);
}

//THIS IS DISPLAY THREAD IT RUN CONTINUOUSLY DISPLAYING THE SPRITES AND ASSETS 
void* display_(void* arg) {
	bool is_open=true;
	//THIS LOOP DISPLAYS THE DATA UNTILL YOU CLOSES THE WINDOW
	while (is_open) {
		sem_wait(&window_lock);
		window->clear();
		window->draw(display_ptr->board);
		for (int i=0;i<4;i++) {
			for (int j=0;j<4;j++) {
				window->draw(*display_ptr->token[i][j].image);
			}
		}
		window->draw(dice_ptr->image);
		window->draw(whos_turn_display);
		window->draw(dice_ptr->text);
		window->display();
		is_open=window->isOpen();
		sem_post(&window_lock);
	}
	pthread_exit(NULL);
}

//POPULATING THE PATH OF THE TOKEN IN EACH SUB BOARD
void populating_path() {
	int s=0;
	int l=247;
	int q=367;

	//INTIALLISING THE PATH OF GREEN MAP
	int k=0;
	for (int j=0; j<3 ; j++) {
		l=247;
		for (int i=0;i<6;i++) {
			token_path[0][k].x = l + (j*40);
			if (j==0) {
				token_path[0][k].y = 207-(40*i);

			}
			else {
				token_path[0][k].y = (40*i);
			}
			token_path[0][k].update();
			k++;
		}
	}

	//NTIALLISING THE PATH OF BLUE MAP
	k=0;
	for (int j=0; j<3 ; j++) {
		for (int i=0;i<6;i++) {
			token_path[1][k].x = 327 - (j*40);
			if (j==0) {
				token_path[1][k].y = 367+(40*i);

			}
			else {
				token_path[1][k].y = 572-(40*i);
			}
			token_path[0][k].update();
			k++;
		}
	}

	//NTIALLISING THE PATH OF YELLOW MAP
	k=0;
	for (int j=0; j<3 ; j++) {
		for (int i=0;i<6;i++) {
			token_path[3][k].y = 247 + (j*40);
			if (j==0) {
				token_path[3][k].x = 367+(40*i);

			}
			else {
				token_path[3][k].x = 572-(40*i);
			}
			token_path[0][k].update();
			k++;
		}
	}

	//NTIALLISING THE PATH OF RED MAP
	k=0;
	for (int j=0; j<3 ; j++) {
		for (int i=0;i<6;i++) {
			token_path[2][k].y = 327 - (j*40);
			if (j==0) {
				token_path[2][k].x = 207 - (40*i);

			}
			else {
				token_path[2][k].x = (40*i);
			}
			token_path[0][k].update();
			k++;
		}
	}
}
/*
THIS THREAD IS CREATED TO LOOK FOR ANY EVENT THAT TAKES
PLACE AND THEN REACTS ACCORDINGLY
*/
void* events(void* arg){
	bool event_not_happened=1;
	while (event_not_happened) {
		//THIS OBJECT IS CREATED TO PICK UP ANY EVENT THAT TAKES PLACE
		sf::Event event;
		sem_wait(&window_lock);
		while (window->pollEvent(event)) {
			//CHECKING IF THE EVENT IS CLOSING THE WINDOW
			if (event.type == sf::Event::Closed) {
				window->close();
				pthread_cancel(t_id);
				event_not_happened = 0;
			}
			else if (event.type == sf::Event::MouseButtonPressed && event.key.code == sf::Mouse::Left) {
				sem_wait(&co_ordinates_lock);
				co_ordinates = sf::Mouse::getPosition(*window);
				std::cout<<"X:"<<co_ordinates.x<<" Y:"<<co_ordinates.y<<std::endl;
				sem_post(&co_ordinates_lock);
			}
		}
		sem_post(&window_lock);
		
	}
	pthread_exit(NULL);
}

//THIS FUNTION TAKES CARE OF THE TURN. EACH PLAYER HAVE ONE TURN PER ROUND AND 
//TURNS ARE RANDOM
void round(){
	int k;
	srand (time(NULL));
	round_values[0] = rand() % 4;

	turn = round_values[0];
	pthread_create(&t_player,NULL,player,NULL);
	pthread_join(t_player,NULL);


	k = rand() % 4;
	while (k==round_values[0]) {
		k = rand() % 4;
	}
	round_values[1] = k;

	turn = round_values[1];
	pthread_create(&t_player,NULL,player,NULL);
	pthread_join(t_player,NULL);

	k = rand() % 4;
	while (k==round_values[0] || k==round_values[1]) {
		k = rand() % 4;
	}
	round_values[2] = k;

	turn = round_values[2];
	pthread_create(&t_player,NULL,player,NULL);
	pthread_join(t_player,NULL);


	k = rand() % 4 + 1;
	while (k==round_values[0] || k==round_values[1] || k==round_values[2]) {
		k = rand() % 4;
	}
	round_values[3] = k;

	turn = round_values[3];
	pthread_create(&t_player,NULL,player,NULL);
	pthread_join(t_player,NULL);

}

//THIS IS THE MAIN THREAD OR MASTER THREAD
//THIS MANAGES EVERYTHING AND CREATES AND DELETES
void* MainThread(void* arg){
	//CREATING THREADS TO DISPLAY BACKGROUND
	//AND TOKENS
	pthread_create(&t_display,NULL,display_,NULL);
	pthread_create(&t_event,NULL,events,NULL);
	while (display_ptr->won[0]==false || display_ptr->won[1]==false || display_ptr->won[2]==false || display_ptr->won[3]==false) {
		round();
	}
	pthread_join(t_event,NULL);
	pthread_join(t_display,NULL);
	pthread_exit(NULL);
}

int main() {

	//Initializing semaphores
	sem_init(&window_lock, 0, 1);
	sem_init(&co_ordinates_lock, 0, 1);
	sem_init(&turn_lock, 0, 1);
	sem_init(&dice_lock, 0, 1);

	//intialiser();
	populating_path();
	//=======================================

	//ALLOCATING MEMORY FOR THE MEMORY
	window = new sf::RenderWindow(sf::VideoMode(750, 612), "LUDO");

	//DECLARING TEXTURE AND LOADING TEXTURE FROM THE FILE
	sf::Texture texture;
	texture.loadFromFile("../i190441_i190476/ludo_pic.jpg");

	//ALLOCATING MEMORY OF THE SPRITE OF THE TOKENS
	//DYNAMIC ALLOCATION IS DONE SO THAT THE MEMORY IS ALLOCATED IN HEAP
	display_ptr = new display;
	display_ptr->board.setTexture(texture);

	sf::Texture token_tex[4];
	token_tex[0].loadFromFile("../i190441_i190476/token_1.png");
	token_tex[1].loadFromFile("../i190441_i190476/token_2.png");
	token_tex[2].loadFromFile("../i190441_i190476/token_3.png");
	token_tex[3].loadFromFile("../i190441_i190476/token_4.png");


	display_ptr->token[0][0].image->setTexture(token_tex[0]);
	display_ptr->token[0][1].image->setTexture(token_tex[0]);
	display_ptr->token[0][2].image->setTexture(token_tex[0]);
	display_ptr->token[0][3].image->setTexture(token_tex[0]);

	display_ptr->token[0][0].image->setScale(0.5,0.5);
	display_ptr->token[0][1].image->setScale(0.5,0.5);
	display_ptr->token[0][2].image->setScale(0.5,0.5);
	display_ptr->token[0][3].image->setScale(0.5,0.5);

	display_ptr->token[0][0].position.left = 427;
	display_ptr->token[0][0].position.top = 63;
	display_ptr->token[0][0].position.height = 40;
	display_ptr->token[0][0].position.width = 40;
	display_ptr->token[0][0].image->setPosition(427,63);
	display_ptr->token[0][0].home_position=display_ptr->token[0][0].position;
	display_ptr->token[0][1].position.left = 507;
	display_ptr->token[0][1].position.top = 63;
	display_ptr->token[0][1].position.height = 40;
	display_ptr->token[0][1].position.width = 40;
	display_ptr->token[0][1].image->setPosition(507,63);
	display_ptr->token[0][1].home_position=display_ptr->token[0][1].position;
	display_ptr->token[0][2].position.left = 427;
	display_ptr->token[0][2].position.top = 143;
	display_ptr->token[0][2].position.height = 40;
	display_ptr->token[0][2].position.width = 40;
	display_ptr->token[0][2].image->setPosition(427,143);
	display_ptr->token[0][2].home_position=display_ptr->token[0][2].position;
	display_ptr->token[0][3].position.left = 507;
	display_ptr->token[0][3].position.top = 143;
	display_ptr->token[0][3].position.height = 40;
	display_ptr->token[0][3].position.width = 40;
	display_ptr->token[0][3].image->setPosition(507,143);
	display_ptr->token[0][3].home_position=display_ptr->token[0][3].position;

	display_ptr->token[1][0].image->setTexture(token_tex[1]);
	display_ptr->token[1][1].image->setTexture(token_tex[1]);
	display_ptr->token[1][2].image->setTexture(token_tex[1]);
	display_ptr->token[1][3].image->setTexture(token_tex[1]);

	display_ptr->token[1][0].image->setScale(0.5,0.5);
	display_ptr->token[1][1].image->setScale(0.5,0.5);
	display_ptr->token[1][2].image->setScale(0.5,0.5);
	display_ptr->token[1][3].image->setScale(0.5,0.5);

	display_ptr->token[1][0].position.left = 62;
	display_ptr->token[1][0].position.top = 427;
	display_ptr->token[1][0].position.height = 40;
	display_ptr->token[1][0].position.width = 40;
	display_ptr->token[1][0].image->setPosition(62,427);
	display_ptr->token[1][0].home_position=display_ptr->token[1][0].position;
	display_ptr->token[1][1].position.left = 143;
	display_ptr->token[1][1].position.top = 427;
	display_ptr->token[1][1].position.height = 40;
	display_ptr->token[1][1].position.width = 40;
	display_ptr->token[1][1].image->setPosition(143,427);
	display_ptr->token[1][1].home_position=display_ptr->token[1][1].position;
	display_ptr->token[1][2].position.left = 62;
	display_ptr->token[1][2].position.top = 508;
	display_ptr->token[1][2].position.height = 40;
	display_ptr->token[1][2].position.width = 40;
	display_ptr->token[1][2].image->setPosition(62,508);
	display_ptr->token[1][2].home_position=display_ptr->token[1][2].position;
	display_ptr->token[1][3].position.left = 143;
	display_ptr->token[1][3].position.top = 508;
	display_ptr->token[1][3].position.height = 40;
	display_ptr->token[1][3].position.width = 40;
	display_ptr->token[1][3].image->setPosition(143,508);
	display_ptr->token[1][3].home_position=display_ptr->token[1][3].position;


	display_ptr->token[2][0].image->setTexture(token_tex[2]);
	display_ptr->token[2][1].image->setTexture(token_tex[2]);
	display_ptr->token[2][2].image->setTexture(token_tex[2]);
	display_ptr->token[2][3].image->setTexture(token_tex[2]);

	display_ptr->token[2][0].image->setScale(0.5,0.5);
	display_ptr->token[2][1].image->setScale(0.5,0.5);
	display_ptr->token[2][2].image->setScale(0.5,0.5);
	display_ptr->token[2][3].image->setScale(0.5,0.5);

	display_ptr->token[2][0].position.left = 62;
	display_ptr->token[2][0].position.top = 62;
	display_ptr->token[2][0].position.height = 40;
	display_ptr->token[2][0].position.width = 40;
	display_ptr->token[2][0].image->setPosition(62,62);
	display_ptr->token[2][0].home_position=display_ptr->token[2][0].position;
	display_ptr->token[2][1].position.left = 143;
	display_ptr->token[2][1].position.top = 62;
	display_ptr->token[2][1].position.height = 40;
	display_ptr->token[2][1].position.width = 40;
	display_ptr->token[2][1].image->setPosition(143,62);
	display_ptr->token[2][1].home_position=display_ptr->token[2][1].position;
	display_ptr->token[2][2].position.left = 62;
	display_ptr->token[2][2].position.top = 143;
	display_ptr->token[2][2].position.height = 40;
	display_ptr->token[2][2].position.width = 40;
	display_ptr->token[2][2].image->setPosition(62,143);
	display_ptr->token[2][2].home_position=display_ptr->token[2][2].position;
	display_ptr->token[2][3].position.left = 143;
	display_ptr->token[2][3].position.top = 143;
	display_ptr->token[2][3].position.height = 40;
	display_ptr->token[2][3].position.width = 40;
	display_ptr->token[2][3].image->setPosition(143,143);
	display_ptr->token[2][3].home_position=display_ptr->token[2][3].position;


	display_ptr->token[3][0].image->setTexture(token_tex[3]);
	display_ptr->token[3][1].image->setTexture(token_tex[3]);
	display_ptr->token[3][2].image->setTexture(token_tex[3]);
	display_ptr->token[3][3].image->setTexture(token_tex[3]);

	display_ptr->token[3][0].image->setScale(0.5,0.5);
	display_ptr->token[3][1].image->setScale(0.5,0.5);
	display_ptr->token[3][2].image->setScale(0.5,0.5);
	display_ptr->token[3][3].image->setScale(0.5,0.5);

	display_ptr->token[3][0].position.left = 427;
	display_ptr->token[3][0].position.top = 427;
	display_ptr->token[3][0].position.height = 40;
	display_ptr->token[3][0].position.width = 40;
	display_ptr->token[3][0].image->setPosition(427,427);
	display_ptr->token[3][0].home_position=display_ptr->token[3][0].position;
	display_ptr->token[3][1].position.left = 507;
	display_ptr->token[3][1].position.top = 427;
	display_ptr->token[3][1].position.height = 40;
	display_ptr->token[3][1].position.width = 40;
	display_ptr->token[3][1].image->setPosition(507,427);
	display_ptr->token[3][1].home_position=display_ptr->token[3][1].position;
	display_ptr->token[3][2].position.left = 427;
	display_ptr->token[3][2].position.top = 508;
	display_ptr->token[3][2].position.height = 40;
	display_ptr->token[3][2].position.width = 40;
	display_ptr->token[3][2].image->setPosition(427,508);
	display_ptr->token[3][2].home_position=display_ptr->token[3][2].position;
	display_ptr->token[3][3].position.left = 507;
	display_ptr->token[3][3].position.top = 508;
	display_ptr->token[3][3].position.height = 40;
	display_ptr->token[3][3].position.width = 40;
	display_ptr->token[3][3].image->setPosition(507,508);
	display_ptr->token[3][3].home_position=display_ptr->token[3][3].position;
	//=======================================
	dice_ptr = new dice;
	dice_ptr->image.setPosition(620,5);
	dice_ptr->position.left=620;
	dice_ptr->position.top=5;
	dice_ptr->position.width=85;
	dice_ptr->position.height=85;
	dice_ptr->text.setPosition(10,10);
	dice_ptr->text.setCharacterSize(30);
	dice_ptr->text.setStyle(sf::Text::Bold);
	dice_ptr->text.setFillColor(sf::Color::Red);
	//==========================================
	whos_turn_display.setPosition(620,560);
	sf::Vector2f size;
	size.x=100;
	size.y=60;
	whos_turn_display.setSize(size);
	whos_turn_display.setFillColor(sf::Color::Red);

	//CREATING MAIN THREAD
	pthread_create(&t_id,NULL,MainThread,NULL);
	pthread_join(t_id,NULL);
	return 0;
}