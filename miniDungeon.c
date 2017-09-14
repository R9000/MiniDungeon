/*
miniDungeon.c - A game about exploring a small 'dungeon', fighting monsters, collecting items and escaping after defeating the boss.
By Rory Charlesworth
Date: 10/05/2015
This program requires the correct file 'equipment.txt' to function correctly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <conio.h> //For getch()
#include <windows.h>  //For Sleep()
#include <time.h>

#define ROWS 10     //ROWS and COLS define the dimensions of the dungeon grid.
#define COLS 10

struct equipment{	//For equipment like swords and armor. protectionDiv is the divisor of an attacker's damage, damageMult is the amount of damage the weapon does.
	char itemName[20];
	int protectionDiv, damageMult;
};

struct player{
	int health;	//Player's current health. Refills after each battle.
	int inHand;	//The inventory slot number of the equipment currently in the player's hand.
	int wearing;	//The inventory slot number of the item the player is currently wearing.
	struct equipment inventory[10]; //An array of structure equipment which stores all the player's posessions.
	int inventoryFilled[10];	//An array to keep track of which slots in the player's inventory are full. 1 = full, 0 = empty.
	char name[12];	//The player's name (chosen at the beginning of the game).
	int pos[2];	//The player's coordinates on the ROWSxCOLS grid.
};

struct enemy{
	char name[21];	//The name of the enemy creature.
	int health, attack, rank;	//Attack is the pre-armour-calculation damage done to the player in battle. Rank decides advice given to the player and some of the actions the player and monster can take in battle.
};

void printLS(char landscape[ROWS][COLS]);	//Prints out the current landscape grid, which is a 2D ROWSxCOLS char array.
void printHelp(char *name);	//Prints the help text below the current game.
void inventoryMenu(struct player *Player1);	//Used by the player to manipulate and equip their inventory items in Player1.inventory.
int findObject(char object, struct player *Player1);	//Used whenever the player encounters a character on the grid which is not a '.'.
int move(int *pos, char keypress, char landscape[][COLS], struct player *Player1);	//Takes the player's keypress and handles if the player can move to a certain location, and what happens if they do.
int battle(struct enemy *monster, struct player *Player1);	//Handles a battle with an enemy monster.
void printResults(char *playerName, struct equipment *inventory, int *inventoryFilled, int escaped); //Prints the results of the player's adventure to 'results.txt'.

int main()
{
	//Initialization step:
	int x, y, r, count, *chesty, *chestx, battleResult, conversion, moveReturn, exit = 0, escaped = 0;	/*Declare x, y and count as counter variables, r as a random variable, chesty and chestx as pointers to be used to make arrays with calloc,
	battleResult to handle the outcome of a battle, conversion to handle the conversion of a player's input to an int, moveReturn to handle the output from move(), and exit to allow the user to exit the game. escaped keeps track of whether or not the player escaped on exit.*/
	char tempName[12];	//Used when temporarily handling a short string.
	char control;	//Used to capture a player's keypress from getch().
	
	//Declare the player's character, set health to full, inventory to empty, and position to the bottom-middle of the grid.
	struct player Player1;
	Player1.health = 15;
	Player1.inHand = -1;
	Player1.wearing = -1;
	memset(Player1.inventory,0,sizeof(Player1.inventory));
	memset(Player1.inventoryFilled,0,sizeof(Player1.inventoryFilled));
	Player1.pos[0] = ROWS-1;
	Player1.pos[1] = COLS/2;
	
	char landscape[ROWS][COLS];	//Declare the landscape 2D char array.
	
	printf("Hello player! Please enter your name (no more than 12 characters).\nAnything over 12 will not be included.\n");
	fgets (Player1.name, 13, stdin);	//Allow the player to enter a string with spaces, and reject anything longer than 13 characters (this is to include the newline from fgets()). Set the player's name to this.
	Player1.name[strcspn(Player1.name, "\n")] = 0;	//Remove any newline character from the string.
	printf("Thanks! Your chosen name is: %s.\n\n", Player1.name);	//Display the input name.
	
	printf("How many chests would you like to start the game with? Please choose between\n1 and 8. The more you have, the more likely you are to find useful items.\n");
	conversion = 0;	//Prompt the player to enter a string of a number, and loop until a valid conversion between 1 and 8 has been made. The conversion is to validate the input, rather than just getting an input integer.
	while(conversion==0){
		fgets(tempName, 6, stdin);
		conversion = atoi(tempName);
		if(conversion >8 || conversion < 1) conversion = 0;
	}
	chesty = (int*)calloc(conversion, sizeof(int));	//Dynamically allocate the arrays of x and y coordinates for the chests.
	chestx = (int*)calloc(conversion, sizeof(int));
	
	for(count = 0; count < conversion; count++)
	{
		//Randomly allocate coordinates for the chests between ROWS and COLS.
		chesty[count] = rand() % ROWS;
		chestx[count] = rand() % COLS;
	}
	
	//Loop through the landscape array, filling it with '.' characters.
	for(y = 0; y < ROWS; y++)
	{
	    for(x = 0; x < COLS; x++)
	    {
		    landscape[y][x] = '.';
	    }
	}
	
	system("cls");	//Clear the window to make way for the main grid.
	
	//Pre-game object generation:
	//Put the chests at the allocated coordinates on the grid, in the form of 'C' characters.
	for(count = 0; count < conversion; count++)
	{
		landscape[chesty[count]][chestx[count]] = 'C';
	}
	
	//Put all essential items on the grid afterwards (including the player), at the risk of overwriting some of the chests.
	landscape[0][5] = 'D';	//Door out
	landscape[0][1] = ']';	//Wall
	landscape[0][0] = '*';	//Key
	landscape[1][0] = 'M';	//Boss
	landscape[Player1.pos[0]][Player1.pos[1]] = '@';
	
	//Declare the parameters for all monsters to appear in the game (see structure declaration above for insight).
	struct enemy monster1;
	strcpy(monster1.name, "Vampire Bat");	//Like a bigger, nastier mosquito.
	monster1.health = 3;
	monster1.attack = 5;
	monster1.rank = 1;
	
	struct enemy monster2;
	strcpy(monster2.name, "Serpent Warrior");	//He's fierce, be careful or he'll throw a hissy-fit.
	monster2.health = 7;
	monster2.attack = 5;
	monster2.rank = 2;
	
	struct enemy monster3;
	strcpy(monster3.name, "Big-boned Skeleton");	//He's not fat, he's just big-boned! In fact, that's all he is.
	monster3.health = 14;
	monster3.attack = 2;
	monster3.rank = 1;
	
	struct enemy boss1;
	strcpy(boss1.name, "Heavy Metal Minotaur");	//His favourite band is Labyrinth.
	boss1.health = 30;
	boss1.attack = 8;
	boss1.rank = 3;
	
	printLS(landscape);
	
	//Starting equipment:
	//Give the player a wooden sword, and set the first inventory slot to filled. The parameters here are the same as those on equipment.txt.
	struct equipment sword;
	
	sword.damageMult = 2;
	sword.protectionDiv = 1;
	strcpy(sword.itemName, "Wooden Sword");
	
	Player1.inventory[0] = sword;
	Player1.inventoryFilled[0] = 1;
	//End of starting equipment.
	
	//End initialization.
	
	while(exit == 0)
	{
		//Get an unbuffered keypress from the player, and run move() with it. Then act appropriately given the response.
		control = getch();
		moveReturn = move(Player1.pos, control, landscape, &Player1);
		
		switch (moveReturn)
		{
		case 0:	//If 0 is returned, the player can move into the new space, but also has a 10% chance of encountering a monster.
			landscape[Player1.pos[0]][Player1.pos[1]] = '@';
			r = rand() % 10;
			if (r==0)
			{	//If the player encounters a monster, randomize between the three lower-level monsters and initiate a battle with the chosen one.
				r = rand() % 3;
				switch(r){
					case 0:
						monster1.health = 3;	//Refill the monster's health.
						battleResult = battle(&monster1, &Player1);
						break;
					case 1:
						monster2.health = 7;	//Refill the monster's health.
						battleResult = battle(&monster2, &Player1);
						break;
					case 2:
						monster3.health = 14;	//Refill the monster's health.
						battleResult = battle(&monster3, &Player1);
						break;
				}
				if(battleResult == 2) exit = 1;	//If the player was defeated, exit the program.
				Player1.health = 15;	//Refills the player's health.
			}
			system("cls");	//Clear the screen and re-print the landscape.
			printLS(landscape);
			break;
		
		case 2:
			printHelp(Player1.name);	//Print the help screen below the current game. This goes away the next time the player does anything.
			break;
		
		case 3:
			inventoryMenu(&Player1);	//Open the inventory. When done, clear the screen and re-print the landscape.
			system("cls");
			printLS(landscape);
			break;
			
		case 4:
			printf("Really quit? (y/n)\n");	//Allow the player to cancel quitting, by looping until a press of 'y' or 'n'. Exit if 'y' is pressed. Do not exit if 'n' is pressed.
			while(1)
			{
				control = getch();
				if(control == 'y'){
					exit = 1;
					break;
				}
				if(control == 'n')
				{
					printf("Did not quit.\n");
					break;
				}
			}
			break;
			
		case 5:
			printf("This is the exit of the dungeon. Are you sure you wish to proceed? This will end the game. (y/n)\n");
			while(1)	//Just like case 4, allows the player to think twice before exiting. Displays a message on exit. This case is only called if the player has the key for the door.
			{
				control = getch();
				if(control == 'y'){
					escaped = 1;
					printf("Exited the dungeon. Thanks for playing!\n");
					exit = 1;
					break;
				}
				if(control == 'n')
				{
					printf("Did not exit.\n");
					break;
				}
			}
			break;
			
		case 6:	//Handle a boss battle in the case of a player encountering an 'M'. Just like case 0, ends the game if the player was defeated (battleResult = 2).
			battleResult = battle(&boss1, &Player1);
			if(battleResult == 2) exit = 1;
			Player1.health = 15;	//Refills the player's health.
			break;
			
		default:
			break;
		}
	}
	printResults(Player1.name, Player1.inventory, Player1.inventoryFilled, escaped);	//Print the player's final results to the file 'results.txt' before quitting.
	printf("Your results sheet has been printed in results.txt, in the program folder!\n");       //Tell the user that their results have been printed.
	system("pause");
}

void printLS(char landscape[ROWS][COLS])
{
	int x, y;	//x and y are looping variables.
	
	printf("Press h for help.\n[ LAME BOY ]\n------------\n");	//Get it? It's like Game Boy, only... bad. Prints at the top of each landscape.
	for(y = 0; y < ROWS; y++)	//Loops through all the elements of the landscape array, and prints each character in a grid, bordered by square brackets.
	{
		printf("[");
	    for(x = 0; x < COLS; x++)
	    {
		    printf("%c", landscape[y][x]);
	    }
	    printf("]\n");
	}
}

int move(int *pos, char keypress, char landscape[][COLS], struct player *Player1)
{
	int found;	//found handles the output from findObject.
	
	//keypress is the character of the key pressed by the player, passed through from main.
	switch (keypress)
	{
	case 'w':
		if((pos[0]-1) > -1 && landscape[pos[0]-1][pos[1]] == '.'){	//If there is a space above the player, move into it and set the character's previous position to empty.
			landscape[pos[0]][pos[1]] = '.';
			pos[0]--;	//Change the player's position accordingly.
			return 0;
		}else if((pos[0]-1) > -1 && landscape[pos[0]-1][pos[1]] != '[' && landscape[pos[0]-1][pos[1]] != ']')
		{	//If the player tries to move into an object that is not a '.' or a square bracket (wall), run findObject() with the found character.
			found = findObject(landscape[pos[0]-1][pos[1]], Player1);
			switch(found)
			{
				case 0:
					landscape[pos[0]-1][pos[1]] = '.';	//Set the found character to '.' and break out.
					break;
				case 1:
					return 5;	//Return 5 if the output is 1, which means the player has found the door and has a key.
					break;
				case 2:
					landscape[pos[0]-1][pos[1]] = '.';	//Set the found character to '.' and return 6. This initiates a boss battle.
					return 6;
					break;
				default:
					break;
			}
			return 0;	//Return '0' to allow the grid to be reprinted.
		}
		break;
	case 's':
		if((pos[0]+1) < (ROWS) && landscape[pos[0]+1][pos[1]] == '.'){	//If there is a space below the player, move into it and set the character's previous position to empty.
			landscape[pos[0]][pos[1]] = '.';
			pos[0]++;	//Change the player's position accordingly.
			return 0;
		}else if((pos[0]+1) <(ROWS) && landscape[pos[0]+1][pos[1]] != '[' && landscape[pos[0]+1][pos[1]] != ']')
		{	//If the player tries to move into an object that is not a '.' or a square bracket (wall), run findObject() with the found character.
			found = findObject(landscape[pos[0]+1][pos[1]], Player1);
			switch(found)
			{
				case 0:
					landscape[pos[0]+1][pos[1]] = '.';	//Set the found character to '.' and break out.
					break;
				case 1:
					return 5;	//Return 5 if the output is 1, which means the player has found the door and has a key.
					break;
				case 2:
					landscape[pos[0]+1][pos[1]] = '.';	//Set the found character to '.' and return 6. This initiates a boss battle.
					return 6;
					break;
				default:
					break;
			}
			return 0;	//Return '0' to allow the grid to be reprinted.
		}
		break;
	case 'a':
		if((pos[1]-1) > -1 && landscape[pos[0]][pos[1]-1] == '.'){	//If there is a space to the left of the player, move into it and set the character's previous position to empty.
			landscape[pos[0]][pos[1]] = '.';
			pos[1]--;
			return 0;
		}else if((pos[1]-1) > -1 && landscape[pos[0]][pos[1]-1] != '[' && landscape[pos[0]][pos[1]-1] != ']')
		{	//If the player tries to move into an object that is not a '.' or a square bracket (wall), run findObject() with the found character.
			found = findObject(landscape[pos[0]][pos[1]-1], Player1);
			switch(found)
			{
				case 0:
					landscape[pos[0]][pos[1]-1] = '.';	//Set the found character to '.' and break out.
					break;
				case 1:
					return 5;	//Return 5 if the output is 1, which means the player has found the door and has a key.
					break;
				case 2:
					landscape[pos[0]][pos[1]-1] = '.';	//Set the found character to '.' and return 6. This initiates a boss battle.
					return 6;
					break;
				default:
					break;
			}
			return 0;	//Return '0' to allow the grid to be reprinted.
		}
		break;
	case 'd':
		if((pos[1]+1) < (COLS) && landscape[pos[0]][pos[1]+1] == '.'){	//If there is a space to the right of the player, move into it and set the character's previous position to empty.
			landscape[pos[0]][pos[1]] = '.';
			pos[1]++;
			return 0;
		}else if((pos[1]+1) < (COLS) && landscape[pos[0]][pos[1]+1] != '[' && landscape[pos[0]][pos[1]+1] != ']')
		{	//If the player tries to move into an object that is not a '.' or a square bracket (wall), run findObject() with the found character.
			found = findObject(landscape[pos[0]][pos[1]+1], Player1);
			switch(found)
			{
				case 0:
					landscape[pos[0]][pos[1]+1] = '.';	//Set the found character to '.' and break out.
					break;
				case 1:
					return 5;	//Return 5 if the output is 1, which means the player has found the door and has a key.
					break;
				case 2:
					landscape[pos[0]][pos[1]+1] = '.';	//Set the found character to '.' and return 6. This initiates a boss battle.
					return 6;
					break;
				default:
					break;
			}
			return 0;	//Return '0' to allow the grid to be reprinted.
		}
		break;
	case 'h':	//Return 2 to display the help screen.
		return 2;
	case 'i':	//Return 3 to display the inventory.
		return 3;
	case 'q':	//Return 4 to quit.
		return 4;
	default:	//1 is the error condition. Should never be returned.
		return 1;
	}
	return 1;
}


int findObject(char object, struct player *Player1)
{
	int r, count, slotNo = -1;	//r is a random variable, count is a loop variable, and slotNo is the current inventory slot.
	char temp[200];	//A temporary variable for handling strings.
	FILE *fp;	//The pointer to the file 'equipment.txt'.
	fp = fopen("equipment.txt", "r");	//Open 'equipment.txt' with fp, in read-only mode.
	
	switch (object)
	{
	case 'C':	//If a 'C' has been found, randomize a number between 1 and 6 (one for each item in equipment.txt).
		r = (rand() % 6)+1;
		for(count = 0; count < 10; count ++)
		{
			//Loop through each element of the inventoryFilled array, and if an empty slot is found, set slotNo to it and break.
			if(Player1->inventoryFilled[count]==0)
			{
				slotNo = count;
				break;
			}
		}
		
		if(slotNo > -1){	//If an empty slot has been found, multiply r by 3 and minus 2. This allows us to loop through each line of 'equipment.txt' until we are at the randomized item's name.
			for(count = 0; count < (r*3)-3; count ++){
				fgets(temp, 200, fp);
			}
			fgets(Player1->inventory[slotNo].itemName, 20, fp);	//Get the item name from the file, remove the newline character and put it in the empty inventory slot.
			Player1->inventory[slotNo].itemName[strcspn(Player1->inventory[slotNo].itemName, "\n")] = 0;
			fgets(temp, 4, fp);
			Player1->inventory[slotNo].damageMult = atoi(temp);	//Convert the damage and protection values from read strings to integers using atoi() on the result of fgets().
			fgets(temp, 4, fp);
			Player1->inventory[slotNo].protectionDiv = atoi(temp);
			printf("You got a %s! Don't forget to equip it!\n", Player1->inventory[slotNo].itemName);	//Tell the user the item that they recieved.
			Player1->inventoryFilled[slotNo] = 1;	//Mark the inventory slot as filled.
			fclose(fp);	//Close 'equipment.txt'.
			Sleep(2000);	//Pause to let user read the information.
			return 0;
		}else{
			printf("Inventory full!\n");	//Print out that the inventory is full as there are no free slots.
			Sleep(1000);	//Pause to let user read the information.
			return -1;	//Return -1 to let move() know not to overwrite the character in landscape.
		}
		break;
	
	case '*':
		for(count = 0; count < 10; count ++)
		{
			if(Player1->inventoryFilled[count] == 0)	//Find the first free slot, as before.
			{
				slotNo = count;
				break;
			}
		}
			
		if(slotNo > -1){	//Give the player a key (a piece of equipment with no extra protection or damage).
			strcpy(Player1->inventory[slotNo].itemName, "Dungeon Key");
			Player1->inventory[slotNo].damageMult = 1;
			Player1->inventory[slotNo].protectionDiv = 1;
			Player1->inventoryFilled[slotNo] = 1;
			printf("You got the dungeon key! Now you can exit the dungeon!\n");	//Tell the user what they have found.
			Sleep(2000);	//Pause to let user read the information.
			return 0;
		}else{
			printf("Inventory full!\n");	//Print out that the inventory is full as there are no free slots.
			Sleep(1000);	//Pause to let user read the information.
			return -1;	//Return -1 to let move() know not to overwrite the character in landscape.
		}	
		return 0;
		break;
	
	case 'D':	//If the player has a key in any of their inventory slots, return 1 to allow the user to exit the dungeon.
		for(count = 0; count < 10; count++)
		{
			if(Player1->inventoryFilled[count] > 0)
			{
				if(strcmp("Dungeon Key", Player1->inventory[count].itemName) == 0)
				{
					return 1;
				}
			}
		}
		printf("The door is locked! Looks like you'll need a key.\n");	//Tell the user that they don't have the key.
		Sleep(2000);	//Pause to let user read the information.
		return -1;	//Return -1 to let move() know not to overwrite the character in landscape.
		break;
	
	case 'M':
		return 2;	//Return 2 to let move() know that the user has found the boss.
		break;
	
	default:
		return 0;	//Return 0 as a default condition. 
		break;
	}
}

void inventoryMenu(struct player *Player1)
{
	int i, swapper, exit = 0, selected = 0;	//i is a looping variable, swapper is a variable that temporaily holds an integer while items are swapped. Exit allows the exiting of loops, and selected is the index of the currently selected item slot.
	char control;	//control is the user's input keystroke.
	
	while(exit == 0)
	{
		system("cls");	//Clear the screen.
		printf("Inventory:\n");	//Print out information for navigation of the inventory.
		printf("\nSelect an inventory slot with a and d, and equip it to your hand with h,\n or wear it with w.\n");
		printf("Press t to toss the selected item. Leave the inventory to go back to the game by pressing i again.\n[");
		for(i = 0; i < 10; i++)	//Print out the numbers 1 to 10, with a chevron ('>') next to the one which is selected+1 (makes more sense to have slots 1-10 than 0-9).
		{
			if((i) == selected)
			{
				printf(">");
			}
			printf("%d ", i+1);
		}
		printf("]\n\n");
		
		if(Player1->inventoryFilled[selected] == 1)	//If the currently selected slot is occupied, print out its contents.
		{
			printf("Item Name: %s\n", Player1->inventory[selected].itemName);
			printf("Protection: %d\n", Player1->inventory[selected].protectionDiv-1);
			printf("Damage Value: %d\n\n", Player1->inventory[selected].damageMult-1);
		}else{
			printf("Slot empty.\n\n\n\n");	//Otherwise, print out that it is empty.
		}
		
		if(Player1->inHand > -1 && Player1->inHand < 10)	//If the player has an item in-hand (inHand is between 0 and 9), display the item's name and slot number.
		{
			printf("In hand: %s (slot %d)\n",  Player1->inventory[Player1->inHand].itemName, Player1->inHand+1);
		}else{
			printf("In hand: Nothing\n");	//Otherwise, print out that it is empty.
		}
		
		if(Player1->wearing > -1 && Player1->wearing < 10)	//If the player is wearing an item (wearing is between 0 and 9), display the item's name and slot number.
		{
			printf("Wearing: %s (slot %d)\n",  Player1->inventory[Player1->wearing].itemName, Player1->wearing+1);
		}else{
			printf("Wearing: T-shirt and jeans.\n");	//Otherwise, print out default normal clothes.
		}
		//Get a keypress from the user.
		control = getch();
		
		switch(control)
		{
			case 'd':
				if(selected < 9)
				{
					selected ++;	//If keypress was d (and selected is not at maximum), add 1 to the selected slot.
				}
				break;
				
			case 'a':
				if(selected > 0)
				{
					selected --;	//If keypress was a (and selected is not at minimum), minus 1 from the selected slot.
				}
				break;
				
			case 'h':
			if(Player1->inventoryFilled[selected] == 1)	//If there is something in the slot:
			{
				if(selected != Player1->wearing){	//If the player is not wearing the selected item:
					Player1->inHand = selected;	//Put the selected item slot number in inHand.
				}else{
					swapper = Player1->inHand;	//Otherwise, swap the contents of inHand and wearing.
					Player1->inHand = Player1->wearing;
					Player1->wearing = swapper;
				}
			}else{
				printf("Cannot equip nothing.");	//Tell the player that they cannot equip from an empty slot.
				Sleep(1000);
			}
			break;
			
			case 'w':
			if(Player1->inventoryFilled[selected] == 1)	//If there is something in the slot:
			{
				if(selected != Player1->inHand){	//If the player is not holding the selected item:
					Player1->wearing = selected;	//Put the selected item slot number in wearing.
				}else{
					swapper = Player1->inHand;	//Otherwise, swap the contents of inHand and wearing.
					Player1->inHand = Player1->wearing;
					Player1->wearing = swapper;
				}
			}else{
				printf("Cannot equip nothing.");	//Tell the player that they cannot equip from an empty slot.
				Sleep(1000);
			}
			break;
			
			case 't':
				if(strcmp("Dungeon Key", Player1->inventory[selected].itemName) != 0)	//If the selected item is not the Dungeon Key:
				{
					printf("Really toss this item? (y/n)\n");	//Allow the user to think twice about tossing the item by entering 'y' or 'n', and loop until they press one or the other.
					while(1)
					{
						control = getch();
						if(control == 'y'){
							Player1->inventoryFilled[selected] = 0;	//Empty the selected slot.
							if(Player1->inHand == selected)
							{
								Player1->inHand = -1;	//If the player was holding the item, set 'inHand' to -1.
							}
							if(Player1->wearing == selected)
							{
								Player1->wearing = -1;	//If the player was wearing the item, set 'wearing' to -1.
							}
							break;
						}
						if(control == 'n')
						{
							break;
						}
					}
				}else{
					printf("I don't think you really want to toss that, do you?\n");	//Tell the user that they probably shouldn't drop the only key they have to get out of the dungeon.
					Sleep(2000);
				}
				break;
				
			case 'i':
				exit = 1;	//Break out of the inventory loop to go back to the main game.
				break;
			
			default:
				break;
		}
	}
}

int battle(struct enemy *monster, struct player *Player1)
{
	int r, actionTaken, recharge, exit = 0;	//r is a randomized variable, actionTaken handles what the player chose to do this turn, recharge counts turns taken to recharge (for boss attacks), and exit allows the program to exit the battle loop.
	char control;	//control is the user's input keystroke.
	system("cls");	//Clear the screen.
	printf("An enemy [%s] appeared! Prepare to battle!\n", monster->name);	//Print out the enemy monster's name.
	recharge = 0;	//Set recharge to 0, meaning a boss monster can attack right away.
	switch(monster->rank)	//Give the player information based on the enemy monster's rank.
	{
		case 1:
			printf("It doesn't look very strong.\n");
			break;
		case 2:
			printf("It looks a little tough. Be careful!\n");
			break;
		case 3:
			printf("It looks very dangerous. Watch out!!\n");
			break;
		default:
			break;
	}
	
	while(exit == 0)	//While the battle continues:
	{
		printf("\n[Your health: %d]\n", Player1->health);	//Display player's health, enemy health, and control options.
		printf("The enemy creature has [%d health]. Choose an action!\n", monster->health);
		printf("Press f to fight, r to run, or g to guard.\n-----------------------------------\n");
		actionTaken = 0;	//Set actionTaken to no action.
		while(actionTaken == 0)	//While the user has not chosen an appropriate action, loop through, and get a key press from the keyboard with getch().
		{
			control = getch();
			
			switch(control)
			{
			case 'f':
				printf("You strike at the monster with all your might!\n");	//Print that the player has attacked, and set actionTaken to 1.
				actionTaken = 1;
				break;
				
			case'r':
				if(monster->rank < 3)	//If the monster is not a boss:
				{
					r = rand() % 2;	//There is a 50% chance the player will escape. If so, exit the battle. If not, actionTaken = 2. Notify the player.
					if(r == 1)
					{
						printf("You managed to escape!\n");
						Sleep(2000);        //Pause to allow the player to read the information.
						exit = 1;
						return 0;
					}else{
						printf("The monster catches up with you! The battle rages on.\n");
					}
				}else{
					printf("You cannot escape from such a powerful monster!\n");	//If the monster is a boss, there is no chance of escape. Notify the player.
				}
				actionTaken = 2;	//Set actionTaken to 2.
				break;
				
			case'g':
				printf("You brace yourself in anticipation the monster's attack.\n");
				actionTaken = 3;	//Print that the player has guarded, and set actionTaken to 3.
				break;
			
			default:
				break;
			}
		}
		
		if(actionTaken == 1)	//If the player has attacked, if they have a weapon equipped, deal the weapon's damage to the monster. If no weapon equipped, just deal 1 damage, for a punch.
		{
			if(Player1->inHand < 0 || Player1->inHand > 10)
			{
				monster->health -= 1;
				printf("The monster takes [1 damage!]\n");	//Print the amount of damage the monster took.
			}else{
				monster->health -= Player1->inventory[Player1->inHand].damageMult;
				printf("The monster takes [%d damage!]\n", Player1->inventory[Player1->inHand].damageMult);
			}
		}
		
		if(monster->health <= 0)	//If the monster's health is 0 or below after the attack, print that the player has won, add randomized loot to inventory (by the same way as finding a chest in findObject()), and after the next keypress, return 1 to exit the battle.
		{
			printf("You defeated the monster!\n");
			findObject('C', Player1);	//Get loot!
			exit = 1;
			system("pause");          //Pause to allow the player to read the information.
			return 1;
		}
		
		if(recharge == 0){	//If the monster does not need to recharge:
			printf("The monster strikes!\n");
			if(actionTaken == 3)
			{	//If the player has guarded, calculate if what damage they would take is less than 1. Subtract the lowest of the two from their health, and print the result to the screen.
				if(Player1->wearing > -1 && (monster->attack)/(Player1->inventory[Player1->wearing].protectionDiv) < 1)
				{
					printf("Thanks to your armour, you took [no damage!]\n");
				}else{
					Player1->health -= 1;
					printf("You were ready for the monster's attack, so you only take [1 damage.]\n");
				}
			}else{	//If the player did not guard, if they are not wearing armour, subtract the monster's attack from their health, and print the result.
				if(Player1->wearing < 0 || Player1->wearing > 10){
					Player1->health -= monster->attack;
					printf("You take [%d damage!]\n", monster->attack);
				}else{	//If the player was wearing armour, divide the monster's attack by that armour's protection before subtracting it from the player's health, and print the result to the screen.
					Player1->health -= (monster->attack)/(Player1->inventory[Player1->wearing].protectionDiv);
					printf("You take [%d damage!]\n", (monster->attack)/Player1->inventory[Player1->wearing].protectionDiv);
				}
			}
			if(monster->rank==3){
				recharge = 1;	//If the monster is a boss, set recharge to 1 so that next turn, they won't get an attack.
			}
		}else{
			printf("The monster gathers its strength for its next attack.\n");	//If recharge is not 0, print that the monster is recharging, and set recharge to 0 for the next turn.
			recharge = 0;
		}
		
		if(Player1->health <= 0)	//If the player's health hits 0 or below after the monster's attack, print that the player was defeated, the monster's name, and return 2 to end the game.
		{
			printf("You have been defeated by the [%s!] Game over.\n", monster->name);
			Sleep(3000);	//Pause to allow the user to read the information.
			return 2;
			exit = 1;
		}
	}
}

void printResults(char *playerName, struct equipment *inventory, int *inventoryFilled, int escaped)
{
	int count;	//Count is a loop variable.
	FILE *fp;	//Declare the pointer for 'results.txt'.
	fp = fopen("results.txt", "w");	//Open 'results.txt' in write-only mode.
	fprintf(fp, "Name: ");	//Print the player's name to the file.
	fprintf(fp, playerName);
	if(escaped == 1)	//Print whether or not the player escaped to the file.
	{
		fprintf(fp, "\nEscaped!\n");
	}else{
		fprintf(fp, "\nDid not escape. :(\n");
	}
	fprintf(fp, "Inventory:\n");	//Print the player's inventory to the file.
	for(count = 0; count < 10; count ++)
	{
		if(inventoryFilled[count] == 1)
		{
			fprintf(fp, inventory[count].itemName);
			fprintf(fp, "\n");
		}
	}
	fclose(fp);	//Close 'results.txt'.
}

void printHelp(char *name)
{	//Print out the controls for the game, and the player's chosen name.
	printf("Help:\n");
	printf("Hello! Your chosen character name is: %s.\n", name);
	printf("Use the w, a, s and d keys to move about, and i to access your inventory.\n");
	printf("Your aim is to survive and find a way out of the dungeon.\n");
	printf("The '@' symbol marks your current position.\n");
	printf("To get rid of this help info, just do any of the above.\n");
	printf("Press q to quit.\n");
}
