// MARIOS SPILIOPOULOS
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int X = 25; // Grid size
int Y = 25;
int speed = 100; // speed
int snake_cells = 1; // how big the snake is at the start of the game
int direction = RIGHT; // initial direction of the snake
int frames = 1; // the frames of the game
int score = 0; // score
int collision = 0; // collided with itself

// every snakes cell
typedef struct {
    int x;
    int y;
} Snake_Cell;

// apple
typedef struct {
    int apple_x;
    int apple_y;
} Apple;

// like a tuple from python that stores 2 points
typedef struct {
    int x;
    int y;
} Point;

void update_monitor(int arr[X][Y]);
void set_grid(int arr[X][Y]);
void snake_body_to_main_grid(Snake_Cell *snake_body, Apple *apple, int grid[X][Y], Point *pos);
Apple generate_apple(Snake_Cell *sb, int grid[X][Y]);
void get_direction();
void clear_buffer() {
    while (getchar() != '\n');
}
void hide_cursor() { // DISABLING THE CURSOR FOR THE INPUT BECAUSE ITS "BLINKING" AND IT MIGHT BE DISTRURBING.
                     // (COMPILE THE CODE WITHOUT THIS AND SEE IT FOR YOURSELF WHAT I MEAN)
                     // disabling the cursor for the input because its "blinking" and it might be disturbing.
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    // Get the current cursor info
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);

    // Set the cursor visibility to false
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}
void show_cursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    // Get the current cursor info
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);

    // Set the cursor visibility to true
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}
void set_console_font_size(int size_x, int size_y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX fontInfo;

    fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX); // Set structure size
    GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo); // Retrieve current font info

    fontInfo.dwFontSize.X = size_x; // Set the width of the font
    fontInfo.dwFontSize.Y = size_y; // Set the height of the font

    SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo); // Apply the font settings
}
void move_console_to_top_left() { // opening the console at the top left corner of the players screen
    HWND consoleWindow = GetConsoleWindow(); // Get the console window handle

    if (consoleWindow != NULL) {
        SetWindowPos(consoleWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER); 
        // Move to (0,0) without changing size or z-order
    }
}

void update_monitor(int arr[X][Y]) { // updating the monitor with every frame

    // rather than clreaing the whole console, im moving the cursors position to the start of the console
    // and then im printing. This stops the "blinking" that i get when i clear the console and print it all over again from 0
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coords = {0, 0};
    SetConsoleCursorPosition(hConsole, coords);

    printf("Score: %d\n", score);
    frames++; // INCREASING THE FRAMES BY 1
    printf("---------------------------------------------------\n");
    for (int i = 0; i < X; i++) {
        printf("|");
        for (int j = 0; j < Y; j++) {
            switch (arr[i][j]) {
                case 0: { // 0 means an empty cell
                    printf("  "); 
                    break;
                }
                case -1: { // -1, -2, -3, -4 means the direction of the head
                    printf("△ ");
                    break;
                }
                case -2: {
                    printf("▷ ");
                    break;
                }
                case -3: {
                    printf("▽ ");
                    break;
                }
                case -4: {
                    printf("◁ ");
                    break;
                }
                case 1: { // 1 means the snakes body
                    printf("🞕 ");
                    break;
                    //🞕
                }
                case 2: { // 2 means the apple
                    printf("🞜 ");
                    break;
                    //🞜
                }
            }
        }
        printf("\b|\n");
    }
    printf("---------------------------------------------------\n");
    return;
}

void set_grid(int grid[X][Y]) { // set all the values of the grid to 0
    for (int i = 0; i < X; i++) {
        for (int j = 0; j < Y; j++) {
            grid[i][j] = 0;
        }
    }
}

void snake_body_to_main_grid(Snake_Cell *snake_body, Apple *apple, int grid[X][Y], Point *pos) {
    set_grid(grid); // initializing all the grid values to 0

    switch (direction) {
        case UP: {
            grid[snake_body[0].x][snake_body[0].y] = -1; // the head is always at the first position of the snakes body array
            break;                                       // and with what the direction is, i set the corresponding number to know what head to print in the console
        }
        case RIGHT: {
            grid[snake_body[0].x][snake_body[0].y] = -2;
            break;
        }
        case DOWN: {
            grid[snake_body[0].x][snake_body[0].y] = -3;
            break;
        }
        case LEFT: {
            grid[snake_body[0].x][snake_body[0].y] = -4;
            break;
        }
    }
    
    // I CANT REALLY EXPLAIN THE LOGIC BEHIND HOW THE SNAKE MOVES. BASICANNLLY EVERY POSITION THAT THE HEAD HAS TRAVELED IS GETTING STORED
    // ON AN ARRAY POSITIONS. AND THEN EVERY CELL FOLLOWS THE LAST POSITIONS OF THE HEAD ACCORDING TO ITS POSITION IN THE SNAKES BODY
    // THATS THE BEST EXPLANATION I CAN GIVE :D
    for (int i = 1; i < snake_cells; i++) {
        snake_body[i].x = pos[frames - i - 1].x;
        snake_body[i].y = pos[frames - i - 1].y;
        grid[snake_body[i].x][snake_body[i].y] = 1;
    }
    
    grid[apple->apple_x][apple->apple_y] = 2; // the apples position is marked with 2
}

Apple generate_apple(Snake_Cell *sb, int grid[X][Y]) { // generate apples position
    Apple apple;

    int valid_pos = 0;

    while (!valid_pos) {
        valid_pos = 1;
        apple.apple_x = rand() % X;
        apple.apple_y = rand() % Y;

        for (int i = 0; i < snake_cells; i++) { // checking if the apple has spawned in a cell from the snakes body
            if (apple.apple_x == sb[i].x && apple.apple_y == sb[i].y) {
                valid_pos = 0;
                break;
            }
        }
    }

    return apple;
}

void get_direction() { // INPUT FROM THE USER (UP DOWN LEFT RIGHT)
    if (_kbhit()) { // Check if a key is pressed
        int key = _getch(); // Get the first character
        if (key == 224) { // Arrow keys send a two-character sequence
            key = _getch(); // Get the second character

            if (snake_cells == 1) { // if snakes_cells are one, then its only the head so i can move freely
                switch (key) {
                    case 72: direction = UP; break; // Up arrow
                    case 80: direction = DOWN; break; // Down arrow
                    case 75: direction = LEFT; break; // Left arrow
                    case 77: direction = RIGHT; break; // Right arrow
                }
            }
            else { // else im checking the snake cant go back into itself
                if (direction == 0 && key == 80 || direction == 1 && key == 72 || direction == 2 && key == 77 || direction == 3 && key == 75) {
                    return;
                }
                else {
                    switch (key) {
                        case 72: direction = UP; break;
                        case 80: direction = DOWN; break;
                        case 75: direction = LEFT; break;
                        case 77: direction = RIGHT; break;
                    }
                }
            }
        }
        else if (key == 'S' || key == 's') { // S to stop the game
            collision = 1;
        }
    }
}

void detect_collision(Snake_Cell *sb) { // checking for collision

    Snake_Cell head = sb[0]; // taking the head of the snake

    // and then i check if the head has the same position with one of the cells from the snakes body
    for (int i = 1; i < snake_cells; i++) {
        if (head.x == sb[i].x && head.y == sb[i].y) {
            collision = 1; // if it has the same then we have collision
            return;
        }
    }

    return;
}

int main() {
    move_console_to_top_left();
    SetConsoleOutputCP(CP_UTF8); // initializing the encoding to UTF so i cant print the special characters for the snakes body and apple
    srand(time(NULL) + rand()); // seeding the randomness
    SetConsoleTitle("SNAKE"); // title of the console for the game
    
    // taking the highscore from the file high_score.txt
    FILE *high_score_f = fopen("high_score.txt", "r");
    if (high_score_f == NULL) {
        printf("Fatal error, cant open file for highscore. Game is closing\n");
        system("pause");
        return 0;
    }
    char high_score[4];
    fgets(high_score, 4, high_score_f);
    fclose(high_score_f);

    system("cls");
    
    set_console_font_size(16, 32); // making the console font bigger for better visibility
    
    //  question the player for "hard mode" (dynamic speed)
    printf("Hard Mode!\n");
    printf("Enter 1 for dynamic speed. The bigger the snake the faster it gets. Enter 0 for static speed. (e) to close the game\n");
    printf("Choice: ");
    char choice_speed = '0';
    scanf(" %1c", &choice_speed);
    clear_buffer();
    while (choice_speed != '1' && choice_speed != '0' && choice_speed != 'e' && choice_speed != 'E') {
        system("cls");
        printf("Wrong input. 1 or 0 or (e) to close the game: ");
        scanf(" %1c", &choice_speed);
        clear_buffer();
    }
    if (choice_speed == 'e' || choice_speed == 'E') {
        return 0;
    }
    system("cls");
    printf("IF YOU WANNA STOP THE GAME JUST PRESS S. DONT END THE GAME WITH CTRL+C\nHAVE FUN! :D\n");
    printf("Your current highscore: %s\n", high_score);
    system("pause");

    system("cls");
    hide_cursor();

    int main_grid[X][Y]; // making the main grid of the game
    set_grid(main_grid);

    // CREATING THE SNAKES BODY
    // THE THEORITICAL BIGGEST SNAKE YOU CAN GET IS X * Y
    // SO THE SNAKE CAN HAVE UP TO X * Y
    // COULD USE malloc AND realloc TO MAKE THE SNAKE DYNAMIC, BUT IT MAKES THE GAME SLOWER.
    Snake_Cell snake_body[X * Y];
    snake_body[0].x = rand() % X; // starting the game in a random position in the game (0 up to X)
    snake_body[0].y = rand() % Y; // same with the Y coordinate

    Apple apple = generate_apple(snake_body, main_grid); // generating the apple for the first time

    Point positions[100000]; // creating an array with all the positions that the head has passed
                             // for the other snakes cells
    
    while (1) { // MAIN LOOP OF THE GAME

        switch (choice_speed) {
            case '1': Sleep(speed / snake_cells * 2); break; // HARD MODE - DYNAMIC SPEED
            case '0': Sleep(speed); break; // STATIC DIFFICULTY - SPEED
        }

        get_direction(); // taking the direction that the players wants for the snake

        switch (direction) { // according to the direction im moving the heads position (up down left right)
            case UP: snake_body[0].x--; break;
            case DOWN: snake_body[0].x++; break;
            case LEFT: snake_body[0].y--; break;
            case RIGHT: snake_body[0].y++; break;
        }

        // if at the limits of the grid, go to the opposite side
        if (snake_body[0].x < 0) snake_body[0].x = X - 1;
        if (snake_body[0].x >= X) snake_body[0].x = 0;
        if (snake_body[0].y < 0) snake_body[0].y = Y - 1;
        if (snake_body[0].y >= Y) snake_body[0].y = 0;
        
        // checking if the apple is eaten
        if (snake_body[0].x == apple.apple_x && snake_body[0].y == apple.apple_y) {
            snake_cells++; // adding 1 more cell to the snake
            apple = generate_apple(snake_body, main_grid); // generating new random position for the apple
            score++; // increasing the score
        }

        // for every frame i add the position of the head from the grid into the positions array for the other snakes cells
        positions[frames - 1].x = snake_body[0].x;
        positions[frames - 1].y = snake_body[0].y;

        // passing the snakes body into the grid (0, 1 or 2 for apple)
        snake_body_to_main_grid(snake_body, &apple, main_grid, positions);
        update_monitor(main_grid); // updating the players monitor

        detect_collision(snake_body); // checking for collision
        if (collision) {
            printf("Game Over!\nYour score: %d\n", score); // i print the score that the player got

            if (score > atoi(high_score)) { // and i check for new highscore
                printf("NEW HIGH SCORE!\n");
                FILE *high_score_f = fopen("high_score.txt", "w");
                fprintf(high_score_f, "%d", score);
                fclose(high_score_f);
            }
            system("pause");
            show_cursor();
            set_console_font_size(8, 16); // seting the font of the console back to the default settings
            
            return 0;
        }
    }

    return 0;
}
