#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#define RED   "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE  "\033[34m"
#define MAGENTA "\033[35m"
#define RESET "\033[0m"

const char *COLORS[] = {RED, GREEN, YELLOW, BLUE, MAGENTA};

#define SYM1 "◆"
#define SYM2 "●"
#define SYM3 "▲"
#define SYM4 "■"
#define SYM5 "★"

const char *SYMBOLS[] = {SYM1, SYM2, SYM3, SYM4, SYM5};

const int MAX_SYMBOLS = 5;

typedef struct {
    int symbol_index;
    int color_index;
} Figure;

typedef struct {
    Figure figure1;
    Figure figure2;
} GameRound;

int current_select_index;
int points;
GameRound current_game_round;

void clear_screen() {
    printf("\033[2J\033[H");
}

void increment_selected_index() {
    if (current_select_index >= MAX_SYMBOLS - 1) {
        current_select_index = 0;
    }
    else {
        current_select_index++;
    }
}

void decrement_selected_index() {
    if (current_select_index == 0) {
        current_select_index = MAX_SYMBOLS - 1;
    }
    else {
        current_select_index--;
    }
}

void print_symbol(int symbolIndex, int colorIndex, bool selected) {
    if (selected) {
        printf("[ %s%s%s ]", COLORS[colorIndex], SYMBOLS[symbolIndex], RESET);
    }
    else {
        printf("  %s%s%s  ", COLORS[colorIndex], SYMBOLS[symbolIndex], RESET);
    }
}

void print_figure(Figure figure) {
    printf("  %s%s%s  ", COLORS[figure.color_index], SYMBOLS[figure.symbol_index], RESET);
}

void print_current_game_round() {
    printf("  %s%s%s  ", COLORS[current_game_round.figure1.color_index], SYMBOLS[current_game_round.figure1.symbol_index], RESET);
    printf("  %s%s%s  ", COLORS[current_game_round.figure2.color_index], SYMBOLS[current_game_round.figure2.symbol_index], RESET);
}

void print_game(int time_remaining) {
    printf("Move left with 'a' and right with 'd'\n");
    printf("Press <Enter> to confirm selection\n");
    printf("Wrong answer => 1s time penalty\n");
    
    printf("\n");
    printf("Remaning time: %d", time_remaining);
    printf("\n");
    printf("Current points: %d", points);
    printf("\n\n");
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (i == current_select_index) {
            print_symbol(i, i, true);
        }
        else {
            print_symbol(i, i, false);
        }
    }

    printf("\n\n");
    printf("\n");
    printf("------>");
    print_current_game_round();
    printf("<------");
    printf("\n");
}

void wait_for_input() {
    fcntl(STDIN_FILENO, F_SETFL, 0);
}

void dont_wait_for_input() {
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void set_raw_terminal_mode() {
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    printf("\033[?25l"); //Hide cursor
}

void reset_terminal() {
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON | ECHO; // restore canonical + echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    printf("\033[?25h");

    wait_for_input();
}

bool is_correct() {
    // Figure is showing correct symbol and shape
    bool first_figure_is_correct = current_game_round.figure1.symbol_index == current_game_round.figure1.color_index;
    if (first_figure_is_correct) {
        return current_select_index == current_game_round.figure1.symbol_index;
    }

    bool second_figure_is_correct = current_game_round.figure2.symbol_index == current_game_round.figure2.color_index;
    if (first_figure_is_correct || second_figure_is_correct) {
        return current_select_index == current_game_round.figure2.symbol_index;
    }

    // Figure is not showing correct color for symbol.
    // We must check if the selected symbol is not represented by the game round figures
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (current_select_index != current_game_round.figure1.symbol_index &&
            current_select_index != current_game_round.figure1.color_index &&
            current_select_index != current_game_round.figure2.symbol_index &&
            current_select_index != current_game_round.figure2.color_index) {
            return true;
        }
    }
    
    return false;
}

int get_random_int_with_one_exlusion(int integer_to_exclude) {
    int r;
    do {
        r = rand() % MAX_SYMBOLS;
    } while (r == integer_to_exclude);

    return r;
}

int get_random_int_with_two_exlusions(int first_integer_to_exclude, int second_integer_to_exclude) {
    int index;
    do {
        index = rand() % MAX_SYMBOLS;
    } while (index == first_integer_to_exclude || index == second_integer_to_exclude);

    return index;
}

Figure get_incorrect_figure_with_exlusion(Figure figure_to_exlude) {
    Figure figure;
    
    figure.symbol_index = get_random_int_with_one_exlusion(figure_to_exlude.symbol_index);
    figure.color_index = get_random_int_with_two_exlusions(figure_to_exlude.color_index, figure.symbol_index);

    return figure;
}

GameRound create_round() {
    GameRound game_round;

    bool create_round_with_one_correct_figure = rand() & 2;
    if (create_round_with_one_correct_figure) {
        int index_for_correct_figure = rand() % MAX_SYMBOLS;
        Figure correct_figure = { index_for_correct_figure, index_for_correct_figure};

        Figure incorrect_figure = get_incorrect_figure_with_exlusion(correct_figure);

        bool first_figure_should_be_correct = rand() & 2;
        if (first_figure_should_be_correct) {
            game_round.figure1 = correct_figure;
            game_round.figure2 = incorrect_figure;
        }
        else {
            game_round.figure2 = correct_figure;
            game_round.figure1 = incorrect_figure;
        }
    }
    else {
        int first_symbol_index = rand() % MAX_SYMBOLS;
        int first_color_index = get_random_int_with_one_exlusion(first_symbol_index);

        Figure first_figure = {first_symbol_index, first_color_index}; 
        game_round.figure1 = first_figure;
        game_round.figure2 = get_incorrect_figure_with_exlusion(game_round.figure1);
    }

    return game_round;
}

void print_help_text() {
    printf("The game consists of 5 figures, each with a unique shape and color:\n");

    for (int i = 0; i < MAX_SYMBOLS; i++) {
        print_symbol(i, i, false);
    }

    printf("\n\nEach round, you will be presented with 2 figures based on the shapes and colors above:\n");

    print_symbol(0, 2, false);
    print_symbol(3, 3, false);

    printf("\nIn this example, the first figure is yellow, but it should have been red.\n");
    printf("The second figure is a blue square, which matches the original in the list above,\nso this figure should be selected.\n");

    printf("\nNote: Both figures will never have their correct color in the same round.\n");

    printf("\nThere is also a case where neither figure has the correct color:\n");

    print_symbol(0, 1, false);
    print_symbol(2, 3, false);

    printf("\nIn that case, you must select the figure that is not represented\nin shape or color by the two shown figures.\n");

    printf("\nThat would be: ");
    print_symbol(4, 4, false);

    printf("\n\nThe game lasts 30 seconds. Try to get as many points as possible before time runs out!\n");

}

void flush_user_inputs() {
    char flush;
    while (read(STDIN_FILENO, &flush, 1) > 0);
}

void init_game_round() {
    current_select_index = 0;
    points = 0;
    current_game_round = create_round();
}

void game_round() {
    init_game_round();

    time_t start_time = time(NULL);
    const int game_duration = 30;
    int time_penalty = 0;

    while (true) {
        time_t current_time = time(NULL);
        int elapsed = (int)(current_time - start_time + time_penalty);

        if (elapsed >= game_duration) {
            break;
        }

        char c;

        clear_screen();
        print_game(game_duration - elapsed); 

        ssize_t bytes = read(STDIN_FILENO, &c, 1);
        if (bytes > 0) {
            if (c == 'q') {
                break;
            }
            else if (c == 'a') {
                decrement_selected_index();
            }
            else if (c == 'd') {
                increment_selected_index();
            }
            else if (c == '\n') {
                if (is_correct()) {
                    points++;
                }
                else {
                    time_penalty++;
                }

                current_game_round = create_round();
            }
        }

        usleep(50000);
    }

    printf("\n\n-------------------------------------\n\n");
    printf("You got %d points", points);
    printf("\n\n-------------------------------------\n\n");
}

void run_game() {
    while (true) {
        clear_screen();

        printf("Press 's' to start game\n");
        printf("Press 'h' for help\n");
        printf("Press 'q' to quit\n");

        wait_for_input();

        char c;
        ssize_t bytes = read(STDIN_FILENO, &c, 1);
        if (bytes > 0) {
            if (c == 'q') {
                break;
            }
            else if (c == 's') {
                dont_wait_for_input();

                game_round();

                // Sleep a bit so the player doesn't click further immediately
                usleep(2000000);

                printf(GREEN "Press any key to continue! \n" RESET);

                flush_user_inputs();

                wait_for_input();

                ssize_t bytes = read(STDIN_FILENO, &c, 1);
            }
            else if (c == 'h') {
                clear_screen();

                print_help_text();

                printf(YELLOW "\n\nPress any character to return \n" RESET);

                wait_for_input();

                ssize_t bytes = read(STDIN_FILENO, &c, 1);
            }
        }
    }
}

int main(void)
{
    set_raw_terminal_mode();

    srand(time(NULL));

    run_game();

    reset_terminal();

    return EXIT_SUCCESS;
}
